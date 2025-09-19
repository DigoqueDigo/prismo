#include <io/backend/engine.h>

namespace BackendEngine {

    IOUringEngine::IOUringEngine(const BackendEngineConfig::IOUringConfig& _config, std::shared_ptr<spdlog::logger>& _logger)
        : iovecs(_config.batch_size), buffer_pool(_config.batch_size * _config.block_size), submitted_ops(0), config(_config), logger(_logger) {
            if (io_uring_queue_init(config.queue_depth, &ring, config.ring_flags)) {
                throw std::runtime_error("io_uring initialization failed");
            }

            for (size_t i = 0; i < config.batch_size; i++) {
                iovecs[i].iov_base = buffer_pool.data() + i * config.block_size;
                iovecs[i].iov_len = config.block_size;
            }

            if (io_uring_register_buffers(&ring, iovecs.data(), iovecs.size())) {
                throw std::runtime_error("io_uring register buffers failed");
            }
        }

    int IOUringEngine::_open(const char* filename, int flags, mode_t mode) {
        int fd = open(filename, flags, mode);
        if (fd < 0) {
            this->logger->error("Failed to open file {}: {}", filename, strerror(errno));
            throw std::runtime_error("Failed to open file: " + std::string(strerror(errno)));
        } 
        if (io_uring_register_files(&this->ring, &fd, 1)){
            this->logger->error("io_uring register files failed: {}", strerror(errno));
            throw std::runtime_error("io_uring register files failed: " + std::string(strerror(errno)));
        } else {
            this->logger->info("Opened file {} with fd {}", filename, fd);
            this->logger->info("Registered file descriptor {} with io_uring", fd);
            return fd;
        }
    }

    void IOUringEngine::_read(int fd, void* buffer, size_t size, off_t offset) {
        io_uring_sqe* sqe;

        if (!(sqe = io_uring_get_sqe(&this->ring))) {
            this->logger->error("Failed to get SQE during read: {}", strerror(errno));
            return;
        }

        io_uring_prep_read_fixed(sqe, fd, this->iovecs[this->submitted_ops].iov_base, size, offset, this->submitted_ops);
        io_uring_sqe_set_data(sqe, iovecs[this->submitted_ops].iov_base);
        this->submitted_ops += 1;

        this->logger->info("Prepared read at offset {}: requested {}", offset, size);

        if (this->submitted_ops == this->config.batch_size) {
            io_uring_submit(&ring);

            while (this->submitted_ops > 0) {
                std::vector<io_uring_cqe*> cqe_batch(this->config.batch_size);
                int count = io_uring_peek_batch_cqe(&ring, cqe_batch.data(), cqe_batch.size());

                if (count == 0){
                    sleep(1);
                    continue;
                }

                for (int i = 0; i < count; i++) {
                    io_uring_cqe* cqe = cqe_batch[i];
                    //char* buf = static_cast<char*>(io_uring_cqe_get_data(cqe));

                    if (cqe->res < 0) {
                        this->logger->error("read failed: " + std::string(strerror(-cqe->res)));
                    } else {
                        this->logger->info("Read at offset {}: requested {}, got {}", offset, size, cqe->res);
                    }

                    io_uring_cqe_seen(&ring, cqe);
                    this->submitted_ops--;
                }
            }
        }
    }

    void IOUringEngine::_write(int fd, const void* data, size_t size, off_t offset) {
        io_uring_sqe* sqe;

        if (!(sqe = io_uring_get_sqe(&this->ring))) {
            this->logger->error("Failed to get SQE during write: {}", strerror(errno));
            return;
        }

        std::memcpy(this->iovecs[this->submitted_ops].iov_base, data, size);
        io_uring_prep_write_fixed(sqe, fd, this->iovecs[this->submitted_ops].iov_base, size, offset, this->submitted_ops);
        io_uring_sqe_set_data(sqe, this->iovecs[this->submitted_ops].iov_base);
        this->submitted_ops += 1;

        this->logger->info("Prepared write at offset {}: requested {}", offset, size);

        if (this->submitted_ops == this->config.batch_size) {
            io_uring_submit(&ring);

            while (this->submitted_ops > 0) {
                std::vector<io_uring_cqe*> cqe_batch(this->config.batch_size);
                int count = io_uring_peek_batch_cqe(&ring, cqe_batch.data(), cqe_batch.size());

                if (count == 0) {
                    sleep(1);
                    continue;
                }

                for (int i = 0; i < count; i++) {
                    io_uring_cqe* cqe = cqe_batch[i];
                    //char* buf = static_cast<char*>(io_uring_cqe_get_data(cqe));

                    if (cqe->res < 0) {
                        this->logger->error("write failed: {}", strerror(-cqe->res));
                    } else {
                        this->logger->info("Write at offset {}: requested {}, wrote {}", offset, size, cqe->res);
                    }

                    io_uring_cqe_seen(&ring, cqe);
                    this->submitted_ops--;
                }
            }
        }
    }

    void IOUringEngine::_close(int fd) {
        if (io_uring_unregister_files(&ring)) {
            logger->warn("Failed to unregister files: {}", strerror(errno));
            throw std::runtime_error("Failed to unregister files: " + std::string(strerror(errno)));
        }

        if (io_uring_unregister_buffers(&ring)) {
            logger->warn("Failed to unregister buffers: {}", strerror(errno));
            throw std::runtime_error("Failed to unregister buffers: " + std::string(strerror(errno)));
        }

        io_uring_queue_exit(&ring);

        if (close(fd)) {
            logger->warn("Failed to close fd {}: {}", fd, strerror(errno));
        }

        logger->info("IOUringEngine cleaned up successfully");
    }
}
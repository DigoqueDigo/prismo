#ifndef URING_ENGINE_H
#define URING_ENGINE_H

#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <stdexcept>
#include <io/metric.h>
#include <io/logger.h>
#include <io/flag.h>

namespace Engine {
    template <typename LoggerT, typename MetricT>
    struct UringEngine {
        private:        
            LoggerT logger;
            UringConfig config;

            uint32_t submitted;
            struct io_uring ring;
            std::vector<struct iovec> iovecs;

            inline int fsync(int fd);
            inline int fdatasync(int fd);
            inline ssize_t read(int fd, void* buffer, size_t size, off_t offset);
            inline ssize_t write(int fd, const void* buffer, size_t size, off_t offset);

        public:
            explicit UringEngine(const UringConfig& _config, const LoggerT& _logger);

            int open(const char* filename, OpenFlags flags, mode_t mode);       
            void close(int fd);

            template<Operation::OperationType OperationT>
            void submit(int fd, void* buffer, size_t size, off_t offset);
    };

    template<typename LoggerT, typename MetricT>
    UringEngine<LoggerT, MetricT>::UringEngine(const UringConfig& _config, const LoggerT& _logger) 
        : config(_config), logger(_logger), submitted(0), iovecs(_config.batch) {

            if (io_uring_queue_init_params(config.entries, &ring, config.params)) {
                throw std::runtime_error("Uring initialization failed: " + std::string(strerror(errno)));
            }

            for (auto& buffer : iovecs) {
                buffer.iov_base = std::malloc(config.block_size);
                buffer.iov_len = config.block_size;
                if (buffer.iov_base == nullptr) {
                    throw std::bad_alloc();
                }
            }
            
            if (io_uring_register_buffers(&ring, iovecs, config.batch)) {
                throw std::runtime_error("Uring register buffers failed: " + std::string(strerror(errno)));
            }
        }
    
    template<typename LoggerT, typename MetricT>
    int UringEngine<LoggerT, MetricT>::open(const char* filename, OpenFlags flags, mode_t mode) {
        int fd = ::open(filename, flags.value, mode);
        if (fd < 0) {
            throw std::runtime_error("Failed to open file: " + std::string(strerror(errno)));
        }
        if (io_uring_register_files(&this->ring, &fd, 1)){
            throw std::runtime_error("Register file failed: " + std::string(strerror(errno)));
        }
        return fd;
    }

    template<typename LoggerT, typename MetricT>
    inline ssize_t UringEngine<LoggerT, MetricT>::read(int fd, void* buffer, size_t size, off_t offset) {
        io_uring_prep_read_fixed(sqe, fd, this->iovecs[this->submitted].iov_base, size, offset, this->submitted);
        io_uring_sqe_set_data(sqe, iovecs[this->submitted_ops].iov_base);
        this->submitted_ops++;
    }

    template<typename LoggerT, typename MetricT>
    void UringEngine<LoggerT, MetricT>::close(int fd) {
        if (io_uring_unregister_files(&ring)) {
            throw std::runtime_error("Failed to unregister files: " + std::string(strerror(errno)));
        }
        if (io_uring_unregister_buffers(&ring)) {
            throw std::runtime_error("Failed to unregister buffers: " + std::string(strerror(errno)));
        }
        for (auto& buffer : iovecs) {
            std::free(buffer.iov_base);
        }
        io_uring_queue_exit(&ring);
        if (::close(fd) < 0) {
            throw std::runtime_error("Failed to close fd: " + std::string(strerror(errno)));
        }
    }



    // template<bool enable_logging>
    // void IOUringEngine<enable_logging>::_read(int fd, void* buffer, size_t size, off_t offset) {
    //     io_uring_sqe* sqe = io_uring_get_sqe(&this->ring);

    //     if (sqe == NULL) {
    //         throw std::runtime_error("Failed to get SQE during read: " + std::string(strerror(errno)));
    //     }

    //     io_uring_prep_read_fixed(sqe, fd, this->iovecs[this->submitted_ops].iov_base, size, offset, this->submitted_ops);
    //     io_uring_sqe_set_data(sqe, iovecs[this->submitted_ops].iov_base);
    //     this->submitted_ops += 1;

    //     if constexpr (enable_logging) {
    //         this->logger->info("Prepared read at offset {}: requested {}", offset, size);
    //     }

    //     if (this->submitted_ops == this->config.batch_size) {
    //         io_uring_submit(&ring);

    //         while (this->submitted_ops > 0) {
    //             std::vector<io_uring_cqe*> cqe_batch(this->config.batch_size);
    //             int count = io_uring_peek_batch_cqe(&ring, cqe_batch.data(), cqe_batch.size());

    //             if (count == 0){
    //                 sleep(1);
    //                 continue;
    //             }

    //             for (int i = 0; i < count; i++) {
    //                 io_uring_cqe* cqe = cqe_batch[i];

    //                 if constexpr (enable_logging) {
    //                     if (cqe->res < 0) {
    //                         this->logger->error("read failed: " + std::string(strerror(-cqe->res)));
    //                     } else {
    //                         this->logger->info("Read at offset {}: requested {}, got {}", offset, size, cqe->res);
    //                     }
    //                 }

    //                 io_uring_cqe_seen(&ring, cqe);
    //                 this->submitted_ops--;
    //             }
    //         }
    //     }
    // }

    // template<bool enable_logging>
    // void IOUringEngine<enable_logging>::_write(int fd, const void* data, size_t size, off_t offset) {
    //     io_uring_sqe* sqe = io_uring_get_sqe(&this->ring);

    //     if (sqe == NULL) {
    //         throw std::runtime_error("Failed to get SQE during write: " + std::string(strerror(errno)));
    //     }

    //     std::memcpy(this->iovecs[this->submitted_ops].iov_base, data, size);
    //     io_uring_prep_write_fixed(sqe, fd, this->iovecs[this->submitted_ops].iov_base, size, offset, this->submitted_ops);
    //     io_uring_sqe_set_data(sqe, this->iovecs[this->submitted_ops].iov_base);
    //     this->submitted_ops += 1;

    //     if constexpr (enable_logging) {
    //         this->logger->info("Prepared write at offset {}: requested {}", offset, size);
    //     }

    //     if (this->submitted_ops == this->config.batch_size) {
    //         io_uring_submit(&ring);

    //         while (this->submitted_ops > 0) {
    //             std::vector<io_uring_cqe*> cqe_batch(this->config.batch_size);
    //             int count = io_uring_peek_batch_cqe(&ring, cqe_batch.data(), cqe_batch.size());

    //             if (count == 0) {
    //                 sleep(1);
    //                 continue;
    //             }

    //             for (int i = 0; i < count; i++) {
    //                 io_uring_cqe* cqe = cqe_batch[i];
    //                 //char* buf = static_cast<char*>(io_uring_cqe_get_data(cqe));

    //                 if constexpr (enable_logging) {
    //                     if (cqe->res < 0) {
    //                         this->logger->error("write failed: {}", strerror(-cqe->res));
    //                     } else {
    //                         this->logger->info("Write at offset {}: requested {}, wrote {}", offset, size, cqe->res);
    //                     }
    //                 }

    //                 io_uring_cqe_seen(&ring, cqe);
    //                 this->submitted_ops--;
    //             }
    //         }
    //     }
    // }

};

#endif
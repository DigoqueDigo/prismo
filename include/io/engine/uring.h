#ifndef URING_ENGINE_H
#define URING_ENGINE_H

#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <stdexcept>
#include <io/metric.h>
#include <io/engine/config.h>
#include <iostream>

namespace Engine {

    struct UringEngine {
        private:        
            UringConfig config;
            uint32_t submitted;

            struct io_uring ring;
            std::vector<struct iovec> iovecs;

            inline void read(int fd, size_t size, off_t offset, io_uring_sqe* sqe);
            inline void write(int fd, const void* buffer, size_t size, off_t offset, io_uring_sqe* sqe);

        public:
            inline explicit UringEngine(const UringConfig& _config);
            inline ~UringEngine();

            inline int open(const char* filename, OpenFlags flags, mode_t mode);       
            inline void close(int fd);

            inline void poll_completion();

            template<typename LoggerT, typename MetricT, Operation::OperationType OperationT>
            inline void submit(LoggerT& logger, int fd, void* buffer, size_t size, off_t offset);
    };

    inline UringEngine::UringEngine(const UringConfig& _config)
        : config(_config), submitted(0), iovecs(_config.batch) {

            int return_code = io_uring_queue_init_params(config.entries, &ring, &config.params);
            if (return_code) {
                throw std::runtime_error("Uring initialization failed: " + std::string(strerror(return_code)));
            }

            for (auto& iv : iovecs) {
                iv.iov_len = config.block_size;
                if (posix_memalign(&iv.iov_base, 4096, config.block_size))
                    throw std::bad_alloc();
                // TODO :: remove this shit
                std::memset(iv.iov_base, 0, config.block_size);
            }

            return_code = io_uring_register_buffers(&ring, iovecs.data(), iovecs.size());
            if (return_code) {
                io_uring_queue_exit(&ring);
                throw std::runtime_error("Uring register buffers failed: " + std::string(strerror(errno)));
            }
        }

    inline UringEngine::~UringEngine() {
        for (auto& iv : iovecs) {
            std::free(iv.iov_base);
        }
        io_uring_queue_exit(&ring);
    }

    inline int UringEngine::open(const char* filename, OpenFlags flags, mode_t mode) {
        int fd = ::open(filename, flags.value, mode);
        if (fd < 0) {
            throw std::runtime_error("Failed to open file: " + std::string(strerror(errno)));
        }
        int return_code = io_uring_register_files(&ring, &fd, 1);
        if (return_code) {
            throw std::runtime_error("Register file failed: " + std::string(strerror(return_code)));
        }
        return 0;
    }

    inline void UringEngine::close(int fd) {
        if (io_uring_unregister_files(&ring)) {
            throw std::runtime_error("Failed to unregister files: " + std::string(strerror(errno)));
        }
        if (::close(fd) < 0) {
            throw std::runtime_error("Failed to close fd: " + std::string(strerror(errno)));
        }
        if (io_uring_unregister_buffers(&ring)) {
            throw std::runtime_error("Failed to unregister buffers: " + std::string(strerror(errno)));
        }
    }

    inline void UringEngine::read(int fd_index, size_t size, off_t offset, io_uring_sqe* sqe) {
        io_uring_prep_read_fixed(sqe, fd_index, iovecs[submitted].iov_base, size, offset, submitted);
        sqe->flags |= IOSQE_FIXED_FILE;
        submitted++;
    }

    inline void UringEngine::write(int fd_index, const void* buffer, size_t size, off_t offset, io_uring_sqe* sqe) {
        // TODO :: uncomment this shit
        // std::memcpy(iovecs[submitted].iov_base, buffer, config.block_size);
        (void) buffer;
        io_uring_prep_write_fixed(sqe, fd_index, iovecs[submitted].iov_base, size, offset, submitted);
        sqe->flags |= IOSQE_FIXED_FILE;
        submitted++;
    }

    template<typename LoggerT, typename MetricT, Operation::OperationType OperationT>
    inline void UringEngine::submit(LoggerT& logger, int fd, void* buffer, size_t size, off_t offset) {

        (void) logger;
        io_uring_sqe* sqe = io_uring_get_sqe(&ring);
        std::cout << "submitted: " << submitted << std::endl;

        while (sqe == nullptr || submitted == config.batch) {
            io_uring_submit(&ring);
            poll_completion();
            sqe = io_uring_get_sqe(&ring);
        }

        if constexpr (OperationT == Operation::OperationType::READ) {
            read(fd, size, offset, sqe);
        } else if constexpr (OperationT == Operation::OperationType::WRITE) {
            write(fd, buffer, size, offset, sqe);
        }
    }

    inline void UringEngine::poll_completion(void) {
        while (submitted == config.batch) {
            std::vector<io_uring_cqe*> cqe_batch(submitted);
            int count = io_uring_peek_batch_cqe(&ring, cqe_batch.data(), cqe_batch.size());
            for (int index = 0; index < count; index++) {
                io_uring_cqe* cqe = cqe_batch[index];
                io_uring_cqe_seen(&ring, cqe);
                //std::cout << "Return: " << cqe->res << std::endl;
            }
            std::cout << "completed: " << count << std::endl;
            submitted -= count;
        }
    }
};

#endif

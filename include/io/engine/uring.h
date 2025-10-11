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
#include <iostream>

namespace Engine {
    template <typename LoggerT, typename MetricT>
    struct UringEngine {
        private:        
            LoggerT logger;
            UringConfig config;

            uint32_t submitted;
            struct io_uring ring;
            std::vector<struct iovec> iovecs;

            // inline int fsync(int fd);
            // inline int fdatasync(int fd);
            inline void read(int fd, size_t size, off_t offset, io_uring_sqe* sqe);
            inline void write(int fd, const void* buffer, size_t size, off_t offset, io_uring_sqe* sqe);

        public:
            explicit UringEngine(const UringConfig& _config, const LoggerT& _logger);
            ~UringEngine();

            int open(const char* filename, OpenFlags flags, mode_t mode);       
            void close(int fd);

            template<Operation::OperationType OperationT>
            void submit(int fd, void* buffer, size_t size, off_t offset);
    };

    template<typename LoggerT, typename MetricT>
    UringEngine<LoggerT, MetricT>::UringEngine(const UringConfig& _config, const LoggerT& _logger)
        : logger(_logger), config(_config), submitted(0), iovecs(_config.batch) {

            if (io_uring_queue_init(config.entries, &ring, 0)) {
                throw std::runtime_error("Uring initialization failed: " + std::string(strerror(errno)));
            }

            for (auto& buffer : iovecs) {
                buffer.iov_base = std::malloc(config.block_size);
                buffer.iov_len = config.block_size;
                if (buffer.iov_base == nullptr) {
                    io_uring_queue_exit(&ring);
                    throw std::bad_alloc();
                }
            }
 
            if (io_uring_register_buffers(&ring, iovecs.data(), iovecs.size())) {
                io_uring_queue_exit(&ring);
                throw std::runtime_error("Uring register buffers failed: " + std::string(strerror(errno)));
            }
        }

    template<typename LoggerT, typename MetricT>
    UringEngine<LoggerT, MetricT>::~UringEngine() {
        for (auto& buffer : iovecs) {
            std::free(buffer.iov_base);
        }
        std::cout << "olassssss" << std::endl;
        io_uring_queue_exit(&ring);
    }

    template<typename LoggerT, typename MetricT>
    int UringEngine<LoggerT, MetricT>::open(const char* filename, OpenFlags flags, mode_t mode) {
        int fd = ::open(filename, flags.value, mode);
        if (fd < 0) {
            throw std::runtime_error("Failed to open file: " + std::string(strerror(errno)));
        }
        int return_code = io_uring_register_files(&this->ring, &fd, 1);
        if (return_code) {
            throw std::runtime_error("Register file failed: " + std::string(strerror(return_code)));
        }
        return fd;
    }

    template<typename LoggerT, typename MetricT>
    inline void UringEngine<LoggerT, MetricT>::read(int fd, size_t size, off_t offset, io_uring_sqe* sqe) {
        io_uring_prep_read_fixed(sqe, fd, this->iovecs[this->submitted].iov_base, size, offset, this->submitted);
        this->submitted++;
    }

    template<typename LoggerT, typename MetricT>
    inline void UringEngine<LoggerT, MetricT>::write(int fd, const void* buffer, size_t size, off_t offset, io_uring_sqe* sqe) {
        std::memcpy(this->iovecs[this->submitted].iov_base, buffer, this->config.block_size);
        io_uring_prep_write_fixed(sqe, fd, this->iovecs[this->submitted].iov_base, size, offset, this->submitted);
        this->submitted++;
    }

    template<typename LoggerT, typename MetricT>
    void UringEngine<LoggerT, MetricT>::close(int fd) {
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

    template<typename LoggerT, typename MetricT>
    template<Operation::OperationType OperationT>
    void UringEngine<LoggerT, MetricT>::submit(int fd, void* buffer, size_t size, off_t offset) {

        io_uring_sqe* sqe = io_uring_get_sqe(&this->ring);

        if (sqe == NULL) {
            throw std::runtime_error("Failed to get SQE: " + std::string(strerror(errno)));
        }


        if constexpr (OperationT == Operation::OperationType::READ) {
            this->read(fd, size, offset, sqe);
        } else if constexpr (OperationT == Operation::OperationType::WRITE) {
            this->write(fd, buffer, size, offset, sqe);
        }

        if (this->submitted == this->config.batch) {
            io_uring_submit(&ring);

            while (this->submitted > 0) {
                std::vector<io_uring_cqe*> cqe_batch(this->submitted);
                int count = io_uring_peek_batch_cqe(&ring, cqe_batch.data(), cqe_batch.size());

                if (count > 0){
                    for (int i = 0; i < count; i++) {
                        io_uring_cqe* cqe = cqe_batch[i];
                        // std::cout <<  cqe->res << std::endl;
                        io_uring_cqe_seen(&ring, cqe);
                        this->submitted--;
                    }
                }
            }
        }
    }
};

#endif
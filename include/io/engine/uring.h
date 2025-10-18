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

    struct UserData {
        size_t size;
        off_t offset;
        uint32_t index;
        int64_t start_timestamp;
        Operation::OperationType operation_type;
    };

    struct UringEngine {
        private:
            UringConfig config;
            struct io_uring ring;

            std::vector<iovec> iovecs;
            std::vector<UserData> user_data;

            uint32_t submitted_ops;
            std::vector<uint32_t> available_indexs;

            inline void read(int fd_index, void* buffer, size_t size, off_t offset, io_uring_sqe* sqe, uint32_t free_index);
            inline void write(int fd_index, const void* buffer, size_t size, off_t offset, io_uring_sqe* sqe, uint32_t free_index);

        public:
            inline explicit UringEngine(const UringConfig& _config);
            inline ~UringEngine();

            inline int open(const char* filename, OpenFlags flags, mode_t mode);
            inline void close(int fd);

            template<typename LoggerT, typename MetricT>
            inline uint32_t reap_completion(LoggerT& logger);

            template<typename LoggerT, typename MetricT>
            inline void reap_left_completions(LoggerT& logger);

            template<typename LoggerT, typename MetricT, Operation::OperationType OperationT>
            inline void submit(LoggerT& logger, int fd, void* buffer, size_t size, off_t offset);
    };

    inline UringEngine::UringEngine(const UringConfig& _config)
        : config(_config), iovecs(), user_data(), submitted_ops(0), available_indexs() {

            int return_code = io_uring_queue_init_params(config.entries, &ring, &config.params);
            if (return_code) {
                throw std::runtime_error("Uring initialization failed: " + std::string(strerror(return_code)));
            }

            iovecs.resize(config.params.sq_entries);
            user_data.resize(config.params.sq_entries);
            available_indexs.resize(config.params.sq_entries);

            for (uint32_t index = 0; index < config.params.sq_entries; index++) {
                available_indexs[index] = config.params.sq_entries - index - 1;
                iovecs[index].iov_len = config.block_size;
                iovecs[index].iov_base = std::malloc(config.block_size);
                if (iovecs[index].iov_base == nullptr) {
                    throw std::bad_alloc();
                }
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
        return fd;
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

    inline void UringEngine::read(int fd_index, void* buffer, size_t size, off_t offset, io_uring_sqe* sqe, uint32_t free_index) {
        (void) buffer;
        io_uring_prep_read_fixed(sqe, fd_index, iovecs[free_index].iov_base, size, offset, free_index);
        io_uring_sqe_set_data(sqe, &user_data[free_index]);
        sqe->flags |= IOSQE_FIXED_FILE;
    }

    inline void UringEngine::write(int fd_index, const void* buffer, size_t size, off_t offset, io_uring_sqe* sqe, uint32_t free_index) {
        std::memcpy(iovecs[free_index].iov_base, buffer, size);
        io_uring_prep_write_fixed(sqe, fd_index, iovecs[free_index].iov_base, size, offset, free_index);
        io_uring_sqe_set_data(sqe, &user_data[free_index]);
        sqe->flags |= IOSQE_FIXED_FILE;
    }

    template<typename LoggerT, typename MetricT, Operation::OperationType OperationT>
    inline void UringEngine::submit(LoggerT& logger, int fd, void* buffer, size_t size, off_t offset) {
        (void) fd;
        uint32_t free_index;
        io_uring_sqe* sqe = io_uring_get_sqe(&ring);

        while (sqe == nullptr || available_indexs.empty()) {
            if (sqe == nullptr) {
                io_uring_submit(&ring);
            }

            free_index = this->template reap_completion<LoggerT, MetricT>(logger);
            available_indexs.push_back(free_index);

            if (sqe == nullptr) {
                sqe = io_uring_get_sqe(&ring);
            }
        }

        free_index = available_indexs.back();
        available_indexs.pop_back();

        user_data[free_index].size = size;
        user_data[free_index].offset = offset;
        user_data[free_index].index = free_index;
        user_data[free_index].operation_type = OperationT;
        user_data[free_index].start_timestamp =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count();

        if constexpr (OperationT == Operation::OperationType::READ) {
            read(0, buffer, size, offset, sqe, free_index);
        } else if constexpr (OperationT == Operation::OperationType::WRITE) {
            write(0, buffer, size, offset, sqe, free_index);
        }

        submitted_ops++;
    }

    template<typename LoggerT, typename MetricT>
    inline uint32_t UringEngine::reap_completion(LoggerT& logger) {
        int return_code;
        io_uring_cqe* cqe = nullptr;

        do {
            return_code = io_uring_peek_cqe(&ring, &cqe);
        } while (return_code);

        UserData* user_data = static_cast<UserData*>(io_uring_cqe_get_data(cqe));

        if constexpr (!std::is_same_v<MetricT, std::monostate>) {
            MetricT metric{};
            metric.start_timestamp = user_data->start_timestamp;
            metric.operation_type = user_data->operation_type;

            metric.end_timestamp =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()
                ).count();

            if constexpr (std::is_base_of_v<Metric::StandardMetric, MetricT>) {
                metric.pid = ::getpid();
                metric.tid = static_cast<uint64_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
            }

            if constexpr (std::is_base_of_v<Metric::FullMetric, MetricT>) {
                metric.error_no        = errno;
                metric.return_code     = cqe->res;
                metric.requested_bytes = user_data->size;
                metric.offset          = user_data->offset;
                metric.processed_bytes = (cqe->res > 0) ? static_cast<uint32_t>(cqe->res) : 0;
            }

            logger.info(metric);
        }

        io_uring_cqe_seen(&ring, cqe);
        return user_data->index;
    }

    template<typename LoggerT, typename MetricT>
    inline void UringEngine::reap_left_completions(LoggerT& logger) {
        io_uring_submit(&ring);
        while (available_indexs.size() < available_indexs.capacity()) {
            uint32_t free_index = this->template reap_completion<LoggerT, MetricT>(logger);
            available_indexs.push_back(free_index);
        }
    }
};

#endif

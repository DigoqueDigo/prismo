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

            uint32_t free_iovec;
            uint32_t submitted_ops;
            uint32_t reaped_ops;

            std::vector<iovec> iovecs;
            std::vector<UserData> user_data;

            inline void read(int fd_index, void* buffer, size_t size, off_t offset, io_uring_sqe* sqe);
            inline void write(int fd_index, const void* buffer, size_t size, off_t offset, io_uring_sqe* sqe);

        public:
            inline explicit UringEngine(const UringConfig& _config);
            inline ~UringEngine();

            inline int open(const char* filename, OpenFlags flags, mode_t mode);       
            inline void close(int fd);

            template<typename LoggerT, typename MetricT>
            inline int reap_completion(LoggerT& logger);

            template<typename LoggerT, typename MetricT>
            inline void reap_left_completions(LoggerT& logger);

            template<typename LoggerT, typename MetricT, Operation::OperationType OperationT>
            inline void submit(LoggerT& logger, int fd, void* buffer, size_t size, off_t offset);
    };

    inline UringEngine::UringEngine(const UringConfig& _config)
        : config(_config), free_iovec(0), submitted_ops(0), reaped_ops(0), iovecs(), user_data() {

            int return_code = io_uring_queue_init_params(config.entries, &ring, &config.params);
            if (return_code) {
                throw std::runtime_error("Uring initialization failed: " + std::string(strerror(return_code)));
            }

            user_data.resize(config.params.sq_entries);
            iovecs.resize(config.params.sq_entries);

            std::cout << "SQE size: " << config.params.sq_entries << std::endl;

            for (auto& iv : iovecs) {
                iv.iov_len = config.block_size;
                iv.iov_base = std::malloc(config.block_size);
                if (iv.iov_base == nullptr) {
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

    inline void UringEngine::read(int fd_index, void* buffer, size_t size, off_t offset, io_uring_sqe* sqe) {
        (void) buffer;
        io_uring_prep_read_fixed(sqe, fd_index, iovecs[free_iovec].iov_base, size, offset, free_iovec);
        io_uring_sqe_set_data(sqe, &user_data[free_iovec]);
        sqe->flags |= IOSQE_FIXED_FILE;
    }

    inline void UringEngine::write(int fd_index, const void* buffer, size_t size, off_t offset, io_uring_sqe* sqe) {
        std::memcpy(iovecs[free_iovec].iov_base, buffer, size);
        io_uring_prep_write_fixed(sqe, fd_index, iovecs[free_iovec].iov_base, size, offset, free_iovec);
        io_uring_sqe_set_data(sqe, &user_data[free_iovec]);
        sqe->flags |= IOSQE_FIXED_FILE;
    }

    template<typename LoggerT, typename MetricT, Operation::OperationType OperationT>
    inline void UringEngine::submit(LoggerT& logger, int fd, void* buffer, size_t size, off_t offset) {
        (void) fd;
        std::cout << "free_iovec: " << free_iovec << std::endl;

        io_uring_sqe* sqe = io_uring_get_sqe(&ring);
        UserData& ud = user_data[free_iovec];

        ud.size = size;
        ud.offset = offset;
        ud.index = free_iovec;
        ud.operation_type = OperationT;
        ud.start_timestamp =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count();

        while (sqe == nullptr) {
            if (submitted_ops % config.params.sq_entries == 0) {
                io_uring_submit(&ring);
                std::cout << "SUBMIT !!!!!!!!!!!!!!!!" << std::endl;
            }
            free_iovec = this->template reap_completion<LoggerT, MetricT>(logger);
            sqe = io_uring_get_sqe(&ring);
        }

        if constexpr (OperationT == Operation::OperationType::READ) {
            read(0, buffer, size, offset, sqe);
        } else if constexpr (OperationT == Operation::OperationType::WRITE) {
            write(0, buffer, size, offset, sqe);
        }

        if (submitted_ops < iovecs.size() - 1){
            free_iovec++;
        }

        submitted_ops++;
        std::cout << "total_submited_ops: " << submitted_ops << std::endl;
    }

    template<typename LoggerT, typename MetricT>
    inline int UringEngine::reap_completion(LoggerT& logger) {
        io_uring_cqe* cqe;
        while (io_uring_peek_cqe(&ring, &cqe)) {}
        UserData* user_data = static_cast<UserData*>(io_uring_cqe_get_data(cqe));

        std::cout << "free inside reap: " << user_data->index << std::endl;

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

        reaped_ops++;
        io_uring_cqe_seen(&ring, cqe);
        return user_data->index;
    }

    template<typename LoggerT, typename MetricT>
    inline void UringEngine::reap_left_completions(LoggerT& logger) {
        io_uring_submit(&ring);
        std::cout << "FLUSH SUBMIT !!!!!!!!!!!!!!!!" << std::endl;
        while (reaped_ops < submitted_ops) {
            free_iovec = this->template reap_completion<LoggerT, MetricT>(logger);
        }
    }
};

#endif

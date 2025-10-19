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
            std::vector<uint32_t> available_indexs;

            inline void nop(int fd_index, io_uring_sqe* sqe, uint32_t free_index);
            inline void fsync(int fd_index, io_uring_sqe* sqe, uint32_t free_index);
            inline void fdatasync(int fd_index, io_uring_sqe* sqe, uint32_t free_index);

            inline void read(int fd_index, void* buffer, size_t size, off_t offset, io_uring_sqe* sqe, uint32_t free_index);
            inline void write(int fd_index, const void* buffer, size_t size, off_t offset, io_uring_sqe* sqe, uint32_t free_index);

            template<typename MetricT>
            inline uint32_t reap_completion(MetricT& metric);

        public:
            inline explicit UringEngine(const UringConfig& _config);
            inline ~UringEngine();

            inline int open(const char* filename, OpenFlags flags, OpenMode mode);
            inline void close(int fd);

            template<typename MetricT>
            inline std::vector<MetricT> reap_left_completions();

            template<typename MetricT, Operation::OperationType OperationT>
            inline std::optional<MetricT> submit(int fd, void* buffer, size_t size, off_t offset);
    };

    inline UringEngine::UringEngine(const UringConfig& _config)
        : config(_config), iovecs(), user_data(), available_indexs() {

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

            return_code = io_uring_register_buffers(&ring, iovecs.data(), config.params.sq_entries);
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

    inline int UringEngine::open(const char* filename, OpenFlags flags, OpenMode mode) {
        int fd = ::open(filename, flags.value, mode.value);
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

    inline void UringEngine::nop(int fd_index, io_uring_sqe* sqe, uint32_t free_index) {
        (void) fd_index;
        io_uring_prep_nop(sqe);
        io_uring_sqe_set_data(sqe, &user_data[free_index]);
        sqe->flags |= IOSQE_FIXED_FILE;
    }

    inline void UringEngine::fsync(int fd_index, io_uring_sqe* sqe, uint32_t free_index) {
        io_uring_prep_fsync(sqe, fd_index, 0);
        io_uring_sqe_set_data(sqe, &user_data[free_index]);
        sqe->flags |= IOSQE_FIXED_FILE;
    }

    inline void UringEngine::fdatasync(int fd_index, io_uring_sqe* sqe, uint32_t free_index) {
        io_uring_prep_fsync(sqe, fd_index, IORING_FSYNC_DATASYNC);
        io_uring_sqe_set_data(sqe, &user_data[free_index]);
        sqe->flags |= IOSQE_FIXED_FILE;
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

    template<typename MetricT, Operation::OperationType OperationT>
    inline std::optional<MetricT> UringEngine::submit(int fd, void* buffer, size_t size, off_t offset) {
        (void) fd;
        uint32_t free_index;
        std::optional<MetricT> op_metric = std::nullopt;

        io_uring_sqe* sqe = io_uring_get_sqe(&ring);

        if (sqe == nullptr) {
            io_uring_submit(&ring);
        }

        if (available_indexs.empty()) {
            MetricT metric{};
            free_index = this->template reap_completion<MetricT>(metric);
            available_indexs.push_back(free_index);
            op_metric = std::move(metric);
        }

        while (sqe == nullptr) {
            sqe = io_uring_get_sqe(&ring);
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
        } else if constexpr (OperationT == Operation::OperationType::FSYNC) {
            fsync(0, sqe, free_index);
        } else if constexpr (OperationT == Operation::OperationType::FDATASYNC) {
            fdatasync(0, sqe, free_index);
        } else if constexpr (OperationT == Operation::OperationType::NOP) {
            nop(0, sqe, free_index);
        }

        return op_metric;
    }

    template<typename MetricT>
    inline uint32_t UringEngine::reap_completion(MetricT& metric) {
        int return_code;
        io_uring_cqe* cqe = nullptr;

        do {
            return_code = io_uring_peek_cqe(&ring, &cqe);
        } while (return_code);

        UserData* cqe_user_data = static_cast<UserData*>(io_uring_cqe_get_data(cqe));

        if constexpr (std::is_base_of_v<Metric::BaseMetric, MetricT>) {
            metric.start_timestamp = cqe_user_data->start_timestamp;
            metric.operation_type = cqe_user_data->operation_type;
            metric.end_timestamp =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()
                ).count();
        }

        if constexpr (std::is_base_of_v<Metric::StandardMetric, MetricT>) {
            metric.pid = ::getpid();
            metric.tid = static_cast<uint64_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
        }

        if constexpr (std::is_base_of_v<Metric::FullMetric, MetricT>) {
            metric.requested_bytes = cqe_user_data->size;
            metric.processed_bytes = (cqe->res > 0) ? static_cast<size_t>(cqe->res) : 0;
            metric.offset          = cqe_user_data->offset;
            metric.return_code     = static_cast<int32_t>(cqe->res);
            metric.error_no        = static_cast<int32_t>(errno);
        }

        io_uring_cqe_seen(&ring, cqe);
        return cqe_user_data->index;
    }

    template<typename MetricT>
    inline std::vector<MetricT> UringEngine::reap_left_completions() {
        MetricT metric;
        std::vector<MetricT> collected_metrics;
        io_uring_submit(&ring);

        while (available_indexs.size() < available_indexs.capacity()) {
            uint32_t free_index = this->template reap_completion<MetricT>(metric);
            available_indexs.push_back(free_index);
            collected_metrics.push_back(metric);
        }

        return collected_metrics;
    }
};

#endif

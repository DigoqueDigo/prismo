#ifndef POSIX_ENGINE_H
#define POSIX_ENGINE_H

#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <stdexcept>
#include <io/metric.h>
#include <operation/type.h>
#include <io/engine/config.h>

namespace Engine {

    struct PosixEngine {
        private:
            inline int nop(int fd);
            inline int fsync(int fd);
            inline int fdatasync(int fd);

            inline ssize_t read(int fd, void* buffer, size_t size, off_t offset);
            inline ssize_t write(int fd, const void* buffer, size_t size, off_t offset);

        public:
            PosixEngine() = default;

            inline int open(const char* filename, OpenFlags flags, OpenMode mode);
            inline int close(int fd);

            template<typename MetricT, Operation::OperationType OperationT>
            inline void submit(int fd, void* buffer, size_t size, off_t offset, std::vector<MetricT>& metrics);
    };

    inline int PosixEngine::open(const char* filename, OpenFlags flags, OpenMode mode) {
        int fd = ::open(filename, flags.value, mode.value);
        if (fd < 0) {
            throw std::runtime_error("Failed to open file: " + std::string(strerror(errno)));
        }
        return fd;
    }

    inline int PosixEngine::close(int fd) {
        int return_code = ::close(fd);
        if (return_code < 0) {
            throw std::runtime_error("Failed to close fd: " + std::string(strerror(errno)));
        }
        return return_code;
    }

    inline int PosixEngine::nop(int fd) {
        (void) fd;
        return 0;
    }

    inline int PosixEngine::fsync(int fd) {
        return ::fsync(fd);
    }

    inline int PosixEngine::fdatasync(int fd) {
        return ::fdatasync(fd);
    }

    inline ssize_t PosixEngine::read(int fd, void* buffer, size_t size, off_t offset) {
        return ::pread(fd, buffer, size, offset);
    }

    inline ssize_t PosixEngine::write(int fd, const void* buffer, size_t size, off_t offset) {
        return ::pwrite(fd, buffer, size, offset);
    }

    template<typename MetricT, Operation::OperationType OperationT>
    inline void PosixEngine::submit(int fd, void* buffer, size_t size, off_t offset, std::vector<MetricT>& metrics) {
        MetricT metric{};
        ssize_t result = 0;

        if constexpr (std::is_base_of_v<Metric::BaseMetric, MetricT>) {
            metric.operation_type = OperationT;
            metric.start_timestamp =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()
                ).count();
        }

        if constexpr (OperationT == Operation::OperationType::READ) {
            result = read(fd, buffer, size, offset);
        } else if constexpr (OperationT == Operation::OperationType::WRITE) {
            result = write(fd, buffer, size, offset);
        } else if constexpr (OperationT == Operation::OperationType::FSYNC) {
            result = fsync(fd);
        } else if constexpr (OperationT == Operation::OperationType::FDATASYNC) {
            result = fdatasync(fd);
        } else if constexpr (OperationT == Operation::OperationType::NOP) {
            result = nop(fd);
        }

        if constexpr (std::is_base_of_v<Metric::BaseMetric, MetricT>) {
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
            metric.requested_bytes = size;
            metric.processed_bytes = (result > 0) ? static_cast<size_t>(result) : 0;
            metric.offset          = offset;
            metric.return_code     = static_cast<int32_t>(result);
            metric.error_no        = static_cast<int32_t>(errno);
        }

        if constexpr (std::is_base_of_v<Metric::BaseMetric, MetricT>) {
            metrics.push_back(std::move(metric));
        }
    }
};

#endif
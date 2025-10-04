#ifndef POSIX_BACKEND_ENGINE_H
#define POSIX_BACKEND_ENGINE_H

#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <stdexcept>
#include <io/metric.h>
#include <io/logger.h>

namespace BackendEngine {
    template <typename LoggerT, typename MetricT>
    struct PosixEngine {
        const LoggerT logger;

        explicit PosixEngine(const LoggerT& _logger);

        int _open(const char* filename, int flags, mode_t mode);        
        void _read(int fd, void* buffer, size_t size, off_t offset); 
        void _write(int fd, const void* buffer, size_t size, off_t offset);
        void _close(int fd);
    };

    template<typename LoggerT, typename MetricT>
    PosixEngine<LoggerT, MetricT>::PosixEngine(const LoggerT& _logger)
        : logger(_logger) {}

    template<typename LoggerT, typename MetricT>
    int PosixEngine<LoggerT, MetricT>::_open(const char* filename, int flags, mode_t mode) {
        ssize_t fd = ::open(filename, flags, mode);
        if (fd < 0) {
            throw std::runtime_error("Failed to open file: " + std::string(strerror(errno)));
        }
        return fd;
    }

    template<typename LoggerT, typename MetricT>
    void PosixEngine<LoggerT, MetricT>::_read(int fd, void* buffer, size_t size, off_t offset) {
        if constexpr (!std::is_same_v<MetricT, std::monostate>) {
            MetricT metric{};

            metric.start_timestamp =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count();

            ssize_t bytes_read = ::pread(fd, buffer, size, offset);

            metric.end_timestamp =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()
                ).count();

            if constexpr (std::is_base_of_v<Metric::BaseSyncMetric, MetricT>) {
                metric.operation_type = OperationPattern::OperationType::READ;
            }

            if constexpr (std::is_base_of_v<Metric::StandardSyncMetric, MetricT>) {
                metric.pid = ::getpid();
                metric.tid = static_cast<uint64_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
            }

            if constexpr (std::is_base_of_v<Metric::FullSyncMetric, MetricT>) {
                metric.requested_bytes  = static_cast<uint32_t>(size);
                metric.processed_bytes  = (bytes_read > 0) ? static_cast<uint32_t>(bytes_read) : 0;
                metric.offset           = static_cast<uint64_t>(offset);
                metric.return_code      = static_cast<int32_t>(bytes_read);
                metric.error_no         = errno;
            }

            this->logger.info(metric);
        } else {
            pread(fd, buffer, size, offset);
        }
    }

    template<typename LoggerT, typename MetricT>
    void PosixEngine<LoggerT, MetricT>::_write(int fd, const void* buffer, size_t size, off_t offset) {
        if constexpr (!std::is_same_v<MetricT, std::monostate>) {
            MetricT metric{};

            metric.start_timestamp =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()
                ).count();

            ssize_t bytes_write = ::pwrite(fd, buffer, size, offset);

            metric.end_timestamp =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()
                ).count();

            if constexpr (std::is_base_of_v<Metric::BaseSyncMetric, MetricT>) {
                metric.operation_type = OperationPattern::OperationType::WRITE;
            }

            if constexpr (std::is_base_of_v<Metric::StandardSyncMetric, MetricT>) {
                metric.pid = ::getpid();
                metric.tid = static_cast<uint64_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
            }

            if constexpr (std::is_base_of_v<Metric::FullSyncMetric, MetricT>) {
                metric.requested_bytes  = static_cast<uint32_t>(size);
                metric.processed_bytes  = (bytes_write > 0) ? static_cast<uint32_t>(bytes_write) : 0;
                metric.offset           = static_cast<uint64_t>(offset);
                metric.return_code      = static_cast<int32_t>(bytes_write);
                metric.error_no         = errno;
            }

            this->logger.info(metric);
        } else {
            pwrite(fd, buffer, size, offset);
        }
    }

    template<typename LoggerT, typename MetricT>
    void PosixEngine<LoggerT, MetricT>::_close(int fd) {
        int return_code = ::close(fd);
        if (return_code < 0) {
            throw std::runtime_error("Failed to close fd: " + std::string(strerror(errno)));
        }
    }
};

#endif
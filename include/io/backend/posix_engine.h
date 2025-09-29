#ifndef POSIX_ENGINE_H
#define POSIX_ENGINE_H

#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <io/metric.h>
#include <spdlog/spdlog.h>

namespace BackendEngine {
    template <typename Metric = void>
    struct PosixEngine {
        // TODO :: save metrics in memory???
        // std::vector<Metric> metrics;
        const std::shared_ptr<spdlog::logger> logger;

        explicit PosixEngine(
            const std::shared_ptr<spdlog::logger>& _logger
        );

        int _open(const char* filename, int flags, mode_t mode);        
        void _read(int fd, void* buffer, size_t size, off_t offset); 
        void _write(int fd, const void* buffer, size_t size, off_t offset);
        void _close(int fd);
    };

    template<typename Metric>
    PosixEngine<Metric>::PosixEngine(const std::shared_ptr<spdlog::logger>& _logger)
        : logger(_logger) {}

    template<typename Metric>
    int PosixEngine<Metric>::_open(const char* filename, int flags, mode_t mode) {
        ssize_t fd = open(filename, flags, mode);
        if (fd < 0) {
            throw std::runtime_error("Failed to open file: " + std::string(strerror(errno)));
        }
        return fd;
    }

    template<typename Metric>
    void PosixEngine<Metric>::_read(int fd, void* buffer, size_t size, off_t offset) {
        if constexpr (!std::is_void_v<Metric>) {
            Metric metric{};
            
            metric.start_timestamp =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count();
            
            ssize_t bytes_read = pread(fd, buffer, size, offset);

            metric.end_timestamp =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()
                ).count();

            if constexpr (std::is_base_of_v<IOMetric::BaseSyncMetric, Metric>) {
                metric.operation_type = OperationPattern::OperationType::READ;
            }

            if constexpr (std::is_base_of_v<IOMetric::ThreadSyncMetric, Metric>) {
                metric.pid = ::getpid();
                metric.tid = static_cast<int32_t>(::syscall(SYS_gettid));
            }

            if constexpr (std::is_base_of_v<IOMetric::FullSyncMetric, Metric>) {
                metric.requested_bytes  = static_cast<uint32_t>(size);
                metric.processed_bytes  = (bytes_read > 0) ? static_cast<uint32_t>(bytes_read) : 0;
                metric.offset           = offset;
                metric.return_code      = static_cast<int32_t>(bytes_read);
                metric.error_no         = (bytes_read < 0) ? errno : 0;
            }

            this->logger->info(metric);
        }
    }

    template<typename Metric>
    void PosixEngine<Metric>::_write(int fd, const void* buffer, size_t size, off_t offset) {
        if constexpr (!std::is_void_v<Metric>) {
            Metric metric{};

            metric.start_timestamp =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()
                ).count();

            ssize_t bytes_write = pwrite(fd, buffer, size, offset);

            metric.end_timestamp =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()
                ).count();

            if constexpr (std::is_base_of_v<IOMetric::BaseSyncMetric, Metric>) {
                metric.operation_type = OperationPattern::OperationType::WRITE;
            }

            if constexpr (std::is_base_of_v<IOMetric::ThreadSyncMetric, Metric>) {
                metric.pid = ::getpid();
                metric.tid = static_cast<int32_t>(::syscall(SYS_gettid));
            }

            if constexpr (std::is_base_of_v<IOMetric::FullSyncMetric, Metric>) {
                metric.requested_bytes  = static_cast<uint32_t>(size);
                metric.processed_bytes  = (bytes_write > 0) ? static_cast<uint32_t>(bytes_write) : 0;
                metric.offset           = offset;
                metric.return_code      = static_cast<int32_t>(bytes_write);
                metric.error_no         = (bytes_write < 0) ? errno : 0;
            }

            this->logger->info(metric);
        }
    }

    template<typename Metric>
    void PosixEngine<Metric>::_close(int fd) {
        int return_code = close(fd);
        if (return_code < 0) {
            throw std::runtime_error("Failed to close fd: " + std::string(strerror(errno)));
        }
    }
};

#endif
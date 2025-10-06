#ifndef POSIX_ENGINE_H
#define POSIX_ENGINE_H

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
    struct PosixEngine {
        private:
            const LoggerT logger;

            inline int fsync(int fd);
            inline int fdatasync(int fd);
            inline ssize_t read(int fd, void* buffer, size_t size, off_t offset);
            inline ssize_t write(int fd, const void* buffer, size_t size, off_t offset);

        public:
            explicit PosixEngine(const LoggerT& _logger);

            int open(const char* filename, Flags flags, mode_t mode);        
            void close(int fd);

            template<Operation::OperationType OperationT>
            void submit(int fd, void* buffer, size_t size, off_t offset);
    };

    template<typename LoggerT, typename MetricT>
    PosixEngine<LoggerT, MetricT>::PosixEngine(const LoggerT& _logger)
        : logger(_logger) {}

    template<typename LoggerT, typename MetricT>
    int PosixEngine<LoggerT, MetricT>::open(const char* filename, Flags flags, mode_t mode) {
        ssize_t fd = ::open(filename, flags.value, mode);
        if (fd < 0) {
            throw std::runtime_error("Failed to open file: " + std::string(strerror(errno)));
        }
        return fd;
    }

    template<typename LoggerT, typename MetricT>
    inline int PosixEngine<LoggerT, MetricT>::fsync(int fd) {
        return ::fsync(fd);
    }

    template<typename LoggerT, typename MetricT>
    inline int PosixEngine<LoggerT, MetricT>::fdatasync(int fd) {
        return ::fdatasync(fd);
    }

    template<typename LoggerT, typename MetricT>
    inline ssize_t PosixEngine<LoggerT, MetricT>::read(int fd, void* buffer, size_t size, off_t offset) {
        return ::pread(fd, buffer, size, offset);
    }

    template<typename LoggerT, typename MetricT>
    inline ssize_t PosixEngine<LoggerT, MetricT>::write(int fd, const void* buffer, size_t size, off_t offset) {
        return ::pwrite(fd, buffer, size, offset);
    }

    template<typename LoggerT, typename MetricT>
    void PosixEngine<LoggerT, MetricT>::close(int fd) {
        int return_code = ::close(fd);
        if (return_code < 0) {
            throw std::runtime_error("Failed to close fd: " + std::string(strerror(errno)));
        }
    }

    template<typename LoggerT, typename MetricT>
    template<Operation::OperationType OperationT>
    void PosixEngine<LoggerT, MetricT>::submit(int fd, void* buffer, size_t size, off_t offset) {
        if constexpr (!std::is_same_v<MetricT, std::monostate>) {
            MetricT metric{};
            ssize_t result = 0;
            metric.operation_type = OperationT;

            metric.start_timestamp =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()
                ).count();

            if constexpr (OperationT == Operation::OperationType::READ) {
                result = this->read(fd, buffer, size, offset);
            } else if constexpr (OperationT == Operation::OperationType::WRITE) {
                result = this->write(fd, buffer, size, offset);
            } else if constexpr (OperationT == Operation::OperationType::FSYNC) {
                result = this->fsync(fd);
            } else if constexpr (OperationT == Operation::OperationType::FDATASYNC) {
                result = this->fdatasync(fd);
            }

            metric.end_timestamp =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()
                ).count();

            if constexpr (std::is_base_of_v<Metric::StandardSyncMetric, MetricT>) {
                metric.pid = ::getpid();
                metric.tid = static_cast<uint64_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
            }

            if constexpr (std::is_base_of_v<Metric::FullSyncMetric, MetricT>) {
                metric.requested_bytes = static_cast<uint32_t>(size);
                metric.processed_bytes = (result > 0) ? static_cast<uint32_t>(result) : 0;
                metric.offset          = static_cast<uint64_t>(offset);
                metric.return_code     = static_cast<int32_t>(result);
                metric.error_no        = errno;
            }

            this->logger.info(metric);
        } else {
            if constexpr (OperationT == Operation::OperationType::READ) {
                this->read(fd, buffer, size, offset);
            } else if constexpr (OperationT == Operation::OperationType::WRITE) {
                this->write(fd, buffer, size, offset);
            } else if constexpr (OperationT == Operation::OperationType::FSYNC) {
                this->fsync(fd);
            } else if constexpr (OperationT == Operation::OperationType::FDATASYNC) {
                this->fdatasync(fd);
            }
        }
    }
};

#endif
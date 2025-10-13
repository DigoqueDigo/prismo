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
            inline int fsync(int fd);
            inline int fdatasync(int fd);

            inline ssize_t read(int fd, void* buffer, size_t size, off_t offset);
            inline ssize_t write(int fd, const void* buffer, size_t size, off_t offset);            

        public:
            PosixEngine() = default;

            inline int open(const char* filename, OpenFlags flags, mode_t mode);    
            inline int close(int fd);

            template<typename LoggerT, typename MetricT, Operation::OperationType OperationT>
            inline void submit(LoggerT& logger, int fd, void* buffer, size_t size, off_t offset);
    };

    inline int PosixEngine::open(const char* filename, OpenFlags flags, mode_t mode) {
        int fd = ::open(filename, flags.value, mode);
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

    template<typename LoggerT, typename MetricT, Operation::OperationType OperationT>
    inline void PosixEngine::submit(LoggerT& logger, int fd, void* buffer, size_t size, off_t offset) {
        if constexpr (!std::is_same_v<MetricT, std::monostate>) {
            MetricT metric{};
            ssize_t result = 0;
            metric.operation_type = OperationT;

            metric.start_timestamp =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()
                ).count();

            if constexpr (OperationT == Operation::OperationType::READ) {
                result = read(fd, buffer, size, offset);
            } else if constexpr (OperationT == Operation::OperationType::WRITE) {
                result = write(fd, buffer, size, offset);
            } else if constexpr (OperationT == Operation::OperationType::FSYNC) {
                result = fsync(fd);
            } else if constexpr (OperationT == Operation::OperationType::FDATASYNC) {
                result = fdatasync(fd);
            }

            metric.end_timestamp =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()
                ).count();

            if constexpr (std::is_base_of_v<Metric::StandardMetric, MetricT>) {
                metric.pid = ::getpid();
                metric.tid = static_cast<uint64_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
            }

            if constexpr (std::is_base_of_v<Metric::FullMetric, MetricT>) {
                metric.requested_bytes = static_cast<uint32_t>(size);
                metric.processed_bytes = (result > 0) ? static_cast<uint32_t>(result) : 0;
                metric.offset          = static_cast<uint64_t>(offset);
                metric.return_code     = static_cast<int32_t>(result);
                metric.error_no        = errno;
            }

            logger.info(metric);
        } else {
            if constexpr (OperationT == Operation::OperationType::READ) {
                read(fd, buffer, size, offset);
            } else if constexpr (OperationT == Operation::OperationType::WRITE) {
                write(fd, buffer, size, offset);
            } else if constexpr (OperationT == Operation::OperationType::FSYNC) {
                fsync(fd);
            } else if constexpr (OperationT == Operation::OperationType::FDATASYNC) {
                fdatasync(fd);
            }
        }
    }
};

#endif
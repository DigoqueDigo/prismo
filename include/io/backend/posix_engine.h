#ifndef POSIX_ENGINE_H
#define POSIX_ENGINE_H

#include <fcntl.h>

namespace BackendEngine {
    template <bool enable_logging>
    struct PosixEngine {
        const std::shared_ptr<spdlog::logger> logger;

        explicit PosixEngine(
            const std::shared_ptr<spdlog::logger>& _logger
        );

        int _open(const char* filename, int flags, mode_t mode);        
        void _read(int fd, void* buffer, size_t size, off_t offset); 
        void _write(int fd, const void* buffer, size_t size, off_t offset);
        void _close(int fd);
    };

    template<bool enable_logging>
    PosixEngine<enable_logging>::PosixEngine(const std::shared_ptr<spdlog::logger>& _logger)
        : logger(_logger) {}

    template<bool enable_logging>
    int PosixEngine<enable_logging>::_open(const char* filename, int flags, mode_t mode) {
        int fd = open(filename, flags, mode);
        if (fd < 0) {
            if constexpr (enable_logging)
                logger->error("Failed to open file {}: {}", filename, strerror(errno));
            throw std::runtime_error("Failed to open file: " + std::string(strerror(errno)));
        }

        if constexpr (enable_logging)
            logger->info("Opened file {} with fd {}", filename, fd);

        return fd;
    }

    template<bool enable_logging>
    void PosixEngine<enable_logging>::_read(int fd, void* buffer, size_t size, off_t offset) {
        ssize_t bytes_read = pread(fd, buffer, size, offset);
        if constexpr (enable_logging) {
            if (bytes_read < 0) {
                this->logger->error("Read error at offset {}: {}", offset, strerror(errno));
            } else {
                this->logger->info("Read at offset {}: requested {}, got {}", offset, size, bytes_read);
            }
        }
    }

    template<bool enable_logging>
    void PosixEngine<enable_logging>::_write(int fd, const void* buffer, size_t size, off_t offset) {
        ssize_t bytes_written = pwrite(fd, buffer, size, offset);
        if constexpr (enable_logging) {
            if (bytes_written < 0) {
                this->logger->error("Write error at offset {}: {}", offset, strerror(errno));
            } else {
                this->logger->info("Wrote at offset {}: requested {}, got {}", offset, size, bytes_written);
            }
        }
    }

    template<bool enable_logging>
    void PosixEngine<enable_logging>::_close(int fd) {
        int rc = close(fd);
        if (rc < 0) {
            if constexpr(enable_logging)
                this->logger->error("Failed to close fd {}: {}", fd, strerror(errno));
            throw std::runtime_error("Failed to close fd: " + std::string(strerror(errno)));
        }
        
        if constexpr (enable_logging)
            this->logger->info("Closed fd {}", fd);
    }
};

#endif
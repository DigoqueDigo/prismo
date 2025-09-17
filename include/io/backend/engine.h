#ifndef BACKEND_ENGINE_H
#define BACKEND_ENGINE_H

#include <unistd.h>
#include <fcntl.h>
#include <cstddef>
#include <liburing.h>
#include <sys/types.h>
#include <io/logger.h>
#include <io/backend/config.h>

namespace BackendEngine {
    struct PosixEngine {
        const std::shared_ptr<spdlog::logger> logger;

        explicit PosixEngine(const std::shared_ptr<spdlog::logger>& _logger)
            : logger(_logger) {}

        int _open(const char* filename, int flags, mode_t mode);        
        void _read(int fd, void* buffer, size_t size, off_t offset); 
        void _write(int fd, const void* buffer, size_t size, off_t offset);
        void _close(int fd);
    };

    struct IOUringEngine {
        io_uring ring;
        const BackendEngineConfig::IOUringConfig& config;
        const std::shared_ptr<spdlog::logger> logger;

        explicit IOUringEngine(const BackendEngineConfig::IOUringConfig& _config, std::shared_ptr<spdlog::logger>& _logger)
            : config(_config), logger(_logger) {
                if (io_uring_queue_init(config.queue_depth, &ring, config.flags) < 0) {
                    throw std::runtime_error("io_uring initialization failed");
                }
            }

        int _open(const char* filename, int flags, mode_t mode);        
        void _read(int fd, void* buffer, size_t size, off_t offset); 
        void _write(int fd, const void* buffer, size_t size, off_t offset);
        void _close(int fd);
    };
};

#endif
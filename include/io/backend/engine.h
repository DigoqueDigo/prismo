#ifndef BACKEND_ENGINE_H
#define BACKEND_ENGINE_H

#include <unistd.h>
#include <fcntl.h>
#include <cstddef>
#include <sys/types.h>
#include <io/logger.h>


namespace BackendEngine {
    struct PosixEngine {
        std::shared_ptr<spdlog::logger> logger;

        explicit PosixEngine(const std::shared_ptr<spdlog::logger>& _logger)
            : logger(_logger) {}

        int _open(const char* filename, int flags, mode_t mode);        
        void _read(int fd, void* buffer, size_t size, off_t offset); 
        void _write(int fd, const void* buffer, size_t size, off_t offset);
        void _close(int fd);
    };

    struct IOUringEngine {
        std::shared_ptr<spdlog::logger> logger;

        explicit IOUringEngine(const std::shared_ptr<spdlog::logger>& _logger)
            : logger(_logger) {}
    };
};

#endif
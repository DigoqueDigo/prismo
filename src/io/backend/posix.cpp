#include <io/backend/engine.h>

namespace BackendEngine {

    int PosixEngine::_open(const char* filename, int flags, mode_t mode) {
        int fd = open(filename, flags, mode);
        if (fd < 0) {
            this->logger->error("Failed to open file {}: {}", filename, strerror(errno));
            throw std::runtime_error("Failed to open file: " + std::string(strerror(errno)));
        } else {
            this->logger->info("Opened file {} with fd {}", filename, fd);
        }
        return fd;
    }

    void PosixEngine::_read(int fd, void* buffer, size_t size, off_t offset) {
        ssize_t bytes_read = pread(fd, buffer, size, offset);
        if (bytes_read < 0) {
            this->logger->error("Read error at offset {}: {}", offset, strerror(errno));
        } else {
            this->logger->info("Read at offset {}: requested {}, got {}", offset, size, bytes_read);
        }
    }

    void PosixEngine::_write(int fd, const void* buffer, size_t size, off_t offset) {
        ssize_t bytes_written = pwrite(fd, buffer, size, offset);
        if (bytes_written < 0) {
            this->logger->error("Write error at offset {}: {}", offset, strerror(errno));
        } else {
            this->logger->info("Wrote at offset {}: requested {}, got {}", offset, size, bytes_written);
        }
    }

    void PosixEngine::_close(int fd) {
        if (close(fd) < 0) {
            this->logger->error("Failed to close fd {}: {}", fd, strerror(errno));
            throw std::runtime_error("Failer to close fd: " + std::string(strerror(errno)));
        } else {
            this->logger->info("Closed fd {}", fd);
        }
    }
};
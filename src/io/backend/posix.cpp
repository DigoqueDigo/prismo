#include <io/backend/engine.h>

namespace BackendEngine {

    int PosixEngine::_open(const char* filename, int flags, mode_t mode) {
        int fd = open(filename, flags, mode);
        if (fd < 0) {
            logger->error("Failed to open file {}: {}", filename, errno);
        } else {
            logger->info("Opened file {} with fd {}", filename, fd);
        }
        return fd;
    }

    void PosixEngine::_read(int fd, void* buffer, size_t size, off_t offset) {
        ssize_t bytes_read = pread(fd, buffer, size, offset);
        if (bytes_read < 0) {
            logger->error("Read error at offset {}: {}", offset, errno);
        } else {
            logger->warn("Read at offset {}: requested {}, got {}", offset, size, bytes_read);
        }
    }

    void PosixEngine::_write(int fd, const void* buffer, size_t size, off_t offset) {
        ssize_t bytes_written = pwrite(fd, buffer, size, offset);
        if (bytes_written < 0) {
            logger->error("Write error at offset {}: {}", offset, errno);
        } else {
            logger->info("Wrote at offset {}: requested {}, got {}", offset, size, bytes_written);
        }
    }

    void PosixEngine::_close(int fd) {
        if (close(fd) < 0) {
            logger->error("Failed to close fd {}: {}", fd, errno);
        } else {
            logger->info("Closed fd {}", fd);
        }
    }
};
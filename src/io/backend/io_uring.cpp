#include <io/backend/engine.h>

namespace BackendEngine {

    int IOUringEngine::_open(const char* filename, int flags, mode_t mode) {
        io_uring_sqe* sqe;
        io_uring_cqe* cqe;

        if (!(sqe = io_uring_get_sqe(&this->ring))) {
            this->logger->error("Failed to get SQE during open: {}", strerror(errno));
            throw std::runtime_error("Failed to get SQE during open" + std::string(strerror(errno)));
        } else {
            this->logger->info("Got SQE for open operation");
        }

        io_uring_prep_openat(sqe, AT_FDCWD, filename, flags, mode);
        io_uring_submit(&this->ring);

        if (io_uring_wait_cqe(&this->ring, &cqe) < 0) {
            this->logger->error("wait_cqe failed during open: {}", strerror(errno));
            throw std::runtime_error("wait_cqe failed during open" + std::string(strerror(errno)));
        } else {
            this->logger->info("Open submitted for file {}", filename);
        }

        int fd = cqe->res;
        io_uring_cqe_seen(&this->ring, cqe);

        if (fd < 0) {
            this->logger->error("open failed: {}", strerror(errno));
            throw std::runtime_error("open failed: " + std::string(strerror(errno)));
        } else {
            this->logger->info("Opened file {} with fd {}", filename, fd);
        }

        return fd;
    }

    void IOUringEngine::_read(int fd, void* buffer, size_t size, off_t offset) {
        io_uring_sqe* sqe;
        io_uring_cqe* cqe;

        if (!(sqe = io_uring_get_sqe(&this->ring))) {
            this->logger->error("Failed to get SQE during read: {}", strerror(errno));
        } else {
            this->logger->info("Got SQE for read operation");
        }

        io_uring_prep_read(sqe, fd, buffer, size, static_cast<__u64>(offset));
        io_uring_submit(&ring);

        if (io_uring_wait_cqe(&this->ring, &cqe) < 0) {
            this->logger->error("wait_cqe failed during read at offset: {}", offset, strerror(errno));
        } else {
            this->logger->info("Read submitted at offset {}: requested {}", offset, size);
        }

        if (cqe->res < 0) {
            this->logger->error("read failed: " + std::string(strerror(-cqe->res)));
        } else {
            this->logger->info("Read at offset {}: requested {}, got {}", offset, size, cqe->res);
        }

        io_uring_cqe_seen(&ring, cqe);
    }

    void IOUringEngine::_write(int fd, const void* buffer, size_t size, off_t offset) {
        io_uring_sqe* sqe;
        io_uring_cqe* cqe;

        if (!(sqe = io_uring_get_sqe(&this->ring))) {
            this->logger->error("Failed to get SQE during write: {}", strerror(errno));
        } else {
            this->logger->info("Got SQE for write operation");
        }

        io_uring_prep_write(sqe, fd, buffer, size, static_cast<__u64>(offset));
        io_uring_submit(&ring);

        if (io_uring_wait_cqe(&this->ring, &cqe) < 0) {
            this->logger->error("wait_cqe failed during write at offset {}: {}", offset, strerror(errno));
        } else {
            this->logger->info("Write submitted at offset {}: requested {}", offset, size);
        }

        if (cqe->res < 0) {
            this->logger->error("write failed: " + std::string(strerror(-cqe->res)));
        } else {
            this->logger->info("Wrote at offset {}: requested {}, got {}", offset, size, cqe->res);
        }

        io_uring_cqe_seen(&ring, cqe);
    }

    void IOUringEngine::_close(int fd) {
        io_uring_sqe* sqe;
        io_uring_cqe* cqe;

        if (!(sqe = io_uring_get_sqe(&this->ring))) {
            this->logger->error("Failed to get SQE during close: {}", strerror(errno));
        } else {
            this->logger->info("Got SQE for close operation");
        }

        io_uring_prep_close(sqe, fd);
        io_uring_submit(&ring);

        if (io_uring_wait_cqe(&this->ring, &cqe) < 0) {
            this->logger->error("wait_cqe failed during close: {}", strerror(errno));
        } else {
            this->logger->info("Close submitted for fd {}", fd);
        }

        if (cqe->res < 0) {
            this->logger->error("Close failed: {}", strerror(errno));
        } else {
            this->logger->info("Closed fd {}", fd);
        }

        io_uring_cqe_seen(&ring, cqe);
    }
}
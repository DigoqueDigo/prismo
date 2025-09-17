#include <io/backend/engine.h>

namespace BackendEngine {

    int IOUringEngine::_open(const char* filename, int flags, mode_t mode) {
        io_uring_sqe* sqe = io_uring_get_sqe(&this->ring);

        if (!sqe){
            this->logger->error("Failed to get SQE for open");
            throw std::runtime_error("Failed to get SQE for open");
        }

        io_uring_prep_openat(sqe, AT_FDCWD, filename, flags, mode);
        io_uring_submit(&this->ring);

        io_uring_cqe* cqe;
        int ret = io_uring_wait_cqe(&ring, &cqe);

        if (ret < 0) {
            this->logger->error("wait_cqe failed for open: {}", strerror(errno));
            throw std::runtime_error("wait_cqe failed for open");
        }

        int fd = cqe->res;
        io_uring_cqe_seen(&ring, cqe);

        if (fd < 0) {
            this->logger->error("open failed: {}", strerror(-fd));
            throw std::runtime_error("open failed: " + std::string(strerror(-fd)));
        }

        return fd;
    }
}
#include <engine/posix.h>

namespace Engine {

    PosixEngine::PosixEngine(Metric::MetricType _metric_type, std::unique_ptr<Logger::Logger> _logger)
        : Engine(_metric_type, std::move(_logger)) {}

    PosixEngine::~PosixEngine() {
        std::cout << "~Destroying PosixEngine" << std::endl;
    }

    int PosixEngine::open(Protocol::OpenRequest& request) {
        int fd = ::open(request.filename.c_str(), request.flags, request.mode);
        if (fd < 0) {
            throw std::runtime_error("Failed to open file: " + std::string(strerror(errno)));
        }
        return fd;
    }

    int PosixEngine::close(Protocol::CloseRequest& request) {
        int rt = ::close(request.fd);
        if (rt < 0) {
            throw std::runtime_error("Failed to close fd: " + std::string(strerror(errno)));
        }
        return rt;
    }

    int PosixEngine::nop(Protocol::CommonRequest& request) {
        (void) request;
        return 0;
    }

    int PosixEngine::fsync(Protocol::CommonRequest& request) {
        return ::fsync(request.fd);
    }

    int PosixEngine::fdatasync(Protocol::CommonRequest& request) {
        return ::fdatasync(request.fd);
    }

    ssize_t PosixEngine::read(Protocol::CommonRequest& request) {
        return ::pread(request.fd, request.buffer, request.size, request.offset);
    }

    ssize_t PosixEngine::write(Protocol::CommonRequest& request) {
        return ::pwrite(request.fd, request.buffer, request.size, request.offset);
    }

    void PosixEngine::submit(Protocol::CommonRequest& request) {
        ssize_t result = 0;
        int64_t start_timestamp = Metric::get_current_timestamp();

        switch (request.operation) {
            case Operation::OperationType::READ:
                result = this->read(request);
                break;
            case Operation::OperationType::WRITE:
                result = this->write(request);
                break;
            case Operation::OperationType::FSYNC:
                result = this->fsync(request);
                break;
            case Operation::OperationType::FDATASYNC:
                result = this->fdatasync(request);
                break;
            case Operation::OperationType::NOP:
                result = this->nop(request);
                break;
            default:
                throw std::invalid_argument("Unsupported operation type by PosixEngine");
        }

        Metric::fill_metric(
            Engine::metric_type,
            *Engine::metric,
            request.operation,
            start_timestamp,
            Metric::get_current_timestamp(),
            result,
            request.size,
            request.offset
        );

        Engine::logger->info(*Engine::metric);
    }
}

#ifndef POSIX_ENGINE_H
#define POSIX_ENGINE_H

#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <io/metric.h>
#include <io/protocol.h>
#include <engine/utils.h>
#include <operation/type.h>

namespace Engine {

    class PosixEngine {
        private:
            inline ssize_t read(Protocol::CommonRequest& request);
            inline ssize_t write(Protocol::CommonRequest& request);

            inline int nop(Protocol::CommonRequest& request);
            inline int fsync(Protocol::CommonRequest& request);
            inline int fdatasync(Protocol::CommonRequest& request);

        public:
            PosixEngine() = default;

            inline int open(Protocol::OpenRequest& request);
            inline int close(Protocol::CloseRequest& request);

            template<typename MetricT>
            inline void submit(Protocol::CommonRequest& request, std::vector<MetricT>& metrics);
    };

    inline int PosixEngine::open(Protocol::OpenRequest& request) {
        int fd = ::open(request.filename.c_str(), request.flags, request.mode);
        if (fd < 0) {
            throw std::runtime_error("Failed to open file: " + std::string(strerror(errno)));
        }
        return fd;
    }

    inline int PosixEngine::close(Protocol::CloseRequest& request) {
        int rt = ::close(request.fd);
        if (rt < 0) {
            throw std::runtime_error("Failed to close fd: " + std::string(strerror(errno)));
        }
        return rt;
    }

    inline int PosixEngine::nop(Protocol::CommonRequest& request) {
        (void) request;
        return 0;
    }

    inline int PosixEngine::fsync(Protocol::CommonRequest& request) {
        return ::fsync(request.fd);
    }

    inline int PosixEngine::fdatasync(Protocol::CommonRequest& request) {
        return ::fdatasync(request.fd);
    }

    inline ssize_t PosixEngine::read(Protocol::CommonRequest& request) {
        return ::pread(request.fd, request.buffer, request.size, request.offset);
    }

    inline ssize_t PosixEngine::write(Protocol::CommonRequest& request) {
        return ::pwrite(request.fd, request.buffer, request.size, request.offset);
    }

    template<typename MetricT>
    inline void PosixEngine::submit(Protocol::CommonRequest& request, std::vector<MetricT>& metrics) {
        MetricT metric{};
        ssize_t result = 0;
        Metric::start_base_metric<MetricT>(metric, request.operation);

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
                throw std::invalid_argument("Unsupported operation type");
        }

        Metric::fill_standard_metric<MetricT>(metric);
        Metric::fill_full_metric<MetricT>(metric, result, request.size, request.offset);
        Metric::save_on_complete<MetricT>(metrics, metric);
    }
};

#endif
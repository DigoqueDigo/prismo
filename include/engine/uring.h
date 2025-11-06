#ifndef URING_ENGINE_H
#define URING_ENGINE_H

#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <stdexcept>
#include <io/metric.h>
#include <io/protocol.h>
#include <engine/utils.h>
#include <iostream>

namespace Engine {

    class UringEngine {
        private:
            io_uring ring;
            std::vector<iovec> iovecs;
            std::vector<UringUserData> user_data;
            std::vector<uint32_t> available_indexs;
            std::vector<io_uring_cqe*> completed_cqes;

            inline void nop(Protocol::CommonRequest& request, io_uring_sqe* sqe);
            inline void fsync(Protocol::CommonRequest& request, io_uring_sqe* sqe);
            inline void fdatasync(Protocol::CommonRequest& request, io_uring_sqe* sqe);

            inline void read(Protocol::CommonRequest& request, io_uring_sqe* sqe, uint32_t free_index);
            inline void write(Protocol::CommonRequest& request, io_uring_sqe* sqe, uint32_t free_index);

            template<typename MetricT>
            inline void reap_completions(std::vector<MetricT>& metrics);

        public:
            inline explicit UringEngine(const UringConfig& _config);
            inline ~UringEngine();

            inline int open(Protocol::OpenRequest& request);
            inline void close(Protocol::CloseRequest& request);

            template<typename MetricT>
            inline void reap_left_completions(std::vector<MetricT>& metrics);

            template<typename MetricT>
            inline void submit(Protocol::CommonRequest& request, std::vector<MetricT>& metrics);
    };

    inline UringEngine::UringEngine(const UringConfig& _config)
        : ring(), iovecs(), user_data(), available_indexs(), completed_cqes() {

            UringConfig config = _config;
            int return_code = io_uring_queue_init_params(config.entries, &ring, &config.params);
            if (return_code) {
                throw std::runtime_error("Uring initialization failed: " + std::string(strerror(return_code)));
            }

            iovecs.resize(config.params.sq_entries);
            user_data.resize(config.params.sq_entries);
            available_indexs.resize(config.params.sq_entries);
            completed_cqes.resize(config.params.cq_entries);

            for (uint32_t index = 0; index < config.params.sq_entries; index++) {
                available_indexs[index] = config.params.sq_entries - index - 1;
                iovecs[index].iov_len = config.block_size;
                iovecs[index].iov_base = std::malloc(config.block_size);
                if (iovecs[index].iov_base == nullptr) {
                    throw std::bad_alloc();
                }
            }

            return_code = io_uring_register_buffers(&ring, iovecs.data(), config.params.sq_entries);
            if (return_code) {
                io_uring_queue_exit(&ring);
                throw std::runtime_error("Uring register buffers failed: " + std::string(strerror(errno)));
            }
        }

    inline UringEngine::~UringEngine() {
        if (io_uring_unregister_buffers(&ring)) {
            std::cerr << "Uring unregister buffers failed: " << strerror(errno) << std::endl;
        }
        for (auto& iv : iovecs) {
            std::free(iv.iov_base);
        }
        io_uring_queue_exit(&ring);
    }

    inline int UringEngine::open(Protocol::OpenRequest& request) {
        int fd = ::open(request.filename.c_str(), request.flags, request.mode);
        if (fd < 0) {
            throw std::runtime_error("Failed to open file: " + std::string(strerror(errno)));
        }
        int return_code = io_uring_register_files(&ring, &fd, 1);
        if (return_code) {
            throw std::runtime_error("Uring register file failed: " + std::string(strerror(return_code)));
        }
        return fd;
    }

    inline void UringEngine::close(Protocol::CloseRequest& request) {
        if (io_uring_unregister_files(&ring)) {
            throw std::runtime_error("Uring unregister files failed: " + std::string(strerror(errno)));
        }
        if (::close(request.fd) < 0) {
            throw std::runtime_error("Failed to close fd: " + std::string(strerror(errno)));
        }
    }

    inline void UringEngine::nop(Protocol::CommonRequest& request, io_uring_sqe* sqe) {
        (void) request;
        io_uring_prep_nop(sqe);
    }

    inline void UringEngine::fsync(Protocol::CommonRequest& request, io_uring_sqe* sqe) {
        io_uring_prep_fsync(sqe, request.fd, 0);
    }

    inline void UringEngine::fdatasync(Protocol::CommonRequest& request, io_uring_sqe* sqe) {
        io_uring_prep_fsync(sqe, request.fd, IORING_FSYNC_DATASYNC);
    }

    inline void UringEngine::read(Protocol::CommonRequest& request, io_uring_sqe* sqe, uint32_t free_index) {
        io_uring_prep_read_fixed(sqe, request.fd, iovecs[free_index].iov_base, request.size, request.offset, free_index);
    }

    inline void UringEngine::write(Protocol::CommonRequest& request, io_uring_sqe* sqe, uint32_t free_index) {
        std::memcpy(iovecs[free_index].iov_base, request.buffer, request.size);
        io_uring_prep_write_fixed(sqe, request.fd, iovecs[free_index].iov_base, request.size, request.offset, free_index);
    }

    template<typename MetricT>
    inline void UringEngine::submit(Protocol::CommonRequest& request, std::vector<MetricT>& metrics) {
        uint32_t free_index;
        io_uring_sqe* sqe = io_uring_get_sqe(&ring);
        request.fd = 0;

        if (sqe == nullptr) {
            int submitted = io_uring_submit(&ring);
            std::cout << "Submitted " << submitted << " entries to uring." << std::endl;
        }

        if (available_indexs.empty()) {
            this->template reap_completions<MetricT>(metrics);
        }

        while (sqe == nullptr) {
            sqe = io_uring_get_sqe(&ring);
        }

        free_index = available_indexs.back();
        available_indexs.pop_back();

        user_data[free_index].index = free_index;
        user_data[free_index].size = request.size;
        user_data[free_index].offset = request.offset;
        user_data[free_index].operation_type = request.operation;
        user_data[free_index].start_timestamp =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count();

        switch (request.operation) {
            case Operation::OperationType::READ:
                this->read(request, sqe, free_index);
                break;
            case Operation::OperationType::WRITE:
                this->write(request, sqe, free_index);
                break;
            case Operation::OperationType::FSYNC:
                this->fsync(request, sqe);
                break;
            case Operation::OperationType::FDATASYNC:
                this->fdatasync(request, sqe);
                break;
            case Operation::OperationType::NOP:
                this->nop(request, sqe);
                break;
            default:
                throw std::invalid_argument("Unsupported operation type for UringEngine");
        }

        io_uring_sqe_set_data(sqe, &user_data[free_index]);
        sqe->flags |= IOSQE_FIXED_FILE;
    }

    template<typename MetricT>
    inline void UringEngine::reap_completions(std::vector<MetricT>& metrics) {
        int completions;

        do {
            completions = io_uring_peek_batch_cqe(
                &ring,
                completed_cqes.data(),
                completed_cqes.capacity()
            );
        } while (!completions);

        for (int index = 0; index < completions; index++) {
            MetricT metric{};
            io_uring_cqe* cqe = completed_cqes[index];
            UringUserData* cqe_user_data = static_cast<UringUserData*>(io_uring_cqe_get_data(cqe));

            Metric::end_base_metric<MetricT>(
                metric,
                cqe_user_data->operation_type,
                cqe_user_data->start_timestamp
            );

            Metric::fill_standard_metric<MetricT>(metric);
            Metric::fill_full_metric<MetricT>(
                metric,
                cqe->res,
                cqe_user_data->size,
                cqe_user_data->offset
            );

            Metric::save_on_complete<MetricT>(metrics, metric);

            io_uring_cqe_seen(&ring, cqe);
            available_indexs.push_back(cqe_user_data->index);
        }
    }

    template<typename MetricT>
    inline void UringEngine::reap_left_completions(std::vector<MetricT>& metrics) {
        int submitted = io_uring_submit(&ring);
        std::cout << "Final Submitted " << submitted << " entries to uring." << std::endl;
        while (available_indexs.size() < available_indexs.capacity()) {
            this->template reap_completions<MetricT>(metrics);
        }
    }
};

#endif

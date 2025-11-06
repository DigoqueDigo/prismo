#ifndef AIO_ENGINE_H
#define AIO_ENGINE_H

#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <io/metric.h>
#include <io/protocol.h>
#include <operation/type.h>
#include <engine/utils.h>
#include <iostream>

namespace Engine {

    class AioEngine {
        private:
            io_context_t io_context;
            std::vector<iocb> iocbs;
            std::vector<iocb*> iocb_ptrs;
            std::vector<io_event> io_events;

            std::vector<AioTask> tasks;
            std::vector<uint32_t> available_indexs;

            inline void nop(Protocol::CommonRequest& request, uint32_t free_index);
            inline void fsync(Protocol::CommonRequest& request, uint32_t free_index);
            inline void fdatasync(Protocol::CommonRequest& request, uint32_t free_index);

            inline void read(Protocol::CommonRequest& request, uint32_t free_index);
            inline void write(Protocol::CommonRequest& request, uint32_t free_index);

            template<typename MetricT>
            inline void reap_completions(std::vector<MetricT>& metrics);

        public:
            inline explicit AioEngine(const AioConfig& _config);
            inline ~AioEngine();

            inline int open(Protocol::OpenRequest& request);
            inline int close(Protocol::CloseRequest& request);

            template<typename MetricT>
            inline void reap_left_completions(std::vector<MetricT>& metrics);

            template<typename MetricT>
            inline void submit(Protocol::CommonRequest& request, std::vector<MetricT>& metrics);
    };

    inline AioEngine::AioEngine(const AioConfig& _config)
        : io_context(0), iocbs(), iocb_ptrs(), io_events(), tasks(), available_indexs() {

            int return_code = io_queue_init(_config.entries, &io_context);
            if (return_code < 0) {
                throw std::runtime_error("Aio queue init failed: " + std::string(strerror(-return_code)));
            }

            iocbs.resize(_config.entries);
            tasks.resize(_config.entries);
            io_events.resize(_config.entries);
            iocb_ptrs.reserve(_config.entries);
            available_indexs.resize(_config.entries);

            for (uint32_t index = 0; index < _config.entries; index++) {
                available_indexs[index] = _config.entries - index - 1;
                tasks[index].buffer = std::malloc(_config.block_size);
                if (tasks[index].buffer == nullptr) {
                    throw std::bad_alloc();
                }
            }
    }

    inline AioEngine::~AioEngine() {
        for (auto& task : tasks) {
            std::free(task.buffer);
        }
        if (io_queue_release(io_context)) {
            std::cerr << "Aio destroy failed: " << strerror(errno) << std::endl;
        };
    }

    inline int AioEngine::open(Protocol::OpenRequest& request) {
        int fd = ::open(request.filename.c_str(), request.flags, request.mode);
        if (fd < 0) {
            throw std::runtime_error("Failed to open file: " + std::string(strerror(errno)));
        }
        return fd;
    }

    inline int AioEngine::close(Protocol::CloseRequest& request) {
        int return_code = ::close(request.fd);
        if (return_code < 0) {
            throw std::runtime_error("Failed to close fd: " + std::string(strerror(errno)));
        }
        return return_code;
    }

    inline void AioEngine::nop(Protocol::CommonRequest& request, uint32_t free_index) {
        io_prep_pwrite(&iocbs[free_index], request.fd, nullptr, 0, 0);
    }

    inline void AioEngine::fsync(Protocol::CommonRequest& request, uint32_t free_index) {
        io_prep_fsync(&iocbs[free_index], request.fd);
    }

    inline void AioEngine::fdatasync(Protocol::CommonRequest& request, uint32_t free_index) {
        io_prep_fdsync(&iocbs[free_index], request.fd);
    }

    inline void AioEngine::read(Protocol::CommonRequest& request, uint32_t free_index) {
        io_prep_pread(&iocbs[free_index], request.fd, tasks[free_index].buffer, request.size, request.offset);
    }

    inline void AioEngine::write(Protocol::CommonRequest& request, uint32_t free_index) {
        std::memcpy(tasks[free_index].buffer, request.buffer, request.size);
        io_prep_pwrite(&iocbs[free_index], request.fd, tasks[free_index].buffer, request.size, request.offset);
    }

    template<typename MetricT>
    inline void AioEngine::submit(Protocol::CommonRequest& request, std::vector<MetricT>& metrics) {
        if (iocb_ptrs.size() == iocb_ptrs.capacity()) {
            int submit_result = io_submit(io_context, iocb_ptrs.size(), &iocb_ptrs[0]);
            if (submit_result != static_cast<int>(iocb_ptrs.size())) {
                throw std::runtime_error("Aio submission failed");
            }
            iocb_ptrs.clear();
        }

        while (available_indexs.empty()) {
            this->template reap_completions(metrics);
        }

        uint32_t free_index = available_indexs.back();
        available_indexs.pop_back();

        tasks[free_index].index = free_index;
        tasks[free_index].size = request.size;
        tasks[free_index].offset = request.offset;
        tasks[free_index].operation_type = request.operation;
        tasks[free_index].start_timestamp =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count();

        switch (request.operation) {
            case Operation::OperationType::READ:
                this->read(request, free_index);
                break;
            case Operation::OperationType::WRITE:
                this->write(request, free_index);
                break;
            case Operation::OperationType::FSYNC:
                this->fsync(request, free_index);
                break;
            case Operation::OperationType::FDATASYNC:
                this->fdatasync(request, free_index);
                break;
            case Operation::OperationType::NOP:
                this->nop(request, free_index);
                break;
            default:
                throw std::invalid_argument("Unsupported operation type for AioEngine");
        }

        iocbs[free_index].data = &tasks[free_index];
        iocb_ptrs.push_back(&iocbs[free_index]);
    }

    template<typename MetricT>
    inline void AioEngine::reap_completions(std::vector<MetricT>& metrics) {
        AioTask* completed_task;
        io_event completed_event;
        int events_returned = io_getevents(io_context, 1, io_events.capacity(), io_events.data(), nullptr);

        if (events_returned < 0) {
            throw std::runtime_error("Aio getevents failed: " + std::string(strerror(-events_returned)));
        }

        for (int event_index = 0; event_index < events_returned; event_index++) {
            MetricT metric{};
            completed_event = io_events[event_index];
            completed_task = static_cast<AioTask*>(completed_event.data);

            Metric::end_base_metric<MetricT>(
                metric,
                completed_task->operation_type,
                completed_task->start_timestamp
            );

            Metric::fill_standard_metric<MetricT>(metric);
            Metric::fill_full_metric<MetricT>(
                metric,
                completed_event.res,
                completed_task->size,
                completed_task->offset
            );

            Metric::save_on_complete<MetricT>(metrics, metric);

            available_indexs.push_back(completed_task->index);
        }
    }

    template<typename MetricT>
    inline void AioEngine::reap_left_completions(std::vector<MetricT>& metrics) {
        int submit_result = io_submit(io_context, iocb_ptrs.size(), &iocb_ptrs[0]);
        if (submit_result != static_cast<int>(iocb_ptrs.size())) {
            throw std::runtime_error("Flush Invalid submission");
        }
        iocb_ptrs.clear();

        while (available_indexs.size() < available_indexs.capacity()) {
            this->template reap_completions(metrics);
        }
    }
};


#endif
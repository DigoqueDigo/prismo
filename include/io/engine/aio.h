#ifndef AIO_ENGINE_H
#define AIO_ENGINE_H

#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <io/metric.h>
#include <operation/type.h>
#include <io/engine/config.h>
#include <iostream>

namespace Engine {

    struct AioTask {
        void* buffer;
        size_t size;
        off_t offset;
        uint32_t index;
        int64_t start_timestamp;
        Operation::OperationType operation_type;
    };

    struct AioEngine {
        private:
            io_context_t io_context;
            std::vector<iocb> iocbs;
            std::vector<iocb*> iocb_ptrs;
            std::vector<io_event> io_events;

            std::vector<AioTask> tasks;
            std::vector<uint32_t> available_indexs;

            inline void nop(int fd, uint32_t free_index);
            inline void fsync(int fd, uint32_t free_index);
            inline void fdatasync(int fd, uint32_t free_index);

            inline void read(int fd, void* buffer, size_t size, off_t offset, uint32_t free_index);
            inline void write(int fd, const void* buffer, size_t size, off_t offset, uint32_t free_index);

            template<typename MetricT>
            inline void reap_completions(std::vector<MetricT>& metrics);

        public:
            inline explicit AioEngine(const AioConfig& _config);
            inline ~AioEngine();

            inline int open(const char* filename, OpenFlags flags, OpenMode mode);
            inline int close(int fd);

            template<typename MetricT>
            inline void reap_left_completions(std::vector<MetricT>& metrics);

            template<typename MetricT, Operation::OperationType OperationT>
            inline void submit(int fd, void* buffer, size_t size, off_t offset, std::vector<MetricT>& metrics);
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

    inline int AioEngine::open(const char* filename, OpenFlags flags, OpenMode mode) {
        int fd = ::open(filename, flags.value, mode.value);
        if (fd < 0) {
            throw std::runtime_error("Failed to open file: " + std::string(strerror(errno)));
        }
        return fd;
    }

    inline int AioEngine::close(int fd) {
        int return_code = ::close(fd);
        if (return_code < 0) {
            throw std::runtime_error("Failed to close fd: " + std::string(strerror(errno)));
        }
        return return_code;
    }

    inline void AioEngine::nop(int fd, uint32_t free_index) {
        (void) fd;
        // std::memset(&iocbs[free_index], 0, sizeof(iocb));
        // iocbs[free_index].aio_fildes = -1;
        // iocbs[free_index].aio_lio_opcode = io_iocb_cmd::IO_CMD_NOOP;
        io_prep_pwrite(&iocbs[free_index], fd, nullptr, 0, 0);
    }

    inline void AioEngine::fsync(int fd, uint32_t free_index) {
        io_prep_fsync(&iocbs[free_index], fd);
    }

    inline void AioEngine::fdatasync(int fd, uint32_t free_index) {
        io_prep_fdsync(&iocbs[free_index], fd);
    }

    inline void AioEngine::read(int fd, void* buffer, size_t size, off_t offset, uint32_t free_index) {
        (void) buffer;
        io_prep_pread(&iocbs[free_index], fd, tasks[free_index].buffer, size, offset);
    }

    inline void AioEngine::write(int fd, const void* buffer, size_t size, off_t offset, uint32_t free_index) {
        std::memcpy(tasks[free_index].buffer, buffer, size);
        io_prep_pwrite(&iocbs[free_index], fd, tasks[free_index].buffer, size, offset);
    }

    template<typename MetricT, Operation::OperationType OperationT>
    inline void AioEngine::submit(int fd, void* buffer, size_t size, off_t offset, std::vector<MetricT>& metrics) {

        uint32_t free_index;

        if (iocb_ptrs.size() == iocb_ptrs.capacity()) {
            int submit_result = io_submit(io_context, iocb_ptrs.size(), &iocb_ptrs[0]);
            // std::cout << "Submitted: " << submit_result << std::endl;
            if (submit_result != static_cast<int>(iocb_ptrs.size())) {
                throw std::runtime_error("Aio submission failed");
            }
            iocb_ptrs.clear();
        }

        while (available_indexs.empty()) {
            this->template reap_completions(metrics);
        }

        free_index = available_indexs.back();
        available_indexs.pop_back();

        tasks[free_index].size = size;
        tasks[free_index].offset = offset;
        tasks[free_index].index = free_index;
        tasks[free_index].operation_type = OperationT;
        tasks[free_index].start_timestamp =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count();

        if constexpr (OperationT == Operation::OperationType::READ) {
            read(fd, buffer, size, offset, free_index);
        } else if constexpr (OperationT == Operation::OperationType::WRITE) {
            write(fd, buffer, size, offset, free_index);
        } else if constexpr (OperationT == Operation::OperationType::FSYNC) {
            fsync(fd, free_index);
        } else if constexpr (OperationT == Operation::OperationType::FDATASYNC) {
            fdatasync(fd, free_index);
        } else if constexpr (OperationT == Operation::OperationType::NOP) {
            nop(fd, free_index);
        }

        iocbs[free_index].data = &tasks[free_index];
        iocb_ptrs.push_back(&iocbs[free_index]);
    }

    template<typename MetricT>
    inline void AioEngine::reap_completions(std::vector<MetricT>& metrics) {
        AioTask* completed_task;
        io_event completed_event;

        int events_returned = io_getevents(io_context, 1, io_events.capacity(), io_events.data(), nullptr);
        // std::cout << "Returned events: " << events_returned << std::endl;

        if (events_returned < 0) {
            throw std::runtime_error("Aio getevents failed: " + std::string(strerror(-events_returned)));
        }

        for (int event_index = 0; event_index < events_returned; event_index++) {
            MetricT metric{};
            completed_event = io_events[event_index];
            completed_task = static_cast<AioTask*>(completed_event.data);

            // std::cout << "free index: " << completed_task->index << std::endl;
            // std::cout << "Offset: " << completed_task->offset << " returned: " << completed_event.res << std::endl;

            if constexpr (std::is_base_of_v<Metric::BaseMetric, MetricT>) {
                metric.start_timestamp = completed_task->start_timestamp;
                metric.operation_type = completed_task->operation_type;
                metric.end_timestamp =
                    std::chrono::duration_cast<std::chrono::nanoseconds>(
                        std::chrono::steady_clock::now().time_since_epoch()
                    ).count();
            }

            if constexpr (std::is_base_of_v<Metric::StandardMetric, MetricT>) {
                metric.pid = ::getpid();
                metric.tid = static_cast<uint64_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
            }

            if constexpr (std::is_base_of_v<Metric::FullMetric, MetricT>) {
                metric.requested_bytes = completed_task->size;
                metric.processed_bytes = (completed_event.res > 0) ? static_cast<size_t>(completed_event.res) : 0;
                metric.offset          = completed_task->offset;
                metric.return_code     = static_cast<int32_t>(completed_event.res);
                metric.error_no        = static_cast<int32_t>(errno);
            }

            if constexpr (std::is_base_of_v<Metric::BaseMetric, MetricT>) {
                metrics.push_back(std::move(metric));
            }

            available_indexs.push_back(completed_task->index);
        }
    }

    template<typename MetricT>
    inline void AioEngine::reap_left_completions(std::vector<MetricT>& metrics) {
        int submit_result = io_submit(io_context, iocb_ptrs.size(), &iocb_ptrs[0]);
        // std::cout << "Flush Submitted: " << submit_result << std::endl;
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
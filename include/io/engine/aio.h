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

namespace Engine {

    struct AIOTask {
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
            std::vector<AIOTask> tasks;

            inline void read(int fd, void* buffer, size_t size, off_t offset);
            inline void write(int fd, const void* buffer, size_t size, off_t offset);

        public:
            inline explicit AioEngine(const AIOConfig& _config);
            inline ~AioEngine();

            inline int open(const char* filename, OpenFlags flags, OpenMode mode);
            inline int close(int fd);

            template<typename MetricT, Operation::OperationType OperationT>
            inline std::optional<MetricT> submit(int fd, void* buffer, size_t size, off_t offset);
    };

    inline AioEngine::AioEngine(const AIOConfig& _config)
        : io_context(0), iocbs(_config.entries), iocb_ptrs(_config.entries), io_events(_config.entries), tasks(_config.entries) {

            int return_code = io_setup(_config.entries, &io_context);
            if (return_code < 0) {
                throw std::runtime_error("aio_setup failed: " + std::string(strerror(-return_code)));
            }

            for (uint32_t index = 0; index < _config.entries; index++) {
                iocb_ptrs[index] = &iocbs[index];
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
        io_destroy(io_context);
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

    inline void AioEngine::read(int fd, void* buffer, size_t size, off_t offset) {
        io_prep_pread(&iocbs[0], fd, buffer, size, offset);
    }

    inline void AioEngine::write(int fd, const void* buffer, size_t size, off_t offset) {
        io_prep_pwrite(&iocbs[0], fd, const_cast<void*>(buffer), size, offset);
    }

    template<typename MetricT, Operation::OperationType OperationT>
    inline std::optional<MetricT> AioEngine::submit(int fd, void* buffer, size_t size, off_t offset) {

        std::optional<MetricT> op_metric = std::nullopt;




        if constexpr (OperationT == Operation::OperationType::READ) {
            read(fd, buffer, size, offset);
        } else if constexpr (OperationT == Operation::OperationType::WRITE) {
            write(fd, buffer, size, offset);
        }



        return op_metric;
    }
};


#endif
#ifndef AIO_ENGINE_H
#define AIO_ENGINE_H

#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <engine/engine.h>
#include <engine/utils.h>
#include <operation/type.h>

namespace Engine {

    class AioEngine : public Engine {
        private:
            io_context_t io_context;
            std::vector<iocb> iocbs;
            std::vector<iocb*> iocb_ptrs;
            std::vector<io_event> io_events;

            std::vector<AioTask> tasks;
            std::vector<uint32_t> available_indexes;

            void nop(Protocol::CommonRequest& request, uint32_t free_index);
            void fsync(Protocol::CommonRequest& request, uint32_t free_index);
            void fdatasync(Protocol::CommonRequest& request, uint32_t free_index);

            void read(Protocol::CommonRequest& request, uint32_t free_index);
            void write(Protocol::CommonRequest& request, uint32_t free_index);

            void reap_completions(void);

        public:
            AioEngine(
                Metric::MetricType _metric_type,
                std::unique_ptr<Logger::Logger> _logger,
                const AioConfig& _config
            );

            ~AioEngine() override;

            int open(Protocol::OpenRequest& request) override;
            int close(Protocol::CloseRequest& request) override;
            void submit(Protocol::CommonRequest& request) override;
            void reap_left_completions(void) override;
    };
}

#endif
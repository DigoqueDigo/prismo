#ifndef POSIX_ENGINE_H
#define POSIX_ENGINE_H

#include <iostream>
#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <engine/utils.h>
#include <engine/engine.h>
#include <operation/type.h>

namespace Engine {

    class PosixEngine : public Engine {
        private:
            int nop(Protocol::CommonRequest& request);
            int fsync(Protocol::CommonRequest& request);
            int fdatasync(Protocol::CommonRequest& request);

            ssize_t read(Protocol::CommonRequest& request);
            ssize_t write(Protocol::CommonRequest& request);

        public:
            explicit PosixEngine(
                Metric::MetricType _metric_type,
                std::unique_ptr<Logger::Logger> _logger
            );

            ~PosixEngine();

            int open(Protocol::OpenRequest& request) override;
            int close(Protocol::CloseRequest& request) override;
            void submit(Protocol::CommonRequest& request) override;
            void reap_left_completions(void) override {};
    };
}

#endif
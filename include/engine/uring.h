#ifndef URING_ENGINE_H
#define URING_ENGINE_H

#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <engine/utils.h>
#include <engine/engine.h>
#include <liburing.h>

namespace Engine {

    class UringEngine : public Engine {
        private:
            io_uring ring;
            std::vector<iovec> iovecs;
            std::vector<UringUserData> user_data;
            std::vector<uint32_t> available_indexs;
            std::vector<io_uring_cqe*> completed_cqes;

            void nop(io_uring_sqe* sqe);
            void fsync(Protocol::CommonRequest& request, io_uring_sqe* sqe);
            void fdatasync(Protocol::CommonRequest& request, io_uring_sqe* sqe);

            void read(Protocol::CommonRequest& request, io_uring_sqe* sqe, uint32_t free_index);
            void write(Protocol::CommonRequest& request, io_uring_sqe* sqe, uint32_t free_index);

            void reap_completions(void);

        public:
            explicit UringEngine(
                Metric::MetricType _metric_type,
                std::unique_ptr<Logger::Logger> _logger,
                const UringConfig& _config
            );

            ~UringEngine() override;

            int open(Protocol::OpenRequest& request) override;
            int close(Protocol::CloseRequest& request) override;
            void submit(Protocol::CommonRequest& request) override;
            void reap_left_completions(void) override;
    };
};

#endif
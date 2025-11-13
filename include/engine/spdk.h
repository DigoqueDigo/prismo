#ifndef SPDK_ENGINE_H
#define SPDK_ENGINE_H

#include <spdk/env.h>
#include <spdk/bdev.h>
#include <spdk/thread.h>
#include <spdk/event.h>
#include <engine/engine.h>
#include <engine/utils.h>

namespace Engine {

    class SpdkEngine : public Engine {
        private:
            spdk_env_opts env_opts;
            spdk_bdev_desc* desc;

            std::vector<spdk_io_channel*> io_channels;
            std::vector<spdk_thread*> spdk_polling_threads;
            std::vector<std::thread> os_polling_threads;

            std::atomic<bool> running{true};
            std::atomic<size_t> outstanding_io{0};

            int nop(SpdkUserData* spdk_user_data);
            int fsync(Protocol::CommonRequest& request, SpdkUserData* spdk_user_data);
            int fdatasync(Protocol::CommonRequest& request, SpdkUserData* spdk_user_data);

            int read(Protocol::CommonRequest& request, SpdkUserData* spdk_user_data);
            int write(Protocol::CommonRequest& request, SpdkUserData* spdk_user_data);

            static void io_complete(spdk_bdev_io* bdev_io, bool success, void* cb_arg);

        public:
            explicit SpdkEngine(
                Metric::MetricType _metric_type,
                std::unique_ptr<Logger::Logger> _loggger,
                const SpdkConfig& config
            );

            ~SpdkEngine() override;

            int open(Protocol::OpenRequest& request) override { (void) request; return 0; };
            int close(Protocol::CloseRequest& request) override { (void) request; return 0; };
            void submit(Protocol::CommonRequest& request) override;
            void reap_left_completions(void) override;
        };
}

#endif
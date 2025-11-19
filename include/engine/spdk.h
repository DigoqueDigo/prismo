#ifndef SPDK_ENGINE_H
#define SPDK_ENGINE_H

#include <spdk/env.h>
#include <spdk/bdev.h>
#include <spdk/thread.h>
#include <spdk/event.h>
#include <engine/engine.h>
#include <engine/utils.h>
#include <lib/readerwriterqueue/readerwritercircularbuffer.h>

using namespace moodycamel;

namespace Engine {

    struct spdk_context_t {
        spdk_bdev* bdev;
        spdk_bdev_desc* bdev_desc;
        spdk_io_channel* bdev_io_channel;
    };

    class SpdkEngine : public Engine {
        private:
            BlockingReaderWriterCircularBuffer<Protocol::Packet*> internal_queue;
            std::thread spdk_main_thread;

            static void run(void* arg);

            static int nop(SpdkUserData* spdk_user_data);
            static int fsync(Protocol::CommonRequest& request, SpdkUserData* spdk_user_data);
            static int fdatasync(Protocol::CommonRequest& request, SpdkUserData* spdk_user_data);

            static int read(Protocol::CommonRequest& request, SpdkUserData* spdk_user_data);
            static int write(Protocol::CommonRequest& request, SpdkUserData* spdk_user_data);

            static void io_complete(spdk_bdev_io* bdev_io, bool success, void* cb_arg);
            static void hello_bdev_event_cb(enum spdk_bdev_event_type type, struct spdk_bdev *bdev, void *event_ctx);

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
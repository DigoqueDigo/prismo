#ifndef SPDK_ENGINE_H
#define SPDK_ENGINE_H

#include <spdk/env.h>
#include <spdk/bdev.h>
#include <spdk/thread.h>
#include <spdk/event.h>
#include <io/metric.h>
#include <engine/utils.h>
#include <engine/engine.h>
#include <worker/utils.h>

namespace Engine {

    struct spdk_context {
        spdk_bdev* bdev;
        spdk_bdev_desc* bdev_desc;
        char* bdev_name;
        char* buffer;
        size_t buffer_size;
        std::atomic<Protocol::CommonRequest*>& request;
    };

    struct spdk_context_t {
        spdk_bdev* bdev;
        spdk_bdev_desc* bdev_desc;
        spdk_io_channel* bdev_io_channel;
        spdk_bdev_io_wait_entry bdev_io_wait;
        Protocol::CommonRequest* request;
    };

    struct spdk_context_t_cb {
        size_t size;
        off_t offset;
        int64_t start_timestamp;
        Operation::OperationType operation_type;
        void* spdk_engine;
    };

    class SpdkEngine : public Engine {
        private:
            std::thread spdk_main_thread;
            std::atomic<Protocol::CommonRequest*> request;

            static int start_spdk_app(
                const SpdkConfig& config,
                std::atomic<Protocol::CommonRequest*>& request
            );

            static void start(void* ctx);

            static void bdev_event_cb(
                enum spdk_bdev_event_type type,
                struct spdk_bdev* bdev,
                void *event_ctx
            );

            static void thread_fn(void* ctx_t);

            static int thread_read(spdk_context_t* ctx_t, spdk_context_t_cb* ctx_t_cb);
            static int thread_write(spdk_context_t* ctx_t, spdk_context_t_cb* ctx_t_cb);
            static int thread_fsync(spdk_context_t* ctx_t, spdk_context_t_cb* ctx_t_cb);
            static int thread_fdatasync(spdk_context_t* ctx_t, spdk_context_t_cb* ctx_t_cb);
            static int thread_nop(spdk_context_t* ctx_t, spdk_context_t_cb* ctx_t_cb);

            static void io_complete(struct spdk_bdev_io *bdev_io, bool success, void *cb_arg);

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
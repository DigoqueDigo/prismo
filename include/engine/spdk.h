#ifndef SPDK_ENGINE_H
#define SPDK_ENGINE_H

#include <vector>
#include <mutex>
#include <thread>
#include <atomic>

#include <spdk/env.h>
#include <spdk/bdev.h>
#include <spdk/thread.h>
#include <spdk/event.h>

#include <io/metric.h>
#include <engine/utils.h>
#include <engine/engine.h>
#include <lib/blockingconcurrentqueue/blockingconcurrentqueue.h>

namespace Engine {

    struct TriggerData {
        bool has_next;
        bool is_shutdown;
        bool has_completed;
        Protocol::CommonRequest* request;
    };

    struct SpdkAppContext {
        spdk_bdev* bdev;
        spdk_bdev_desc* bdev_desc;

        char* bdev_name;
        void* spdk_engine;
        int spdk_threads;

        std::atomic<TriggerData>* trigger_atomic;
    };

    struct SpdkThreadCallBackContext {
        void* spdk_engine;
        MetricData metric_data;

        int index;
        moodycamel::BlockingConcurrentQueue<int>* available_indexes;
    };

    struct SpdkThreadContext {
        spdk_bdev* bdev;
        spdk_bdev_desc* bdev_desc;
        spdk_io_channel* bdev_io_channel;
        spdk_bdev_io_wait_entry bdev_io_wait;

        std::atomic<bool>* submitted;
        Protocol::CommonRequest* request;
        SpdkThreadCallBackContext* thread_cb_ctx;
    };

    class SpdkEngine : public Engine {
        private:
            std::thread spdk_main_thread;
            std::atomic<TriggerData> trigger_atomic;

            static int start_spdk_app(
                void* spdk_engine,
                const SpdkConfig config,
                std::atomic<TriggerData>* trigger_atomic
            );

            static void bdev_event_cb(
                enum spdk_bdev_event_type type,
                struct spdk_bdev* bdev,
                void* event_ctx
            );

            static void io_complete(
                struct spdk_bdev_io* bdev_io,
                bool success,
                void* cb_arg
            );

            static void spdk_main_fn(void* app_ctx);
            static void thread_fn(void* thread_ctx);
            static void thread_setup(void* thread_ctx);
            static void thread_cleanup(void* thread_ctx);

            static int thread_read(SpdkThreadContext* thread_ctx);
            static int thread_write(SpdkThreadContext* thread_ctx);
            static int thread_fsync(SpdkThreadContext* thread_ctx);
            static int thread_fdatasync(SpdkThreadContext* thread_ctx);
            static int thread_nop(SpdkThreadContext* thread_ctx);

        public:
            explicit SpdkEngine(
                std::unique_ptr<Metric::Metric> _metric,
                std::unique_ptr<Logger::Logger> _logger,
                const SpdkConfig& config
            );

            ~SpdkEngine() override;

            int open(Protocol::OpenRequest& request) override { (void) request; return 0; }
            int close(Protocol::CloseRequest& request) override { (void) request; return 0; }
            void submit(Protocol::CommonRequest& request) override;
            void reap_left_completions(void) override;
        };
}

#endif
#ifndef SPDK_ENGINE_H
#define SPDK_ENGINE_H

#include <vector>
#include <mutex>
#include <spdk/env.h>
#include <spdk/bdev.h>
#include <spdk/thread.h>
#include <spdk/event.h>
#include <io/metric.h>
#include <engine/utils.h>
#include <engine/engine.h>
#include <lib/blockingconcurrentqueue/blockingconcurrentqueue.h>

namespace Engine {

    struct spdk_context_request {
        Protocol::CommonRequest* request;
        bool isShutdown;
    };

    struct spdk_context {
        spdk_bdev* bdev;
        spdk_bdev_desc* bdev_desc;
        char* bdev_name;
        uint8_t* dma_buffer;
        void* spdk_engine;
        std::atomic<spdk_context_request*>* request_trigger;
    };

    struct spdk_context_t_cb {
        size_t size;
        off_t offset;
        int64_t start_timestamp;
        Operation::OperationType operation_type;
        void* spdk_engine;

        int free_index;
        std::mutex* metrics_mutex;
        moodycamel::BlockingConcurrentQueue<int>* available_indexes;
    };

    struct spdk_context_t {
        spdk_bdev* bdev;
        spdk_bdev_desc* bdev_desc;
        spdk_io_channel* bdev_io_channel;
        spdk_bdev_io_wait_entry bdev_io_wait;
        Protocol::CommonRequest* request;

        std::atomic<bool>* submitted;
        spdk_context_t_cb* ctx_t_cb;
    };

    class SpdkEngine : public Engine {
        private:
            std::thread spdk_main_thread;
            spdk_context_request pending_request;
            std::atomic<spdk_context_request*> request_trigger;

            static int start_spdk_app(
                void* spdk_engine,
                const SpdkConfig& config,
                std::atomic<spdk_context_request*>* request_trigger
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
                std::unique_ptr<Metric::Metric> _metric,
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
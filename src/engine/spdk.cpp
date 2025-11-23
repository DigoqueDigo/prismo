#include <engine/spdk.h>

namespace Engine {

    SpdkEngine::SpdkEngine(
        std::unique_ptr<Metric::Metric> _metric,
        std::unique_ptr<Logger::Logger> _logger,
        const SpdkConfig& config
    ) :
        Engine(std::move(_metric), std::move(_logger)),
        pending_request(),
        request_trigger(nullptr)
    {
        spdk_main_thread = std::thread([this, config]() {
            start_spdk_app(this, config, &(this->request_trigger));
        });

        std::cout << "spdk_thread started in constructor" << std::endl;
    }

    SpdkEngine::~SpdkEngine() {
        spdk_main_thread.join();
    }

    int SpdkEngine::start_spdk_app(
        void* spdk_engine,
        const SpdkConfig& config,
        std::atomic<spdk_context_request*>* request_trigger
    ) {
        struct spdk_app_opts opts = {};
        struct spdk_context context = {};

        context.request_trigger = request_trigger;
        context.spdk_engine = spdk_engine;

        std::cout << "before spdk_app_opts_init" << std::endl;

        spdk_app_opts_init(&opts, sizeof(opts));
        opts.name = "spdk_engine_bdev";
        opts.rpc_addr = nullptr;
        opts.json_config_file = config.json_config_file.c_str();

        context.bdev_name = strdup(config.bdev_name.c_str());

        std::cout << opts.name << std::endl;
        std::cout << opts.json_config_file << std::endl;
        std::cout << context.bdev_name << std::endl;

        std::cout << "after spdk_app_opts_init" << std::endl;
        std::cout << "before spdk_app_start" << std::endl;

        int rc = spdk_app_start(&opts, start, &context);
        if (rc) {
            SPDK_ERRLOG("Error starting application\n");
        }

        std::cout << "after spdk_app_start" << std::endl;

        free(context.bdev_name);
        spdk_dma_free(context.dma_buffer);
        spdk_app_fini();
        return rc;
    }

    void SpdkEngine::start(void* ctx) {
        std::cout << "init start\n" << std::endl;
        spdk_context* context = static_cast<spdk_context*>(ctx);
        context->bdev = nullptr;
        context->bdev_desc = nullptr;

        std::cout << "init start" << std::endl;

        SPDK_NOTICELOG("Successfully started the application\n");
        SPDK_NOTICELOG("Opening the bdev %s\n", context->bdev_name);

        std::cout << "before spdk_bdev_open_ext" << std::endl;

        int rc = spdk_bdev_open_ext(
            context->bdev_name,
            true,
            bdev_event_cb,
            nullptr,
            &context->bdev_desc
        );

        std::cout << "after spdk_bdev_open_ext" << std::endl;

        if (rc) {
            SPDK_ERRLOG("Could not open bdev: %s\n", context->bdev_name);
            spdk_app_stop(-2);
            return;
        }

        context->bdev = spdk_bdev_desc_get_bdev(context->bdev_desc);

        std::atomic<bool> submitted;
        std::mutex metrics_mutex;

        std::vector<uint8_t*> dma_zones;
        std::vector<spdk_thread*> spdk_threads;
        std::vector<spdk_context_t> spdk_context_ts;
        std::vector<spdk_context_t_cb> spdk_context_t_cbs;

        int num_blocks = spdk_bdev_get_num_blocks(context->bdev);
        moodycamel::BlockingConcurrentQueue<int> available_indexes(num_blocks);

        spdk_thread* main_thread = spdk_get_thread();
        int total_threads = 1;

        for (int index = 0; index < total_threads; index++) {
            SPDK_NOTICELOG("Creating spdk thread\n");
            std::string thread_name = "spdk_thread_" + std::to_string(index);
            spdk_thread* thread = spdk_thread_create(thread_name.c_str(), nullptr);

            if (!thread) {
                SPDK_ERRLOG("Error while creating spdk thread\n");
                return;
            }

            spdk_set_thread(thread);

            SPDK_NOTICELOG("Opening io channel\n");
            spdk_io_channel* io_channel = spdk_bdev_get_io_channel(context->bdev_desc);
            spdk_context_t context_t = {};

            context_t.bdev = context->bdev;
            context_t.bdev_desc = context->bdev_desc;
            context_t.bdev_io_channel = io_channel;
            context_t.submitted = &submitted;

            spdk_set_thread(main_thread);

            if (!context_t.bdev_io_channel) {
                SPDK_ERRLOG("Could not create bdev I/O channel\n");
                spdk_bdev_close(context->bdev_desc);
                spdk_app_stop(-1);
                return;
            }

            spdk_context_ts.push_back(context_t);
        }

        size_t buf_align = spdk_bdev_get_buf_align(context->bdev);
        size_t buf_size =
            spdk_bdev_get_block_size(context->bdev) *
            spdk_bdev_get_write_unit_size(context->bdev) *
            spdk_bdev_get_num_blocks(context->bdev);

        context->dma_buffer = (uint8_t*) spdk_dma_zmalloc(buf_size, buf_align, nullptr);

        if (!context->dma_buffer) {
            SPDK_ERRLOG("Failed to allocate buffer\n");
            for (int index = 0; index < total_threads; index++) {
                spdk_thread* thread = spdk_threads[index];
                spdk_set_thread(thread);
                spdk_put_io_channel(spdk_context_ts[index].bdev_io_channel);
                spdk_thread_exit(thread);
            }
            spdk_bdev_close(context->bdev_desc);
            spdk_app_stop(-1);
            return;
        }

        for (int index = 0; index < num_blocks; index++) {
            spdk_context_t_cb context_t_cb = {};
            context_t_cb.free_index = 0;
            context_t_cb.metrics_mutex = &metrics_mutex;
            context_t_cb.spdk_engine = context->spdk_engine;
            context_t_cb.available_indexes = &available_indexes;

            available_indexes.enqueue(index);
            spdk_context_t_cbs.push_back(context_t_cb);
            dma_zones.push_back(context->dma_buffer + index);
        }

        for (int thread_index = 0; true; thread_index = (thread_index + 1) % total_threads) {
            context->request_trigger->wait(nullptr);

            if (context->request_trigger->load()->isShutdown) {
                break;
            }

            int free_index = 0;
            available_indexes.wait_dequeue(free_index);

            spdk_context_t& ctx = spdk_context_ts[thread_index];
            ctx.ctx_t_cb = &spdk_context_t_cbs[free_index];

            if (ctx.request->operation == Operation::OperationType::WRITE) {
                std::memcpy(dma_zones[free_index], ctx.request->buffer, ctx.request->size);
            }

            ctx.request->buffer = dma_zones[free_index];
            ctx.ctx_t_cb->free_index = free_index;
            ctx.submitted->store(false, std::memory_order_release);
            spdk_thread_send_msg(spdk_threads[thread_index], thread_fn, &ctx);

            ctx.submitted->wait(false);
            context->request_trigger->store(nullptr, std::memory_order_release);
            context->request_trigger->notify_one();
        }

        for (int index = 0; index < total_threads; index++) {
            spdk_thread* thread = spdk_threads[index];
            spdk_set_thread(thread);
            spdk_put_io_channel(spdk_context_ts[index].bdev_io_channel);
            spdk_thread_exit(thread);
        }

        spdk_bdev_close(context->bdev_desc);
        spdk_app_stop(0);

        context->request_trigger->store(nullptr, std::memory_order_release);
        context->request_trigger->notify_one();
    }

    void SpdkEngine::thread_fn(void *ctx_t) {
        int rc = 0;
        spdk_context_t* context_t = static_cast<spdk_context_t*>(ctx_t);
        spdk_context_t_cb* context_t_cb = context_t->ctx_t_cb;
        Protocol::CommonRequest* request = context_t->request;

        context_t_cb->size = request->size;
        context_t_cb->offset = request->offset;
        context_t_cb->start_timestamp = Metric::get_current_timestamp();
        context_t_cb->operation_type = request->operation;

        switch (context_t_cb->operation_type) {
            case Operation::OperationType::READ:
                rc = thread_read(context_t, context_t_cb);
                break;
            case Operation::OperationType::WRITE:
                rc = thread_write(context_t, context_t_cb);
                break;
            case Operation::OperationType::FSYNC:
                rc = thread_fsync(context_t, context_t_cb);
                break;
            case Operation::OperationType::FDATASYNC:
                rc = thread_fdatasync(context_t, context_t_cb);
                break;
            case Operation::OperationType::NOP:
                rc = thread_nop(context_t, context_t_cb);
                break;
        }

        if (rc == -ENOMEM) {
            SPDK_NOTICELOG("Queueing io\n");
            /* In case we cannot perform I/O now, queue I/O */
            context_t->bdev_io_wait.bdev = context_t->bdev;
            context_t->bdev_io_wait.cb_fn = thread_fn;
            context_t->bdev_io_wait.cb_arg = context_t;
            spdk_bdev_queue_io_wait(
                context_t->bdev,
                context_t->bdev_io_channel,
                &context_t->bdev_io_wait
            );
        } else if (!rc) {
            context_t->submitted->store(true, std::memory_order_release);
            context_t->submitted->notify_one();
        }
    }

    int SpdkEngine::thread_read(
        spdk_context_t* ctx_t,
        spdk_context_t_cb* ctx_t_cb
    ) {
        SPDK_NOTICELOG("Reading to the bdev\n");
        return spdk_bdev_read(
            ctx_t->bdev_desc,
            ctx_t->bdev_io_channel,
            ctx_t->request->buffer,
            ctx_t->request->offset,
            ctx_t->request->size,
            io_complete,
            ctx_t_cb
        );
    }

    int SpdkEngine::thread_write(
        spdk_context_t* ctx_t,
        spdk_context_t_cb* ctx_t_cb
    ) {
        SPDK_NOTICELOG("Writing to the bdev\n");
        return spdk_bdev_write(
            ctx_t->bdev_desc,
            ctx_t->bdev_io_channel,
            ctx_t->request->buffer,
            ctx_t->request->offset,
            ctx_t->request->size,
            io_complete,
            ctx_t_cb
        );
    }

    int SpdkEngine::thread_fsync(
        spdk_context_t* ctx_t,
        spdk_context_t_cb* ctx_t_cb
    ) {
        SPDK_NOTICELOG("Flushing to the bdev\n");
        return spdk_bdev_flush(
            ctx_t->bdev_desc,
            ctx_t->bdev_io_channel,
            ctx_t->request->offset,
            ctx_t->request->size,
            io_complete,
            ctx_t_cb
        );
    }

    int SpdkEngine::thread_fdatasync(
        spdk_context_t* ctx_t,
        spdk_context_t_cb* ctx_t_cb
    ) {
        return thread_fsync(ctx_t, ctx_t_cb);
    }

    int SpdkEngine::thread_nop(
        [[maybe_unused]] spdk_context_t* ctx_t,
        spdk_context_t_cb* ctx_t_cb
    ) {
        io_complete(nullptr, true, ctx_t_cb);
        return 0;
    }

    void SpdkEngine::io_complete(
        struct spdk_bdev_io* bdev_io,
        bool success,
        void* ctx_t_cb
    ) {
        spdk_context_t_cb* context_t_cb = static_cast<spdk_context_t_cb*>(ctx_t_cb);
        SpdkEngine* spdk_engine = static_cast<SpdkEngine*>(context_t_cb->spdk_engine);

        if (bdev_io) {
            spdk_bdev_free_io(bdev_io);
        }

        if (success) {
            SPDK_NOTICELOG("bdev io completed successfully\n");
        } else {
            SPDK_ERRLOG("bdev io error: %d\n", EIO);
        }

        context_t_cb->metrics_mutex->lock();

        Metric::fill_metric(
            *spdk_engine->metric,
            context_t_cb->operation_type,
            context_t_cb->start_timestamp,
            Metric::get_current_timestamp(),
            success ? context_t_cb->size : 0,
            context_t_cb->size,
            context_t_cb->offset
        );

        spdk_engine->logger->info(*spdk_engine->metric);

        context_t_cb->metrics_mutex->unlock();
        context_t_cb->available_indexes->enqueue(context_t_cb->free_index);
    }

    void SpdkEngine::bdev_event_cb(
        [[maybe_unused]] enum spdk_bdev_event_type type,
        [[maybe_unused]] struct spdk_bdev* bdev,
        [[maybe_unused]] void *event_ctx
    ) {
        SPDK_NOTICELOG("Unsupported bdev event: type %d\n", type);
    }

    void SpdkEngine::submit(Protocol::CommonRequest& request) {
        pending_request.isShutdown = false;
        pending_request.request = &request;
        request_trigger.store(&pending_request, std::memory_order_release);
        request_trigger.wait(&pending_request);
    };

    void SpdkEngine::reap_left_completions(void) {
        pending_request.isShutdown = true;
        pending_request.request = nullptr;
        request_trigger.store(&pending_request, std::memory_order_release);
        request_trigger.wait(&pending_request);
    };
};
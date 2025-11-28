#include <engine/spdk.h>

// NOTE: before exit, wait until all cb completions are done

namespace Engine {

    SpdkEngine::SpdkEngine(
        std::unique_ptr<Metric::Metric> _metric,
        std::unique_ptr<Logger::Logger> _logger,
        const SpdkConfig& config
    ) :
        Engine(std::move(_metric), std::move(_logger)),
        trigger_atomic()
    {
        spdk_main_thread = std::thread([this, config]() {
            start_spdk_app(this, config, &(this->trigger_atomic));
        });
    }

    SpdkEngine::~SpdkEngine() {
        spdk_main_thread.join();
    }

    int SpdkEngine::start_spdk_app(
        void* spdk_engine,
        const SpdkConfig config,
        std::atomic<TriggerData>* trigger_atomic
    ) {
        spdk_app_opts opts = {};
        SpdkAppContext app_context = {
            .bdev = nullptr,
            .bdev_desc = nullptr,
            .bdev_name = strdup(config.bdev_name.c_str()),
            .spdk_engine = spdk_engine,
            .spdk_threads = config.spdk_threads,
            .trigger_atomic = trigger_atomic
        };

        spdk_app_opts_init(&opts, sizeof(opts));
        opts.name = "spdk_engine_bdev";
        opts.rpc_addr = nullptr;
        opts.reactor_mask = config.reactor_mask.c_str();
        opts.json_config_file = config.json_config_file.c_str();

        int rc = spdk_app_start(&opts, thread_main_fn, &app_context);
        if (rc) {
            SPDK_ERRLOG("Error starting application\n");
        }

        SPDK_NOTICELOG("Perform final shutdown operations on spdk application\n");
        free(app_context.bdev_name);
        spdk_app_fini();
        return rc;
    }

    void SpdkEngine::thread_main_fn(
        void* app_ctx
    ) {
        SpdkAppContext* app_context = static_cast<SpdkAppContext*>(app_ctx);
        SPDK_NOTICELOG("Successfully started the application\n");
        SPDK_NOTICELOG("Opening the bdev %s\n", app_context->bdev_name);

        int rc = spdk_bdev_open_ext(
            app_context->bdev_name,
            true,
            bdev_event_cb,
            nullptr,
            &app_context->bdev_desc
        );

        if (rc) {
            SPDK_ERRLOG("Could not open bdev: %s\n", app_context->bdev_name);
            spdk_app_stop(-1);
            return;
        }

        SPDK_NOTICELOG("Successfully opened the bdev: %s\n", app_context->bdev_name);
        SPDK_NOTICELOG("Getting bdev structure from the descriptor\n");
        app_context->bdev = spdk_bdev_desc_get_bdev(app_context->bdev_desc);

        std::atomic<bool> submitted;
        int total_workers = app_context->spdk_threads;
        size_t total_blocks = spdk_bdev_get_num_blocks(app_context->bdev);

        std::vector<spdk_thread*> workers(total_workers);
        std::vector<SpdkThreadContext*> thread_contexts(total_workers);
        std::vector<SpdkThreadCallBackContext*> thread_cb_contexts(total_blocks);
        moodycamel::BlockingConcurrentQueue<int> available_indexes(total_blocks);

        SPDK_NOTICELOG("Initialize spdk worker threads\n");
        init_threads(workers);

        SPDK_NOTICELOG("Initialize spdk thread contexts\n");
        init_thread_contexts(
            app_context,
            submitted,
            workers,
            thread_contexts
        );

        SPDK_NOTICELOG("Initialize available indexes queue\n");
        init_available_indexes(
            total_blocks,
            available_indexes
        );

        SPDK_NOTICELOG("Initialize spdk thread callback contexts\n");
        init_thread_cb_contexts(
            app_context,
            thread_cb_contexts,
            available_indexes
        );

        SPDK_NOTICELOG("Alocate DMA buffer for IO operations\n");
        size_t block_size =
            spdk_bdev_get_block_size(app_context->bdev) *
            spdk_bdev_get_write_unit_size(app_context->bdev);

        uint8_t* dma_buf = (uint8_t*) allocate_dma_buffer(
            app_context,
            total_blocks,
            block_size
        );

        SPDK_NOTICELOG("Start main dispatch loop on spdk threads\n");
        thread_main_dispatch(
            app_context,
            workers,
            thread_contexts,
            thread_cb_contexts,
            available_indexes,
            dma_buf,
            block_size
        );

        SPDK_NOTICELOG("Cleanup spdk threads and contexts\n");
        threads_cleanup(
            workers,
            thread_contexts
        );

        SPDK_NOTICELOG("Close block device on thread: %s\n", spdk_thread_get_name(spdk_get_thread()));
        spdk_bdev_close(app_context->bdev_desc);

        SPDK_NOTICELOG("Free dma buffer\n");
        spdk_dma_free(dma_buf);

        for (auto& thread_cb_context : thread_cb_contexts) {
            free(thread_cb_context);
        }

        for (auto& thread_contexts : thread_contexts) {
            free(thread_contexts);
        }

        SPDK_NOTICELOG("Stop spdk framework\n");
        spdk_app_stop(0);
    }

    void SpdkEngine::init_threads(
        std::vector<spdk_thread*>& workers
    ) {
        for (size_t i = 0; i < workers.size(); i++) {
            std::string name = "worker" + std::to_string(i);
            SPDK_NOTICELOG("Creating spdk thread: %s\n", name.c_str());
            spdk_thread* worker = spdk_thread_create(name.c_str(), nullptr);

            if (!worker) {
                SPDK_ERRLOG("Error while creating spdk thread\n");
                spdk_app_stop(-1);
                return;
            }

            workers[i] = worker;
        }
    }

    void SpdkEngine::init_thread_contexts(
        SpdkAppContext* app_context,
        std::atomic<bool>& submitted,
        std::vector<spdk_thread*>& workers,
        std::vector<SpdkThreadContext*>& thread_contexts
    ) {
        for (size_t i = 0; i < thread_contexts.size(); i++) {
            SPDK_NOTICELOG("Creating spdk thread context for thread: %s\n", spdk_thread_get_name(workers[i]));
            SpdkThreadContext* thread_context =
                (SpdkThreadContext*) std::malloc(sizeof(SpdkThreadContext));

            thread_context->bdev = app_context->bdev;
            thread_context->bdev_desc = app_context->bdev_desc;
            thread_context->submitted = &submitted;
            thread_contexts[i] = thread_context;

            spdk_thread_send_msg(
                workers[i],
                thread_setup_io_channel_cb,
                thread_contexts[i]
            );
            // i think this is not needed
            // while (context_t.bdev_io_channel == nullptr) {
            //     spdk_thread_poll(worker, 0, 0);
            // }
        }
    }

    void SpdkEngine::init_thread_cb_contexts(
        SpdkAppContext* app_context,
        std::vector<SpdkThreadCallBackContext*>& thread_cb_contexts,
        moodycamel::BlockingConcurrentQueue<int>& available_indexes
    ) {
        for (size_t i = 0; i < thread_cb_contexts.size(); i++) {
            SpdkThreadCallBackContext* thread_cb_context =
                (SpdkThreadCallBackContext*) std::malloc(sizeof(SpdkThreadCallBackContext));

            thread_cb_context->spdk_engine = app_context->spdk_engine;
            thread_cb_context->available_indexes = &available_indexes;
            thread_cb_contexts[i] = thread_cb_context;
        }
    }

    void SpdkEngine::init_available_indexes(
        int total_indexes,
        moodycamel::BlockingConcurrentQueue<int>& available_indexes
    ) {
        for (int i = 0; i < total_indexes; i++) {
            available_indexes.enqueue(i);
        }
    }

    void* SpdkEngine::allocate_dma_buffer(
        SpdkAppContext* app_context,
        size_t total_blocks,
        size_t block_size
    ) {
        size_t buf_align = spdk_bdev_get_buf_align(app_context->bdev);
        size_t buf_size = block_size * total_blocks;

        SPDK_NOTICELOG("Allocate a pinned memory buffer with size %lu and alignment %lu\n", buf_size, buf_align);
        uint8_t* dma_buf = (uint8_t*) spdk_dma_zmalloc(buf_size, buf_align, nullptr);

        if (!dma_buf) {
            SPDK_ERRLOG("Failed to allocate buffer\n");
            spdk_bdev_close(app_context->bdev_desc);
            spdk_app_stop(-1);
            return nullptr;
        }

        return dma_buf;
    }

    void SpdkEngine::threads_cleanup(
        std::vector<spdk_thread*>& workers,
        std::vector<SpdkThreadContext*>& thread_contexts
    ) {
        for (size_t i = 0; i < workers.size(); i++) {
            SPDK_NOTICELOG("Send cleanup message to: %s\n", spdk_thread_get_name(workers[i]));
            spdk_thread_send_msg(
                workers[i],
                thread_cleanup_cb,
                thread_contexts[i]
            );

            SPDK_NOTICELOG("Wait for thread to exit: %s\n", spdk_thread_get_name(workers[i]));
            while (!spdk_thread_is_exited(workers[i])) {
                spdk_thread_poll(workers[i], 0, 0);
            }
	    }
    }

    void SpdkEngine::thread_setup_io_channel_cb(
        void* thread_ctx
    ) {
        spdk_thread* thread = spdk_get_thread();
        SpdkThreadContext* thread_context = static_cast<SpdkThreadContext*>(thread_ctx);

        SPDK_NOTICELOG("Opening io channel on thread: %s\n", spdk_thread_get_name(thread));
        thread_context->bdev_io_channel = spdk_bdev_get_io_channel(thread_context->bdev_desc);

        if (!thread_context->bdev_io_channel) {
            SPDK_ERRLOG("Failed to get io channel\n");
            spdk_app_stop(-1);
            return;
        }
    }

    void SpdkEngine::thread_cleanup_cb(
        void* thread_ctx
    ) {
        spdk_thread* thread = spdk_get_thread();
        SpdkThreadContext* thread_context = static_cast<SpdkThreadContext*>(thread_ctx);
        SPDK_NOTICELOG("Cleanup on thread: %s\n", spdk_thread_get_name(thread));

        if (thread_context->bdev_io_channel) {
            SPDK_NOTICELOG("Close io channel on thread: %s\n", spdk_thread_get_name(thread));
            spdk_put_io_channel(thread_context->bdev_io_channel);
        }

        SPDK_NOTICELOG("Exit thread on: %s\n", spdk_thread_get_name(thread));
        spdk_thread_exit(thread);
    }

    void SpdkEngine::thread_main_dispatch(
        SpdkAppContext* app_context,
        std::vector<spdk_thread*>& workers,
        std::vector<SpdkThreadContext*>& thread_contexts,
        std::vector<SpdkThreadCallBackContext*>& thread_cb_contexts,
        moodycamel::BlockingConcurrentQueue<int>& available_indexes,
        uint8_t* dma_buf,
        int block_size
    ) {
        for (int i = 0, iter = 0; true; i = (i + 1) % workers.size(), iter++) {
            SPDK_NOTICELOG("Dispatcher iteration: %d\n", iter);

            TriggerData snap = app_context
                ->trigger_atomic
                ->load(std::memory_order_acquire);

            while (!snap.has_next) {
                snap = app_context
                    ->trigger_atomic
                    ->load(std::memory_order_acquire);
            }

            if (snap.is_shutdown) {
                SPDK_NOTICELOG("Dispatcher receive shutdown\n");
                break;
            }

            int free_index = 0;
            available_indexes.wait_dequeue(free_index);

            spdk_thread* worker = workers[i];
            SPDK_NOTICELOG("Free index: %d (%s)\n", free_index, spdk_thread_get_name(worker));

            SpdkThreadContext* thread_context = thread_contexts[i];
            thread_context->request = snap.request;

            uint8_t* original_buf = thread_context->request->buffer;
            uint8_t* dma_chunk = dma_buf + (free_index * block_size);

            if (thread_context->request->operation == Operation::OperationType::WRITE) {
                SPDK_NOTICELOG("Memcopy to dma zone with index: %d\n", free_index);
                std::memcpy(
                    dma_chunk,
                    thread_context->request->buffer,
                    thread_context->request->size
                );
            }

            thread_context->request->buffer = dma_chunk;
            thread_context->thread_cb_ctx = thread_cb_contexts[free_index];
            thread_context->thread_cb_ctx->index = free_index;

            thread_context->thread_cb_ctx->metric_data = {
                .size = thread_context->request->size,
                .offset = thread_context->request->offset,
                .start_timestamp = Metric::get_current_timestamp(),
                .operation_type = thread_context->request->operation
            };

            thread_context->submitted->store(false, std::memory_order_release);

            SPDK_NOTICELOG("Dispatching request to thread: %s\n", spdk_thread_get_name(worker));
            spdk_thread_send_msg(worker, thread_fn, thread_context);
            spdk_thread_poll(worker, 0, 0);

            SPDK_NOTICELOG("Dispatcher waiting for submission done\n");
            thread_context->submitted->wait(false);
            thread_context->request->buffer = original_buf;

            SPDK_NOTICELOG("Dispatcher marking submission done\n");
            TriggerData done_snap = {};
            done_snap.has_submitted = true;
            app_context->trigger_atomic->store(done_snap, std::memory_order_release);
        }

        SPDK_NOTICELOG("Dispatcher marking shutdown done\n");
        TriggerData done_snap = {};
        done_snap.has_submitted = true;
        app_context->trigger_atomic->store(done_snap, std::memory_order_release);
    }

    void SpdkEngine::thread_fn(
        void* thread_ctx
    ) {
        int rc = 0;
        SpdkThreadContext* thread_context = static_cast<SpdkThreadContext*>(thread_ctx);
        SPDK_NOTICELOG("thread_fn on: %s\n", spdk_thread_get_name(spdk_get_thread()));

        switch (thread_context->request->operation) {
            case Operation::OperationType::READ:
                rc = thread_read(thread_context);
                break;
            case Operation::OperationType::WRITE:
                rc = thread_write(thread_context);
                break;
            case Operation::OperationType::FSYNC:
                rc = thread_fsync(thread_context);
                break;
            case Operation::OperationType::FDATASYNC:
                rc = thread_fdatasync(thread_context);
                break;
            case Operation::OperationType::NOP:
                rc = thread_nop(thread_context);
                break;
        }

        SPDK_NOTICELOG("Value of rc: %d (%s)\n", rc, spdk_thread_get_name(spdk_get_thread()));

        if (rc == -ENOMEM) {
            SPDK_NOTICELOG("Queueing io\n");
            thread_context->bdev_io_wait.bdev = thread_context->bdev;
            thread_context->bdev_io_wait.cb_fn = thread_fn;
            thread_context->bdev_io_wait.cb_arg = thread_context;
            spdk_bdev_queue_io_wait(
                thread_context->bdev,
                thread_context->bdev_io_channel,
                &thread_context->bdev_io_wait
            );
        } else if (!rc) {
            thread_context->submitted->store(true, std::memory_order_release);
            thread_context->submitted->notify_one();
        }
    }

    int SpdkEngine::thread_read(
        SpdkThreadContext* thread_ctx
    ) {
        SPDK_NOTICELOG("Reading to the bdev\n");
        return spdk_bdev_read(
            thread_ctx->bdev_desc,
            thread_ctx->bdev_io_channel,
            thread_ctx->request->buffer,
            thread_ctx->request->offset,
            thread_ctx->request->size,
            io_complete,
            thread_ctx->thread_cb_ctx
        );
    }

    int SpdkEngine::thread_write(
        SpdkThreadContext* thread_ctx
    ) {
        SPDK_NOTICELOG("Writing to the bdev\n");
        SPDK_NOTICELOG("bdev desc: %p\n", (void*) thread_ctx->bdev_desc);
        SPDK_NOTICELOG("bdev io channel: %p\n", (void*) thread_ctx->bdev_io_channel);
        SPDK_NOTICELOG("buffer: %p\n", thread_ctx->request->buffer);
        SPDK_NOTICELOG("offset: %lu\n", thread_ctx->request->offset);
        SPDK_NOTICELOG("size: %lu\n", thread_ctx->request->size);

        return spdk_bdev_write(
            thread_ctx->bdev_desc,
            thread_ctx->bdev_io_channel,
            thread_ctx->request->buffer,
            thread_ctx->request->offset,
            thread_ctx->request->size,
            io_complete,
            thread_ctx->thread_cb_ctx
        );
    }

    int SpdkEngine::thread_fsync(
        SpdkThreadContext* thread_ctx
    ) {
        SPDK_NOTICELOG("Flushing to the bdev\n");
        return spdk_bdev_flush(
            thread_ctx->bdev_desc,
            thread_ctx->bdev_io_channel,
            thread_ctx->request->offset,
            thread_ctx->request->size,
            io_complete,
            thread_ctx->thread_cb_ctx
        );
    }

    int SpdkEngine::thread_fdatasync(
        SpdkThreadContext* thread_ctx
    ) {
        return thread_fsync(thread_ctx);
    }

    int SpdkEngine::thread_nop(
        SpdkThreadContext* thread_ctx
    ) {
        io_complete(nullptr, true, thread_ctx->thread_cb_ctx);
        return 0;
    }

    void SpdkEngine::io_complete(
        struct spdk_bdev_io* bdev_io,
        bool success,
        void* thread_cb_ctx
    ) {
        SpdkThreadCallBackContext* thread_cb_context = static_cast<SpdkThreadCallBackContext*>(thread_cb_ctx);
        SpdkEngine* spdk_engine = static_cast<SpdkEngine*>(thread_cb_context->spdk_engine);

        if (bdev_io) {
            spdk_bdev_free_io(bdev_io);
        }

        if (success) {
            SPDK_NOTICELOG("bdev io completed successfully\n");
        } else {
            SPDK_ERRLOG("bdev io error: %d\n", EIO);
        }

        Metric::fill_metric(
            *spdk_engine->metric,
            thread_cb_context->metric_data.operation_type,
            thread_cb_context->metric_data.start_timestamp,
            Metric::get_current_timestamp(),
            success ? thread_cb_context->metric_data.size : 0,
            thread_cb_context->metric_data.size,
            thread_cb_context->metric_data.offset
        );

        spdk_engine->logger->info(*spdk_engine->metric);
        thread_cb_context->available_indexes->enqueue(thread_cb_context->index);
    }

    void SpdkEngine::bdev_event_cb(
        [[maybe_unused]] enum spdk_bdev_event_type type,
        [[maybe_unused]] struct spdk_bdev* bdev,
        [[maybe_unused]] void *event_ctx
    ) {
        SPDK_NOTICELOG("Unsupported bdev event: type %d\n", type);
    }

    void SpdkEngine::submit(
        Protocol::CommonRequest& request
    ) {
        std::cout << "SpdkEngine submit request" << std::endl;
        TriggerData snap = {};
        snap.has_next = true;
        snap.request = &request;
        publish_and_wait(snap);
        std::cout << "SpdkEngine submit request complet" << std::endl;
    }

    void SpdkEngine::reap_left_completions(void) {
        std::cout << "SpdkEngine submit shutdown" << std::endl;
        TriggerData snap = {};
        snap.has_next = true;
        snap.is_shutdown = true;
        publish_and_wait(snap);
        std::cout << "SpdkEngine submit shutdown complet" << std::endl;
    }

    void SpdkEngine::publish_and_wait(
        const TriggerData& snap
    ) {
        trigger_atomic.store(snap, std::memory_order_release);
        TriggerData current = trigger_atomic.load(std::memory_order_acquire);

        while (!current.has_submitted) {
            current = trigger_atomic.load(std::memory_order_acquire);
        }
    }
};
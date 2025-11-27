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

        spdk_app_opts_init(&opts, sizeof(opts));
        opts.name = "spdk_engine_bdev";
        opts.rpc_addr = nullptr;
        opts.reactor_mask = config.reactor_mask.c_str();
        opts.json_config_file = config.json_config_file.c_str();

        context.spdk_threads = config.spdk_threads;
        context.bdev_name = strdup(config.bdev_name.c_str());

        int rc = spdk_app_start(&opts, start, &context);
        if (rc) {
            SPDK_ERRLOG("Error starting application\n");
        }

        SPDK_NOTICELOG("Perform final shutdown operations on spdk application\n");
        free(context.bdev_name);
        spdk_dma_free(context.dma_buffer);
        spdk_app_fini();
        return rc;
    }

    void SpdkEngine::start(void* ctx) {
        spdk_context* context = static_cast<spdk_context*>(ctx);
        SPDK_NOTICELOG("Successfully started the application\n");
        SPDK_NOTICELOG("Opening the bdev %s\n", context->bdev_name);

        int rc = spdk_bdev_open_ext(
            context->bdev_name,
            true,
            bdev_event_cb,
            nullptr,
            &context->bdev_desc
        );

        if (rc) {
            SPDK_ERRLOG("Could not open bdev: %s\n", context->bdev_name);
            spdk_app_stop(-1);
            return;
        }

        context->bdev = spdk_bdev_desc_get_bdev(context->bdev_desc);

        std::atomic<bool> submitted;
        std::mutex metrics_mutex;

        std::vector<uint8_t*> dma_zones;
        std::vector<spdk_thread*> spdk_workers;
        std::vector<spdk_context_t> spdk_context_ts;
        std::vector<spdk_context_t_cb> spdk_context_t_cbs;

        int total_workers = context->spdk_threads;
        size_t num_blocks = spdk_bdev_get_num_blocks(context->bdev);
        moodycamel::BlockingConcurrentQueue<int> available_indexes(num_blocks);

        for (int worker_index = 0; worker_index < total_workers; worker_index++) {
            spdk_context_t context_t = {
                .bdev = context->bdev,
                .bdev_desc = context->bdev_desc,
                .bdev_io_channel = nullptr,
                .bdev_io_wait = {},
                .request = nullptr,
                .submitted = &submitted,
                .ctx_t_cb = nullptr
            };

            std::string worker_name = "worker" + std::to_string(worker_index);
            SPDK_NOTICELOG("Creating spdk thread: %s\n", worker_name.c_str());
            spdk_thread* worker = spdk_thread_create(worker_name.c_str(), nullptr);

            if (!worker) {
                SPDK_ERRLOG("Error while creating spdk thread\n");
                spdk_app_stop(-1);
                return;
            }

            spdk_thread_send_msg(worker, thread_setup, &context_t);
            while (context_t.bdev_io_channel == nullptr) {
                spdk_thread_poll(worker, 0, 0);
            }

            spdk_workers.push_back(worker);
            spdk_context_ts.push_back(context_t);
        }

        size_t buf_align = spdk_bdev_get_buf_align(context->bdev);
        size_t block_size =
            spdk_bdev_get_block_size(context->bdev) *
            spdk_bdev_get_write_unit_size(context->bdev);

        size_t buf_size = block_size * num_blocks;

        SPDK_NOTICELOG("Allocate a pinned memory buffer with size %lu and alignment %lu\n", buf_size, buf_align);
        context->dma_buffer = (uint8_t*) spdk_dma_zmalloc(buf_size, buf_align, nullptr);

        if (!context->dma_buffer) {
            SPDK_ERRLOG("Failed to allocate buffer\n");
            spdk_bdev_close(context->bdev_desc);
            spdk_app_stop(-1);
            return;
        }

        for (size_t index = 0; index < num_blocks; index++) {
            spdk_context_t_cb context_t_cb = {};
            context_t_cb.free_index = 0;
            context_t_cb.metrics_mutex = &metrics_mutex;
            context_t_cb.spdk_engine = context->spdk_engine;
            context_t_cb.available_indexes = &available_indexes;

            available_indexes.enqueue(index);
            spdk_context_t_cbs.push_back(context_t_cb);
            dma_zones.push_back(context->dma_buffer + (index * block_size));
        }

        for (int worker_index = 0, iter = 0; true; worker_index = (worker_index + 1) % total_workers, iter++) {
            SPDK_NOTICELOG("Wait for trigger on worker index: %d (iteration: %d)\n", worker_index, iter);
            context->request_trigger->wait(nullptr);

            if (context->request_trigger->load()->isShutdown) {
                SPDK_NOTICELOG("Receive shutdown\n");
                break;
            }

            int free_index = 0;
            available_indexes.wait_dequeue(free_index);

            spdk_thread* worker = spdk_workers[worker_index];
            spdk_context_t& context_t = spdk_context_ts[worker_index];
            context_t.ctx_t_cb = &spdk_context_t_cbs[free_index];
            context_t.request = context->request_trigger->load(std::memory_order_acquire)->request;

            SPDK_NOTICELOG("Free index: %d (%s)\n", free_index, spdk_thread_get_name(worker));

            if (context_t.request->operation == Operation::OperationType::WRITE) {
                SPDK_NOTICELOG("Memcopy to dma zone with index: %d\n", free_index);
                std::memcpy(dma_zones[free_index], context_t.request->buffer, context_t.request->size);
            }

            uint8_t* temp_buffer = context_t.request->buffer;
            context_t.request->buffer = dma_zones[free_index];
            context_t.ctx_t_cb->free_index = free_index;
            context_t.submitted->store(false, std::memory_order_release);

            SPDK_NOTICELOG("Send message to thread: %s\n", spdk_thread_get_name(worker));
            spdk_thread_send_msg(worker, thread_fn, &context_t);
            spdk_thread_poll(worker, 0, 0);

            SPDK_NOTICELOG("Wait for submmit in thread: %s\n", spdk_thread_get_name(spdk_get_thread()));
            context_t.submitted->wait(false);
            context_t.request->buffer = temp_buffer;

            context->request_trigger->store(nullptr, std::memory_order_release);
            context->request_trigger->notify_one();
        }

        context->request_trigger->store(nullptr, std::memory_order_release);
        context->request_trigger->notify_one();

        for (int worker_index = 0; worker_index < total_workers; worker_index++) {
            spdk_thread* worker = spdk_workers[worker_index];
            spdk_context_t& context_t = spdk_context_ts[worker_index];

            SPDK_NOTICELOG("Send cleanup message to: %s\n", spdk_thread_get_name(worker));
            spdk_thread_send_msg(worker, thread_cleanup, &context_t);

            while (!spdk_thread_is_exited(worker)) {
                SPDK_NOTICELOG("Thread %s exited: %d\n", spdk_thread_get_name(worker), spdk_thread_is_exited(worker));
                spdk_thread_poll(worker, 0, 0);
            }
	    }

        SPDK_NOTICELOG("Close block device on thread: %s\n", spdk_thread_get_name(spdk_get_thread()));
        spdk_bdev_close(context->bdev_desc);

        SPDK_NOTICELOG("Stop spdk framework\n");
        spdk_app_stop(0);
    }

    void SpdkEngine::thread_setup(void* ctx){
        spdk_thread* thread = spdk_get_thread();
        spdk_context_t* context_t = static_cast<spdk_context_t*>(ctx);
        SPDK_NOTICELOG("Opening io channel on thread: %s\n", spdk_thread_get_name(thread));
        context_t->bdev_io_channel = spdk_bdev_get_io_channel(context_t->bdev_desc);

        if (!context_t->bdev_io_channel) {
            SPDK_ERRLOG("Failed to get io channel\n");
            spdk_app_stop(-1);
            return;
        }
    }

    void SpdkEngine::thread_cleanup(void* ctx_t) {
        spdk_thread* thread = spdk_get_thread();
        spdk_context_t* context_t = static_cast<spdk_context_t*>(ctx_t);
        SPDK_NOTICELOG("Cleanup on thread: %s\n", spdk_thread_get_name(thread));

        if (context_t->bdev_io_channel) {
            SPDK_NOTICELOG("Put io channel on thread: %s\n", spdk_thread_get_name(thread));
            spdk_put_io_channel(context_t->bdev_io_channel);
        }

        SPDK_NOTICELOG("Exit thread on: %s\n", spdk_thread_get_name(thread));
        spdk_thread_exit(thread);
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

        SPDK_NOTICELOG("thread_fn on: %s\n", spdk_thread_get_name(spdk_get_thread()));

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

        SPDK_NOTICELOG("Value of rc: %d (%s)\n", rc, spdk_thread_get_name(spdk_get_thread()));

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
        SPDK_NOTICELOG("bdev desc: %p\n", (void*) ctx_t->bdev_desc);
        SPDK_NOTICELOG("bdev io channel: %p\n", (void*) ctx_t->bdev_io_channel);
        SPDK_NOTICELOG("buffer: %p\n", ctx_t->request->buffer);
        SPDK_NOTICELOG("offset: %lu\n", ctx_t->request->offset);
        SPDK_NOTICELOG("size: %lu\n", ctx_t->request->size);


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
        std::cout << "init submit" << std::endl;
        pending_request.isShutdown = false;
        pending_request.request = &request;
        request_trigger.store(&pending_request, std::memory_order_release);
        request_trigger.notify_one();
        request_trigger.wait(&pending_request);
        std::cout << "end submit" << std::endl;
    };

    void SpdkEngine::reap_left_completions(void) {
        std::cout << "init reap leaf completions" << std::endl;
        pending_request.isShutdown = true;
        pending_request.request = nullptr;
        request_trigger.store(&pending_request, std::memory_order_release);
        request_trigger.notify_one();
        request_trigger.wait(&pending_request);
        std::cout << "end reap leaf completions" << std::endl;
    };
};
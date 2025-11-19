#include <engine/spdk_core.h>


int start_spdk_app(SpdkConfig& config) {
	spdk_app_opts opts = {};
	spdk_context context = {};

	spdk_app_opts_init(&opts, sizeof(opts));
	opts.name = "spdk_engine_bdev";
	opts.rpc_addr = NULL;
	opts.reactor_mask = config.reactor_mask.c_str();
	opts.json_config_file = config.json_config_file.c_str();

    context.bdev_name = strdup(config.bdev_name.c_str());

    int rc = spdk_app_start(&opts, start, &context);
	if (rc) {
		SPDK_ERRLOG("Error starting application\n");
	}

    free(context.bdev_name);
	spdk_app_fini();
	return rc;
}

static void start(void *ctx) {
    spdk_context* context = static_cast<spdk_context*>(ctx);
    context->bdev = nullptr;
    context->bdev_desc = nullptr;

	SPDK_NOTICELOG("Successfully started the application\n");
	SPDK_NOTICELOG("Opening the bdev %s\n", context->bdev_name);

	int rc = spdk_bdev_open_ext(
        context->bdev_name,
        true,
        hello_bdev_event_cb,
        nullptr,
		&context->bdev_desc
    );

    if (rc) {
		SPDK_ERRLOG("Could not open bdev: %s\n", context->bdev_name);
		spdk_app_stop(-1);
		return;
	}

	context->bdev = spdk_bdev_desc_get_bdev(context->bdev_desc);

	SPDK_NOTICELOG("Opening io channel\n");
    spdk_context_t context_t = {};
	context_t.bdev_io_channel = spdk_bdev_get_io_channel(context->bdev_desc);

    if (!context_t.bdev_io_channel) {
		SPDK_ERRLOG("Could not create bdev I/O channel\n");
		spdk_bdev_close(context->bdev_desc);
		spdk_app_stop(-1);
		return;
	}

    SPDK_NOTICELOG("Creating spdk thread\n");
	struct spdk_thread* t1 = spdk_thread_create("spdk_thread_0", nullptr);

    if (!t1) {
		SPDK_ERRLOG("Error while creating spdk thread\n");
        return;
    }

    size_t buf_align = spdk_bdev_get_buf_align(context->bdev);
    context->buffer = (char*) spdk_dma_zmalloc(context->buffer_size, buf_align, nullptr);

    if (!context->buffer) {
		SPDK_ERRLOG("Failed to allocate buffer\n");
		spdk_put_io_channel(context_t.bdev_io_channel);
		spdk_bdev_close(context->bdev_desc);
		spdk_app_stop(-1);
		return;
	}

	if (spdk_bdev_is_zoned(context->bdev)) {
		// hello_reset_zone(hello_context);
		/* If bdev is zoned, the callback, reset_zone_complete, will call hello_write() */
		return;
	}

    Protocol::CommonRequest* request = nullptr;

    while (true) {
        Worker::dequeue(*context->queue, request);
        context_t.request = *request;
        spdk_thread_send_msg(t1, nullptr, &context_t);
    }


	for (int i = 0; i < 50; i++) {
		spdk_thread_send_msg(t2, worker_fn, hello_context);
		spdk_thread_send_msg(t3, worker_fn, hello_context);
	}
}

static void hello_bdev_event_cb(enum spdk_bdev_event_type type, struct spdk_bdev *bdev, void *event_ctx) {
	SPDK_NOTICELOG("Unsupported bdev event: type %d\n", type);
}

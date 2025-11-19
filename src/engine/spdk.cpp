#include <engine/spdk.h>

namespace Engine {

    SpdkEngine::SpdkEngine(
        Metric::MetricType _metric_type,
        std::unique_ptr<Logger::Logger> _logger,
        const SpdkConfig& config
    ) :
        Engine(_metric_type, std::move(_logger)),
        internal_queue(128)
    {
        spdk_main_thread = 

        spdk_app_opts opts = {};
        spdk_context_t context = {};

        spdk_app_opts_init(&opts, sizeof(opts));
        opts.rpc_addr = nullptr;
        opts.reactor_mask = config.reactor_mask.c_str();
        opts.json_config_file = config.bdev_name.c_str();

        if (spdk_app_start(&opts, run, &context)) {
            throw std::runtime_error("error starting spdk application");
	    }

        spdk_app_fini();
    }

    // SpdkEngine::~SpdkEngine() {
    //     running.store(false);

    //     for (auto& t : os_polling_threads)
    //         if (t.joinable()) t.join();

    //     for (auto& ch : io_channels)
    //         if (ch) spdk_put_io_channel(ch);

    //     if (desc)
    //         spdk_bdev_close(desc);

    //     for (auto& t : spdk_polling_threads) {
    //         if (t) {
    //             spdk_thread_exit(t);
    //             spdk_thread_destroy(t);
    //         }
    //     }

    //     spdk_env_fini();
    // }

    // int SpdkEngine::nop(SpdkUserData* spdk_user_data) {
    //     return spdk_bdev_read(
    //         desc,
    //         io_channels[0],
    //         nullptr,
    //         0,
    //         0,
    //         io_complete,
    //         spdk_user_data
    //     );
    // }

    // int SpdkEngine::fsync(Protocol::CommonRequest& request, SpdkUserData* spdk_user_data) {
    //     return spdk_bdev_flush(
    //         desc,
    //         io_channels[0],
    //         request.offset,
    //         request.size,
    //         io_complete,
    //         spdk_user_data
    //     );
    // }

    // int SpdkEngine::fdatasync(Protocol::CommonRequest& request, SpdkUserData* spdk_user_data) {
    //     return spdk_bdev_flush_blocks(
    //         desc,
    //         io_channels[0],
    //         request.offset,
    //         1,
    //         io_complete,
    //         spdk_user_data
    //     );
    // }

    // int SpdkEngine::read(Protocol::CommonRequest& request, SpdkUserData* spdk_user_data) {
    //     return spdk_bdev_read(
    //         desc,
    //         io_channels[0],
    //         request.buffer,
    //         request.offset,
    //         request.size,
    //         io_complete,
    //         spdk_user_data
    //     );
    // }

    // int SpdkEngine::write(Protocol::CommonRequest& request, SpdkUserData* spdk_user_data) {
    //     return spdk_bdev_write(
    //         desc,
    //         io_channels[0],
    //         request.buffer,
    //         request.offset,
    //         request.size,
    //         io_complete,
    //         spdk_user_data
    //     );
    // }

    // void SpdkEngine::hello_bdev_event_cb(enum spdk_bdev_event_type type, struct spdk_bdev *bdev, void *event_ctx) {
    // 	SPDK_NOTICELOG("Unsupported bdev event: type %d\n", type);
    // }

    // void SpdkEngine::io_complete(spdk_bdev_io* bdev_io, bool success, void* cb_arg) {
    //     auto* spdk_user_data = static_cast<SpdkUserData*>(cb_arg);
    //     auto* engine = static_cast<SpdkEngine*>(spdk_user_data->engine);

    //     Metric::fill_metric(
    //         engine->metric_type,
    //         *engine->metric,
    //         spdk_user_data->operation_type,
    //         spdk_user_data->start_timestamp,
    //         Metric::get_current_timestamp(),
    //         success ? spdk_user_data->size : 0,
    //         spdk_user_data->size,
    //         spdk_user_data->offset
    //     );

    //     engine->logger->info(
    //         engine->metric_type,
    //         *engine->metric
    //     );

    //     delete spdk_user_data;
    //     spdk_bdev_free_io(bdev_io);
    //     engine->outstanding_io.fetch_sub(1, std::memory_order_relaxed);
    // }

    // void SpdkEngine::submit(Protocol::CommonRequest& request) {
    //     int rc = 0;
    //     outstanding_io.fetch_add(1, std::memory_order_relaxed);

    //     auto* spdk_user_data = new SpdkUserData{
    //         .size = request.size,
    //         .offset = request.offset,
    //         .start_timestamp = Metric::get_current_timestamp(),
    //         .operation_type = request.operation,
    //         .engine = this
    //     };

    //     switch (request.operation) {
    //         case Operation::OperationType::READ:
    //             rc = this->read(request, spdk_user_data);
    //             break;
    //         case Operation::OperationType::WRITE:
    //             rc = this->write(request, spdk_user_data);
    //             break;
    //         case Operation::OperationType::FSYNC:
    //             rc = this->fsync(request, spdk_user_data);
    //             break;
    //         case Operation::OperationType::FDATASYNC:
    //             rc = this->fdatasync(request, spdk_user_data);
    //             break;
    //         case Operation::OperationType::NOP:
    //             rc = this->nop(spdk_user_data);
    //             break;
    //         default:
    //             throw std::invalid_argument("Unsupported operation type by SpdkEngine");
    //     }

    //     if (rc) {
    //         delete spdk_user_data;
    //         throw std::runtime_error("spdk_bdev failed: " + std::to_string(rc));
    //     }
    // }

    // void SpdkEngine::reap_left_completions(void) {
    //     while (outstanding_io.load(std::memory_order_relaxed) > 0) {
    //         std::this_thread::sleep_for(std::chrono::microseconds(50));
    //     }
    // }
};
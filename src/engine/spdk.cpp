#include <engine/spdk.h>

namespace Engine {

    SpdkEngine::SpdkEngine(
        Metric::MetricType _metric_type,
        std::unique_ptr<Logger::Logger> _logger,
        const SpdkConfig& config
    ) :
        Engine(_metric_type, std::move(_logger)),
        env_opts(),
        desc(nullptr),
        io_channels(),
        spdk_polling_threads(),
        os_polling_threads()
    {
        spdk_env_opts_init(&env_opts);
        env_opts.name = config.bdev_name.c_str();

        if (spdk_env_init(&env_opts) < 0) {
            throw std::runtime_error("Failed to initialize SPDK env: " + std::string(strerror(errno)));
        }

        if (spdk_bdev_open_ext(config.bdev_name.c_str(), true, nullptr, nullptr, &desc) != 0) {
            throw std::runtime_error("Failed to open bdev '" + config.bdev_name + "': " + std::string(strerror(errno)));
        }

        spdk_cpuset mask;
        spdk_cpuset_zero(&mask);
        spdk_cpuset_set_cpu(&mask, 0, true);

        spdk_thread* spdk_t = spdk_thread_create("spdk_poll_thread", &mask);
        if (!spdk_t) {
            throw std::runtime_error("Failed to create SPDK thread");
        }

        spdk_set_thread(spdk_t);
        spdk_io_channel* channel = spdk_bdev_get_io_channel(desc);

        if (!channel) {
            throw std::runtime_error("Failed to get I/O channel");
        }

        io_channels.push_back(channel);

        os_polling_threads.emplace_back([this, spdk_t]() {
            spdk_set_thread(spdk_t);
            while (running.load(std::memory_order_relaxed)) {
                spdk_thread_poll(spdk_t, 0, 0);
                std::this_thread::sleep_for(std::chrono::microseconds(50));
            }
        });
    }

    SpdkEngine::~SpdkEngine() {
        running.store(false);

        for (auto& t : os_polling_threads)
            if (t.joinable()) t.join();

        for (auto& ch : io_channels)
            if (ch) spdk_put_io_channel(ch);

        if (desc)
            spdk_bdev_close(desc);

        for (auto& t : spdk_polling_threads) {
            if (t) {
                spdk_thread_exit(t);
                spdk_thread_destroy(t);
            }
        }

        spdk_env_fini();
    }

    int SpdkEngine::nop(SpdkUserData* spdk_user_data) {
        return spdk_bdev_read(
            desc,
            io_channels[0],
            nullptr,
            0,
            0,
            io_complete,
            spdk_user_data
        );
    }

    int SpdkEngine::fsync(Protocol::CommonRequest& request, SpdkUserData* spdk_user_data) {
        return spdk_bdev_flush(
            desc,
            io_channels[0],
            request.offset,
            request.size,
            io_complete,
            spdk_user_data
        );
    }

    int SpdkEngine::fdatasync(Protocol::CommonRequest& request, SpdkUserData* spdk_user_data) {
        return spdk_bdev_flush_blocks(
            desc,
            io_channels[0],
            request.offset,
            1,
            io_complete,
            spdk_user_data
        );
    }

    int SpdkEngine::read(Protocol::CommonRequest& request, SpdkUserData* spdk_user_data) {
        return spdk_bdev_read(
            desc,
            io_channels[0],
            request.buffer,
            request.offset,
            request.size,
            io_complete,
            spdk_user_data
        );
    }

    int SpdkEngine::write(Protocol::CommonRequest& request, SpdkUserData* spdk_user_data) {
        return spdk_bdev_write(
            desc,
            io_channels[0],
            request.buffer,
            request.offset,
            request.size,
            io_complete,
            spdk_user_data
        );
    }

    void SpdkEngine::io_complete(spdk_bdev_io* bdev_io, bool success, void* cb_arg) {
        auto* spdk_user_data = static_cast<SpdkUserData*>(cb_arg);
        auto* engine = static_cast<SpdkEngine*>(spdk_user_data->engine);

        Metric::fill_metric(
            engine->metric_type,
            *engine->metric,
            spdk_user_data->operation_type,
            spdk_user_data->start_timestamp,
            Metric::get_current_timestamp(),
            success ? spdk_user_data->size : 0,
            spdk_user_data->size,
            spdk_user_data->offset
        );

        engine->logger->info(
            engine->metric_type,
            *engine->metric
        );

        delete spdk_user_data;
        spdk_bdev_free_io(bdev_io);
        engine->outstanding_io.fetch_sub(1, std::memory_order_relaxed);
    }

    void SpdkEngine::submit(Protocol::CommonRequest& request) {
        int rc = 0;
        outstanding_io.fetch_add(1, std::memory_order_relaxed);

        auto* spdk_user_data = new SpdkUserData{
            .size = request.size,
            .offset = request.offset,
            .start_timestamp = Metric::get_current_timestamp(),
            .operation_type = request.operation,
            .engine = this
        };

        switch (request.operation) {
            case Operation::OperationType::READ:
                rc = this->read(request, spdk_user_data);
                break;
            case Operation::OperationType::WRITE:
                rc = this->write(request, spdk_user_data);
                break;
            case Operation::OperationType::FSYNC:
                rc = this->fsync(request, spdk_user_data);
                break;
            case Operation::OperationType::FDATASYNC:
                rc = this->fdatasync(request, spdk_user_data);
                break;
            case Operation::OperationType::NOP:
                rc = this->nop(spdk_user_data);
                break;
            default:
                throw std::invalid_argument("Unsupported operation type by SpdkEngine");
        }

        if (rc) {
            delete spdk_user_data;
            throw std::runtime_error("spdk_bdev failed: " + std::to_string(rc));
        }
    }

    void SpdkEngine::reap_left_completions(void) {
        while (outstanding_io.load(std::memory_order_relaxed) > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }
    }
};
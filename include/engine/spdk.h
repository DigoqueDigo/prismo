#ifndef SPDK_ENGINE_H
#define SPDK_ENGINE_H

#include <spdk/env.h>
#include <spdk/bdev.h>
#include <spdk/thread.h>
#include <spdk/event.h>

#include <io/metric.h>
#include <io/protocol.h>
#include <engine/utils.h>

#include <vector>
#include <stdexcept>
#include <string>
#include <cstring>
#include <cerrno>

namespace Engine {

    class SpdkEngine {
        private:
            spdk_env_opts env_opts;
            spdk_bdev_desc* desc;
            spdk_io_channel* channel;

            template<typename MetricT>
            static void io_complete(spdk_bdev_io* bdev_io, bool success, void* cb_arg);

            template<typename MetricT>
            void read(Protocol::CommonRequest& request, SpdkUserData* spdk_user_data);

            template<typename MetricT>
            void write(Protocol::CommonRequest& request, SpdkUserData* spdk_user_data);

        public:
            explicit SpdkEngine(const SpdkConfig& config);
            ~SpdkEngine();

            template<typename MetricT>
            void submit(Protocol::CommonRequest& request, std::vector<MetricT>& metrics);

            template<typename MetricT>
            void reap_left_completions(std::vector<MetricT>& metrics);
        };

        inline SpdkEngine::SpdkEngine(const SpdkConfig& config)
            : env_opts(), desc(nullptr), channel(nullptr) {

                spdk_env_opts_init(&env_opts);
                env_opts.name = config.bdev_name.c_str();

                if (spdk_env_init(&env_opts) < 0) {
                    throw std::runtime_error("Failed to initialize SPDK env: " + std::string(strerror(errno)));
                }

                if (spdk_bdev_open_ext(config.bdev_name.c_str(), true, nullptr, nullptr, &desc) != 0) {
                    throw std::runtime_error("Failed to open bdev '" + config.bdev_name + "': " + std::string(strerror(errno)));
                }

                channel = spdk_bdev_get_io_channel(desc);
                if (!channel) {
                    spdk_bdev_close(desc);
                    throw std::runtime_error("Failed to get IO channel for bdev: " + config.bdev_name);
                }
            }

        inline SpdkEngine::~SpdkEngine() {
            if (channel) {
                spdk_put_io_channel(channel);
            }
            if (desc) {
                spdk_bdev_close(desc);
            }
        }

        template<typename MetricT>
        void SpdkEngine::io_complete(spdk_bdev_io* bdev_io, bool success, void* cb_arg) {
            auto* spdk_user_data = static_cast<SpdkUserData*>(cb_arg);
            MetricT metric{};

            Metric::fill_base_metric<MetricT>(
                metric,
                spdk_user_data->operation_type,
                spdk_user_data->start_timestamp,
                Metric::get_current_timestamp()
            );

            Metric::fill_standard_metric<MetricT>(metric);

            int64_t res = success ? 0 : -1;
            if (bdev_io) {
                res = bdev_io->internal.res;
            }

            Metric::fill_full_metric<MetricT>(
                metric,
                res,
                spdk_user_data->size,
                spdk_user_data->offset
            );

            delete spdk_user_data;
            spdk_bdev_free_io(bdev_io);
        }

        template<typename MetricT>
        void SpdkEngine::read(Protocol::CommonRequest& request, SpdkUserData* spdk_user_data) {
            int rc = spdk_bdev_read(
                desc, channel,
                request.buffer,
                request.offset,
                request.size,
                io_complete<MetricT>,
                spdk_user_data
            );
            if (rc != 0) {
                delete spdk_user_data;
                throw std::runtime_error("spdk_bdev_read failed: " + std::to_string(rc));
            }
        }

        template<typename MetricT>
        void SpdkEngine::write(Protocol::CommonRequest& request, SpdkUserData* spdk_user_data) {
            int rc = spdk_bdev_write(
                desc, channel,
                request.buffer,
                request.offset,
                request.size,
                io_complete<MetricT>,
                spdk_user_data
            );
            if (rc != 0) {
                delete spdk_user_data;
                throw std::runtime_error("spdk_bdev_write failed: " + std::to_string(rc));
            }
        }

        template<typename MetricT>
        void SpdkEngine::submit(Protocol::CommonRequest& request, std::vector<MetricT>& metrics) {
            auto* spdk_user_data = new SpdkUserData{
                .size = request.size,
                .offset = request.offset,
                .start_timestamp = Metric::get_current_timestamp(),
                .operation_type = request.operation
            };

            switch (request.operation) {
                case Operation::READ:
                    read<MetricT>(request, spdk_user_data);
                    break;
                case Operation::WRITE:
                    write<MetricT>(request, spdk_user_data);
                    break;
                default:
                    delete spdk_user_data;
                    throw std::invalid_argument("Unsupported operation");
            }

            // Note: metric will be filled in io_complete, but we need to collect it later
            // So we need a way to reap completions → see reap_left_completions
        }

        template<typename MetricT>
        void SpdkEngine::reap_left_completions(std::vector<MetricT>& metrics) {
            // SPDK uses polling model. You must call spdk_bdev_io_get_thread() or use event framework.
            // This is a simplified version assuming single-threaded reactor.

            struct spdk_bdev_io_wait_entry wait_entry;
            wait_entry.bdev = nullptr;  // all bdevs
            wait_entry.cb_fn = [](void* ctx) { /* no-op */ };
            wait_entry.cb_arg = nullptr;

            // Poll all channels on this thread
            spdk_for_each_channel(
                &wait_entry,
                [](void* ctx, void* ch) {
                    // Process completions on this channel
                    spdk_bdev_io* io;
                    while ((io = spdk_bdev_io_get_next(ch)) != nullptr) {
                        // This shouldn't happen: completions are via callback
                    }
                },
                nullptr,
                [](void* ctx, int status) {}
            );

            // If you want **true async reaping**, you must:
            // 1. Use spdk_thread_poll() in a loop
            // 2. Or integrate with spdk_app / event framework
            // For now, this is a placeholder.
        }
}

#endif
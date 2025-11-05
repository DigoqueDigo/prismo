#ifndef CONSUMER_WORKER_H
#define CONSUMER_WORKER_H

#include <worker/utils.h>

#define METRICS_BATCH_SIZE 1000
using namespace moodycamel;

namespace Worker {

    template<
        typename LoggerT,
        typename EngineT,
        typename MetricT>
    class Consumer {
        private:
            LoggerT& logger;
            EngineT& engine;
            std::shared_ptr<ReaderWriterQueue<Protocol::Packet>> to_producer;
            std::shared_ptr<ReaderWriterQueue<Protocol::Packet>> to_consumer;

        public:
            Consumer(
                LoggerT& _logger,
                EngineT& _engine,
                std::shared_ptr<ReaderWriterQueue<Protocol::Packet>> _to_producer,
                std::shared_ptr<ReaderWriterQueue<Protocol::Packet>> _to_consumer
            ) :
                logger(_logger),
                engine(_engine),
                to_producer(_to_producer),
                to_consumer(_to_consumer) {}

            void run() {
                Protocol::Packet packet;
                std::vector<MetricT> metrics;
                metrics.reserve(METRICS_BATCH_SIZE);

                while (true) {
                    Worker::dequeue(*to_consumer, packet);

                    if (packet.isShutDown) {
                        break;
                    }

                    engine.template submit<MetricT>(packet.request, metrics);
                    Worker::enqueue(*to_producer, packet);

                    if constexpr (!std::is_same_v<MetricT, std::monostate>) {
                        if (metrics.size() >= METRICS_BATCH_SIZE) {
                            for (const auto& metric : metrics) {
                                logger.info(metric);
                            }
                            metrics.clear();
                        }
                    }
                }

                if constexpr (
                    std::is_same_v<EngineT, Engine::UringEngine> ||
                    std::is_same_v<EngineT, Engine::AioEngine>
                ) {
                    engine.template reap_left_completions<MetricT>(metrics);
                }

                for (const auto& metric : metrics) {
                    Metric::log_metric<MetricT, LoggerT>(logger, metric);
                }
            }
    };
};

#endif

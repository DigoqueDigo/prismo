#ifndef CONSUMER_WORKER_H
#define CONSUMER_WORKER_H

#include <worker/utils.h>
#include <parser/parser.h>

#define METRICS_BATCH_SIZE 1000

using namespace moodycamel;

namespace Worker {

    class Consumer {
        private:
            std::unique_ptr<Parser::EngineVariant> engine;
            std::unique_ptr<Parser::LoggerVariant> logger;
            std::unique_ptr<Parser::MetricVariant> metric;
            std::shared_ptr<BlockingReaderWriterCircularBuffer<Protocol::Packet*>> to_producer;
            std::shared_ptr<BlockingReaderWriterCircularBuffer<Protocol::Packet*>> to_consumer;

        public:
            Consumer(
                std::unique_ptr<Parser::EngineVariant> _engine,
                std::unique_ptr<Parser::LoggerVariant> _logger,
                std::unique_ptr<Parser::MetricVariant> _metric,
                std::shared_ptr<BlockingReaderWriterCircularBuffer<Protocol::Packet*>> _to_producer,
                std::shared_ptr<BlockingReaderWriterCircularBuffer<Protocol::Packet*>> _to_consumer
            ) :
                engine(std::move(_engine)),
                logger(std::move(_logger)),
                metric(std::move(_metric)),
                to_producer(_to_producer),
                to_consumer(_to_consumer) {}

            int open(Protocol::OpenRequest& request) {
                return std::visit([&request](auto& engine_concr) {
                    return engine_concr.open(request);
                }, *engine);
            }

            void close(Protocol::CloseRequest& request) {
                std::visit([&request](auto& engine_concr) {
                    engine_concr.close(request);
                }, *engine);
            }

            void run() {
                std::visit([&](auto& engine_concr, auto& logger_concr, auto& metric_concr) {
                    using EngineT = std::decay_t<decltype(engine_concr)>;
                    using LoggerT = std::decay_t<decltype(logger_concr)>;
                    using MetricT = std::decay_t<decltype(metric_concr)>;

                    std::vector<MetricT> metrics;
                    Protocol::Packet* packet = nullptr;

                    while (true) {
                        Worker::dequeue(*to_consumer, packet);

                        if (packet->isShutDown) {
                            Worker::enqueue(*to_producer, packet);
                            break;
                        }

                        engine_concr.template submit<MetricT>(packet->request, metrics);
                        Worker::enqueue(*to_producer, packet);

                        if constexpr (!std::is_same_v<MetricT, std::monostate>) {
                            if (metrics.size() >= METRICS_BATCH_SIZE) {
                                for (const auto& metric : metrics) {
                                    logger_concr.info(metric);
                                }
                                metrics.clear();
                            }
                        }
                    }

                    if constexpr (
                        std::is_same_v<EngineT, Engine::UringEngine> ||
                        std::is_same_v<EngineT, Engine::AioEngine>
                    ) {
                        engine_concr.template reap_left_completions<MetricT>(metrics);
                    }

                    for (const auto& metric : metrics) {
                        Metric::log_metric<MetricT, LoggerT>(logger_concr, metric);
                    }
                }, *engine, *logger, *metric);
            }
    };
};

#endif

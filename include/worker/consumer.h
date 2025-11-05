#ifndef CONSUMER_WORKER_H
#define CONSUMER_WORKER_H

#include <io/protocol.h>
#include <lib/readerwriterqueue/readerwriterqueue.h>
#include <boost/pool/singleton_pool.hpp>

#define METRICS_BATCH_SIZE 1000

namespace Worker {

    template<
        typename LoggerT,
        typename EngineT,
        typename MetricT>
    class Consumer {
        private:
            LoggerT& logger;
            EngineT& engine;
            std::shared_ptr<BufferPool> buffer_pool;
            std::shared_ptr<moodycamel:ReaderWriterQueue<Protocol::CommonRequestPacket>> queue;

        public:
            Consumer(
                LoggerT& _logger,
                EngineT& _engine,
                std::shared_ptr<BufferPoll> _buffer_poll
                std::shared_ptr<moodycamel::ReaderWriterQueue<Protocol::CommonRequestPacket>> _queue
            ) :
                logger(_logger),
                engine(_engine),
                buffer_pool(_buffer_pool),
                queue(_queue) {}

            void run(void) {
                std::vector<MetricT> metrics{};
                metrics.reserve(METRICS_BATCH_SIZE);

                while (true) {
                    Protocol::CommonRequestPacket packet;

                    if (queue->try_dequeue(packet)) {
                        if (packet.isShutDown) {
                            break;
                        }

                        engine.template submit<MetricT>(packet.request, metrics);

                        if (packet.request.operation == Operation::OperationType::WRITE) {
                            buffer_pool->free(packet.request.buffer);
                        }

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
            }
    };
};

#endif

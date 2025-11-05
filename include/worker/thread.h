#ifndef WORKER_THREAD_H
#define WORKER_THREAD_H

#include <parser/parser.h>
#include <operation/barrier.h>
#include <boost/thread.hpp>
#include <lib/readerwriterqueue/readerwriterqueue.h>

using namespace moodycamel;

namespace Worker {

    boost::thread start_producer(
        uint64_t iterations,
        Parser::AccessVariant& access,
        Parser::OperationVariant& operation,
        Parser::GeneratorVariant& generator,
        Operation::MultipleBarrier& barrier,
        std::shared_ptr<ReaderWriterQueue<Protocol::Packet>> to_producer,
        std::shared_ptr<ReaderWriterQueue<Protocol::Packet>> to_consumer
    ) {
        return std::visit(
            [&](auto& acc, auto& op, auto& gen) -> boost::thread {
                using AccessT    = std::decay_t<decltype(acc)>;
                using OperationT = std::decay_t<decltype(op)>;
                using GeneratorT = std::decay_t<decltype(gen)>;
                using ProducerT = Worker::Producer<AccessT, OperationT, GeneratorT>;

                auto producer = std::make_shared<ProducerT>(
                    acc,
                    op,
                    gen,
                    barrier,
                    to_producer,
                    to_consumer
                );

                return boost::thread([producer, iterations]() {
                    producer->run(iterations);
                });
            },
            access, operation, generator
        );
    };

    boost::thread start_consumer(
        Parser::MetricVariant& metric,
        Parser::LoggerVariant& logger,
        Parser::EngineVariant& engine,
        std::shared_ptr<ReaderWriterQueue<Protocol::Packet>> to_producer,
        std::shared_ptr<ReaderWriterQueue<Protocol::Packet>> to_consumer
    ) {
        return std::visit(
            [&](auto& met, auto& log, auto& eng) -> boost::thread {
                using MetricT = std::decay_t<decltype(met)>;
                using LoggerT = std::decay_t<decltype(log)>;
                using EngineT = std::decay_t<decltype(eng)>;
                using ConsumerT = Worker::Consumer<EngineT, LoggerT, MetricT>;

                auto consumer = std::make_shared<ConsumerT>(
                    eng,
                    log,
                    to_producer,
                    to_consumer
                );

                return boost::thread([consumer]() {
                    consumer->run();
                });
            },
            metric, logger, engine
        );
    };
};

#endif
#ifndef PRODUCER_WORKER_H
#define PRODUCER_WORKER_H

#include <worker/utils.h>

using namespace moodycamel;

namespace Worker {

    template <
        typename AccessT,
        typename OperationT,
        typename GeneratorT>
    class Producer {
        private:
            AccessT& access;
            OperationT& operation;
            GeneratorT& generator;
            Operation::MultipleBarrier& barrier;
            std::shared_ptr<ReaderWriterQueue<Protocol::Packet>> to_producer;
            std::shared_ptr<ReaderWriterQueue<Protocol::Packet>> to_consumer;

        public:
            Producer(
                AccessT& _access,
                OperationT& _operation,
                GeneratorT& _generator,
                std::shared_ptr<ReaderWriterQueue<Protocol::Packet>> _to_producer,
                std::shared_ptr<ReaderWriterQueue<Protocol::Packet>> _to_consumer
            ) :
                access(_access),
                operation(_operation),
                generator(_generator),
                barrier(_barrier),
                to_producer(_to_producer),
                to_consumer(_to_consumer) {}

            void run(uint64_t iterations) {
                Protocol::Packet packet;

                for (uint64_t iter = 0; i < iterations; iter++) {
                    Worker::dequeue(*to_producer, packet);

                    packet.isShutDown = false;
                    packet.request.offset = access.nextOffset();
                    packet.operation = barrier.apply(operation.nextOperation());

                    if (request.operation == Operation::OperationType::WRITE) {
                        generator.nextBlock(packet.request.buffer, packet.request.size);
                    }

                    Worker::enqueue(*to_consumer, packet);
                }

                Worker::dequeue(*to_producer, packet);
                packet.isShutDown = true;

                Worker::enqueue(*to_consumer, packet);
            }
    };
};

#endif
#ifndef PRODUCER_WORKER_H
#define PRODUCER_WORKER_H

#include <access/synthetic.h>
#include <generator/synthetic.h>
#include <operation/synthetic.h>
#include <operation/barrier.h>
#include <worker/utils.h>

namespace Worker {

    class Producer {
        private:
            std::unique_ptr<Access::Access> access;
            std::unique_ptr<Operation::Operation> operation;
            std::unique_ptr<Generator::Generator> generator;
            std::unique_ptr<Operation::MultipleBarrier> barrier;
            std::shared_ptr<ReaderWriterQueue<Protocol::Packet*>> to_producer;
            std::shared_ptr<ReaderWriterQueue<Protocol::Packet*>> to_consumer;

        public:
            Producer(
                std::unique_ptr<Access::Access> _access,
                std::unique_ptr<Operation::Operation> _operation,
                std::unique_ptr<Generator::Generator> _generator,
                std::unique_ptr<Operation::MultipleBarrier> _barrier,
                std::shared_ptr<ReaderWriterQueue<Protocol::Packet*>> _to_producer,
                std::shared_ptr<ReaderWriterQueue<Protocol::Packet*>> _to_consumer
            ) :
                access(std::move(_access)),
                operation(std::move(_operation)),
                generator(std::move(_generator)),
                barrier(std::move(_barrier)),
                to_producer(_to_producer),
                to_consumer(_to_consumer) {}

            void run(uint64_t iterations, int fd) {
                Protocol::Packet* packet = nullptr;

                for (uint64_t iter = 0; iter < iterations; iter++) {
                    Worker::dequeue(*to_producer, packet);
                    packet->request.fd = fd;
                    packet->request.offset = access->nextOffset();
                    packet->request.operation = barrier->apply(operation->nextOperation());

                    if (packet->request.operation == Operation::OperationType::WRITE) {
                        generator->nextBlock(packet->request.buffer, packet->request.size);
                    }

                    Worker::enqueue(*to_consumer, packet);
                }

                Worker::dequeue(*to_producer, packet);
                packet->isShutDown = true;
                Worker::enqueue(*to_consumer, packet);
            }
    };
};

#endif
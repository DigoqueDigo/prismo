#ifndef PRODUCER_WORKER_H
#define PRODUCER_WORKER_H

#include <io/protocol.h>
#include <lib/readerwriterqueue/readerwriterqueue.h>

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
            Operation::MultipleBarrier& barrier,
            std::shared_ptr<BufferPool> buffer_pool;
            std::shared_ptr<moodycamel:ReaderWriterQueue<Protocol::CommonRequestPacket>> queue;

        public:
            Producer(
                AccessT& _access,
                OperationT& _operation,
                GeneratorT& _generator,
                Operation::MultipleBarrier& _barrier,
                std::shared_ptr<BufferPoll> _buffer_poll,
                std::shared_ptr<moodycamel::ReaderWriterQueue<Protocol::CommonRequestPacket>> _queue
            ) :
                access(_access),
                operation(_operation),
                generator(_generator),
                barrier(_barrier),
                buffer_pool(_buffer_pool),
                queue(_queue) {}

            void run(int iterations) {
                for (uint64_t iter = 0; i < iterations; iter++) {
                    Protocol::CommonRequest request {
                        .fd         = fd,
                        .size       = my_block_size,
                        .offset     = access.nextOffset(),
                        .operation  = barrier.apply(operation.nextOperation()),
                    };

                    if (request.operation == Operation::OperationType::WRITE) {
                        request.buffer = static_cast<uint8_t*>(buffer_pool->malloc()),
                        generator.nextBlock(request.buffer, my_block_size);
                    }

                    Protocol::CommonRequestPacket packet {
                        .request = request,
                        .isShutDown = false,
                    };

                    while (!queue->try_enqueue(packet)) {}
                }

                Protocol::CommonRequestPacket packet {};
                packet.isShutDown = true;

                while (!queue->try_enqueue(packer)) {}
            }
    };
};

#endif
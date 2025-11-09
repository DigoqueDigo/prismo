#ifndef WORKER_UTILS_H
#define WORKER_UTILS_H

#include <io/protocol.h>
#include <lib/readerwriterqueue/readerwriterqueue.h>

#define QUEUE_INITIAL_CAPACITY 128
using namespace moodycamel;

namespace Worker {

    void enqueue(ReaderWriterQueue<Protocol::Packet>& queue, Protocol::Packet& packet) {
        while (!queue.try_enqueue(packet)) {}
    };

    void dequeue(ReaderWriterQueue<Protocol::Packet>& queue, Protocol::Packet& packet) {
        while (!queue.try_dequeue(packet)) {}
    };

    void init_queue_packet(ReaderWriterQueue<Protocol::Packet>& queue, size_t block_size) {
        for (int index = 0; index < QUEUE_INITIAL_CAPACITY; index++) {
            Protocol::Packet packet {
                .isShutDown = false,
                .request {
                    .fd = 0,
                    .size = block_size,
                    .offset = 0,
                    .buffer = static_cast<uint8_t*>(std::malloc(block_size)),
                    .operation = Operation::OperationType::NOP,
                }
            };

            if (!packet.request.buffer) {
                throw std::bad_alloc();
            }

            enqueue(queue, packet);
        }
    }

    void destroy_queue_packet(ReaderWriterQueue<Protocol::Packet>& queue) {
        Protocol::Packet packet;
        while (queue.try_dequeue(packet)) {
            std::free(packet.request.buffer);
        }
    }

};

#endif
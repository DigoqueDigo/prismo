#ifndef WORKER_UTILS_H
#define WORKER_UTILS_H

#include <io/protocol.h>
#include <lib/readerwriterqueue/readerwriterqueue.h>

#define QUEUE_INITIAL_CAPACITY 128
using namespace moodycamel;

namespace Worker {

    void init_queue_packet(ReaderWriterQueue<Protocol::Packet>& queue, size_t block_size) {
        for (int index = 0; index < QUEUE_INITIAL_CAPACITY; index++) {
            Protocol::Packet packet {
                .isShutDown = false,
                .request{}
            };

            packet.request.size = block_size;
            packet.request.buffer = static_cast<uint8_t*>(std::malloc(block_size));

            if (!packet.request.buffer) {
                throw std::bad_alloc();
            }
        }
    }

    void destroy_queue_packet(ReaderWriterQueue<Protocol::Packet>& queue) {
        Protocol::Packet packet;
        while (queue.try_dequeue(packet)) {
            std::free(packet.request.buffer);
        }
    }

    void enqueue(ReaderWriterQueue<Protocol::Packet>& queue, Protocol::Packet& packet) {
        while (!queue.try_enqueue(packet)) {}
    };

    void dequeue(ReaderWriterQueue<Protocol::Packet>& queue, Protocol::Packet& packet) {
        while (!queue.try_dequeue(packet)) {}
    };
};

#endif
#ifndef WORKER_UTILS_H
#define WORKER_UTILS_H

#include <iostream>
#include <io/protocol.h>
#include <lib/readerwriterqueue/readerwriterqueue.h>

#define QUEUE_INITIAL_CAPACITY 128
using namespace moodycamel;

namespace Worker {

    void enqueue(ReaderWriterQueue<Protocol::Packet*>& queue, Protocol::Packet*& packet) {
        while (!queue.try_enqueue(packet)) {}
    };

    void dequeue(ReaderWriterQueue<Protocol::Packet*>& queue, Protocol::Packet*& packet) {
        while (!queue.try_dequeue(packet)) {}
    };

    void init_queue_packet(ReaderWriterQueue<Protocol::Packet*>& queue, size_t block_size) {
        for (int index = 0; index < QUEUE_INITIAL_CAPACITY; index++) {
            Protocol::Packet* packet = static_cast<Protocol::Packet*>(std::malloc(sizeof(Protocol::Packet)));

            if (!packet) {
                throw std::bad_alloc();
            }

            packet->isShutDown = false;
            packet->request.fd = 0;
            packet->request.offset = 0;
            packet->request.size = block_size;
            packet->request.operation = Operation::OperationType::NOP;
            packet->request.buffer = static_cast<uint8_t*>(std::malloc(block_size));

            if (!packet->request.buffer) {
                throw std::bad_alloc();
            }

            Worker::enqueue(queue, packet);
        }
    }

    void destroy_queue_packet(ReaderWriterQueue<Protocol::Packet*>& queue) {
        Protocol::Packet* packet;
        for (int index = 0; index < QUEUE_INITIAL_CAPACITY; index++) {
            Worker::dequeue(queue, packet);
            std::free(packet->request.buffer);
            std::free(packet);
        }
    }
};

#endif
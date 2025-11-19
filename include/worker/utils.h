#ifndef WORKER_UTILS_H
#define WORKER_UTILS_H

#include <iostream>
#include <io/protocol.h>
#include <lib/readerwriterqueue/readerwritercircularbuffer.h>

#define QUEUE_INITIAL_CAPACITY 1024

using namespace moodycamel;

namespace Worker {

    inline void enqueue(BlockingReaderWriterCircularBuffer<Protocol::Packet*>& queue, Protocol::Packet*& packet) {
        queue.wait_enqueue(packet);
    };

    inline void dequeue(BlockingReaderWriterCircularBuffer<Protocol::Packet*>& queue, Protocol::Packet*& packet) {
        queue.wait_dequeue(packet);
    };

    inline void enqueue(BlockingReaderWriterCircularBuffer<Protocol::CommonRequest*>& queue, Protocol::CommonRequest*& request) {
        queue.wait_enqueue(request);
    };

    inline void dequeue(BlockingReaderWriterCircularBuffer<Protocol::CommonRequest*>& queue, Protocol::CommonRequest*& request) {
        queue.wait_dequeue(request);
    };


    inline void init_queue_packet(BlockingReaderWriterCircularBuffer<Protocol::Packet*>& queue, size_t block_size) {
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

    inline void destroy_queue_packet(BlockingReaderWriterCircularBuffer<Protocol::Packet*>& queue) {
        Protocol::Packet* packet;
        for (int index = 0; index < QUEUE_INITIAL_CAPACITY; index++) {
            Worker::dequeue(queue, packet);
            std::free(packet->request.buffer);
            std::free(packet);
        }
    }

    inline void pin_thread(std::thread &t, int cpu_id) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(cpu_id, &cpuset);

        int rc = pthread_setaffinity_np(t.native_handle(), sizeof(cpu_set_t), &cpuset);
        if (rc)
            throw std::runtime_error("Error setting thread affinity: " + std::to_string(rc));

        rc = pthread_getaffinity_np(t.native_handle(), sizeof(cpuset), &cpuset);
        if (rc)
            throw std::runtime_error("Error getting thread affinity: " + std::to_string(rc));

        if (!CPU_ISSET(cpu_id, &cpuset))
            throw std::runtime_error("Thread not pinned to CPU " + std::to_string(cpu_id));
    }
};

#endif
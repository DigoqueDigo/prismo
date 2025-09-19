#ifndef BACKEND_ENGINE_CONFIG_H
#define BACKEND_ENGINE_CONFIG_H

namespace BackendEngineConfig {
    struct IOUringConfig {
        size_t batch_size = 1;
        size_t block_size = 4096;
        unsigned int queue_depth = 128;
        unsigned int ring_flags = 0;

        explicit IOUringConfig(
            size_t _batch_size,
            size_t _block_size,
            unsigned int _queue_depth,
            unsigned int _ring_flags
        ) : batch_size(_batch_size),
            block_size(_block_size),
            queue_depth(_queue_depth),
            ring_flags(_ring_flags) {}
    };
};


#endif
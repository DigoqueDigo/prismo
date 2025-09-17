#ifndef BACKEND_ENGINE_CONFIG_H
#define BACKEND_ENGINE_CONFIG_H

namespace BackendEngineConfig {
    struct IOUringConfig {
        unsigned int queue_depth = 128;
        unsigned int ring_flags = 0;

        explicit IOUringConfig(
            unsigned int _queue_depth,
            unsigned int _ring_flags
        ) : queue_depth(_queue_depth),
            ring_flags(_ring_flags) {}
    };
};


#endif
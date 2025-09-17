#ifndef BACKEND_ENGINE_CONFIG_H
#define BACKEND_ENGINE_CONFIG_H

namespace BackendEngineConfig {
    struct IOUringConfig {
        unsigned int queue_depth = 128;
        unsigned int flags = 0;
        unsigned int sq_thread_cpu = 0;
        unsigned int sq_thread_idle = 100;

        explicit IOUringConfig(
            unsigned int _queue_depth,
            unsigned int _flags,
            unsigned int _sq_thread_cpu,
            unsigned int _sq_thread_idle
        ) : queue_depth(_queue_depth),
            flags(_flags),
            sq_thread_cpu(_sq_thread_cpu),
            sq_thread_idle(_sq_thread_idle) {}
    };
};


#endif
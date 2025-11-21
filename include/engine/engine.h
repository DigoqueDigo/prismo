#ifndef ENGINE_H
#define ENGINE_H

#include <io/metric.h>
#include <io/protocol.h>
#include <logger/logger.h>

namespace Engine {

    class Engine {
        protected:
            std::unique_ptr<Metric::Metric> metric;
            std::unique_ptr<Logger::Logger> logger;

        public:
            Engine(
                std::unique_ptr<Metric::Metric> _metric,
                std::unique_ptr<Logger::Logger> _logger
            ) :
                metric(std::move(_metric)),
                logger(std::move(_logger)) {}

            virtual ~Engine() {
                // std::cout << "~Destroying Engine" << std::endl;
            }

            virtual int open(Protocol::OpenRequest& request) = 0;
            virtual int close(Protocol::CloseRequest& request) = 0;
            virtual void submit(Protocol::CommonRequest& request) = 0;
            virtual void reap_left_completions(void) = 0;
    };
};

#endif
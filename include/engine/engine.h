#ifndef ENGINE_H
#define ENGINE_H

#include <io/metric.h>
#include <io/protocol.h>
#include <logger/logger.h>

namespace Engine {

    class Engine {
        protected:
            Metric::MetricType metric_type;
            std::unique_ptr<Metric::NoneMetric> metric;
            std::unique_ptr<Logger::Logger> logger;

        public:
            Engine(
                Metric::MetricType _metric_type,
                std::unique_ptr<Logger::Logger> _logger
            ) :
                metric_type(_metric_type),
                logger(std::move(_logger))
            {
                metric_type = _metric_type;
                if (metric_type == Metric::MetricType::Base) {
                    metric = std::make_unique<Metric::BaseMetric>();
                } else if (metric_type == Metric::MetricType::Standard) {
                    metric = std::make_unique<Metric::StandardMetric>();
                } else if (metric_type == Metric::MetricType::Full) {
                    metric = std::make_unique<Metric::FullMetric>();
                } else {
                    throw std::runtime_error("Invalid metric type");
                }
            }

            virtual ~Engine() {
                std::cout << "~Destroying Engine" << std::endl;
            }

            virtual int open(Protocol::OpenRequest& request) = 0;
            virtual int close(Protocol::CloseRequest& request) = 0;
            virtual void submit(Protocol::CommonRequest& request) = 0;
            virtual void reap_left_completions(void) = 0;
    };
};

#endif
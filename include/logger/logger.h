#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <io/metric.h>

namespace Logger {

    class Logger {
        public:
            Logger() = default;
            virtual ~Logger() {
                // std::cout << "~Destroying Logger" << std::endl;
            }

            virtual void info(
                Metric::MetricType type,
                Metric::NoneMetric& metric
            ) = 0;
    };
};

#endif
#include <operation/barrier.h>

namespace Operation {

    BarrierCounter::BarrierCounter(OperationType _barrierOp, OperationType _triggerOp, uint64_t _everyN)
        : barrierOp(_barrierOp), triggerOp(_triggerOp), everyN(_everyN), counter(0) {}

    OperationType BarrierCounter::apply(OperationType operation) {
        if (operation == triggerOp && counter++ == everyN) {
            counter = 0;
            return barrierOp;
        }
        return operation;
    }

    void MultipleBarrier::addBarrier(OperationType barrierOp, OperationType triggerOp, uint64_t everyN) {
        barriers.emplace_back(barrierOp, triggerOp, everyN);
    }

    OperationType MultipleBarrier::apply(OperationType operation) {
        for (auto& barrier : barriers) {
            operation = barrier.apply(operation);
        }
        return operation;
    }

    void from_json(const json& j, MultipleBarrier& config) {
        for (const auto& barrier : j) {
            std::string operation = barrier.at("operation").template get<std::string>();
            std::string trigger = barrier.at("trigger").template get<std::string>();
            uint64_t counter = barrier.at("counter").template get<uint64_t>();

            config.addBarrier(
                operation_from_str(operation),
                operation_from_str(trigger),
                counter
            );
        }
    }
};
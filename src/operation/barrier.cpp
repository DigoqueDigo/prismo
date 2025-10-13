#include <operation/barrier.h>

namespace Operation {

    BarrierCounter::BarrierCounter(OperationType _barrierOp, OperationType _triggerOp, size_t _everyN)
        : barrierOp(_barrierOp), triggerOp(_triggerOp), everyN(_everyN), counter(0) {}

    OperationType BarrierCounter::apply(OperationType operation) {
        if (operation == triggerOp && counter++ == everyN) {
            counter = 0;
            return barrierOp;
        }
        return operation;
    }

    void MultipleBarrier::addBarrier(OperationType barrierOp, OperationType triggerOp, size_t everyN) {
        barriers.emplace_back(barrierOp, triggerOp, everyN);
    }

    OperationType MultipleBarrier::apply(OperationType operation) {
        for (auto& barrier : barriers) {
            operation = barrier.apply(operation);
        }
        return operation;
    }

    void from_json(const json& j, MultipleBarrier& barrier) {
        for (const auto& element : j.items()) {
            const std::string& key = element.key();
            const size_t barrier_value = element.value();
            if (barrier_value > 0) {
                auto it = barrier_types.find(key);
                if (it != barrier_types.end()) {
                    barrier.addBarrier(it->second, OperationType::WRITE, barrier_value);
                } else {
                    throw std::invalid_argument("Barrier type: '" + key + "' is not recognized");
                }
            }
        }
    }
};
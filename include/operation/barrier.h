#ifndef OPERATION_BARRIER_H
#define OPERATION_BARRIER_H

#include <vector>
#include <operation/type.h>

namespace Operation {
    struct BarrierCounter {
        OperationType barrierOp;
        OperationType triggerOp;
        size_t everyN;
        size_t counter = 0;

        BarrierCounter(OperationType _barrierOp, OperationType _triggerOp, size_t _everyN)
            : barrierOp(_barrierOp), triggerOp(_triggerOp), everyN(_everyN), counter(0) {}

        OperationType apply(OperationType operation) {
            if (operation == triggerOp && ++counter >= everyN) {
                counter = 0;
                return barrierOp;
            }
            return operation;
        }
    };

    struct MultipleBarrier {
        std::vector<BarrierCounter> barriers;

        explicit MultipleBarrier() = default;

        void addBarrier(OperationType barrierOp, OperationType triggerOp, size_t everyN) {
            barriers.emplace_back(barrierOp, triggerOp, everyN);
        }

        OperationType apply(OperationType operation) {
            for (auto& barrier : barriers) {
                operation = barrier.apply(operation);
            }
            return operation;
        }
    };
};

#endif
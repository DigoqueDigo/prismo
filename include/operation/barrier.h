#ifndef OPERATION_BARRIER_H
#define OPERATION_BARRIER_H

#include <vector>
#include <operation/type.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Operation {

    class BarrierCounter {
        private:
            OperationType barrierOp;
            OperationType triggerOp;
            uint64_t everyN;
            uint64_t counter = 0;

        public:
            BarrierCounter(OperationType _barrierOp, OperationType _triggerOp, size_t _everyN);
            OperationType apply(OperationType operation);
    };

    class MultipleBarrier {
        private:
            std::vector<BarrierCounter> barriers;

        public:
            explicit MultipleBarrier() = default;
            void addBarrier(OperationType barrierOp, OperationType triggerOp, size_t everyN);
            OperationType apply(OperationType operation);
    };

    void from_json(const json& j, MultipleBarrier& barrier);
};

#endif
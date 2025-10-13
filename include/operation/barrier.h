#ifndef OPERATION_BARRIER_H
#define OPERATION_BARRIER_H

#include <vector>
#include <operation/type.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Operation {

    struct BarrierCounter {
        private:
            OperationType barrierOp;
            OperationType triggerOp;
            size_t everyN;
            size_t counter = 0;

        public:
            BarrierCounter(OperationType _barrierOp, OperationType _triggerOp, size_t _everyN);
            OperationType apply(OperationType operation);
    };

    struct MultipleBarrier {
        private:
            std::vector<BarrierCounter> barriers;

        public:
            explicit MultipleBarrier() = default;
            void addBarrier(OperationType barrierOp, OperationType triggerOp, size_t everyN);
            OperationType apply(OperationType operation);
    };

    inline const std::unordered_map<std::string, OperationType> barrier_types = {
        {"fsync", OperationType::FSYNC},
        {"fdatasync", OperationType::FDATASYNC}
    };

    void from_json(const json& j, MultipleBarrier& barrier);
};

#endif
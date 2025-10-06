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
            BarrierCounter(OperationType _barrierOp, OperationType _triggerOp, size_t _everyN)
                : barrierOp(_barrierOp), triggerOp(_triggerOp), everyN(_everyN), counter(0) {}

            OperationType apply(OperationType operation) {
                if (operation == triggerOp && counter++ == everyN) {
                    counter = 0;
                    return barrierOp;
                }
                return operation;
            }
    };

    struct MultipleBarrier {
        private:
            std::vector<BarrierCounter> barriers;

        public:
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

    void from_json(const json& j, MultipleBarrier& barrier) {
        static const std::unordered_map<std::string, OperationType> barrier_types = {
            {"fsync", OperationType::FSYNC},
            {"fdatasync", OperationType::FDATASYNC}
        };

        for (const auto& element : j.items()) {
            const std::string& key = element.key();
            const size_t value = element.value();

            if (value > 0) {
                auto it = barrier_types.find(key);
                if (it != barrier_types.end()) {
                    barrier.addBarrier(it->second, OperationType::WRITE, value);
                } else {
                    throw std::invalid_argument("Barrier type: '" + key + "' is not recognized");
                }
            }
        }
    }
};

#endif
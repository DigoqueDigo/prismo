#ifndef SYNTHETIC_OPERATION_H
#define SYNTHETIC_GENERATOR_H

#include <operation/type.h>
#include <nlohmann/json.hpp>
#include <lib/distribution/distribution.h>

using json = nlohmann::json;

namespace Operation {

    class Operation {
        public:
            Operation() = default;
            virtual ~Operation() = default;
            virtual OperationType nextOperation(void) = 0;
    };

    class ConstantOperation : public Operation {
        private:
            OperationType operation;

        public:
            ConstantOperation();
            OperationType nextOperation(void);
            friend void from_json(const json& j, ConstantOperation& config);
    };

    class PercentageOperation : public Operation {
        private:
            std::vector<std::pair<OperationType, uint32_t>> percentages;
            Distribution::UniformDistribution<uint32_t> distribution;

        public:
            PercentageOperation();
            OperationType nextOperation(void);
            friend void from_json(const json& j, PercentageOperation& config);
    };

    class SequenceOperation : public Operation {
        private:
            size_t index;
            size_t length;
            std::vector<OperationType> operations;

        public:
            SequenceOperation();
            OperationType nextOperation(void);
            void validate(void) const;
            friend void from_json(const json& j, SequenceOperation& config);
    };

    void from_json(const json& j, ConstantOperation& config);
    void from_json(const json& j, PercentageOperation& config);
    void from_json(const json& j, SequenceOperation& config);
};

#endif
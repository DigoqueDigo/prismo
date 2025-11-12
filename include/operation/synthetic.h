#ifndef SYNTHETIC_OPERATION_H
#define SYNTHETIC_OPERATION_H

#include <operation/type.h>
#include <nlohmann/json.hpp>
#include <lib/distribution/distribution.h>
#include <iostream>
#include <vector>

using json = nlohmann::json;

namespace Operation {

    class Operation {
        public:
            Operation() = default;

            virtual ~Operation() {
                // std::cout << "~Destroying Operation" << std::endl;
            }

            virtual OperationType nextOperation(void) = 0;
    };

    class ConstantOperation : public Operation {
        private:
            OperationType operation;

        public:
            ConstantOperation();

            ~ConstantOperation() override {
                // std::cout << "~Destroying ConstantOperation" << std::endl;
            }

            OperationType nextOperation(void);
            friend void from_json(const json& j, ConstantOperation& config);
    };

    class PercentageOperation : public Operation {
        private:
            std::vector<std::pair<OperationType, uint32_t>> percentages;
            Distribution::UniformDistribution<uint32_t> distribution;

        public:
            PercentageOperation();

            ~PercentageOperation() override {
                // std::cout << "~Destroying PercentageOperation" << std::endl;
            }

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

            ~SequenceOperation() override {
                // std::cout << "~Destroying SequenceOperation" << std::endl;
            }

            OperationType nextOperation(void);
            void validate(void) const;
            friend void from_json(const json& j, SequenceOperation& config);
    };

    void from_json(const json& j, ConstantOperation& config);
    void from_json(const json& j, PercentageOperation& config);
    void from_json(const json& j, SequenceOperation& config);
};

#endif
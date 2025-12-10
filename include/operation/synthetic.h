#ifndef SYNTHETIC_OPERATION_H
#define SYNTHETIC_OPERATION_H

#include <vector>
#include <iostream>
#include <operation/type.h>
#include <nlohmann/json.hpp>
#include <lib/distribution/distribution.h>
#include <lib/distribution/percentage.h>

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
            friend void from_json(const json& j, ConstantOperation& op_generator);
    };

    class PercentageOperation : public Operation {
        private:
            std::vector<PercentageElement<uint32_t, OperationType>> op_percentages;
            Distribution::UniformDistribution<uint32_t> distribution;

        public:
            PercentageOperation();

            ~PercentageOperation() override {
                // std::cout << "~Destroying PercentageOperation" << std::endl;
            }

            void validate(void) const;
            OperationType nextOperation(void);
            friend void from_json(const json& j, PercentageOperation& op_generator);
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

            void validate(void) const;
            OperationType nextOperation(void);
            friend void from_json(const json& j, SequenceOperation& op_generator);
    };

    void from_json(const json& j, ConstantOperation& op_generator);
    void from_json(const json& j, PercentageOperation& op_generator);
    void from_json(const json& j, SequenceOperation& op_generator);
};

#endif
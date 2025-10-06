#ifndef OPERATION_PARSER_H
#define OPERATION_PARSER_H

#include <variant>
#include <stdexcept>
#include <functional>
#include <unordered_map>
#include <operation/constant.h>
#include <operation/percentage.h>
#include <operation/sequence.h>

namespace Parser {
    using OperationVariant = std::variant<
        Operation::ConstantOperation,
        Operation::PercentageOperation,
        Operation::SequenceOperation>;

    inline static const std::unordered_map<
        std::string,
        std::function<OperationVariant(const json& specialized)>>
    operation__variant_map = {
        {"constant", [](const json& specialized) {
            auto config = specialized.template get<Operation::ConstantOperationConfig>();
            return Operation::ConstantOperation(config);
        }},
        {"percentage", [](const json& specialized) {
            auto config = specialized.template get<Operation::PercentageOperationConfig>();
            return Operation::PercentageOperation(config);
        }},
        {"mixed", [](const json& specialized) {
            auto config = specialized.template get<Operation::SequenceOperationConfig>();
            return Operation::SequenceOperation(config);
        }}
    };

    OperationVariant getOperation(const std::string& type, const json& specialized) {
        auto func = operation__variant_map.at(type);
        return func(specialized);
    }
};

#endif

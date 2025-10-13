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
    operation_variant_map = {
        {"constant", [](const json& specialized) {
            auto operation = specialized.template get<Operation::ConstantOperation>();
            return OperationVariant { std::in_place_type<Operation::ConstantOperation>, std::move(operation) };
        }},
        {"percentage", [](const json& specialized) {
            auto operation = specialized.template get<Operation::PercentageOperation>();
            return OperationVariant { std::in_place_type<Operation::PercentageOperation>, std::move(operation) };
        }},
        {"sequence", [](const json& specialized) {
            auto operation = specialized.template get<Operation::SequenceOperation>();
            return OperationVariant { std::in_place_type<Operation::SequenceOperation>, std::move(operation) };
        }}
    };

    OperationVariant getOperation(const json& specialized) {
        std::string type = specialized.at("type").template get<std::string>();
        auto it = operation_variant_map.find(type);
        if (it != operation_variant_map.end()) {
            return it->second(specialized);
        } else {
            throw std::invalid_argument("Operation type '" + type + "' not recognized");
        }
    }
};

#endif

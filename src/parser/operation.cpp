#include <parser/parser.h>

namespace Parser {

    static const std::unordered_map<
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

    OperationVariant getOperationVariant(const json& specialized) {
        std::string type = specialized.at("type").template get<std::string>();
        auto it = operation_variant_map.find(type);
        if (it != operation_variant_map.end()) {
            return it->second(specialized);
        } else {
            throw std::invalid_argument("Operation type '" + type + "' not recognized");
        }
    }
}
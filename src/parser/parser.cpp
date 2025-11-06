#include <parser/parser.h>

namespace Parser {

    std::unique_ptr<Access::Access> getAccess(const json& config) {
        std::string type = config.at("type").get<std::string>();

        if (type == "sequential") {
            return std::make_unique<Access::SequentialAccess>(
                config.get<Access::SequentialAccess>()
            );
        } else if (type == "random") {
            return std::make_unique<Access::RandomAccess>(
                config.get<Access::RandomAccess>()
            );
        } else if (type == "zipfian") {
            return std::make_unique<Access::ZipfianAccess>(
                config.get<Access::ZipfianAccess>()
            );
        } else {
            throw std::invalid_argument("Access type '" + type + "' not recognized");
        }
    }

    std::unique_ptr<Generator::Generator> getGenerator(const json& config) {
        std::string type = config.at("type").get<std::string>();

        if (type == "constant") {
            return std::make_unique<Generator::ConstantGenerator>();
        } else if (type == "random") {
            return std::make_unique<Generator::RandomGenerator>();
        } else {

        }
    }

    std::unique_ptr<Operation::Operation> getOperation(const json& config) {
        std::string type = config.at("type").get<std::string>();

        if (type == "constant") {
            return std::make_unique<Operation::ConstantOperation>(
                config.get<Operation::ConstantOperation>()
            );
        } else if (type == "percentage") {
            return std::make_unique<Operation::PercentageOperation>(
                config.get<Operation::PercentageOperation>()
            );
        } else if (type == "sequential") {
            return std::make_unique<Operation::SequenceOperation>(
                config.get<Operation::SequenceOperation>()
            );
        } else {
            throw std::invalid_argument("Operation type '" + type + "' not recognized");
        }
    }
}
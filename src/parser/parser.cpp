#include <parser/parser.h>

namespace Parser {

    std::unique_ptr<Access::Access> getAccess(const json& specialized) {
        std::string type = specialized.at("type").get<std::string>();

        if (type == "sequential") {
            return std::make_unique<Access::SequentialAccess>(
                specialized.get<Access::SequentialAccess>()
            );
        }
        else if (type == "random") {
            return std::make_unique<Access::RandomAccess>(
                specialized.get<Access::RandomAccess>()
            );
        }
        else if (type == "zipfian") {
            return std::make_unique<Access::ZipfianAccess>(
                specialized.get<Access::ZipfianAccess>()
            );
        }
        else {
            throw std::invalid_argument("Access type '" + type + "' not recognized");
        }
    }
}
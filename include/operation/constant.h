#ifndef CONSTANT_OPERATION_H
#define CONSTANT_OPERATION_H

#include <nlohmann/json.hpp>
#include <operation/type.h>

using json = nlohmann::json;

namespace Operation {

    class ConstantOperation {
        private:
            OperationType operation;

        public:
            ConstantOperation();
            OperationType nextOperation(void);
            friend void from_json(const json& j, ConstantOperation& config);
    };

    void from_json(const json& j, ConstantOperation& config);
};

#endif
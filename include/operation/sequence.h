#ifndef SEQUENCE_OPERATION_H
#define SEQUENCE_OPERATION_H

#include <vector>
#include <operation/type.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Operation {

    class SequenceOperation {
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

    void from_json(const json& j, SequenceOperation& config);
};

#endif
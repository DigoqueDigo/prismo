#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstddef>
#include <fcntl.h>
#include <cstdint>
#include <operation/type.h>

namespace Protocol {

    struct OpenRequest {
        const char* filename;
        int flags;
        mode_t mode;
    };

    struct CloseRequest {
        int fd;
    };

    struct CommonRequest {
        int fd;
        size_t size;
        off_t offset;
        uint8_t* buffer;
        Operation::OperationType operation;
    };
}

#endif
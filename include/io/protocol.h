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

    struct CommonRequestPacket {
        bool isShutDown;
        CommonRequest request;
    };

    struct BufferTag {};

    using BufferPool = boost::singleton_pool<
        Protocol::BufferTag,
        4096,
        boost::default_user_allocator_new_delete,
        boost::details::pool::null_mutex,
        128,
        0
    >;
}

#endif
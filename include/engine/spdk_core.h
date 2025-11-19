#ifndef SPDK_ENGINE_CORE_H
#define SPDK_ENGINE_CORE_H

#include <spdk/env.h>
#include <spdk/bdev.h>
#include <spdk/thread.h>
#include <spdk/event.h>
#include <engine/utils.h>
#include <worker/utils.h>

using namespace Engine;
using namespace moodycamel;

struct spdk_context {
    spdk_bdev* bdev;
    spdk_bdev_desc* bdev_desc;
    char *bdev_name;
    char* buffer;
    size_t buffer_size;
    std::shared_ptr<BlockingReaderWriterCircularBuffer<Protocol::CommonRequest*>>& queue;
};

struct spdk_context_t {
    spdk_io_channel* bdev_io_channel;
    Protocol::CommonRequest request;
};


int start_spdk_app(SpdkConfig& config);

static void start(void* ctx);

static void hello_bdev_event_cb(
    enum spdk_bdev_event_type type,
    struct spdk_bdev *bdev,
    void *event_ctx
);

#endif
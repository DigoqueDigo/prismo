#ifndef CONSUMER_WORKER_H
#define CONSUMER_WORKER_H

#include <engine/engine.h>
#include <worker/utils.h>
#include <parser/parser.h>

namespace Worker {

    class Consumer {
        private:
            std::unique_ptr<Engine::Engine> engine;
            std::shared_ptr<moodycamel::ConcurrentQueue<Protocol::Packet*>> to_producer;
            std::shared_ptr<moodycamel::ConcurrentQueue<Protocol::Packet*>> to_consumer;

        public:
            Consumer(
                std::unique_ptr<Engine::Engine> _engine,
                std::shared_ptr<moodycamel::ConcurrentQueue<Protocol::Packet*>> _to_producer,
                std::shared_ptr<moodycamel::ConcurrentQueue<Protocol::Packet*>> _to_consumer
            ) :
                engine(std::move(_engine)),
                to_producer(_to_producer),
                to_consumer(_to_consumer) {}

            int open(Protocol::OpenRequest& request) {
                return engine->open(request);
            }

            void close(Protocol::CloseRequest& request) {
                engine->close(request);
            }

            void run() {
                Protocol::Packet* packet = nullptr;

                while (true) {
                    while (!to_consumer->try_dequeue(packet));

                    if (packet->isShutDown) {
                        to_producer->enqueue(packet);
                        break;
                    }

                    engine->submit(packet->request);
                    to_producer->enqueue(packet);
                }

                engine->reap_left_completions();
            }
    };
};

#endif

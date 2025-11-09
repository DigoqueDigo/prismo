#include <parser/parser.h>
#include <operation/type.h>
#include <operation/barrier.h>
#include <worker/utils.h>
#include <worker/producer.h>
#include <worker/consumer.h>

#include <memory>
#include <iostream>
#include <iomanip>
#include <fstream>

int main(int argc, char** argv) {

    if (argc < 2) {
        throw std::invalid_argument("Invalid number of arguments");
    }

    std::ifstream config_file(argv[1]);
    json config_j = json::parse(config_file);

    json job_j = config_j.at("job");
    json access_j = config_j.at("access");
    json operation_j = config_j.at("operation");
    json barrier_j = operation_j.at("barrier");
    json generator_j = config_j.at("generator");
    json engine_j = config_j.at("engine");
    json logging_j = config_j.at("logging");

    access_j.merge_patch(job_j);
    engine_j.merge_patch(job_j);

    const size_t block_size = job_j.at("block_size").get<size_t>();
    const uint64_t iterations = job_j.at("iterations").get<uint64_t>();
    const std::string filename = job_j.at("filename").get<std::string>();

    Engine::OpenFlags open_flags = engine_j.at("openflags").get<Engine::OpenFlags>();

    std::cout << "Antes do access" << std::endl;
    std::unique_ptr<Access::Access> access = Parser::getAccess(access_j);

    std::cout << "Antes do operation" << std::endl;
    std::unique_ptr<Operation::Operation> operation = Parser::getOperation(operation_j);

    std::cout << "Antes do generator" << std::endl;
    std::unique_ptr<Generator::Generator> generator = Parser::getGenerator(generator_j);

    std::cout << "Antes do barrier" << std::endl;
    std::unique_ptr<Operation::MultipleBarrier> barrier = Parser::getMultipleBarrier(barrier_j);

    std::cout << "Antes do metric" << std::endl;
    std::unique_ptr<Parser::MetricVariant> metric_variant = Parser::getMetricVariant(job_j);

    std::cout << "Antes do engine" << std::endl;
    std::unique_ptr<Parser::EngineVariant> engine_variant = Parser::getEngineVariant(engine_j);

    std::cout << "Antes do logger" << std::endl;
    std::unique_ptr<Parser::LoggerVariant> logger_variant = Parser::getLoggerVariant(logging_j);

    auto to_producer = std::make_shared<ReaderWriterQueue<Protocol::Packet*>>(QUEUE_INITIAL_CAPACITY);
    auto to_consumer = std::make_shared<ReaderWriterQueue<Protocol::Packet*>>(QUEUE_INITIAL_CAPACITY);

    Worker::init_queue_packet(*to_producer, block_size);

    std::cout << "Antes do producer" << std::endl;
    Worker::Producer producer (
        std::move(access),
        std::move(operation),
        std::move(generator),
        std::move(barrier),
        to_producer,
        to_consumer
    );

    std::cout << "Antes do consumer" << std::endl;
    Worker::Consumer consumer (
        std::move(engine_variant),
        std::move(logger_variant),
        std::move(metric_variant),
        to_producer,
        to_consumer
    );

    std::cout << to_producer << std::endl;

    Protocol::OpenRequest open_request {
        .filename = filename,
        .flags = open_flags.value,
        .mode = 0666,
    };

    int fd = consumer.open(open_request);

    std::cout << "Antes de passar à thread" << std::endl;

    std::thread producer_thread(&Worker::Producer::run, &producer, iterations, fd);
    std::thread consumer_thread(&Worker::Consumer::run, &consumer);

    std::cout << "Antes do join" << std::endl;

    producer_thread.join();
    consumer_thread.join();

    Protocol::CloseRequest close_request {
        .fd = fd
    };

    std::cout << "A preparar close" << std::endl;

    consumer.close(close_request);
    std::cout << "Antes de destruir" << std::endl;
    Worker::destroy_queue_packet(*to_producer);
    std::cout << "depois de destruir" << std::endl;
    config_file.close();

    std::cout << "Antes do return" << std::endl;

    return 0;
}

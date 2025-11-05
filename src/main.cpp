#include <parser/parser.h>
#include <operation/type.h>
#include <operation/barrier.h>
#include <worker/utils.h>
#include <worker/producer.h>
#include <worker/consumer.h>
#include <boost/thread.hpp>

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
    json operation_j = config_j.at("operation");
    json access_j = config_j.at("access");
    json generator_j = config_j.at("generator");
    json engine_j = config_j.at("engine");
    json logging_j = config_j.at("logging");

    access_j.merge_patch(job_j);
    engine_j.merge_patch(job_j);

    const size_t block_size = job_j.at("block_size").template get<size_t>();
    const uint64_t iterations = job_j.at("iterations").template get<uint64_t>();
    const std::string filename = job_j.at("filename").template get<std::string>();

    Engine::OpenMode mode {.value = 0666};
    Engine::OpenFlags flags = engine_j.at("openflags").template get<Engine::OpenFlags>();
    Operation::MultipleBarrier barrier = operation_j.at("barrier").template get<Operation::MultipleBarrier>();

    Parser::AccessVariant access = Parser::getAccessVariant(access_j);
    Parser::OperationVariant operation = Parser::getOperationVariant(operation_j);
    Parser::GeneratorVariant generator = Parser::getGeneratorVariant(generator_j);

    Parser::MetricVariant metric = Parser::getMetricVariant(job_j);
    Parser::LoggerVariant logger = Parser::getLoggerVariant(logging_j);
    Parser::EngineVariant engine = Parser::getEngineVariant(engine_j);

    auto to_producer  = std::make_shared<ReaderWriterQueue<Protocol::Packet>>(QUEUE_INITIAL_CAPACITY);
    auto to_consumer  = std::make_shared<ReaderWriterQueue<Protocol::Packet>>(QUEUE_INITIAL_CAPACITY);
    Worker::init_queue_packet(*to_producer, block_size);

    





    Worker::destroy_queue_packet(*to_producer);
    config_file.close();

    return 0;
}

#include "AMQPWorker.hpp"

#include <Poco/Exception.h>
#include <amqpcpp.h>

agent::AMQPWorker::AMQPWorker(
    int _id,
    std::string _host,
    std::uint16_t _port,
    std::string _name,
    std::string _user,
    std::string _pass,
    std::string _vhost
)
    : _handler(_id, _host, _port, _name),
      _creds(_user, _pass),
      _connection(&_handler, _creds, _vhost),
      _channel(&_connection),
      _logger(nullptr)
{
    // Create the logger first
    _logger = spdlog::get(_handler.GetName());
    if (_logger == nullptr)
        _logger = spdlog::stdout_color_mt(_handler.GetName());

    // Declare the queue and exchange and bind them
    InitializeQueue();

    // Set the worker callbacks
    SetConsumerCallbacks();

    // Start the threads
    try
    {
        _handler.Run();
    }
    catch(const std::exception& e)
    {
        _logger->error("Exception caught: {}", e.what());
    }
    catch(...)
    {
        _logger->error("Anonymous exception caught");
    }
}

agent::AMQPWorker::~AMQPWorker()
{
    _channel.close();
}

void agent::AMQPWorker::InitializeQueue()
{
    _channel.declareQueue(_queue, AMQP::durable);
    _channel.declareExchange(_exchange);
    _channel.bindQueue(_exchange, _queue, _key);
}

void agent::AMQPWorker::SetConsumerCallbacks()
{
    // Set the consumer callbacks here
    _channel.consume(
            _queue,
            _key
        ).onReceived(
            [this](const AMQP::Message& message, uint64_t tag, bool redelivered) {
                auto rawmsg = std::string(message.body(), message.body() + message.bodySize());
                _logger->info("Received message:");
                _logger->info(rawmsg);
            }
        ).onComplete(
            [this](uint64_t tag, bool result) {
                _channel.ack(tag);
                _logger->info("Finished message {}", tag);
            }
        ) ;
}
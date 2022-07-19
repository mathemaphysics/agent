#include "agent/IAMQPWorkerSSL.hpp"
#include "agent/IConnectionHandler.hpp"
#include "agent/IWorker.hpp"
#include "agent/SymbolMaps.hpp"

#include <string>
#include <cstdint>

#include <amqpcpp.h>
#include <json/json.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

agent::IAMQPWorkerSSL::IAMQPWorkerSSL(
    unsigned int _id,
    IWorker *_iworker,
    std::string _host,
    std::uint16_t _port,
    const std::string &_user,
    const std::string &_pass,
    const std::string &_vhost,
    const std::string &_name,
    const std::string &__queue,
    const std::string &__exchange,
    const std::string &__key,
    const int __queueFlags,
    const int __exchangeFlags,
    std::uint32_t __prefetch,
    AMQP::ExchangeType __exchangeType,
    const std::string &__product,
    const std::string &__version,
    const std::string &__copyright,
    const std::string &__information,
    const std::string &__privateKeyFile,
    const std::string &__certificateFile,
    const std::string &__caLocation)
    : _worker(_iworker),
        _creds(_user, _pass),
        _connection(this, _creds, _vhost),
        _channel(&_connection),
        _logger(nullptr),
        _queue(__queue),
        _exchange(__exchange),
        _key(__key),
        _queueFlags(__queueFlags),
        _exchangeFlags(__exchangeFlags),
        _prefetch(__prefetch),
        _exchangeType(__exchangeType),
        IConnectionHandlerSSL(
            _id,
            _host,
            _port,
            _name,
            __privateKeyFile,
            __certificateFile,
            __caLocation,
            __product,
            __version,
            __copyright,
            __information)
{
    // Create the logger first
    _logger = spdlog::get(GetName());
    if (_logger == nullptr)
        _logger = spdlog::stdout_color_mt(GetName());

    // Declare the queue and exchange and bind them
    InitializeQueue();

    // Set the worker callbacks
    SetConsumerCallbacks();

    // Start the threads
    try
    {
        Run();
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

agent::IAMQPWorkerSSL::IAMQPWorkerSSL(
    unsigned int _id,
    IWorker *_iworker,
    Json::Value _config)
    : _worker(_iworker),
        _creds(
            _config["credentials"]["username"].asString(),
            _config["credentials"]["password"].asString()),
        _connection(
            this,
            _creds,
            _config["host"]["vhost"].asString()),
        _channel(&_connection),
        _logger(nullptr),
        _queue(_config["settings"]["queue"].asString()),
        _exchange(_config["settings"]["exchange"].asString()),
        _key(_config["settings"]["key"].asString()),
        _queueFlags([_config](){
            int total = 0;
            for (auto flag : _config["settings"]["queueFlags"])
                total |= allBitFlags[flag.asString()];
            return total; }()),
        _exchangeFlags([_config](){
            int total = 0;
            for (auto flag : _config["settings"]["exchangeFlags"])
                total |= allBitFlags[flag.asString()];
            return total; }()),
        _prefetch(_config["settings"]["prefetch"].asUInt()),
        _exchangeType(exchangeTypeMap[_config["settings"]["exchangeType"].asString()]),
        IConnectionHandlerSSL(
            _id,
            _config["host"]["host"].asString(),
            _config["host"]["port"].asUInt(),
            _config["name"].asString(),
            _config["credentials"]["privateKeyFile"].asString(),
            _config["credentials"]["certificateFile"].asString(),
            _config["credentials"]["caLocation"].asString(),
            _config["information"]["product"].asString(),
            _config["information"]["version"].asString(),
            _config["information"]["copyright"].asString(),
            _config["information"]["information"].asString()
        )
{
    // Create the logger first
    _logger = spdlog::get(GetName());
    if (_logger == nullptr)
        _logger = spdlog::stdout_color_mt(GetName());

    // Declare the queue and exchange and bind them
    InitializeQueue();

    // Set the worker callbacks
    SetConsumerCallbacks();

    // Start the threads
    try
    {
        Run();
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

agent::IAMQPWorkerSSL::~IAMQPWorkerSSL()
{
    _channel.close();
}

void agent::IAMQPWorkerSSL::InitializeQueue()
{
    _channel.declareQueue(_queue, _queueFlags);
    _channel.declareExchange(_exchange, _exchangeType, _exchangeFlags);
    _channel.bindQueue(_exchange, _queue, _key);
    _channel.setQos(_prefetch);
}

void agent::IAMQPWorkerSSL::SetConsumerCallbacks()
{
    // Set the consumer callbacks here
    _channel.consume(
            _queue,
            _key
        ).onReceived(
            [this](const AMQP::Message& message, uint64_t tag, bool redelivered) {
                _logger->info("[onReceived] Received message {}", tag);
                _worker->AddMessage(message.body(), message.bodySize());
            }
        ).onComplete(
            [this](uint64_t tag, bool result) {
                _logger->info("[onComplete] Finished message {}", tag);
                _channel.ack(tag);
            }
        ).onError(
            [this](const char* message) {
                _logger->error("[onError] {}", message);
            }
        );
}

void agent::IAMQPWorkerSSL::AddMessage(const void* _msg, std::uint32_t _size, std::string _exchange, std::string _key)
{
    _channel.publish(_exchange, _key, static_cast<const char*>(_msg), _size, 0);
}

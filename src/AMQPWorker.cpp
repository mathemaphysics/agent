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
      _channel(&_connection)
{
    // Set the consumer callbacks here
    _channel.declareQueue(_queue, AMQP::durable);
    _channel.declareExchange(_exchange);
    _channel.bindQueue(_exchange, _queue, _key);
    _channel.consume(
            _queue,
            _key
        )
        .onReceived(
            [this](const AMQP::Message& message, uint64_t tag, bool redelivered){
                std::cout << "Received a message";
            }
        );

    // Start the threads
    try
    {
        _handler.Run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}
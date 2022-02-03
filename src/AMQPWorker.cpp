#include "AMQPWorker.hpp"

#include <string>
#include <cstdint>

agent::AMQPWorker::AMQPWorker(
    unsigned int _id,
    std::string _host,
    std::uint16_t _port,
    std::string _name,
    std::string _user,
    std::string _pass,
    std::string _vhost
)
    : IAMQPWorker(_id, _host, _port, _name, _user, _pass, _vhost)
{

}

int agent::AMQPWorker::ProcessMessage(const void* _msg, std::uint32_t _size) const
{
    _logger->info("Received a message");

    return 0;
}
#include "Worker.hpp"
#include "Message_generated.h"

#include <iostream>
#include <exception>
#include <utility>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

agent::Worker::Worker(unsigned int _id, bool _start)
    : IWorker(_id, _start)
{
    _logger = spdlog::stdout_color_mt(GetName());
}

agent::Worker::Worker(unsigned int _id, std::string _name, bool _start)
    : IWorker(_id, _name, _start)
{
    _logger = spdlog::stdout_color_mt(GetName());
}

agent::Worker::~Worker()
{
    if (_logger != nullptr)
        _logger->info("Thread {} finished", GetId());
    else
        std::cout << "Thread " << GetId() << " finished" << std::endl;
}

int agent::Worker::ProcessMessage(const void* _msg, flatbuffers::uoffset_t _size) const
{
    auto message = Messages::GetMessage(_msg);
    int id = message->id();
    int width = message->width();
    int height = message->height();
    auto pixels = message->pixels()->Data();

    if (_logger != nullptr)
        _logger->info("Received message: id: {} width: {} height: {}", id, width, height);
    else
        std::cout << "Received message: id: " << id << " width: " << width << " height: " << height << std::endl;

    return id;
}

void agent::Worker::operator()()
{
    while (GetState() != WORKER_QUIT)
    {
        if (!_data.empty())
        {
            // Remove a serialized message from the queue
            _data_lock.lock();
            const auto curMsg = _data.back();
            _data.pop_back();
            _data_lock.unlock();

            // Now process it
            const auto message = curMsg.first;
            const auto size = curMsg.second;
            try
            {
                int msgId = ProcessMessage(message, size);
                if (_logger != nullptr)
                    _logger->info("Successfully processed message {}", msgId);
                else
                    std::cout << "Successfully processed message " << msgId << std::endl;
            }
            catch(const std::exception& e)
            {
                if (_logger != nullptr)
                    _logger->critical(e.what());
                else
                    std::cout << e.what() << std::endl;
            }
        }
    }
}

#include "Worker.hpp"
#include "Message_generated.h"

#include <iostream>
#include <exception>
#include <utility>
#include <string>
#include <sstream>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

agent::Worker::Worker(unsigned int _id)
    : IWorker(_id)
{
    _logger = spdlog::stdout_color_mt(GetName());
}

agent::Worker::Worker(unsigned int _id, std::string _name)
    : IWorker(_id, _name)
{
    _logger = spdlog::stdout_color_mt(GetName());
}

agent::Worker::~Worker()
{
    if (_logger != nullptr)
        _logger->info("Worker {} finished", GetId());
    else
        std::cout << "Worker " << GetId() << " finished" << std::endl;
}

std::string agent::Worker::ThreadToString(std::thread::id _tid)
{
    auto ssThread = std::ostringstream();

    ssThread << _tid;

    return ssThread.str();
}

int agent::Worker::ProcessMessage(const void* _msg, flatbuffers::uoffset_t _size) const
{
    auto message = Messages::GetMessage(_msg);
    int id = message->id();
    int width = message->width();
    int height = message->height();
    auto pixels = message->pixels()->Data();

    if (_logger != nullptr)
        _logger->info("[{}] Received message: id: {} width: {} height: {}",
            ThreadToString(std::this_thread::get_id()),
            id, width, height);
    else
        std::cout << "[" << std::this_thread::get_id() << "] "
            << "Received message: id: " << id
            << " width: " << width
            << " height: " << height
            << std::endl;

    return id;
}

void agent::Worker::operator()()
{
    while (GetState() != WORKER_QUIT)
    {
        // Create space for a potential message
        bool received = false;
        std::pair<void *, flatbuffers::uoffset_t> curmsg;

        // Lock _data and grab a message
        _data_lock.lock();
        if (!_data.empty())
        {
            curmsg = _data.back();
            _data.pop_back();
            received = true;
        }
        _data_lock.unlock();

        // Now the lock is off; process the message
        if (received)
        {
            // Now process it
            const auto message = curmsg.first;
            const auto size = curmsg.second;
            try
            {
                int msgId = ProcessMessage(message, size);
                if (_logger != nullptr)
                    _logger->info("[{}] Successfully processed message {}",
                        ThreadToString(std::this_thread::get_id()),
                        msgId);
                else
                    std::cout << "[" << std::this_thread::get_id() << "] "
                        << "Successfully processed message "
                        << msgId
                        << std::endl;
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

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
    // Check if logger called GetName() exists, else create it
    _logger = spdlog::get(GetName());
    if (_logger == nullptr)
        _logger = spdlog::stdout_color_mt(GetName());
}

agent::Worker::Worker(unsigned int _id, std::string _name)
    : IWorker(_id, _name)
{
    _logger = spdlog::get(GetName());
    if (_logger == nullptr)
        _logger = spdlog::stdout_color_mt(GetName());
}

agent::Worker::~Worker()
{
    _logger->info("Worker {} finished", GetId());
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

    _logger->info("[{}] Received message: id: {} width: {} height: {}",
        ThreadToString(std::this_thread::get_id()),
        id, width, height);

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
                _logger->info("[{}] Successfully processed message {}",
                    ThreadToString(std::this_thread::get_id()),
                    msgId);
            }
            catch(const std::exception& e)
            {
                _logger->critical(e.what());
            }
        }
    }
}

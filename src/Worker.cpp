#include "Worker.hpp"
#include "Message_generated.h"

#include <iostream>
#include <exception>
#include <tuple>

#include <spdlog/spdlog.h>

agent::Worker::Worker(unsigned int _id)
    : IWorker(_id)
{}

int agent::Worker::ProcessMessage(const void* _msg, flatbuffers::uoffset_t _size) const
{
    auto message = Messages::GetMessage(_msg);
    int id = message->id();
    int width = message->width();
    int height = message->height();
    auto pixels = message->pixels()->Data();

    spdlog::info("Received message: id: {} width: {} height: {}", id, width, height);

    return id;
}

void agent::Worker::operator()()
{
    while (true)
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
                int result = ProcessMessage(message, size);
                spdlog::info("Successfully processed message");
            }
            catch(const std::exception& e)
            {
                spdlog::critical(e.what());
            }
        }
    }
}

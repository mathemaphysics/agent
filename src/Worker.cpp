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
}

agent::Worker::Worker(unsigned int _id, std::string _name)
    : IWorker(_id, _name)
{
}

agent::Worker::~Worker()
{
    _logger->info("Worker {} finished", GetId());
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

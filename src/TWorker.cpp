#include "TWorker.hpp"

#include <thread>
#include <functional>
#include <memory>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

agent::TWorker::TWorker(unsigned int _id, std::string _name)
{
    _worker = new Worker(_id);
    _worker->SetName(_name);
    _thread = new std::thread(std::ref(*_worker));
}

agent::TWorker::~TWorker()
{
    _thread->join();
    spdlog::get(_worker->GetName())
        ->info("Thread {} finished", _worker->GetId());
    delete _thread;
    delete _worker;
}

void agent::TWorker::Quit()
{
    _worker->SetQuit();
}

void agent::TWorker::AddMessage(void* _msg, flatbuffers::uoffset_t _size)
{
    _worker->AddMessage(_msg, _size);
}
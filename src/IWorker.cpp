#include "agent/IWorker.hpp"

#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <deque>
#include <cstdint>
#include <memory>
#include <utility>
#include <algorithm>
#include <exception>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

agent::IWorker::IWorker(unsigned int __id)
{
    _id = __id;
    _state.store(WORKER_READY); ///< Sets the default to "ready"

    // Check if logger called GetName() exists, else create it
    _logger = spdlog::get(_name);
    if (_logger == nullptr)
        _logger = spdlog::stdout_color_mt(_name);
    _logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [%t] %v");
}

agent::IWorker::IWorker(unsigned int __id, std::string __name)
{
    _id = __id;
    _name = __name;
    _state.store(WORKER_READY);

    // Use the given name as name for _logger
    _logger = spdlog::get(__name);
    if (_logger == nullptr)
        _logger = spdlog::stdout_color_mt(__name);
    _logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [%t] %v");
}

agent::IWorker::~IWorker()
{
    // Quit the loop
    SetQuit();

    // Wait for threads to join
    std::for_each(
        _threads.begin(),
        _threads.end(),
        [](std::thread &thr){
            if (thr.joinable())
                thr.join();
        }
    );

    // Clear threads out because we're done
    _threads.clear();

    // Just to be pedantic
    _state.store(WORKER_READY);
}

void agent::IWorker::Run(std::size_t _nthread)
{
    for (int tid = 0; tid < _nthread; ++tid)
        _threads.emplace_back(std::ref(*this));
    _state.store(WORKER_RUNNING);
}

void agent::IWorker::Stop()
{
    // Tell threads to quit
    SetQuit();
    
    // Wait for threads to join
    std::for_each(
        _threads.begin(),
        _threads.end(),
        [](std::thread &thr){
            if (thr.joinable())
                thr.join();
        }
    );

    // Clear threads out because we're done
    _threads.clear();

    // Threads stopped; ready for another run
    _state.store(WORKER_READY);
}

unsigned int agent::IWorker::GetId() const
{
    return _id;
}

std::string agent::IWorker::GetName() const
{
    return _name;
}

void agent::IWorker::SetName(std::string __name)
{
    _name = __name;
}

agent::WorkerState agent::IWorker::GetState() const
{
    return _state.load();
}

void agent::IWorker::SetQuit()
{
    _state.store(WORKER_QUIT);
}

void agent::IWorker::AddMessage(const void *_msg, std::uint32_t _size)
{
    _data_lock.lock();
    _data.push_front(std::pair<const void*, std::uint32_t>(_msg, _size));
    _data_lock.unlock();
}

void agent::IWorker::operator()()
{
    while (GetState() != WORKER_QUIT)
    {
        // Create space for a potential message
        bool received = false;
        std::pair<const void *, std::uint32_t> curmsg;

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
                _logger->info("Successfully processed message {}", msgId);
            }
            catch(const std::exception& e)
            {
                _logger->critical(e.what());
            }
        }
    }
}

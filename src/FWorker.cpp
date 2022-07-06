#include "FWorker.hpp"

#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <deque>
#include <cstdint>
#include <memory>
#include <functional>
#include <exception>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

agent::FWorker::FWorker(unsigned int __id, std::function<int(const void*, std::uint32_t, void*, std::uint32_t*)> _msgproc)
{
    _id = __id;
    _state.store(FWORKER_READY); ///< Sets the default to "ready"

    // Check if logger called GetName() exists, else create it
    _logger = spdlog::get(_name);
    if (_logger == nullptr)
        _logger = spdlog::stdout_color_mt(_name);
	_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [%t] %v");

    ProcessMessage = _msgproc;
}

agent::FWorker::FWorker(unsigned int __id, std::string __name, std::function<int(const void*, std::uint32_t, void*, std::uint32_t*)> _msgproc)
{
    _id = __id;
    _name = __name;
    _state.store(FWORKER_READY);

    // Use the given name as name for _logger
    _logger = spdlog::get(__name);
    if (_logger == nullptr)
        _logger = spdlog::stdout_color_mt(__name);
	_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [%t] %v");

    ProcessMessage = _msgproc;
}

agent::FWorker::~FWorker()
{
    // Quit the loop
    SetQuit();

    // Wait for threads to join
    std::for_each(
        _threads.begin(),
        _threads.end(),
        [](std::thread &thr)
        {
            if (thr.joinable())
                thr.join();
        });

    // Clear threads out because we're done
    _threads.clear();

    // Just to be pedantic
    _state.store(FWORKER_READY);
}

void agent::FWorker::Run(std::size_t _nthread)
{
    for (int tid = 0; tid < _nthread; ++tid)
        _threads.emplace_back(std::ref(*this));
    _state.store(FWORKER_RUNNING);
}

void agent::FWorker::Stop()
{
    // Tell threads to quit
    SetQuit();

    // Wait for threads to join
    std::for_each(
        _threads.begin(),
        _threads.end(),
        [](std::thread &thr)
        {
            if (thr.joinable())
                thr.join();
        });

    // Clear threads out because we're done
    _threads.clear();

    // Threads stopped; ready for another run
    _state.store(FWORKER_READY);
}

unsigned int agent::FWorker::GetId() const
{
    return _id;
}

std::string agent::FWorker::GetName() const
{
    return _name;
}

void agent::FWorker::SetName(std::string __name)
{
    _name = __name;
}

agent::FWorkerState agent::FWorker::GetState() const
{
    return _state.load();
}

void agent::FWorker::SetQuit()
{
    _state.store(FWORKER_QUIT);
}

void agent::FWorker::AddMessage(void *_msg, std::uint32_t _size)
{
    _data_lock.lock();
    _data.push_front(std::pair<void *, std::uint32_t>(_msg, _size));
    _data_lock.unlock();
}

void agent::FWorker::operator()()
{
    while (GetState() != FWORKER_QUIT)
    {
        // Create space for a potential message
        bool received = false;
        std::pair<void*, std::uint32_t> curmsg;

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
                auto result = new char[64];
                std::uint32_t rsize;
                int msgId = ProcessMessage(message, size, result, &rsize);
                _logger->info("Successfully processed message {}", msgId);
            }
            catch (const std::exception &e)
            {
                _logger->critical(e.what());
            }
        }
    }
}

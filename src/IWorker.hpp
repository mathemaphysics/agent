#pragma once

#include <iostream>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <deque>
#include <functional>
#include <sstream>
#include <cstdint>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace agent
{
	typedef enum {
		WORKER_READY,
		WORKER_RUNNING,
		WORKER_QUIT
	} WorkerState;

	class IWorker
	{
	public:
		/**
		 * @brief Construct a new IWorker object
		 * 
		 * This is just an interface with the basic parts, so you'll want to
		 * overload it.
		 * 
		 * @param __id The ID number for this worker
		 */
		IWorker(unsigned int __id)
		{
			_id = __id;
			_state.store(WORKER_READY); ///< Sets the default to "ready"

			// Check if logger called GetName() exists, else create it
			_logger = spdlog::get(_name);
			if (_logger == nullptr)
				_logger = spdlog::stdout_color_mt(_name);
			_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] %v");
		}

		/**
		 * @brief Construct a new IWorker object
		 * 
		 * @param __id Desired worker ID
		 * @param __name Desired worker name
		 */
		IWorker(unsigned int __id, std::string __name)
		{
			_id = __id;
			_name = __name;
			_state.store(WORKER_READY);

			// Use the given name as name for _logger
			_logger = spdlog::get(__name);
			if (_logger == nullptr)
				_logger = spdlog::stdout_color_mt(__name);
			_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] %v");
		}

		/**
		 * @brief Destroy the IWorker object
		 * 
		 */
		virtual ~IWorker()
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

		/**
		 * @brief Starts the worker thread itself
		 * 
		 * This function exists for the case that you might want to inherit \c
		 * IWorker and do more setup of the object before starting the thread
		 */
		void Run(std::size_t _nthread = 1)
		{
			for (int tid = 0; tid < _nthread; ++tid)
				_threads.emplace_back(std::ref(*this));
			_state.store(WORKER_RUNNING);
		}

		/**
		 * @brief Stops all threads
		 * 
		 */
		void Stop()
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

		/**
		 * @brief Get the ID of the worker
		 * 
		 * @return unsigned int ID of the worker
		 */
		unsigned int GetId() const
		{
			return _id;
		}
		
		/**
		 * @brief Get the name object
		 * 
		 * @return std::string The name, if set, of the worker object
		 */
		std::string GetName() const
		{
			return _name;
		}

		/**
		 * @brief Set the name object
		 * 
		 * @param __name Desired name of the worker object
		 */
		void SetName(std::string __name)
		{
			_name = __name;
		}

		/**
		 * @brief Gets the state of the worker, a \c WorkerState
		 * 
		 * @return WorkerState State of the worker
		 */
		WorkerState GetState() const
		{
			return _state.load();
		}

		/**
		 * @brief Set the quit state
		 * 
		 * Sets the \c _state to \c WORKER_QUIT to kill the thread.
		 * 
		 */
		void SetQuit()
		{
			_state.store(WORKER_QUIT);
		}

		virtual void AddMessage(const void* _msg, std::uint32_t _size)
		{
			_data_lock.lock();
			_data.push_front(std::pair<const void*, std::uint32_t>(_msg, _size));
			_data_lock.unlock();
		}

		/**
		 * @brief Process the given (serialized) message
		 * 
		 * This is where the work is done in the worker class.
		 * 
		 * @param _msg Serialized message (array of bytes, i.e. void*)
		 * @param _size Number of bytes in the serialized message in \c _msg
		 * @param _result Result data serving as a return from the procedure
		 * @param _rsize Size of the result message (optional)
		 * @return int Unique ID of the message processed
		 */
		virtual int ProcessMessage(const void* _msg, std::uint32_t _size, void* _result = nullptr, std::uint32_t* _rsize = nullptr) const = 0;

		/**
		 * @brief Contains the main work loop
		 * 
		 * Contains the main work loop in which \c ProcessMessage is called
		 * until \c _data is exhausted.
		 * 
		 */
		virtual void operator()()
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


		/**
		 * @brief Converts a \c std::thread::id to \c std::string
		 * 
		 * @param _tid Thread ID
		 * @return std::string Converted string
		 */
		static std::string ThreadToString(std::thread::id _tid)
		{
			auto ssThread = std::ostringstream();

			ssThread << _tid;

			return ssThread.str();
		}

	protected:
		std::deque<std::pair<const void*, std::uint32_t>> _data; ///< Queue of messages
		std::mutex _data_lock; ///< Mutex lock for the \c _data queue
		std::vector<std::thread> _threads; ///< All of the threads running on the worker
		std::shared_ptr<spdlog::logger> _logger = nullptr;

	private:
		unsigned int _id; ///< Unique ID of the worker
		std::string _name = "IWorker"; ///< Name assigned to the worker
		std::atomic<WorkerState> _state; ///< State of the worker; 0 -> Ready
	};
}

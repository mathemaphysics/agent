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
		IWorker(unsigned int __id);

		/**
		 * @brief Construct a new IWorker object
		 * 
		 * @param __id Desired worker ID
		 * @param __name Desired worker name
		 */
		IWorker(unsigned int __id, std::string __name);

		/**
		 * @brief Destroy the IWorker object
		 * 
		 */
		virtual ~IWorker();

		/**
		 * @brief Starts the worker thread itself
		 * 
		 * This function exists for the case that you might want to inherit \c
		 * IWorker and do more setup of the object before starting the thread
		 */
		void Run(std::size_t _nthread = 1);

		/**
		 * @brief Stops all threads
		 * 
		 */
		void Stop();

		/**
		 * @brief Get the ID of the worker
		 * 
		 * @return unsigned int ID of the worker
		 */
		unsigned int GetId() const;
		
		/**
		 * @brief Get the name object
		 * 
		 * @return std::string The name, if set, of the worker object
		 */
		std::string GetName() const;

		/**
		 * @brief Set the name object
		 * 
		 * @param __name Desired name of the worker object
		 */
		void SetName(std::string __name);

		/**
		 * @brief Gets the state of the worker, a \c WorkerState
		 * 
		 * @return WorkerState State of the worker
		 */
		WorkerState GetState() const;

		/**
		 * @brief Set the quit state
		 * 
		 * Sets the \c _state to \c WORKER_QUIT to kill the thread.
		 * 
		 */
		void SetQuit();

		virtual void AddMessage(const void* _msg, std::uint32_t _size);

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
		virtual int ProcessMessage(const void* _msg, std::uint32_t _size, void* _result = nullptr, std::uint32_t* _rsize = nullptr) = 0;

		/**
		 * @brief Contains the main work loop
		 * 
		 * Contains the main work loop in which \c ProcessMessage is called
		 * until \c _data is exhausted.
		 * 
		 */
		virtual void operator()();

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

#pragma once

#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <deque>
#include <cstdint>
#include <memory>
#include <functional>

#include <spdlog/spdlog.h>

namespace agent
{
	typedef enum {
		FWORKER_READY,
		FWORKER_RUNNING,
		FWORKER_QUIT
	} FWorkerState;

	class FWorker
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
		FWorker(unsigned int __id, std::function<int(const void *, std::uint32_t, void *, std::uint32_t *)> _msgproc);

		/**
		 * @brief Construct a new IWorker object
		 * 
		 * @param __id Desired worker ID
		 * @param __name Desired worker name
		 */
		FWorker(unsigned int __id, std::string __name, std::function<int(const void *, std::uint32_t, void *, std::uint32_t *)> _msgproc);

		/**
		 * @brief Destroy the IWorker object
		 * 
		 */
		~FWorker();

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
		 * @brief Gets the state of the worker, a \c FWorkerState
		 * 
		 * @return WorkerState State of the worker
		 */
		FWorkerState GetState() const;

		/**
		 * @brief Set the quit state
		 * 
		 * Sets the \c _state to \c FWORKER_QUIT to kill the thread.
		 * 
		 */
		void SetQuit();

		void AddMessage(void * _msg, std::uint32_t _size);

		/**
		 * @brief Process the given (serialized) message
		 * 
		 * This is where the work is done in the worker class.
		 * 
		 * @param _msg Serialized message (array of bytes, i.e. void *)
		 * @param _size Number of bytes in the serialized message in \c _msg
		 * @param _result Output result that can be used by the caller
		 * @return int Unique ID of the message processed
		 */
		std::function<int(const void *, std::uint32_t, void *, std::uint32_t *)> ProcessMessage;

		/**
		 * @brief Contains the main work loop
		 * 
		 * Contains the main work loop in which \c ProcessMessage is called
		 * until \c _data is exhausted.
		 * 
		 */
		void operator()();

	protected:
		std::deque<std::pair<void *, std::uint32_t>> _data; ///< Queue of messages
		std::mutex _data_lock; ///< Mutex lock for the \c _data queue
		std::vector<std::thread> _threads; ///< All of the threads running on the worker
		std::shared_ptr<spdlog::logger> _logger = nullptr;

	private:
		unsigned int _id; ///< Unique ID of the worker
		std::string _name = "IWorker"; ///< Name assigned to the worker
		std::atomic<FWorkerState> _state; ///< State of the worker; 0 -> Ready
	};
}

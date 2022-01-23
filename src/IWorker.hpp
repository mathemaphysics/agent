#pragma once

#include <iostream>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <deque>
#include <functional>

#include <flatbuffers/flatbuffers.h>

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
		}

		/**
		 * @brief Destroy the IWorker object
		 * 
		 */
		virtual ~IWorker()
		{
			// Wait for threads to finish
			for (auto thr = _threads.begin(); thr != _threads.end(); ++thr)
				if (thr->joinable())
					thr->join();

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

			// Wait until they join
			for (auto thr = _threads.begin(); thr != _threads.end(); ++thr)
				if (thr->joinable())
					thr->join();

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

		void AddMessage(void* _msg, flatbuffers::uoffset_t _size)
		{
			_data_lock.lock();
			_data.push_front(std::pair<void*, flatbuffers::uoffset_t>(_msg, _size));
			_data_lock.unlock();
		}

		/**
		 * @brief Process the given (serialized) message
		 * 
		 * This is where the work is done in the worker class.
		 * 
		 * @param _msg Serialized message (array of bytes, i.e. void*)
		 * @param _size Number of bytes in the serialized message in \c _msg
		 * @return int Unique ID of the message processed
		 */
		virtual int ProcessMessage(const void* _msg, flatbuffers::uoffset_t _size) const = 0;

		/**
		 * @brief Contains the main work loop
		 * 
		 * Contains the main work loop in which \c ProcessMessage is called
		 * until \c _data is exhausted.
		 * 
		 */
		virtual void operator()() = 0;

	protected:
		std::deque<std::pair<void*, flatbuffers::uoffset_t>> _data; ///< Queue of messages
		std::mutex _data_lock; ///< Mutex lock for the \c _data queue
		std::vector<std::thread> _threads; ///< All of the threads running on the worker

	private:
		unsigned int _id; ///< Unique ID of the worker
		std::string _name; ///< Name assigned to the worker
		std::atomic<WorkerState> _state; ///< State of the worker; 0 -> Ready
	};
}

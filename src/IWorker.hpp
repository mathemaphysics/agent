#pragma once

#include <iostream>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
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
		IWorker(unsigned int __id, bool _start = true)
		{
			_id = __id;
			_state.store(WORKER_READY); ///< Sets the default to "ready"

			// Default is to start the thread immediately
			if (_start)
				Run();
		}

		/**
		 * @brief Construct a new IWorker object
		 * 
		 * @param __id Desired worker ID
		 * @param __name Desired worker name
		 */
		IWorker(unsigned int __id, std::string __name, bool _start = true)
		{
			_id = __id;
			_name = __name;
			_state.store(WORKER_READY);

			// Default is to start the thread immediately
			if (_start)
				Run();
		}

		/**
		 * @brief Destroy the IWorker object
		 * 
		 */
		virtual ~IWorker()
		{
			_thread->join();
			delete _thread;
		}

		/**
		 * @brief Starts the worker thread itself
		 * 
		 * This function exists for the case that you might want to inherit \c
		 * IWorker and do more setup of the object before starting the thread
		 */
		void Run()
		{
			_thread = new std::thread(std::ref(*this));
			_state.store(WORKER_RUNNING);
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
			_data.push_back(std::pair<void*, flatbuffers::uoffset_t>(_msg, _size));
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
		std::vector<std::pair<void*, flatbuffers::uoffset_t>> _data; ///< Queue of messages
		std::mutex _data_lock; ///< Mutex lock for the \c _data queue
		std::thread* _thread; ///< Thread object storing the running operator()

	private:
		unsigned int _id; ///< Unique ID of the worker
		std::string _name; ///< Name assigned to the worker
		std::atomic<WorkerState> _state; ///< State of the worker; 0 -> Ready
	};
}

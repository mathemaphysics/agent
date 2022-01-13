#pragma once

#include "IWorker.hpp"
#include "Message_generated.h"

#include <string>
#include <thread>
#include <memory>

#include <spdlog/spdlog.h>

/**
 * @brief agent is the total namespace
 * 
 */
namespace agent
{
	/**
	 * @brief Class that runs the main worker thread
	 * 
	 */
	class Worker : public IWorker
	{
	public:
		/**
		 * @brief Construct a new Worker object
		 * 
		 * @param _id ID to assign to this \c Worker
		 * @param _start Start thread immediately if true, else not
		 */
		Worker(unsigned int _id, bool _start = true);

		/**
		 * @brief Construct a new Worker object
		 * 
		 * @param _id ID to assign to this \c Worker
		 * @param _name Name to assign the \c Worker
		 * @param _start Start thread immediately if true, else not
		 */
		Worker(unsigned int _id, std::string _name, bool _start = true);
		~Worker();

		/**
		 * @brief The function which does the work on each message
		 * 
		 * This is the function which does the job of the \c Worker.
		 * Without this function you just have a fancy constructor
		 * that spawns a thread or a few.
		 * 
		 * @param _msg  Pointer to the memory containing the message to process
		 * @param _size Number of bytes contained in the message
		 * @return int ID of the message it processed
		 */
		int ProcessMessage(const void* _msg, flatbuffers::uoffset_t _size) const override;
		void operator()() override;

	private:
		std::shared_ptr<spdlog::logger> _logger = nullptr;
	};
}

#pragma once

#include "IWorker.hpp"
#include "Message_generated.h"

#include <string>
#include <thread>

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
		Worker(unsigned int _id);
		~Worker() = default;
		int ProcessMessage(const void* _msg, flatbuffers::uoffset_t _size) const override;
		void operator()() override;
	};
}

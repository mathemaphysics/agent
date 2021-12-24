#pragma once

#include <string>
#include <thread>

#include "IWorker.hpp"
#include "IMessage.hpp"

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
		Worker();

		~Worker() = default;

		void operator()();

	private:
		int workerId = -1;
		
	};
}

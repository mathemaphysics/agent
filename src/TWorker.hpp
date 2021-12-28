#pragma once

#include <thread>

namespace agent
{
	class TWorker
	{
	public:
		TWorker() = default;
		~TWorker() = default;

	private:
		std::thread* _thread;
	};
}
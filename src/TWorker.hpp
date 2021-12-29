#pragma once

#include "Worker.hpp"

#include <thread>
#include <memory>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace agent
{
	class TWorker
	{
	public:
		TWorker(unsigned int _id, std::string _name);
		~TWorker();
		void Quit();
		void AddMessage(void* _msg, flatbuffers::uoffset_t _size);

	private:
		Worker* _worker;
		std::thread* _thread;
	};
}
#pragma once

#include "IMessage.hpp"

#include <cstdint>
#include <string>
#include <atomic>
#include <thread>

namespace agent
{
	class IWorker
	{
	public:
		IWorker() = default;
		virtual ~IWorker() = default;
		virtual int ProcessMessage(IMessage) = 0;

	private:
		std::uint32_t _id;
		std::string _name;
	};
}

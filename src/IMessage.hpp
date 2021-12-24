#pragma once

#include <cstdint>

namespace agent
{
	class IMessage
	{
	public:
		IMessage() = default;

		~IMessage() = default;
	
	private:
		std::uint32_t _id;
	};
}

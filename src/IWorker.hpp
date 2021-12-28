#pragma once

#include <cstdint>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <tuple>

#include <flatbuffers/flatbuffers.h>

namespace agent
{
	class IWorker
	{
	public:
		IWorker(unsigned int __id) { _id = __id; };
		virtual ~IWorker() = default;
		unsigned int GetId() { return _id; }
		void SetName(std::string __name) { _name = __name; }
		std::string GetName() { return _name; }
		virtual int ProcessMessage(const void* _msg, flatbuffers::uoffset_t _size) const = 0;
		virtual void operator()() = 0;

	protected:
		std::vector<std::pair<void*, flatbuffers::uoffset_t>> _data;
		std::mutex _data_lock;
		std::atomic<int> _state;

	private:
		unsigned int _id;
		std::string _name;
	};
}

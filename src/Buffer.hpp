#pragma once

#include <agent/agent.hpp>
#include <vector>
#include <cstdint>

namespace agent
{
	class Buffer
	{
	public:
		/**
		 * @brief Construct a new Buffer object
		 * 
		 * @param _size The number of bytes to allocate
		 */
		Buffer(std::size_t _size = AGENT_CONN_BUFFER_SIZE);
		~Buffer() = default;

		/**
		 * @brief Write data into the buffer
		 * 
		 * @param _data Pointer to the data to be written
		 * @param _size The number of bytes to be written
		 * @return std::size_t Number of bytes actually written
		 */
		std::size_t Write(const char* _data, std::size_t _size);

		/**
		 * @brief Requests the number of bytes available to read
		 * 
		 * @return std::size_t Number of bytes available
		 */
		std::size_t Available() const;

		/**
		 * @brief Gets a pointer to the contents of the buffer
		 * 
		 * @return const char* A pointer to the contents
		 */
		const char* Data() const;

		/**
		 * @brief Clears the buffer
		 * 
		 */
		void Drain();

		/**
		 * @brief Shifts the contents of the buffer left
		 * 
		 * @param _size Number of bytes by which to shift
		 */
		void Shift(std::size_t _size);

	private:
		std::vector<char> _data;
		std::size_t _used;
	};
}
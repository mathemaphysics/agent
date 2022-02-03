#pragma once

#include "IAMQPWorker.hpp"

#include <string>
#include <cstdint>

namespace agent
{
	class AMQPWorker : public IAMQPWorker
	{
		public:
			AMQPWorker() = default;

			/**
			 * @brief Construct a new AMQPWorker object
			 *
			 * @param _id Worker ID to assign to this Worker
			 * @param _host Host address to connect to
			 * @param _port AMQP port
			 * @param _name Name of client to report
			 * @param _user AMQP user
			 * @param _pass AMQP password
			 */
			AMQPWorker(
				unsigned int _id,
				std::string _host,
				std::uint16_t _port,
				std::string _name,
				std::string _user = "guest",
				std::string _pass = "guest",
				std::string _vhost = "/"
			);

			~AMQPWorker() = default;

			/**
			 * @brief Define the virtual message processor
			 * 
			 * This performs all processing on the message type your system
			 * processes
			 * 
			 * @param _msg Pointer to the data to process
			 * @param _size Size of the data
			 * @return int The message ID processed
			 */
			int ProcessMessage(const void* _msg, std::uint32_t _size) const;
	};
}
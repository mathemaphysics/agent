#pragma once

#include "ConnectionHandler.hpp"

#include <string>
#include <cstdlib>

#include <amqpcpp.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace agent
{
	class AMQPWorker
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
			int _id,
			std::string _host,
			std::uint16_t _port,
			std::string _name,
			std::string _user = "guest",
			std::string _pass = "guest",
			std::string _vhost = "/"
		);

		~AMQPWorker();

		void InitializeQueue();
		void SetConsumerCallbacks();

	private:
		ConnectionHandler _handler;
		AMQP::Login _creds;
		AMQP::Connection _connection;
		AMQP::Channel _channel;
		std::string _queue = "Queue";
		std::string _exchange = "Exchange";
		std::string _key = "Queue";
		std::shared_ptr<spdlog::logger> _logger;
	};
}
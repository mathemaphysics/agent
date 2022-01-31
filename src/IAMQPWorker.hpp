#pragma once

#include "ConnectionHandler.hpp"

#include <string>
#include <cstdlib>

#include <amqpcpp.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace agent
{
	class IAMQPWorker : public ConnectionHandler
	{
	public:
		IAMQPWorker() = default;

		/**
		 * @brief Construct a new IAMQPWorker object
		 *
		 * @param _id Worker ID to assign to this Worker
		 * @param _host Host address to connect to
		 * @param _port AMQP port
		 * @param _name Name of client to report
		 * @param _user AMQP user
		 * @param _pass AMQP password
		 */
		IAMQPWorker(
			unsigned int _id,
			std::string _host,
			std::uint16_t _port,
			std::string _name,
			std::string _user = "guest",
			std::string _pass = "guest",
			std::string _vhost = "/"
		)
			: _creds(_user, _pass),
      		  _connection(this, _creds, _vhost),
      		  _channel(&_connection), // Inherited protected from ConnectionHandler
      		  _logger(nullptr),
      		  ConnectionHandler(_id, _host, _port, _name)
		{
			// Create the logger first
			_logger = spdlog::get(GetName());
			if (_logger == nullptr)
				_logger = spdlog::stdout_color_mt(GetName());

			// Declare the queue and exchange and bind them
			InitializeQueue();

			// Set the worker callbacks
			SetConsumerCallbacks();

			// Start the threads
			try
			{
				Run();
			}
			catch(const std::exception& e)
			{
				_logger->error("Exception caught: {}", e.what());
			}
			catch(...)
			{
				_logger->error("Anonymous exception caught");
			}
		}

		virtual ~IAMQPWorker()
		{
			_channel.close();
		}

		/**
		 * @brief Create the exchange and bind it to an exchange
		 * 
		 */
		void InitializeQueue()
		{
			_channel.declareQueue(_queue, AMQP::durable);
			_channel.declareExchange(_exchange);
			_channel.bindQueue(_exchange, _queue, _key);
		}

		/**
		 * @brief Set the consumer callbacks
		 * 
		 */
		void SetConsumerCallbacks()
		{
			// Set the consumer callbacks here
			_channel.consume(
					_queue,
					_key
				).onReceived(
					[this](const AMQP::Message& message, uint64_t tag, bool redelivered) {
						ProcessMessage(message.body(), message.bodySize());
					}
				).onComplete(
					[this](uint64_t tag, bool result) {
						_channel.ack(tag);
						_logger->info("Finished message {}", tag);
					}
				);
		}

		/**
		 * @brief Processor specific to the messages consumed here
		 * 
		 * This function must take in the raw serialized data and its size in
		 * bytes and produce the desired result, which may be any action.
		 * 
		 * @param _msg Pointer to the message data
		 * @param _size Size in bytes of the message data
		 * @return int Tag or message ID processed (or anything you like)
		 */
		virtual int ProcessMessage(const void* _msg, flatbuffers::uoffset_t _size) const = 0;

	protected:
		std::shared_ptr<spdlog::logger> _logger; ///< Exposing the logger for subclasses

	private:
		AMQP::Login _creds; ///< Login credentials for AMQP connection
		
		/**
		 * @brief Connection variable _created_ here
		 * 
		 * The reason we need another \c _connection comes from the fact that
		 * the parent class \c ConnectionHandler merely passes around a pointer
		 * to the connection handler we create here; it doesn't actually make
		 * one, only takes what you give it
		 */
		AMQP::Connection _connection;

		AMQP::Channel _channel; ///< Channel object used to communicate with AMQP
		std::string _queue = "Queue"; ///< The queue to pull messages from
		std::string _exchange = "Exchange"; ///< The exchange to bind it to
		std::string _key = "Queue"; ///< The optional key
	};
}
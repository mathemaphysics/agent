#pragma once

#include "IConnectionHandler.hpp"
#include "IWorker.hpp"

#include <string>
#include <cstdint>

#include <amqpcpp.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace agent
{
	class IAMQPWorker : public IConnectionHandler
	{
	public:
		IAMQPWorker() = default;

		/**
		 * @brief Construct a new IAMQPWorker object
		 *
		 * @param _id Worker ID to assign to the connection handler worker
		 * @param _iworker Pointer to the worker who knows how to process a message
		 * @param _host Host address to connect to
		 * @param _port AMQP port
		 * @param _name Name of client to report
		 * @param _user AMQP user
		 * @param _pass AMQP password
		 * @param _vhost AMQP virtual host to use
		 */
		IAMQPWorker(
			unsigned int _id,
			IWorker *_iworker,
			std::string _host,
			std::uint16_t _port,
			const std::string &_user = "guest",
			const std::string &_pass = "guest",
			const std::string &_vhost = "/",
			const std::string &_name = "",
			const std::string &__product = "",
			const std::string &__version = "",
			const std::string &__copyright = "",
			const std::string &__information = "")
			: _worker(_iworker),
			  _creds(_user, _pass),
			  _connection(this, _creds, _vhost),
			  _channel(&_connection),
			  _logger(nullptr),
			  IConnectionHandler(
				  _id,
				  _host,
				  _port,
				  _name,
				  __product,
				  __version,
				  __copyright,
				  __information
			  )
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
						_logger->info("[onReceived] Received message {}", tag);
						_worker->AddMessage(message.body(), message.bodySize());
					}
				).onComplete(
					[this](uint64_t tag, bool result) {
						_logger->info("[onComplete] Finished message {}", tag);
						_channel.ack(tag);
					}
				).onError(
					[this](const char* message){
						_logger->error("[onError] {}", message);
					}
				);
		}

		/**
		 * @brief Overridden version of \c AddMessage from \c IWorker which,
		 * instead of adding to the local \c std::deque of message, adds the
		 * message to the AMQP queue
		 * 
		 * @param _msg  Pointer to the message itself
		 * @param _size Size of the message (in bytes)
		 */
		void AddMessage(const void* _msg, std::uint32_t _size, std::string _exchange = "", std::string _key = "")
		{
			_channel.publish(_exchange, _key, static_cast<const char*>(_msg), _size, 0);
		}

		/**
		 * @brief Worker that runs a single message
		 * 
		 * The worker can be run with any number of threads, the idea being that
		 * if you set the AMQP prefetch within QOS settings to something greater
		 * than one you should be able to pull in multiple messages at once
		 * which can then be processed using your \c IWorker .
		 */
		IWorker *_worker; ///< Pointer to an IWorker that knows how to process a single message

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
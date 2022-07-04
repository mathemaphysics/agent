#pragma once

#include "IConnectionHandlerSSL.hpp"
#include "IWorker.hpp"

#include <string>
#include <cstdint>
#include <algorithm>

#include <amqpcpp.h>
#include <json/json.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "SymbolMaps.hpp"

namespace agent
{
	class IAMQPWorkerSSL : public IConnectionHandlerSSL
	{
	public:
		IAMQPWorkerSSL() = default;

		/**
		 * @brief Construct a new IAMQPWorkerSSL object
		 *
		 * @param _id Worker ID to assign to the connection handler worker
		 * @param _iworker Pointer to the worker who knows how to process a message
		 * @param _host Host address to connect to
		 * @param _port AMQP port
		 * @param _user AMQP user
		 * @param _pass AMQP password
		 * @param _vhost AMQP virtual host to use
		 * @param _name Name of client to report
		 * @param __queue Name of the queue
		 * @param __exchange Name of the exchange
		 * @param __key Key to use for this exchange
		 * @param __queueFlags Flags determining queue behavior
		 * @param __prefetch Number of messages to prefetch
		 * @param __exchangeType What kind of exchange this is
		 * @param __product Name of the product (AMQP client field)
		 * @param __version Version of produce (AMQP client field)
		 * @param __copyright Copyright notice (AMQP client field)
		 * @param __information Information (AMQP client field)
		 * @param __privateKeyFile Location of private key PEM file
		 * @param __certificateFile Location of certificate PEM file
		 * @param __caLocadtion Location of the certificate authority files
		 */
		IAMQPWorkerSSL(
			unsigned int _id,
			IWorker *_iworker,
			std::string _host,
			std::uint16_t _port,
			const std::string &_user = "guest",
			const std::string &_pass = "guest",
			const std::string &_vhost = "/",
			const std::string &_name = "",
			const std::string &__queue = "Queue",
			const std::string &__exchange = "Exchange",
			const std::string &__key = "Queue",
			const int __queueFlags = 0,
			const int __exchangeFlags = 0,
			std::uint32_t __prefetch = 4,
			AMQP::ExchangeType __exchangeType = AMQP::ExchangeType::fanout,
			const std::string &__product = "",
			const std::string &__version = "",
			const std::string &__copyright = "",
			const std::string &__information = "",
			const std::string &__privateKeyFile = "",
			const std::string &__certificateFile = "",
			const std::string &__caLocation = "")
			: _worker(_iworker),
			  _creds(_user, _pass),
			  _connection(this, _creds, _vhost),
			  _channel(&_connection),
			  _logger(nullptr),
			  _queue(__queue),
			  _exchange(__exchange),
			  _key(__key),
			  _queueFlags(__queueFlags),
			  _exchangeFlags(__exchangeFlags),
			  _prefetch(__prefetch),
			  _exchangeType(__exchangeType),
			  IConnectionHandlerSSL(
				  _id,
				  _host,
				  _port,
				  _name,
				  __privateKeyFile,
				  __certificateFile,
				  __caLocation,
				  __product,
				  __version,
				  __copyright,
				  __information)
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

		IAMQPWorkerSSL(
			unsigned int _id,
			IWorker *_iworker,
			Json::Value _config)
			: _worker(_iworker),
			  _creds(
				  _config["credentials"]["username"].asString(),
				  _config["credentials"]["password"].asString()),
			  _connection(
				  this,
				  _creds,
				  _config["host"]["vhost"].asString()),
			  _channel(&_connection),
			  _logger(nullptr),
			  _queue(_config["settings"]["queue"].asString()),
			  _exchange(_config["settings"]["exchange"].asString()),
			  _key(_config["settings"]["key"].asString()),
			  _queueFlags([_config]()
						  {
				int total = 0;
				for (auto flag : _config["settings"]["queueFlags"])
					total |= allBitFlags[flag.asString()];
				return total; }()),
			  _exchangeFlags([_config]()
							 {
				int total = 0;
				for (auto flag : _config["settings"]["exchangeFlags"])
					total |= allBitFlags[flag.asString()];
				return total; }()),
			  _prefetch(_config["settings"]["prefetch"].asUInt()),
			  _exchangeType(exchangeTypeMap[_config["settings"]["exchangeType"].asString()]),
			  IConnectionHandlerSSL(
				  _id,
				  _config["host"]["host"].asString(),
				  _config["host"]["port"].asUInt(),
				  _config["name"].asString(),
				  _config["credentials"]["privateKeyFile"].asString(),
				  _config["credentials"]["certificateFile"].asString(),
				  _config["credentials"]["caLocation"].asString(),
				  _config["information"]["product"].asString(),
				  _config["information"]["version"].asString(),
				  _config["information"]["copyright"].asString(),
				  _config["information"]["information"].asString()
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

		virtual ~IAMQPWorkerSSL()
		{
			_channel.close();
		}

		/**
		 * @brief Create the exchange and bind it to an exchange
		 * 
		 */
		void InitializeQueue()
		{
			_channel.declareQueue(_queue, _queueFlags);
			_channel.declareExchange(_exchange, _exchangeType, _exchangeFlags);
			_channel.bindQueue(_exchange, _queue, _key);
			_channel.setQos(_prefetch);
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

		/* New way of storing exchange and queue configuration flags */
		const int _queueFlags = 0;
		const int _exchangeFlags = 0;
		std::uint16_t _prefetch = 4; ///< The number of messages to prefetch
		AMQP::ExchangeType _exchangeType = AMQP::ExchangeType::fanout;
		const int _eventLoopFlags = 0;
	};
}
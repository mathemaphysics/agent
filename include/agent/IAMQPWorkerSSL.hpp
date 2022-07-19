#pragma once

#include "IConnectionHandlerSSL.hpp"
#include "IWorker.hpp"
#include "SymbolMaps.hpp"

#include <string>
#include <cstdint>

#include <amqpcpp.h>
#include <json/json.h>
#include <spdlog/spdlog.h>

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
			const std::string &__caLocation = "");

		IAMQPWorkerSSL(
			unsigned int _id,
			IWorker *_iworker,
			Json::Value _config);

		virtual ~IAMQPWorkerSSL();

		/**
		 * @brief Create the exchange and bind it to an exchange
		 * 
		 */
		void InitializeQueue();

		/**
		 * @brief Set the consumer callbacks
		 * 
		 */
		void SetConsumerCallbacks();

		/**
		 * @brief Overridden version of \c AddMessage from \c IWorker which,
		 * instead of adding to the local \c std::deque of message, adds the
		 * message to the AMQP queue
		 * 
		 * @param _msg  Pointer to the message itself
		 * @param _size Size of the message (in bytes)
		 */
		void AddMessage(const void* _msg, std::uint32_t _size, std::string _exchange = "", std::string _key = "");

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
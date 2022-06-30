#pragma once

#include "Buffer.hpp"
#include "IWorker.hpp"

#include <amqpcpp.h>
#include <Poco/Net/NetSSL.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/SecureStreamSocket.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <vector>
#include <cstdint>

class SSLInitializer
{
public:
	SSLInitializer()
	{
		Poco::Net::initializeSSL();
	}

	~SSLInitializer()
	{
		Poco::Net::uninitializeSSL();
	}
};

namespace agent
{
	class IConnectionHandlerSSL : public AMQP::ConnectionHandler, public IWorker
	{
	public:
		/**
		 * @brief Construct a new connection potentially with SSL
		 * 
		 * @param __id 
		 * @param _host 
		 * @param _port 
		 * @param _name 
		 * @param __context 
		 * @param __product 
		 * @param __version 
		 * @param __copyright 
		 * @param __information 
		 */
		IConnectionHandlerSSL(unsigned int __id, const std::string& _host, std::uint16_t _port, const std::string& _name, const std::string& _privateKeyFile, const std::string& _certificateFile, const std::string& _caLocation, const std::string& __product = "", const std::string& __version = "", const std::string& __copyright = "", const std::string& __information = "");

		/**
		 * @brief Destroy the Connection Handler object
		 * 
		 * Not default because we need to shut off the \c AMQP::Connection close
		 * the \c StreamSocket and delete the pointer to \c _connection
		 */
		~IConnectionHandlerSSL() = default;

		/**
		 * @brief Callback for the properties action from AMQP server
		 * 
		 * @param _connection The connection object, \c AMQP::Connection 
		 * @param _server Server information table
		 * @param _client Client information table
		 */
		void onProperties(AMQP::Connection* _connection, const AMQP::Table& _server, AMQP::Table& _client) override;

		/**
		 * @brief Callback for negotiating the heartbeat interval
		 * 
		 * @param _connection The connection object, \c AMQP::Connection
		 * @param _interval Requested heartbeat interval
		 * @return uint16_t Negotiated heartbeat interval
		 */
		uint16_t onNegotiate(AMQP::Connection* _connection, uint16_t _interval) override;

		/**
		 * @brief Callback which acts to send data when present
		 * 
		 * @param _connection The connection object, \c AMQP::Connection
		 * @param _data Pointer to the bytes to send
		 * @param _size Number of bytes to send
		 */
		void onData(AMQP::Connection* _connection, const char* _data, size_t _size) override;

		/**
		 * @brief Callback which handles case when heartbeat is received
		 * 
		 * @param _connection The connection object, \c AMQP::Connection
		 */
		void onHeartbeat(AMQP::Connection* _connection) override;

		/**
		 * @brief Callback for error case
		 * 
		 * @param _connection The connection object, \c AMQP::Connection
		 * @param _message Error message
		 */
		void onError(AMQP::Connection* _connection, const char* _message) override;

		/**
		 * @brief Callback which fires when connection is ready
		 * 
		 * @param _connection The connection object, \c AMQP::Connection
		 */
		void onReady(AMQP::Connection* _connection) override;

		/**
		 * @brief Callback for closure of the connection
		 * 
		 * @param _connection The connection object, \c AMQP::Connection
		 */
		void onClosed(AMQP::Connection* _connection) override;

		/**
		 * @brief Overridden function which won't be called
		 * 
		 * @param _msg N/A
		 * @param _size N/A
		 * @return int N/A
		 */
		int ProcessMessage(const void* _msg, std::uint32_t _size, void* _result = nullptr, std::uint32_t* _rsize = nullptr);

		/**
		 * @brief Function which runs the loop
		 * 
		 */
		void operator()() override;

		/**
		 * @brief Sets the quit state to kill the loop
		 * 
		 */
		void quit();

	protected:
		std::shared_ptr<spdlog::logger> _logger;

	private:
		void _connectSocket(Poco::Net::SocketAddress _address);
		std::string _client;
		std::string _product;
		std::string _version;
		std::string _copyright;
		std::string _information;
		bool _connected;
		Poco::Net::SecureStreamSocket _socket;
		Poco::Net::SocketAddress _address;
		AMQP::Connection* _connection;
		Buffer _inpbuffer;
		Buffer _outbuffer;
		std::vector<char> _tmpbuffer;
		void _sendDataFromBuffer();
		SSLInitializer _sslInitializer;
	};
}
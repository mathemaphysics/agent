#pragma once

#include "Buffer.hpp"
#include "IWorker.hpp"
#include "IConnectionHandler.hpp"

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
	class IConnectionHandlerSSL : public IConnectionHandler
	{
	public:
	 	/**
	 	 * @brief Construct a new IConnectionHandlerSSL object
	 	 * 
		 * @param __id Process ID (deprecated, I hope)
		 * @param _host Host address to connect to
		 * @param _port Port to connect to
		 * @param _name Client name to report to the server
	 	 * @param _privateKeyFile Client private key PEM file path
	 	 * @param _certificateFile Client certificate PEM file path
	 	 * @param _caLocation Certificate authority path
		 * @param __product Name of the product software to report
		 * @param __version Version of the product software to report
		 * @param __copyright Copyright string to report
		 * @param __information Additional information/website to report
	 	 */
		IConnectionHandlerSSL(unsigned int __id, const std::string& _host, std::uint16_t _port, const std::string& _name, const std::string& _privateKeyFile, const std::string& _certificateFile, const std::string& _caLocation, const std::string& __product = "", const std::string& __version = "", const std::string& __copyright = "", const std::string& __information = "");

		/**
		 * @brief Destroy the Connection Handler object
		 * 
		 * Not default because we need to shut off the \c AMQP::Connection close
		 * the \c StreamSocket and delete the pointer to \c _connection
		 */
		~IConnectionHandlerSSL() = default;

	protected:
		std::shared_ptr<spdlog::logger> _logger;

	private:
		//Poco::Net::SecureStreamSocket _socket;
		SSLInitializer _sslInitializer;
	};
}
#include "agent/agent.hpp"
#include "IConnectionHandlerSSL.hpp"
#include "IWorker.hpp"

#include <vector>
#include <cstdint>
#include <string>
#include <sstream>
#include <exception>

#include <amqpcpp.h>
#include <spdlog/spdlog.h>
#include <Poco/Net/SecureStreamSocket.h>
#include <Poco/Net/Context.h>

agent::IConnectionHandlerSSL::IConnectionHandlerSSL(
    unsigned int _id,
    const std::string& _host,
    std::uint16_t _port,
    const std::string& _name,
    const std::string& __privateKeyFile,
    const std::string& __certificateFile,
    const std::string& __caLocation,
    const std::string& __product,
    const std::string& __version,
    const std::string& __copyright,
    const std::string& __information
  )
    : _sslInitializer(),
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
  // Set up the AMQP::Connection here and then Run()
  socket() = Poco::Net::SecureStreamSocket::attach(
    socket(),
    new Poco::Net::Context(
        Poco::Net::Context::Usage::CLIENT_USE,
        __privateKeyFile,
        __certificateFile,
        __caLocation,
        Poco::Net::Context::VerificationMode::VERIFY_NONE
    )
  );
}

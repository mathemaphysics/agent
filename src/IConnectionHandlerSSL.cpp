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
#include <Poco/SharedPtr.h>
#include <Poco/Exception.h>

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
      ),
      _socket(
        Poco::Net::SecureStreamSocket::attach(
          socket(),
          new Poco::Net::Context(
              Poco::Net::Context::Usage::CLIENT_USE,
              __privateKeyFile,
              __certificateFile,
              __caLocation,
              Poco::Net::Context::VerificationMode::VERIFY_NONE
          )
        )
      )
{
  // Set up the AMQP::Connection here and then Run()
  _socket.setLazyHandshake(true);
}

void agent::IConnectionHandlerSSL::operator()()
{
  // This is the main worker loop for AMQP transactions
  while (GetState() != WORKER_QUIT)
  {
    //// You might have to resize for larger incoming chunk
    //if (savail > _tmpbuffer.size())
    //  _tmpbuffer.resize(savail, 0);
    
    // Make sure all bytes read were processed
    const int rbytes = _socket.receiveBytes(_tmpbuffer.data(), _tmpbuffer.size());
    if (rbytes < 0)
      _logger->info("Received rbytes = {}", rbytes);
    const int wbytes = _inpbuffer.Write(_tmpbuffer.data(), rbytes);

    if (wbytes != rbytes)
      _logger->debug("Could not write full contents to input buffer");

    const size_t iavail = _inpbuffer.Available();
    if (iavail > 0)
    {
      const size_t parsed = _connection->parse(_inpbuffer.Data(), iavail);

      if (parsed == iavail)
        _inpbuffer.Drain();
      else if (parsed > 0)
        _inpbuffer.Shift(parsed);
    }
    _sendDataFromBuffer();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  if (GetState() == WORKER_QUIT && _outbuffer.Available())
    _sendDataFromBuffer();
}

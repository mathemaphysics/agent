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
    : _client(_name),
      _product(__product),
      _version(__version),
      _copyright(__copyright),
      _information(__information),
      _connected(false),
      _connection(nullptr),
      _inpbuffer(AGENT_CONN_BUFFER_SIZE),
      _tmpbuffer(AGENT_CONN_TEMP_BUFFER_SIZE),
      _outbuffer(AGENT_CONN_BUFFER_SIZE),
      _sslInitializer(),
      _socket(
        Poco::Net::SecureStreamSocket(
          Poco::Net::SocketAddress(
            _host,
            _port
          ),
          new Poco::Net::Context(
            Poco::Net::Context::Usage::CLIENT_USE,
            __privateKeyFile,
            __certificateFile,
            __caLocation,
            Poco::Net::Context::VerificationMode::VERIFY_NONE
          )
        )
      ),
      IWorker(_id, _name)
{
  // Check if logger called GetName() exists, else create it
  _logger = spdlog::get(_client);
  if (_logger == nullptr)
    _logger = spdlog::stdout_color_mt(_client);
  
  // Just announce the creation of the client; can turn this off via log level
  _logger->info("Client {} created", _client);

  // Set up the AMQP::Connection here and then Run()
  //_socket.connect(_address);
}

void agent::IConnectionHandlerSSL::onProperties(AMQP::Connection *__connection, const AMQP::Table &_server, AMQP::Table &__client)
{
  if (_connection == nullptr)
    _connection = __connection;

  // Make sure we know who you are
  __client["connection_name"] = _client;
  __client["product"] = _product;
  __client["version"] = _version;
  __client["copyright"] = _copyright;
  __client["information"] = _information;

  // Set the platform
  #if defined(__linux__)
  __client["platform"] = "Linux";
  #elif defined(__APPLE__)
  __client["platform"] = "Mac OS X";
  #elif defined(__sun)
  __client["platform"] = "Solaris";
  #elif defined(__FreeBSD__)
  __client["platform"] = "FreeBSD";
  #elif defined(__OpenBSD__)
  __client["platform"] = "OpenBSD";
  #elif defined(__NetBSD__)
  __client["platform"] = "NetBSD";
  #elif defined(__hpux)
  __client["platform"] = "HP-UX";
  #elif defined(__osf__)
  __client["platform"] = "Tru64 UNIX";
  #elif defined(__sgi)
  __client["platform"] = "Irix";
  #elif defined(_AIX)
  __client["platform"] = "AIX";
  #elif defined(_WIN32)
  __client["platform"] = "Windows";
  #endif

  // Print details of the client and server
  auto _clientss = std::ostringstream();
  auto _serverss = std::ostringstream();

  _clientss << _client;
  _serverss << _server;

  _logger->info("[onProperties] Client: {}, Server: {}", _clientss.str(), _serverss.str());
}

uint16_t agent::IConnectionHandlerSSL::onNegotiate(AMQP::Connection *__connection, uint16_t _interval)
{
  if (_connection == nullptr)
    _connection = __connection;

  // Print details of the heartbeat negotiation
  _logger->info("[onNegotiate] Accepting interval of length {}", _interval);

  // Just accept the interval
  return _interval;
}

void agent::IConnectionHandlerSSL::onData(AMQP::Connection *__connection, const char *_data, size_t _size)
{
  if (_connection == nullptr)
    _connection = __connection;

  // Send any outgoing data that has shown up in the buffer
  int nbytes = _socket.sendBytes(_data, _size);

  _logger->debug("[onData] Sent {} bytes", _size);
}

void agent::IConnectionHandlerSSL::onHeartbeat(AMQP::Connection *__connection)
{
  if (_connection == nullptr)
    _connection = __connection;

  // Announce that we received a heartbeat from the AMQP server
  _logger->debug("[onHeartbeat] Received a heartbeat from server");

  // Send a return heartbeat; this isn't ideal because it depends on receiving a
  // heartbeat first, which might not happen. Set up an independent thread that
  // takes care of this
  _connection->heartbeat();
}

void agent::IConnectionHandlerSSL::onError(AMQP::Connection *__connection, const char *_message)
{
  if (_connection == nullptr)
    _connection = __connection;

  // Announce that we received a heartbeat from the AMQP server
  _logger->error("[onError] Error: {}", _message);
}

void agent::IConnectionHandlerSSL::onReady(AMQP::Connection *__connection)
{
  if (_connection == nullptr)
    _connection = __connection;

  // Notify the log that the connection is read
  _logger->info("[onReady] Connection is ready");
}

void agent::IConnectionHandlerSSL::onClosed(AMQP::Connection *__connection)
{
  if (_connection == nullptr)
    _connection = __connection;

  // Announce and close the connection
  _logger->info("[onClosed] Connection closed");

  // Exit the loop
  quit();
}

// This function should not exist at all; why can't I get rid of it?
int agent::IConnectionHandlerSSL::ProcessMessage(const void*_msg, std::uint32_t _size, void*_result, std::uint32_t*_rsize)
{
  return 0;
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
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  if (GetState() == WORKER_QUIT && _outbuffer.Available())
    _sendDataFromBuffer();
}

void agent::IConnectionHandlerSSL::quit()
{
  SetQuit();
}

void agent::IConnectionHandlerSSL::_sendDataFromBuffer()
{
  size_t avail = _outbuffer.Available();
  if (avail > 0)
  {
    int sent = _socket.sendBytes(_outbuffer.Data(), avail);
    _logger->info("Sent [{:6d} / {:6d}] bytes from buffer", sent, avail);
  }
}
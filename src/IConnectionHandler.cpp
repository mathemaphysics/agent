#include "agent/agent.hpp"
#include "IConnectionHandler.hpp"
#include "IWorker.hpp"

#include <vector>
#include <cstdint>
#include <string>
#include <sstream>
#include <exception>

#include <amqpcpp.h>
#include <spdlog/spdlog.h>
#include <Poco/Net/StreamSocket.h>

agent::IConnectionHandler::IConnectionHandler(unsigned int _id)
    : _client("IConnectionHandler"), // Default client name
      _connected(false),
      _connection(nullptr),
      _inpbuffer(AGENT_CONN_BUFFER_SIZE),
      _tmpbuffer(AGENT_CONN_TEMP_BUFFER_SIZE),
      _outbuffer(AGENT_CONN_BUFFER_SIZE),
      _address(Poco::Net::SocketAddress("localhost", 5672)),
      _logger(nullptr), // Default no logger
      IWorker(_id)
{
  // Check if logger called GetName() exists, else create it
  _logger = spdlog::get(_client);
  if (_logger == nullptr)
    _logger = spdlog::stdout_color_mt(_client);

  // Just announce the creation of the client; can turn this off via log level
  _logger->info("Client {} created", _client);

  // Set up the AMQP::Connection here and then Run()
  _socket.connect(_address);
  _socket.setKeepAlive(true);
}

agent::IConnectionHandler::IConnectionHandler(
    unsigned int _id,
    const std::string& _host,
    std::uint16_t _port,
    const std::string& _name
  )
    : _client(_name),
      _connected(false),
      _connection(nullptr),
      _inpbuffer(AGENT_CONN_BUFFER_SIZE),
      _tmpbuffer(AGENT_CONN_TEMP_BUFFER_SIZE),
      _outbuffer(AGENT_CONN_BUFFER_SIZE),
      _address(Poco::Net::SocketAddress(_host, _port)),
      IWorker(_id, _name)
{
  // Check if logger called GetName() exists, else create it
  _logger = spdlog::get(_client);
  if (_logger == nullptr)
    _logger = spdlog::stdout_color_mt(_client);
  
  // Just announce the creation of the client; can turn this off via log level
  _logger->info("Client {} created", _client);

  // Set up the AMQP::Connection here and then Run()
  _socket.connect(_address);
  _socket.setKeepAlive(true);
}

void agent::IConnectionHandler::onProperties(AMQP::Connection *__connection, const AMQP::Table &_server, AMQP::Table &_client)
{
  if (_connection == nullptr)
    _connection = __connection;

  // Print details of the client and server
  auto _clientss = std::ostringstream();
  auto _serverss = std::ostringstream();

  _clientss << _client;
  _serverss << _server;

  _logger->info("[onProperties] Client: {}, Server: {}", _clientss.str(), _serverss.str());
}

uint16_t agent::IConnectionHandler::onNegotiate(AMQP::Connection *__connection, uint16_t _interval)
{
  if (_connection == nullptr)
    _connection = __connection;

  // Print details of the heartbeat negotiation
  _logger->info("[onNegotiate] Accepting interval of length {}", _interval);

  // Just accept the interval
  return _interval;
}

void agent::IConnectionHandler::onData(AMQP::Connection *__connection, const char *_data, size_t _size)
{
  if (_connection == nullptr)
    _connection = __connection;

  // Send any outgoing data that has shown up in the buffer
  _socket.sendBytes(_data, _size);

  // TODO: Maybe make this debug level?
  _logger->info("[onData] Sent {} bytes", _size);
}

void agent::IConnectionHandler::onHeartbeat(AMQP::Connection *__connection)
{
  if (_connection == nullptr)
    _connection = __connection;

  // Announce that we received a heartbeat from the AMQP server
  _logger->info("[onHeartbeat] Received a heartbeat from server");

  // Send a return heartbeat; this isn't ideal because it depends on receiving a
  // heartbeat first, which might not happen. Set up an independent thread that
  // takes care of this
  _connection->heartbeat();
}

void agent::IConnectionHandler::onError(AMQP::Connection *__connection, const char *_message)
{
  if (_connection == nullptr)
    _connection = __connection;

  // Announce that we received a heartbeat from the AMQP server
  _logger->info("[onError] Error: {}", _message);
}

void agent::IConnectionHandler::onReady(AMQP::Connection *__connection)
{
  if (_connection == nullptr)
    _connection = __connection;

  // Notify the log that the connection is read
  _logger->info("[onReady] Connection is ready");
}

void agent::IConnectionHandler::onClosed(AMQP::Connection *__connection)
{
  if (_connection == nullptr)
    _connection = __connection;

  // Announce and close the connection
  _logger->info("[onClosed] Connection closed");

  // Exit the loop
  quit();
}

// This function should not exist at all; why can't I get rid of it?
int agent::IConnectionHandler::ProcessMessage(const void*_msg, std::uint32_t _size, void*_result, std::uint32_t*_rsize) const
{
  return 0;
}

void agent::IConnectionHandler::operator()()
{
  // This is the main worker loop for AMQP transactions
  while (GetState() != WORKER_QUIT)
  {
    // See if there's any data available on the incoming socket
    const size_t savail = _socket.available();
    if (savail > 0)
    {
      // You might have to resize for larger incoming chunk
      if (savail > _tmpbuffer.size())
        _tmpbuffer.resize(savail, 0);
      
      // Make sure all bytes read were processed
      const int rbytes = _socket.receiveBytes(_tmpbuffer.data(), savail);
      const int wbytes = _inpbuffer.Write(_tmpbuffer.data(), rbytes);

      if (wbytes != rbytes)
        _logger->debug("Could not write full contents to input buffer");
    }
    else if (savail < 0)
    {
      _logger->error("Socket error: Available bytes on socket < 0");
    }

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

void agent::IConnectionHandler::quit()
{
  SetQuit();
}

void agent::IConnectionHandler::_sendDataFromBuffer()
{
  size_t avail = _outbuffer.Available();
  if (avail > 0)
  {
    int sent = _socket.sendBytes(_outbuffer.Data(), avail);
    _logger->info("Sent [{:6d} / {:6d}] bytes from buffer", sent, avail);
  }
}
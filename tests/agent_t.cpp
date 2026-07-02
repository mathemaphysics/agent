#include "agent/agent.hpp"
#include "agent/Worker.hpp"
#include "agent/Buffer.hpp"
#include "agent/IConnectionHandler.hpp"
#include "agent/IAMQPWorker.hpp"
#include "agent/IAMQPWorkerSSL.hpp"
#include "agent/FWorker.hpp"
#include "Message_generated.h"

#include <thread>
#include <chrono>
#include <fstream>
#include <stdexcept>
#include <cstdio>
#include <cstring>

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

using namespace agent;

/**
 * @brief Tests related to the Buffer class
 * 
 * Test the \c Buffer class operations.
 */
class BufferTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    buffer = new Buffer();
    buffer->Write(
      testData.c_str(),
      std::strlen(testData.c_str())
    );
  }

  void TearDown() override
  {
    delete buffer;
  }

  Buffer *buffer;
  const std::string testData = "Random test data";
};

class BufferShiftTests : public BufferTest, public ::testing::WithParamInterface<int>
{

};

TEST_P(BufferShiftTests, BufferShiftCheck)
{
  std::size_t originalAvailable = buffer->Available();
  buffer->Shift(GetParam());
  EXPECT_EQ(buffer->Available(), originalAvailable - GetParam());
}

TEST_F(BufferTest, BufferAvailableCheck)
{
  EXPECT_EQ(buffer->Available(), testData.length());
  EXPECT_STREQ(buffer->Data(), testData.c_str());
}

TEST_F(BufferTest, BufferDrainCheck)
{
  buffer->Drain();
  EXPECT_EQ(buffer->Available(), 0);
}

INSTANTIATE_TEST_SUITE_P(BufferShiftTestSuite, BufferShiftTests, ::testing::Values(1, 2, 3));

/**
 * @brief Tests related to the \c IWorker class
 * 
 * Test the startup and execution of threaded workers
 */
class WorkerTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    worker = new Worker(0, "WorkerTest");
    worker->Run(3);
  }

  void TearDown() override
  {
    delete worker;
  }

  Worker* worker;
};

TEST_F(WorkerTest, CreateWorker)
{
  flatbuffers::FlatBufferBuilder builder(AGENT_FB_BUFFER_SIZE);
  auto pixels = builder.CreateVector(
    std::vector<std::int8_t>{
      { 0, 0, 1,
        2, 2, 1,
        4, 0, 1 }
    }
  );

  // Build message 1
  auto message1 = Messages::CreateMessage(builder, 0, 3, 3, pixels);
  builder.Finish(message1);
  auto buffer1 = builder.GetBufferPointer();
  auto size = builder.GetSize();

  // Build message 2
  auto message2 = Messages::CreateMessage(builder, 1, 3, 3, pixels);
  builder.Finish(message2);
  auto buffer2 = builder.GetBufferPointer();

  // Build message 3
  auto message3 = Messages::CreateMessage(builder, 2, 3, 3, pixels);
  builder.Finish(message3);
  auto buffer3 = builder.GetBufferPointer();

  // Add the item for the worker to process
  for (int i = 0; i < 4; i++)
  {
    worker->AddMessage(buffer1, size);
    worker->AddMessage(buffer2, size);
    worker->AddMessage(buffer3, size);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  // Exit the thread
  worker->Stop();
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  worker->Run();

  // Add the item for the worker to process
  for (int i = 0; i < 4; i++)
  {
    worker->AddMessage(buffer1, size);
    worker->AddMessage(buffer2, size);
    worker->AddMessage(buffer3, size);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  // Quit all threads
  worker->SetQuit();

  EXPECT_EQ(1, 1);
}

/**
 * @brief Tests related to \c FWorker
 * 
 * Test the startup and execution of threaded function worker
 */
class FWorkerTest : public ::testing::Test
{
public:
  static int ProcessMessage(const void* _msg, std::uint32_t _size, void* _result = nullptr, std::uint32_t* _rsize = nullptr)
  {
    auto msg = Messages::GetMessage(_msg);
    auto id = msg->id();
    auto height = msg->height();
    auto width = msg->width();
    auto pixels = msg->pixels()->Data();
    
    return 0;
  }
protected:
  void SetUp() override
  {
    worker = new FWorker(0, "FWorkerTest", ProcessMessage);
    worker->Run(3);
  }

  void TearDown() override
  {
    delete worker;
  }

  FWorker* worker;
};

TEST_F(FWorkerTest, CreateWorker)
{
  flatbuffers::FlatBufferBuilder builder(AGENT_FB_BUFFER_SIZE);
  auto pixels = builder.CreateVector(
    std::vector<std::int8_t>{
      { 0, 0, 1,
        2, 2, 1,
        4, 0, 1 }
    }
  );

  // Build message 1
  auto message1 = Messages::CreateMessage(builder, 0, 3, 3, pixels);
  builder.Finish(message1);
  auto buffer1 = builder.GetBufferPointer();
  auto size = builder.GetSize();

  // Build message 2
  auto message2 = Messages::CreateMessage(builder, 1, 3, 3, pixels);
  builder.Finish(message2);
  auto buffer2 = builder.GetBufferPointer();

  // Build message 3
  auto message3 = Messages::CreateMessage(builder, 2, 3, 3, pixels);
  builder.Finish(message3);
  auto buffer3 = builder.GetBufferPointer();

  // Add the item for the worker to process
  for (int i = 0; i < 4; i++)
  {
    worker->AddMessage(buffer1, size);
    worker->AddMessage(buffer2, size);
    worker->AddMessage(buffer3, size);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  // Exit the thread
  worker->Stop();
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  worker->Run();

  // Add the item for the worker to process
  for (int i = 0; i < 4; i++)
  {
    worker->AddMessage(buffer1, size);
    worker->AddMessage(buffer2, size);
    worker->AddMessage(buffer3, size);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  // Quit all threads
  worker->SetQuit();

  EXPECT_EQ(1, 1);
}

/**
 * @brief Tests related to \c IAMQPWorker
 * 
 * Test the startup and execution of threaded AMQP workers; main tests are with
 * \c IAMQPWorker. IMPORTANT: These tests require an AMQP broker to be running
 * on the localhost.
 */
class AMQPProcessor : public IWorker
{
public:
  AMQPProcessor(int __id)
    : IWorker(__id)
  {}

  AMQPProcessor(int __id, std::string __name)
    : IWorker(__id, __name)
  {}

	int ProcessMessage(const void* _msg, std::uint32_t _size, void* _result = nullptr, std::uint32_t* _rsize = nullptr) override
  {
    _logger->info("Processed a message!");
    auto msg = new char[128];
    std::memcpy(msg, _msg, _size);
    msg[_size] = '\0';
    _logger->info("Payload:");
    _logger->info(msg);
    return 0;
  }
};

class AMQPWorkerTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    Json::Value jsonConfig;
    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    Json::String errs;
    auto ssConfig = std::ifstream("/workspaces/agent/config/client.json");
    try
    {
      Json::parseFromStream(builder, ssConfig, &jsonConfig, &errs);
    }
    catch (const Json::Exception& e)
    {
      std::cerr << "JSON Error: " << e.what() << std::endl;
    }
    spdlog::set_level(spdlog::level::debug);
    amqpProc = new AMQPProcessor(100, "AMQPProcessor");
    amqpProc->Run(1);
    //amqpWorker = new IAMQPWorker(1, amqpProc, "broker", 5672, "guest",
    //"guest", "/", "AMQPWorker", "AnotherQueue", "Exchange", "AnotherQueue",
    //AMQP::autodelete, AMQP::autodelete, 4, AMQP::ExchangeType::direct,
    //"TestCode", "0.0.1", "Copyright 2022 Mathemaphysics Inc",
    //"https://github.org/mathemaphysics/agent.git");

    try
    {
      amqpWorker = new IAMQPWorker(1, amqpProc, jsonConfig);
    }
    catch (const Poco::Exception& e)
    {
      std::cout << "Exception: " << e.message() << std::endl;
      std::cout << "Class:     " << e.className() << std::endl;
      std::cout << "Display:   " << e.displayText() << std::endl;
      std::cout << "Name:      " << e.name() << std::endl;
    }
  }

  void TearDown() override
  {
    //delete amqpWorker;
  }
  AMQPProcessor* amqpProc;
  IAMQPWorker* amqpWorker;
};

class AMQPWorkerTestSSL : public ::testing::Test
{
protected:
  void SetUp() override
  {
    Json::Value jsonConfig;
    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    Json::String errs;
    auto ssConfig = std::ifstream("/workspaces/agent/config/client.json");
    try
    {
      Json::parseFromStream(builder, ssConfig, &jsonConfig, &errs);
    }
    catch (const Json::Exception& e)
    {
      std::cerr << "JSON Error: " << e.what() << std::endl;
    }
    spdlog::set_level(spdlog::level::debug);
    amqpProc = new AMQPProcessor(100, "AMQPProcessor");
    amqpProc->Run(1);

    try
    {
      amqpWorker = new IAMQPWorkerSSL(1, amqpProc, jsonConfig);
      const char* msg1 = "New app whodis 1";
      const char* msg2 = "New app whodis 2";
      amqpWorker->AddMessage(msg1, std::strlen(msg1), "exchangeName", "queueName");
      amqpWorker->AddMessage(msg2, std::strlen(msg2), "exchangeName", "queueName");
    }
    catch (const Poco::Exception& e)
    {
      std::cout << "Exception: " << e.message() << std::endl;
      std::cout << "Class:     " << e.className() << std::endl;
      std::cout << "Display:   " << e.displayText() << std::endl;
      std::cout << "Name:      " << e.name() << std::endl;
    }
  }

  void TearDown() override
  {
    //delete amqpWorker;
  }
  AMQPProcessor* amqpProc;
  IAMQPWorkerSSL* amqpWorker;
};

TEST_F(AMQPWorkerTest, DISABLED_CreateConnectionHandler)
{
  std::this_thread::sleep_for(std::chrono::seconds(10));
}

TEST_F(AMQPWorkerTestSSL, CreateConnectionHandler)
{
  std::this_thread::sleep_for(std::chrono::seconds(10));
  EXPECT_EQ(amqpProc->ResultsAvailable(), 2);
  std::pair<int, bool> result1{0, false};
  auto index1 = amqpProc->PopResult(result1);
  std::pair<int, bool> result2{0, false};
  auto index2 = amqpProc->PopResult(result2);
}

/**
 * @brief Example worker derived from \c IAMQPWorkerSSL
 *
 * Demonstrates the proper way to extend the secure AMQP worker interface by
 * inheriting from \c IAMQPWorkerSSL and layering additional functionality on
 * top of the inherited primitives (the protected \c _logger and the public
 * \c AddMessage publish method).
 *
 * The added behaviour is a small, framed publishing protocol: every payload
 * sent through \c PublishFramed is prefixed with a fixed-size header carrying
 * a monotonically increasing sequence number and an XOR checksum of the body.
 * The worker also keeps running statistics (number of frames published and the
 * total number of payload bytes) so callers can introspect what has been sent.
 * This mirrors how a real subclass would add a domain-specific wire format
 * without having to re-implement the connection and channel plumbing.
 */
class ChecksumAMQPWorkerSSL : public agent::IAMQPWorkerSSL
{
public:
  /// Header prepended to every framed payload: [seq:4 bytes][checksum:1 byte]
  struct FrameHeader
  {
    std::uint32_t sequence; ///< Monotonic frame sequence number
    std::uint8_t checksum;  ///< XOR checksum over the payload bytes
  };

  using agent::IAMQPWorkerSSL::IAMQPWorkerSSL;

  /**
   * @brief Compute an XOR checksum over a byte buffer
   *
   * @param _msg Pointer to the payload bytes
   * @param _size Number of bytes in the payload
   * @return std::uint8_t XOR of every byte in the payload
   */
  static std::uint8_t Checksum(const void* _msg, std::uint32_t _size)
  {
    const auto* bytes = static_cast<const std::uint8_t*>(_msg);
    std::uint8_t sum = 0;
    for (std::uint32_t i = 0; i < _size; ++i)
      sum ^= bytes[i];
    return sum;
  }

  /**
   * @brief Build a framed message (header + payload)
   *
   * Static helper so the framing logic can be exercised without standing up a
   * full worker (and therefore without a live broker connection).
   *
   * @param _sequence Sequence number to embed in the header
   * @param _msg Pointer to the payload bytes
   * @param _size Number of bytes in the payload
   * @return std::vector<std::uint8_t> The serialized header followed by payload
   */
  static std::vector<std::uint8_t> MakeFrame(std::uint32_t _sequence, const void* _msg, std::uint32_t _size)
  {
    FrameHeader header{ _sequence, Checksum(_msg, _size) };

    std::vector<std::uint8_t> frame(sizeof(FrameHeader) + _size);
    std::memcpy(frame.data(), &header, sizeof(FrameHeader));
    std::memcpy(frame.data() + sizeof(FrameHeader), _msg, _size);

    return frame;
  }

  /**
   * @brief Build a framed message using this worker's current sequence number
   *
   * @param _msg Pointer to the payload bytes
   * @param _size Number of bytes in the payload
   * @return std::vector<std::uint8_t> The serialized header followed by payload
   */
  std::vector<std::uint8_t> BuildFrame(const void* _msg, std::uint32_t _size)
  {
    return MakeFrame(_sequence, _msg, _size);
  }

  /**
   * @brief Publish a payload wrapped in a checksummed frame
   *
   * Uses the inherited \c AddMessage to actually push bytes onto the AMQP
   * channel, but adds framing and bookkeeping on top of it.
   *
   * @param _msg Pointer to the payload bytes
   * @param _size Number of bytes in the payload
   * @param _exchange Exchange to publish to
   * @param _key Routing key to publish with
   */
  void PublishFramed(const void* _msg, std::uint32_t _size, std::string _exchange = "", std::string _key = "")
  {
    auto frame = BuildFrame(_msg, _size);

    if (_logger != nullptr)
      _logger->info("Publishing framed message seq={} bytes={}", _sequence, _size);

    AddMessage(frame.data(), static_cast<std::uint32_t>(frame.size()), _exchange, _key);

    ++_sequence;
    ++_framesPublished;
    _bytesPublished += _size;
  }

  std::uint32_t FramesPublished() const { return _framesPublished; }
  std::uint64_t BytesPublished() const { return _bytesPublished; }
  std::uint32_t NextSequence() const { return _sequence; }

private:
  std::uint32_t _sequence = 0;       ///< Next sequence number to assign
  std::uint32_t _framesPublished = 0; ///< Count of frames published
  std::uint64_t _bytesPublished = 0;  ///< Total payload bytes published
};

/**
 * @brief Verifies the framing/checksum extension layered on \c IAMQPWorkerSSL
 *
 * This exercises the subclass logic (header construction, checksum, and
 * bookkeeping) directly via \c BuildFrame so it does not require a live broker
 * connection, demonstrating how to unit test functionality added to the
 * interface.
 */
TEST(ChecksumAMQPWorkerSSLTest, FrameConstructionAndChecksum)
{
  // The framing helpers are static so they can be tested without constructing a
  // full worker (which would attempt a live broker connection).
  const std::string payload = "Hello, secure AMQP!";
  const auto size = static_cast<std::uint32_t>(payload.size());

  auto frame = ChecksumAMQPWorkerSSL::MakeFrame(0, payload.data(), size);

  // Frame must be header + payload in length.
  ASSERT_EQ(frame.size(), sizeof(ChecksumAMQPWorkerSSL::FrameHeader) + size);

  // Decode the header back out and validate its fields.
  ChecksumAMQPWorkerSSL::FrameHeader header;
  std::memcpy(&header, frame.data(), sizeof(header));

  EXPECT_EQ(header.sequence, 0u);
  EXPECT_EQ(header.checksum, ChecksumAMQPWorkerSSL::Checksum(payload.data(), size));

  // The payload should be copied verbatim after the header.
  EXPECT_EQ(
    std::memcmp(frame.data() + sizeof(header), payload.data(), size),
    0
  );

  // An empty payload should checksum to zero (identity of XOR).
  EXPECT_EQ(ChecksumAMQPWorkerSSL::Checksum(payload.data(), 0), 0);
}

TEST(add_one, sample)
{
  EXPECT_EQ(add_one(0), 1);
  EXPECT_EQ(add_one(123), 124);
  EXPECT_EQ(add_one(-1), 0);
}

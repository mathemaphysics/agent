#include "agent/agent.hpp"
#include "Worker.hpp"
#include "IConnectionHandler.hpp"
#include "IAMQPWorker.hpp"
#include "FWorker.hpp"
#include "Message_generated.h"

#include <thread>
#include <chrono>

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

	int ProcessMessage(const void* _msg, std::uint32_t _size, void* _result = nullptr, std::uint32_t* _rsize = nullptr) const override
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
    amqpProc = new AMQPProcessor(100, "AMQPProcessor");
    amqpProc->Run(2);
    amqpWorker = new IAMQPWorker(1, amqpProc, "rabbitmq", 5672, "AMQPWorker", "guest", "guest", "/");
  }

  void TearDown() override
  {
    //delete amqpWorker;
  }
  AMQPProcessor* amqpProc;
  IAMQPWorker* amqpWorker;
};

//TEST_F(AMQPWorkerTest, CreateConnectionHandler)
//{
//  std::this_thread::sleep_for(std::chrono::seconds(120));
//}

TEST(add_one, sample)
{
  EXPECT_EQ(add_one(0), 1);
  EXPECT_EQ(add_one(123), 124);
  EXPECT_EQ(add_one(-1), 0);
}

#include "agent/agent.hpp"
#include "Worker.hpp"
#include "ConnectionHandler.hpp"
#include "Message_generated.h"

#include <thread>
#include <chrono>

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

using namespace agent;

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

class ConnectionHandlerTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    handler = new ConnectionHandler(0);
  }

  void TearDown() override
  {
    
  }

  ConnectionHandler* handler;
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

  // Wait for a bit
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Exit the thread
  //worker->SetQuit();
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

  //worker->Stop();
  worker->SetQuit();

  EXPECT_EQ(1, 1);
}

TEST(add_one, sample)
{
  EXPECT_EQ(add_one(0), 1);
  EXPECT_EQ(add_one(123), 124);
  EXPECT_EQ(add_one(-1), 0);
}

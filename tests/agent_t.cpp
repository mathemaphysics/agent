#include "agent/agent.hpp"
#include "Worker.hpp"
#include "TWorker.hpp"
#include "Message_generated.h"
#include <gtest/gtest.h>

using namespace agent;

class AgentWorkerTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    worker = new Worker(0);
    worker->SetName(std::string("Test"));
  }

  void TearDown() override
  {
    delete worker;
  }

  Worker* worker;
};

class AgentTWorkerTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    auto logger = spdlog::stdout_color_mt("AgentTWorkerTest");
    tworker = new TWorker(0, "AgentTWorkerTest");
  }

  void TearDown() override
  {
    delete tworker;
  }

  TWorker* tworker;
};

TEST_F(AgentWorkerTest, CreateWorker)
{
  flatbuffers::FlatBufferBuilder builder(AGENT_FB_BUFFER_SIZE);
  auto pixels = builder.CreateVector(
    std::vector<std::int8_t>{
      { 0, 0, 1,
        2, 2, 1,
        4, 0, 1 }
    }
  );
  auto message = Messages::CreateMessage(builder, 0, 3, 3, pixels);
  builder.Finish(message);
  auto buffer = builder.GetBufferPointer();
  auto size = builder.GetSize();
  EXPECT_EQ(worker->ProcessMessage(buffer, size), 0);
}

TEST_F(AgentTWorkerTest, CreateTWorker)
{
  // Delay a second then add a message
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Make another message to add to the thread worker
  flatbuffers::FlatBufferBuilder builder(AGENT_FB_BUFFER_SIZE);
  auto pixels = builder.CreateVector(
    std::vector<std::int8_t>{
      { 0, 0, 1,
        2, 2, 1,
        4, 0, 1 }
    }
  );
  auto message = Messages::CreateMessage(builder, 0, 3, 3, pixels);
  builder.Finish(message);
  auto buffer = builder.GetBufferPointer();
  auto size = builder.GetSize();

  // Add the message
  tworker->AddMessage(buffer, size);

  // Wait and add another one
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Add it again
  tworker->AddMessage(buffer, size);

  // Wait again and quit
  std::this_thread::sleep_for(std::chrono::seconds(2));

  // Stop the thread
  tworker->Quit();
  EXPECT_EQ(1, 1);
}

TEST(add_one, sample)
{
  EXPECT_EQ(add_one(0), 1);
  EXPECT_EQ(add_one(123), 124);
  EXPECT_EQ(add_one(-1), 0);
}

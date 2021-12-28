#include "agent/agent.hpp"
#include "Worker.hpp"
#include "Message_generated.h"
#include <gtest/gtest.h>

using namespace agent;

class AgentTest : public ::testing::Test
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

TEST_F(AgentTest, CreateWorker)
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

TEST(add_one, sample)
{
  EXPECT_EQ(add_one(0), 1);
  EXPECT_EQ(add_one(123), 124);
  EXPECT_EQ(add_one(-1), 0);
}

/**
 * @file basic_worker.cpp
 * @brief Minimal end-to-end example using the built-in agent::Worker.
 *
 * This is the introductory example referenced in the top-level README. It
 * creates the library's built-in agent::Worker, starts it on a couple of
 * threads, serializes a small "image" message with FlatBuffers, and submits a
 * handful of messages for the worker to process.
 *
 * The agent::Worker::ProcessMessage implementation simply logs the message it
 * received; see custom_worker.cpp for an example that does real work.
 */

#include "agent/Worker.hpp"
#include "agent/agent_config.hpp"
#include "Message_generated.h"

#include <chrono>
#include <thread>
#include <vector>
#include <cstdint>

#include <flatbuffers/flatbuffers.h>

int main()
{
    // Create the built-in worker, give it an ID and a name.
    agent::Worker worker(0, "BasicWorker");

    // Run the worker with two threads.
    worker.Run(2);

    // Give the threads a moment to spin up.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Serialize a small 3x3 "image" message using FlatBuffers.
    flatbuffers::FlatBufferBuilder builder(AGENT_FB_BUFFER_SIZE);
    auto pixels = builder.CreateVector(
        std::vector<std::int8_t>{
            0, 0, 1,
            2, 2, 1,
            4, 0, 1
        }
    );
    auto message = agent::Messages::CreateMessage(builder, 0, 3, 3, pixels);
    builder.Finish(message);

    auto buffer = builder.GetBufferPointer();
    auto size = builder.GetSize();

    // Simulate receiving asynchronous messages: send three messages per
    // iteration, four times, separated by about half a second.
    for (int i = 0; i < 4; ++i)
    {
        worker.AddMessage(buffer, size);
        worker.AddMessage(buffer, size);
        worker.AddMessage(buffer, size);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Tell the worker threads to quit and let them drain.
    worker.SetQuit();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    return 0;
}

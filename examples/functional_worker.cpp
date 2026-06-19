/**
 * @file functional_worker.cpp
 * @brief Example of using agent::FWorker with a std::function / lambda.
 *
 * Sometimes deriving a whole new class is overkill. agent::FWorker lets you
 * provide the processing logic as a callable (a free function, a lambda, or any
 * std::function) at construction time. This is handy for quick jobs or when the
 * behaviour needs to be chosen at runtime.
 *
 * The callable has the same signature as IWorker::ProcessMessage:
 *   int(const void* msg, std::uint32_t size, void* result, std::uint32_t* rsize)
 */

#include "agent/FWorker.hpp"
#include "agent/agent_config.hpp"
#include "Message_generated.h"

#include <chrono>
#include <cstdint>
#include <thread>
#include <vector>

#include <flatbuffers/flatbuffers.h>
#include <spdlog/spdlog.h>

int main()
{
    // Provide the message-processing logic inline as a lambda.
    auto processImage = [](const void* msg, std::uint32_t size,
                           void* result, std::uint32_t* rsize) -> int
    {
        (void)size;
        (void)result;
        (void)rsize;

        auto message = agent::Messages::GetMessage(msg);
        const int id = message->id();
        const int width = message->width();
        const int height = message->height();

        spdlog::info("FWorker handled message id={} ({}x{})", id, width, height);

        return id;
    };

    agent::FWorker worker(42, "FunctionalWorker", processImage);
    worker.Run(2);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Serialize a message to submit. Note FWorker::AddMessage takes a
    // non-const pointer, so we keep our own mutable buffer.
    flatbuffers::FlatBufferBuilder builder(AGENT_FB_BUFFER_SIZE);
    auto pixels = builder.CreateVector(
        std::vector<std::int8_t>{
            9, 8, 7,
            6, 5, 4,
            3, 2, 1
        }
    );
    auto message = agent::Messages::CreateMessage(builder, 1, 3, 3, pixels);
    builder.Finish(message);

    std::vector<std::uint8_t> payload(
        builder.GetBufferPointer(),
        builder.GetBufferPointer() + builder.GetSize());
    const std::uint32_t size = static_cast<std::uint32_t>(payload.size());

    for (int i = 0; i < 4; ++i)
    {
        worker.AddMessage(payload.data(), size);
        worker.AddMessage(payload.data(), size);
        worker.AddMessage(payload.data(), size);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    worker.SetQuit();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    return 0;
}

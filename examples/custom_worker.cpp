/**
 * @file custom_worker.cpp
 * @brief Example of writing your own worker by deriving from agent::IWorker.
 *
 * The built-in agent::Worker only logs the messages it receives. In a real
 * application you want your worker to actually do something with the payload.
 * This example shows the recommended pattern: derive from agent::IWorker and
 * override ProcessMessage to deserialize the message and operate on its data.
 *
 * Here the "work" is a trivial image operation: we sum all of the pixel values
 * of the incoming image and log the brightness. Swap this out for OpenCV calls,
 * a neural-network inference, a database write, or anything else you need.
 */

#include "agent/IWorker.hpp"
#include "agent/agent_config.hpp"
#include "Message_generated.h"

#include <chrono>
#include <cstdint>
#include <string>
#include <thread>
#include <vector>

#include <flatbuffers/flatbuffers.h>
#include <spdlog/spdlog.h>

namespace
{
    /**
     * @brief A worker that computes the total brightness of each image.
     */
    class ImageWorker : public agent::IWorker
    {
    public:
        explicit ImageWorker(unsigned int id)
            : agent::IWorker(id)
        {
        }

        ImageWorker(unsigned int id, std::string name)
            : agent::IWorker(id, std::move(name))
        {
        }

        int ProcessMessage(const void* msg, std::uint32_t size,
                           void* result = nullptr,
                           std::uint32_t* rsize = nullptr) override
        {
            (void)size;

            // Step 1: deserialize the FlatBuffers message.
            auto message = agent::Messages::GetMessage(msg);
            const int id = message->id();
            const int width = message->width();
            const int height = message->height();
            const auto* pixels = message->pixels();

            // Step 2: do some real work on the data. Here we sum the pixels.
            long long brightness = 0;
            if (pixels != nullptr)
            {
                for (auto value : *pixels)
                {
                    brightness += value;
                }
            }

            _logger->info(
                "Processed image id={} ({}x{}) total brightness={}",
                id, width, height, brightness);

            // Step 3 (optional): hand a result back to the caller.
            if (result != nullptr && rsize != nullptr)
            {
                *static_cast<long long*>(result) = brightness;
                *rsize = static_cast<std::uint32_t>(sizeof(brightness));
            }

            // ProcessMessage returns the ID of the message it processed.
            return id;
        }
    };
}

int main()
{
    ImageWorker worker(7, "ImageWorker");
    worker.Run(3);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Build a few distinct messages so we can see different IDs processed.
    flatbuffers::FlatBufferBuilder builder(AGENT_FB_BUFFER_SIZE);

    std::vector<std::pair<const std::uint8_t*, std::uint32_t>> messages;
    std::vector<std::vector<std::uint8_t>> storage;

    for (unsigned int id = 0; id < 3; ++id)
    {
        builder.Clear();
        auto pixels = builder.CreateVector(
            std::vector<std::int8_t>{
                static_cast<std::int8_t>(id),     1, 2,
                3,                                 4, 5,
                6,                                 7, 8
            }
        );
        auto message = agent::Messages::CreateMessage(builder, id, 3, 3, pixels);
        builder.Finish(message);

        // Copy the serialized bytes so they outlive the builder reuse.
        const std::uint8_t* data = builder.GetBufferPointer();
        const std::uint32_t size = builder.GetSize();
        storage.emplace_back(data, data + size);
        messages.emplace_back(storage.back().data(), size);
    }

    for (int round = 0; round < 4; ++round)
    {
        for (const auto& [data, size] : messages)
        {
            worker.AddMessage(data, size);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    worker.SetQuit();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    return 0;
}

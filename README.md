# Welcome to Agent

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![GitHub Workflow Status](https://img.shields.io/github/workflow/status/mathemaphysics/agent/CI)](https://github.com/mathemaphysics/agent/actions?query=workflow%3ACI)
[![PyPI Release](https://img.shields.io/pypi/v/agent.svg)](https://pypi.org/project/agent)
[![Documentation Status](https://readthedocs.org/projects/agent/badge/)](https://agent.readthedocs.io/)
[![codecov](https://codecov.io/gh/mathemaphysics/agent/branch/main/graph/badge.svg)](https://codecov.io/gh/mathemaphysics/agent)

# Introduction
Agent is a worker thread library that you can use to easily process any chunk of
work that you can package into a chunk of memory. In the source you'll find a
class called `Worker` which inherits from `IWorker` as its base and which writes
a function `ProcessMessage` that performs some job on data stored in a pointer
by calling `AddMessage` on the data from the main thread, your `main` entry
point. This call adds this data message to the queue of work that the `Worker`
needs to perform. Once you start up the `Worker` you can then proceed to call
`AddMessage(message, size)` to add a job to perform.

# An example `Worker` thread

Start with design of your message. For the purposes of this project, we'll be
using Google's FlatBuffers.  An example used to write the `Worker` class is just
called `Message`. To see the final result of this setup take a look at the
contents of the examples directory. Our example will be a very simple image
processor.

First we create a FlatBuffers definition in a file named `Message.fbs`. It will
define the format for the image and related data we'll be giving to the worker
to work on. The contents are as follows.

```
namespace Messages;

table Message {
    id:uint;
    height:uint;
    width:uint;
    pixels:[byte];
}

root_type Message;
```

In this step what you've just done is to create a message format that will be
passed to your worker containing all the information and data needed to do the
job you require it to do. In this case the data is an image which includes the
message ID, its width and height, and the image data itself.

At the top you'll see the first line gives the namespace in which you'd like the
object `Message` to be created. So the compiler will produce a header file for a
class named `Messages::Message`. After this you'll see the definition of
`Message` itself. The first field in your `table`, is an `unsigned int` called
`id`. The next two are `height` and `width`, each also `unsigned int` fields
collectively telling us how much data to expect in the next field, `pixels`,
which is an array of `byte`, denoted `[byte]`. The final line in `Message.fbs`
tells the compiler that you want the functions it creates to be named using
`Message` in the name. For example, as a result of this line `flatc` will write
a function named `CreateMessage`. Without this line the resulting function won't
be named accordingly. Once you've made it through this step you can run the
FlatBuffers compiler to produce the header file that can be included in your
project directly. It turns out that FlatBuffers has a built-in CMake module
which allows you to create these header files as proper targets if your project
uses CMake. More on this later.

Now that we have `Message.fbs` completed we can compile it with `flatc`, the
FlatBuffers compiler. You may have `flatc` installed separated, but it is
included in `agent`'s `CMakeLists.txt`. If you installed `agent` then you
should have `flatc` if you installed it into your path.

```sh
flatc --cpp Message.fbs
```

The end result should be a file named `Message_generated.hpp`, which you'll want
to include in your worker's source.

Now for the meat of the sample code. You need create your `Worker` class to
inherit from `IWorker`. `Worker.hpp` will declare a constructor which takes care of some details
that the abstract class `IWorker` needs to know.

```cpp
#pragma once

#include "IWorker.hpp"
#include <string>
#include <cstdint>

class Worker : public IWorker
{
public:
    Worker(unsigned int _id);
    Worker(unsigned int _id, std::string _name);
    ~Worker();

    int ProcessMessage(const void* _msg, std::uint32_t _size) const override;
};
```

Your source file for `Worker` will only need a couple definitions. You'll see that there are two
constructors we've defined. One will match a similar constructor for `IWorker` which takes only
a single number which becomes the worker's identification number. There's also another `IWorker`
constructor which takes a name to identify it to the logger as well. If you call the first
constructor the name will be filled in with a default value.

```cpp
#include "Worker.hpp"
#include "Message_generated.h"

#include <string>

// We use spdlog embedded into IWorker
#include <spdlog/spdlog.h>
#include <flatbuffers/flatbuffers.h>

Worker::Worker(unsigned int _id)
    : IWorker(_id) {}

Worker::Worker(unsigned int _id, std::string _name)
    : IWorker(_id, _name) {}

Worker::~Worker()
{
    _logger->info("Worker {} finished", GetId());
}

int Worker::ProcessMessage(const void* _msg, std::uint32_t _size) const
{
    // The first step: Deserialize and get your data out
    auto message = Messages::GetMessage(_msg);
    int id = message->id();
    int width = message->width();
    int height = message->height();
    auto pixels = message->pixels()->Data();

    /**
     * Use the built-in logger to announce events
     */
    _logger->info("[{}] Received message: id: {} width: {} height: {}",
        agent::ThreadToString(std::this_thread::get_id()),
        id, width, height);

    /**
     * Now you can do some work; maybe pull in OpenCV?
     */

    // ProcessMessage is expected to return the ID of the message you processed
    return id;
}
```

The most important part is in the definition of `ProcessMessage`. Its sole job
is to take in a message, deserialize it, and process whatever data it contains.
This could be anything you want it to be. You could import any other library and
do any additional work you need. For image processing you might just want to
import OpenCV. You can imagine limitless use cases.

Now that we have a `Worker` class you'll want to create one and try to do some
work on a few threads. To do this create a source file with a main entry point
named `main.cpp` and within it create a `Worker`, name it, and run it. Once you
have it running you'll need to serialize a message and send it off to be
processed.

```cpp
#include "Worker.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char** argv)
{
    // Create your worker
    auto worker = agent::Worker(0, "MyWorker");

    // Run the worker with two threads
    worker.Run(2);

    // Give it a second to get moving
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Load in some data and send it off
    flatbuffers::FlatBufferBuilder builder(8096);
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

    /**
     * Now simulate receiving asynchronous messages; send your worker a chunk of
     * three messages four times separated by about half a second 
     */
    for (int i = 0; i < 4; i++)
    {
        worker.AddMessage(buffer, size);
        worker.AddMessage(buffer, size);
        worker.AddMessage(buffer, size);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0;
}
```

At this point you're ready to build. If you're accustomed to CMake. You'll
notice that the samples directory already contains a pre-built version of the
`Worker` class you just wrote in this introduction. At this point `Worker` is a
part of the basic unit tests which can be found in `tests/agent_t.cpp`. Making
sure that `BUILD_TESTS=ON` is set during your build with CMake will ensure that
it will be compiled into the `bin/tests` binary. We're in the process of
rearranging the code. Eventually this will be placed into a samples or examples
directory.

# Prerequisites

Building Agent requires the following software installed:

* A C++17-compliant compiler
* CMake `>= 3.9`
* Doxygen (optional, documentation building is skipped if missing)* Python `>= 3.6` for building Python bindings

# Building Agent

The following sequence of commands builds Agent.
It assumes that your current working directory is the top-level directory
of the freshly cloned repository:

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

The build process can be customized with the following CMake variables,
which can be set by adding `-D<var>={ON, OFF}` to the `cmake` call:

* `BUILD_TESTING`: Enable building of the test suite (default: `ON`)
* `BUILD_DOCS`: Enable building the documentation (default: `ON`)
* `BUILD_PYTHON`: Enable building the Python bindings (default: `ON`)

# Documentation

Agent provides a Sphinx-based documentation, that can
be browsed [online at readthedocs.org](https://agent.readthedocs.io).

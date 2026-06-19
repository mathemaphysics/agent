# Agent Examples

This directory contains runnable examples that demonstrate how to use the
`agent` worker library. Each example is built as its own executable when the
project is configured with `-DBUILD_EXAMPLES=ON` (the default).

| Example | Source | What it shows |
| --- | --- | --- |
| `basic_worker` | [basic_worker.cpp](basic_worker.cpp) | The minimal end-to-end flow from the README: build a FlatBuffers message and feed it to the built-in `agent::Worker`. |
| `custom_worker` | [custom_worker.cpp](custom_worker.cpp) | Writing your own worker by deriving from `agent::IWorker` and overriding `ProcessMessage`. |
| `functional_worker` | [functional_worker.cpp](functional_worker.cpp) | Using `agent::FWorker` to supply the processing logic as a `std::function`/lambda instead of subclassing. |

All examples share the same `Message` FlatBuffers schema that the library uses
(`src/Message.fbs`). The generated header (`Message_generated.h`) is produced by
the `schemas` build target, so the examples simply depend on it.

## Building

From the repository root:

```sh
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON ..
cmake --build . --target basic_worker custom_worker functional_worker
```

The resulting binaries are placed in the build tree (for example
`build/examples/basic_worker`).

## Running

```sh
./examples/basic_worker
./examples/custom_worker
./examples/functional_worker
```

Each program spins up a small pool of worker threads, submits a few serialized
messages, and logs what it processed using the `spdlog` logger that is embedded
in the worker classes.

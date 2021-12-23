# Welcome to Agent

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![GitHub Workflow Status](https://img.shields.io/github/workflow/status/mathemaphysics/agent/CI)](https://github.com/mathemaphysics/agent/actions?query=workflow%3ACI)
[![PyPI Release](https://img.shields.io/pypi/v/agent.svg)](https://pypi.org/project/agent)
[![Documentation Status](https://readthedocs.org/projects/agent/badge/)](https://agent.readthedocs.io/)
[![codecov](https://codecov.io/gh/mathemaphysics/agent/branch/main/graph/badge.svg)](https://codecov.io/gh/mathemaphysics/agent)


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

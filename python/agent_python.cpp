#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "agent/agent.hpp"

namespace py = pybind11;

namespace agent {

PYBIND11_MODULE(agent, m)
{
  m.doc() = "Python Bindings for Agent";
  m.def("add_one", &add_one, "Increments an integer value");
}

} // namespace agent

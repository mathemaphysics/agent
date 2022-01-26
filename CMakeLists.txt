cmake_minimum_required(VERSION 3.9)

# Set a name and a version number for your project:
project(agent VERSION 0.0.1 LANGUAGES CXX)

# Initialize some default paths
include(GNUInstallDirs)

# Define the minimum C++ standard that is required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Compilation options
set(BUILD_PYTHON ON CACHE BOOL "Enable building of Python bindings")
set(BUILD_DOCS ON CACHE BOOL "Enable building of documentation")

set(AGENT_FB_BUFFER_SIZE "1024")
set(AGENT_CONN_BUFFER_SIZE "8*1024*1024")
set(AGENT_CONN_TEMP_BUFFER_SIZE "8*1024*1024")

configure_file(include/agent/agent_config.hpp.in include/agent/agent_config.hpp)

include_directories(${CMAKE_BINARY_DIR}/include)

include(FetchContent)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY  https://github.com/gabime/spdlog.git
    GIT_TAG         v1.9.2
)

FetchContent_Declare(
    flatbuffers
    GIT_REPOSITORY  https://github.com/google/flatbuffers.git
    GIT_TAG         v2.0.0
)

FetchContent_Declare(
    poco
    GIT_REPOSITORY  https://github.com/pocoproject/poco.git
    GIT_TAG         poco-1.11.0-release
)

FetchContent_Declare(
    amqpcpp
    GIT_REPOSITORY  https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git
    GIT_TAG         v4.3.14
)

FetchContent_Declare(
    jsoncpp
    GIT_REPOSITORY  https://github.com/open-source-parsers/jsoncpp.git
    GIT_TAG         1.9.4
)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY  https://github.com/google/googletest.git
    GIT_TAG         release-1.11.0
)

FetchContent_MakeAvailable(amqpcpp)
FetchContent_MakeAvailable(poco)
FetchContent_MakeAvailable(flatbuffers)
FetchContent_MakeAvailable(jsoncpp)
FetchContent_MakeAvailable(spdlog)
FetchContent_MakeAvailable(googletest)

FetchContent_GetProperties(amqpcpp)
set(AMQPCPP_INCLUDE_DIR ${amqpcpp_SOURCE_DIR}/include)

FetchContent_GetProperties(flatbuffers)
include(${flatbuffers_SOURCE_DIR}/CMake/BuildFlatBuffers.cmake)

# compile the library
add_subdirectory(src)

# compile the application
add_subdirectory(app)

# compile the tests
include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

if(BUILD_DOCS)
  # Add the documentation
  add_subdirectory(doc)
endif()
if(BUILD_PYTHON)
  # Add Python bindings
  add_subdirectory(ext/pybind11)
  
  add_subdirectory(python)
endif()

# Add an alias target for use if this project is included as a subproject in another project
add_library(agent::agent ALIAS agent)

# Install targets and configuration
install(
  TARGETS agent spdlog
  EXPORT agent-config
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
  EXPORT agent-config
  NAMESPACE agent::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/agent
)

install(
  DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# This prints a summary of found dependencies
include(FeatureSummary)
feature_summary(WHAT ALL)

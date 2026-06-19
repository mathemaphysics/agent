# Examples demonstrating how to use the agent worker library.
#
# Each example links against the `agent` library and needs the generated
# FlatBuffers header (Message_generated.h). That header is produced by the
# `schemas` target defined in src/, and is generated into the src/ directory,
# so we add it to the include path and depend on the `schemas` target.

set(AGENT_EXAMPLES
  basic_worker
  custom_worker
  functional_worker
)

foreach(example ${AGENT_EXAMPLES})
  add_executable(${example} ${example}.cpp)
  target_compile_features(${example} PRIVATE cxx_std_17)
  target_include_directories(${example} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
  add_dependencies(${example} schemas)
  target_link_libraries(${example} PRIVATE agent spdlog::spdlog flatbuffers)
endforeach()

# Convenience target to build all examples at once: `cmake --build . --target examples`
add_custom_target(examples DEPENDS ${AGENT_EXAMPLES})

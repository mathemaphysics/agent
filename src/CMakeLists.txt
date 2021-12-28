build_flatbuffers(Message.fbs "" schemas "" . "" "")

add_library(agent agent.cpp Worker.cpp TWorker.cpp)
add_dependencies(agent schemas)
target_include_directories(agent PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(agent flatbuffers spdlog::spdlog)

build_flatbuffers(Message.fbs "" schemas "" . "" "")

add_library(agent agent.cpp Worker.cpp FWorker.cpp IConnectionHandler.cpp IConnectionHandlerSSL.cpp Buffer.cpp)
target_compile_features(agent PUBLIC cxx_std_17)
add_dependencies(agent schemas)
target_include_directories(agent PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  $<BUILD_INTERFACE:${AMQPCPP_INCLUDE_DIR}>
)

target_link_libraries(agent flatbuffers spdlog::spdlog amqpcpp Poco::Net Poco::NetSSL Poco::Crypto jsoncpp_lib)
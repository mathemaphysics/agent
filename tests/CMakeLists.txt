add_executable(tests tests.cpp agent_t.cpp)
target_compile_features(tests PRIVATE cxx_std_17)
target_include_directories(tests PUBLIC ../src)
target_link_libraries(tests PUBLIC agent GTest::gtest GTest::gtest_main spdlog::spdlog)

add_executable(SSLTestClient SSLTestClient.cpp)
target_link_libraries(SSLTestClient PUBLIC Poco::Net Poco::Crypto Poco::NetSSL ssl3 spdlog)

add_executable(SSLTestServer SSLTestServer.cpp)
target_link_libraries(SSLTestServer PUBLIC Poco::Net Poco::Crypto Poco::NetSSL ssl3 spdlog)

# allow user to run tests with `make test` or `ctest`
include(GoogleTest)
gtest_discover_tests(tests)

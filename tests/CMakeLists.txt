add_executable(tests tests.cpp agent_t.cpp)
target_compile_features(tests PRIVATE cxx_std_17)
target_include_directories(tests PUBLIC ../src)
target_link_libraries(tests PUBLIC agent GTest::gtest GTest::gtest_main spdlog::spdlog)

# allow user to run tests with `make test` or `ctest`
include(GoogleTest)
gtest_discover_tests(tests)

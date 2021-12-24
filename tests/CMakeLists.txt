add_executable(tests tests.cpp agent_t.cpp)
target_link_libraries(tests PUBLIC agent GTest::gtest GTest::gtest_main)

# allow user to run tests with `make test` or `ctest`
include(GoogleTest)
gtest_discover_tests(tests)

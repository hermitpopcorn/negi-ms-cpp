cmake_minimum_required(VERSION 3.22)
project(negi-ms-cpp LANGUAGES CXX)

find_package(nlohmann_json CONFIG REQUIRED)

# common library
file(GLOB_RECURSE COMMONLIB_FILES "src/lib/*.cpp")
add_library(commonlib STATIC ${COMMONLIB_FILES})
target_include_directories(commonlib PUBLIC "src/")
target_link_libraries(commonlib PRIVATE curl nlohmann_json::nlohmann_json)

# reporter
file(GLOB_RECURSE reporter_FILES "src/reporter/*.cpp")
add_executable(reporter ${reporter_FILES})
target_include_directories(reporter PUBLIC "src/")
target_link_libraries(reporter PRIVATE commonlib)
target_link_libraries(reporter PRIVATE curl nlohmann_json::nlohmann_json)

# test
enable_testing()
file(GLOB_RECURSE tests_FILES "test/*.cpp")
add_executable(tests ${tests_FILES})
target_include_directories(tests PUBLIC "src/")
target_include_directories(tests PUBLIC "test/")
target_link_libraries(tests PRIVATE commonlib)
find_package(GTest CONFIG REQUIRED)
target_link_libraries(tests PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

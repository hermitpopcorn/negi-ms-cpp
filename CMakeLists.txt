cmake_minimum_required(VERSION 3.22)
project(negi-ms-cpp LANGUAGES CXX)

# common library
file(GLOB_RECURSE COMMONLIB_FILES "src/lib/*.cpp")
add_library(commonlib STATIC ${COMMONLIB_FILES})
target_include_directories(commonlib PUBLIC "src/")
target_link_libraries(commonlib curl)

# reporter
file(GLOB_RECURSE reporter_FILES "src/reporter/*.cpp")
add_executable(reporter ${reporter_FILES})
target_include_directories(reporter PUBLIC "src/")
target_link_libraries(reporter PRIVATE commonlib)

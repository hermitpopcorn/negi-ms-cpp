cmake_minimum_required(VERSION 4.1)
project(negi-ms-cpp LANGUAGES CXX)

# common library
file(GLOB_RECURSE COMMONLIB_FILES "src/lib/*.cpp")
add_library(commonlib STATIC ${COMMONLIB_FILES})
target_include_directories(commonlib PUBLIC "src/lib")
# external dependencies
find_package(google_cloud_cpp_common CONFIG REQUIRED)
target_link_libraries(commonlib PRIVATE google-cloud-cpp::common)
find_package(google_cloud_cpp_oauth2 CONFIG REQUIRED)
target_link_libraries(commonlib PRIVATE google-cloud-cpp::oauth2)
find_package(google_cloud_cpp_rest_internal CONFIG REQUIRED)
target_link_libraries(commonlib PRIVATE google-cloud-cpp::rest_internal)

# reporter
file(GLOB_RECURSE reporter_FILES "src/reporter/*.cpp")
add_executable(reporter ${reporter_FILES})
target_include_directories(reporter PUBLIC "src/lib")
target_include_directories(reporter PUBLIC "src/reporter")
target_link_libraries(reporter PRIVATE commonlib)

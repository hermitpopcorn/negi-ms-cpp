cmake_minimum_required(VERSION 3.22)
project(negi-ms-cpp LANGUAGES CXX)

# Set clang as the C++ compiler
set(CMAKE_CXX_COMPILER clang++ CACHE STRING "C++ compiler" FORCE)
set(CMAKE_C_COMPILER clang CACHE STRING "C compiler" FORCE)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags for clang
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wno-unknown-pragmas")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion -Wsign-conversion -Wdouble-promotion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnull-dereference -Wformat=2 -Warray-bounds")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")

# Debug flags with sanitizers
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fsanitize=undefined")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=bounds -fsanitize=alignment")

# Release flags
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Enable clang-tidy integration
find_program(CLANG_TIDY clang-tidy)
if(CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY
        ${CLANG_TIDY};
        -warnings-as-errors=*;
    )
endif()

# Enable clang static analyzer
find_program(CLANG_ANALYZER scan-build)
if(CLANG_ANALYZER)
    message(STATUS "clang static analyzer found: ${CLANG_ANALYZER}")
endif()

find_package(nlohmann_json CONFIG REQUIRED)

# common library
set(COMMONLIB_FILES
    src/lib/network/requester.cpp
    src/lib/sheet/client.cpp
    src/lib/external/exec.cpp
)
add_library(commonlib STATIC ${COMMONLIB_FILES})
target_include_directories(commonlib PUBLIC "src/")
target_link_libraries(commonlib PRIVATE curl nlohmann_json::nlohmann_json)

# reporter
set(REPORTER_FILES
    src/reporter/main.cpp
    src/reporter/discord.cpp
)
add_executable(reporter ${REPORTER_FILES})
target_include_directories(reporter PUBLIC "src/")
target_link_libraries(reporter PRIVATE commonlib)
target_link_libraries(reporter PRIVATE curl nlohmann_json::nlohmann_json)

# marksman modules library
set(MARKSMAN_LIB_FILES
    src/marksman/duplifinder.cpp
    src/marksman/categorizer.cpp
)
add_library(marksman_lib STATIC ${MARKSMAN_LIB_FILES})
target_include_directories(marksman_lib PUBLIC "src/")

# marksman executable
set(MARKSMAN_MAIN_FILES
    src/marksman/main.cpp
)
add_executable(marksman ${MARKSMAN_MAIN_FILES})
target_include_directories(marksman PUBLIC "src/")
target_link_libraries(marksman PRIVATE commonlib marksman_lib)

# clerk
set(CLERK_FILES
    src/clerk/main.cpp
    src/lib/network/http_server.cpp
)
add_executable(clerk ${CLERK_FILES})
target_link_libraries(clerk PRIVATE commonlib)
target_link_libraries(clerk PRIVATE curl nlohmann_json::nlohmann_json)

# test
enable_testing()
set(TESTS_FILES
    test/categorizer.cpp
    test/duplifinder.cpp
    test/sheet.cpp
)
add_executable(tests ${TESTS_FILES})
target_include_directories(tests PUBLIC "src/")
target_include_directories(tests PUBLIC "test/")
target_link_libraries(tests PRIVATE commonlib marksman_lib)
find_package(GTest CONFIG REQUIRED)
target_link_libraries(tests PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
add_test(NAME unit_tests COMMAND tests)

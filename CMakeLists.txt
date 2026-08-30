cmake_minimum_required(VERSION 3.10)
project(FrameZero VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Threads REQUIRED)
set(CMAKE_POLICY_VERSION_MINIMUM "3.5" CACHE STRING "Minimum CMake policy version" FORCE)

include(FetchContent)
FetchContent_Declare(raylib URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.zip)
FetchContent_MakeAvailable(raylib)

add_library(framezero STATIC src/fixed_point.cpp)
target_include_directories(framezero PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/framezero>
)
target_link_libraries(framezero PUBLIC raylib)
if(WIN32)
    target_link_libraries(framezero PUBLIC ws2_32 wsock32)
endif()

add_executable(FrameZeroTests tests/test_engine.cpp)
target_link_libraries(FrameZeroTests PRIVATE framezero)

add_subdirectory(tools)

install(TARGETS framezero
    EXPORT FrameZeroTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include/framezero
)
install(DIRECTORY include/ DESTINATION include/framezero)

if(UNIX AND NOT APPLE)
    target_link_libraries(framezero PUBLIC m Threads::Threads)
    target_link_libraries(FrameZeroTests PRIVATE m Threads::Threads)
endif()

add_custom_target(run_tests
    COMMAND $<TARGET_FILE:FrameZeroTests>
    DEPENDS FrameZeroTests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running FrameZero Engine Test Suite"
)

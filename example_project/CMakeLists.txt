cmake_minimum_required(VERSION 3.10)
project(FrameZeroExample)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# In a real scenario where FrameZero is installed system-wide, you would use:
# find_package(FrameZero REQUIRED)
# For this example, we assume it's sitting one directory above:
add_subdirectory(../ framezero_build)

add_executable(example_app main.cpp)

# Link against the framezero target
target_link_libraries(example_app PRIVATE framezero)

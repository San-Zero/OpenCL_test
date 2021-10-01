cmake_minimum_required(VERSION 3.20)
project(OpenCL_test)

set(CMAKE_CXX_STANDARD 14)
#set(${INC_DIR} C:/Program Files (x86)/AMD APP SDK/3.0/include)
#set(${LINK_DIR} C:/Program Files (x86)/AMD APP SDK/3.0/lib/x86_64)

include_directories(include)
link_directories(lib/x86)
link_libraries(OpenCL)

add_executable(OpenCL_test main.cpp)
target_link_libraries (OpenCL_test OpenCL)
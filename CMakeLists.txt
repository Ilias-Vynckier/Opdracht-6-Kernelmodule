cmake_minimum_required(VERSION 3.0.0)
project(Opdracht6-Kernelmodule VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(Opdracht6-Kernelmodule main.c)

target_link_libraries(Opdracht6-Kernelmodule)
cmake_minimum_required(VERSION 3.16)
project(Lox1)
set(CMAKE_CXX_STANDARD 17)

include_directories(include)
add_executable(Lox1 src/main.cpp)

target_compile_options(Lox1 PRIVATE -Wall -Wextra -Werror)

cmake_minimum_required(VERSION 3.16)
project(Lox1)
set(CMAKE_CXX_STANDARD 17)

set(SOURCES src/main.cpp
            src/lexer/lexer.cpp
)

include_directories(include)
add_executable(Lox1 ${SOURCES})

target_compile_options(Lox1 PRIVATE -Wall -Wextra -Werror)

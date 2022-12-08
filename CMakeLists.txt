cmake_minimum_required(VERSION 3.16)
project(Lox1)
set(CMAKE_CXX_STANDARD 17)

# Googletest dependency.
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# Target for compiler.
set(SOURCES src/lexer/Lexer.cpp
            src/parser/Parser.cpp
)
include_directories(include)
include_directories(generated)
add_library(Lox1 ${SOURCES})

# Binary for main.
add_executable(main src/main.cpp)
target_link_libraries(main PUBLIC Lox1)

# Set compiler options for above targets.
target_compile_options(Lox1 PRIVATE -Wall -Wextra -Werror)

# Discover googletest tests and make test binary.
enable_testing()

set(TEST_SOURCES  test/lexer/TokenTests.cpp
                  test/lexer/LexerTests.cpp
                  test/ast/AstTests.cpp
                  test/visit/PrettyPrinterTests.cpp
                  test/parser/ParserTests.cpp
                  test/visit/EvaluatorTests.cpp
)
add_executable(
  tests
  ${TEST_SOURCES}
)
target_link_libraries(
  tests
  GTest::gtest_main
  Lox1
)

include(GoogleTest)
gtest_discover_tests(tests)

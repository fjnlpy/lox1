#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>

#include "utils/Logging.hpp"

namespace fs = std::filesystem;

void
run(const std::string &program)
{
  LOGD("The input is ", program);
}

void
runPrompt()
{
  std::string input;
  while (std::cout << "> ", getline(std::cin, input)) {
    try {
      run(input);
    } catch (...) {
      // TODO: actual error reporting.
      LOGE("Error with input: ", input);
    }
  }
}

void
runFile(const char *fileName)
{
  std::ifstream fileStream(fileName);
  if (!fileStream.good()) {
    LOGE("Could not open for I/O: ", fileName);
    return;
  }

  std::stringstream stringStream;
  stringStream << fileStream.rdbuf();

  try {
    run(stringStream.str());
  } catch (...) {
    // TODO: actual error reporting.
    LOGE("Error in file: ", fileName);
    // Should use non-zero exit code in this case.
    exit(-1);
  }
}

int
main(int argc, char **argv)
{
  if (argc == 1) {
    runPrompt();
  } else if (argc == 2) {
    runFile(argv[1]);
  } else {
    LOGI(
R"(
Pass pass the path to the file to be interpreted, or nothing if you want to use
the interactive prompt.
)"
    );
    return -1;
  }
}

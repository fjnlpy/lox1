#pragma once

#include <string>
#include <exception>
#include <utility>
#include <sstream>
#include <vector>

#include "utils/Assert.hpp"

class CompileError {
public:
  CompileError(
    unsigned lineNumber,
    std::string errorType,
    std::string errorMessage,
    std::string sourceSnippet
  )
    : lineNumber_(lineNumber)
    , errorType_(std::move(errorType))
    , errorMessage_(std::move(errorMessage))
    , sourceSnippet_(std::move(sourceSnippet))
  { }

  std::string
  what() const
  {
    std::stringstream stream;

    stream << "[";
    stream << errorType_;
    stream << " | Line ";
    stream << lineNumber_;
    stream << "] ";
    stream << errorMessage_;
    stream << std::endl;

    stream << sourceSnippet_;
    stream << std::endl;

    return stream.str();
  }

private:
  const unsigned lineNumber_;
  const std::string errorType_;
  const std::string errorMessage_;
  const std::string sourceSnippet_;
};

class ErrorCollection {
public:
  ErrorCollection(std::vector<CompileError> errors)
    : errors_(std::move(errors))
  {
    ASSERT(!errors_.empty() && "Expecting at least one error in the collection");
  }

  std::string
  what() const
  {
    std::stringstream stream;

    for (auto &error : errors_) {
      stream << error.what() << std::endl;
    }

    return stream.str();
  }

  const std::vector<CompileError> &
  errors() const
  {
    return errors_;
  }

private:
  const std::vector<CompileError> errors_;
};

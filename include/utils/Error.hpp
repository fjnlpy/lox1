#pragma once

#include <string>
#include <exception>
#include <utility>
#include <sstream>
#include <vector>
#include <string_view>

#include "utils/Assert.hpp"

class SourceReference {
public:
  // TODO: refactor out somewhere?
  using ProgramLines = std::vector<std::string_view>;

  virtual ~SourceReference() =default;

  virtual std::string resolve(const ProgramLines &) const;
};

class SingleLineReference final : public SourceReference {
public:
  unsigned lineNumber_;
  unsigned columnStart_;
  unsigned columnEnd_;

  virtual std::string resolve(const ProgramLines &lines) const override
  {
    std::string result;

    // TODO: ensure that all lines in `lines` end with a newline.
    result += lines[lineNumber_];

    // Point out the columns containing the error.
    result += std::string(columnStart_, '-');
    result += std::string(columnEnd_ - columnStart_, '^');
    result += '\n';

    return result;
  }
};

class MultilineReference final : public SourceReference {
public:
  unsigned lineStart_;
  unsigned lineEnd_;

  virtual std::string resolve(const ProgramLines &lines) const override
  {
    std::string result;

    for (size_t i = lineStart_; i <= lineEnd_; ++i) {
      result += lines[i];
    }

    return result;
  }
};

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

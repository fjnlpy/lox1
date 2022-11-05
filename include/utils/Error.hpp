#pragma once

#include <string>
#include <memory>
#include <exception>
#include <utility>
#include <functional>
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
    std::unique_ptr<const SourceReference> sourceReference
    // TODO: replace ^ this with a SourceReference thing that has two subclasses:
    // one taking a single line plus column start and end info, and one taking a start
    // and ending line. Take a unique ptr to it, since subclassing -- and can pass null for
    // unit tests, where there is no source code info.
    // Create this thing during lexing and pass it around into the AST etc.
    // Then when what()ing the error, call resolve on it, passing the source code (vector of string views?),
    // and it will grab the correct string, and for the single line case it will add some arrows pointing
    // to correct columns.
    // Also will need to extend the code generator a bit to deal with the extra ctor params.
  )
    : lineNumber_(lineNumber)
    , errorType_(std::move(errorType))
    , errorMessage_(std::move(errorMessage))
    , sourceReference_(std::move(sourceReference))
  { }

  std::string
  what(std::optional<std::reference_wrapper<const std::vector<std::string_view>>> lines = std::nullopt) const
  {
    std::stringstream stream;

    stream << "[";
    stream << errorType_;
    stream << " | Line ";
    stream << lineNumber_;
    stream << "] ";
    stream << errorMessage_;
    stream << std::endl;

    if (lines && sourceReference_) {
      stream << sourceReference_->resolve(*lines) << std::endl;
    }

    return stream.str();
  }

private:
  const unsigned lineNumber_;
  const std::string errorType_;
  const std::string errorMessage_;
  const std::unique_ptr<const SourceReference> sourceReference_;
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

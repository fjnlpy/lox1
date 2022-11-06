#pragma once

#include <string>
#include <memory>
#include <exception>
#include <utility>
#include <functional>
#include <sstream>
#include <algorithm>
#include <vector>
#include <string_view>

#include "utils/Assert.hpp"

class SourceReference {
public:
  // TODO: refactor out somewhere?
  using ProgramLines = std::vector<std::string_view>;

  virtual ~SourceReference() =default;

  virtual std::string resolve(const ProgramLines &) const;

  virtual unsigned getLineNumber() const;
};

class SingleLineReference final : public SourceReference {
public:
  SingleLineReference(unsigned lineNumber, unsigned columnStart, unsigned columnEnd)
    : lineNumber_(lineNumber), columnStart_(columnStart), columnEnd_(columnEnd)
  {
    ASSERT(columnEnd_ >= columnStart && "End column should not be smaller than start column.");
  }

  virtual std::string resolve(const ProgramLines &lines) const override
  {
    std::string result;

    // TODO: ensure that all lines in `lines` end with a newline.
    result += lines[lineNumber_];

    const unsigned numCharsBeforeError = std::max(0u, columnStart_ - 1);

    // Point out the columns containing the error.
    result += std::string(numCharsBeforeError, '-');
    result += std::string(columnEnd_ - numCharsBeforeError, '^');
    result += '\n';

    return result;
  }

  virtual unsigned getLineNumber() const override
  {
    return lineNumber_;
  }

private:
  unsigned lineNumber_;
  unsigned columnStart_;
  unsigned columnEnd_;
};

class MultiLineReference final : public SourceReference {
public:
  MultiLineReference(unsigned lineStart, unsigned lineEnd)
    : lineStart_(lineStart), lineEnd_(lineEnd)
  {
    ASSERT(lineEnd_ >= lineStart_ && "End line should not be smaller than start line.");
  }

  virtual std::string resolve(const ProgramLines &lines) const override
  {
    std::string result;

    for (size_t i = lineStart_; i <= lineEnd_; ++i) {
      result += lines[i];
    }

    return result;
  }

  virtual unsigned getLineNumber() const override
  {
    // There are many line numbers. The first one is Good enough...
    return lineStart_;
  }

private:
  unsigned lineStart_;
  unsigned lineEnd_;
};

class CompileError {
public:
  CompileError(
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
    : errorType_(std::move(errorType)), errorMessage_(std::move(errorMessage)), sourceReference_(std::move(sourceReference))
  { }

  std::string
  what(std::optional<std::reference_wrapper<const std::vector<std::string_view>>> lines = std::nullopt) const
  {
    std::stringstream stream;

    stream << "[";
    stream << errorType_;

    stream << " | Line ";
    if (sourceReference_) {
      stream << sourceReference_->getLineNumber();
    } else {
      stream << "??";
    }
    stream << "] ";

    stream << errorMessage_;
    stream << std::endl;

    if (lines && sourceReference_) {
      stream << sourceReference_->resolve(*lines) << std::endl;
    }

    return stream.str();
  }

private:
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

#pragma once

#include <string>
#include <exception>
#include <utility>
#include <sstream>
#include <functional>
#include <vector>

#include "utils/SourceReference.hpp"
#include "utils/Assert.hpp"

class CompileError {
public:
  CompileError(
    std::string errorType,
    std::string errorMessage,
    std::optional<const SourceReference> sourceReference
  ) : errorType_(std::move(errorType)),
    errorMessage_(std::move(errorMessage)),
    sourceReference_(std::move(sourceReference))
  { }

  std::string
  what(std::optional<std::reference_wrapper<std::vector<std::string_view>>> lines = std::nullopt) const
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

    if (sourceReference_ && lines) {
      stream << sourceReference_->resolve(*lines);
      stream << std::endl;
    }

    return stream.str();
  }

private:
  std::string errorType_;
  std::string errorMessage_;
  std::optional<const SourceReference> sourceReference_;
};

class ErrorCollection {
public:
  ErrorCollection(std::vector<CompileError> errors)
    : errors_(std::move(errors))
  {
    ASSERT(!errors_.empty() && "Expecting at least one error in the collection");
  }

  std::string
  what(std::optional<std::reference_wrapper<std::vector<std::string_view>>> lines = std::nullopt) const
  {
    std::stringstream stream;

    for (const auto &error : errors_) {
      stream << error.what(lines) << std::endl;
    }

    return stream.str();
  }

  const std::vector<CompileError> &
  errors() const
  {
    return errors_;
  }

private:
  std::vector<CompileError> errors_;
};

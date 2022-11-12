#pragma once

#include <functional>
#include <vector>
#include <string_view>
#include <algorithm>

#include "utils/Assert.hpp"

class SourceReference final {
public:
  using ProgramLines = std::vector<std::string_view>;

  SourceReference(unsigned line, unsigned colStart, unsigned colEnd)
  : SourceReference(line, colStart, std::optional(colEnd))
  {
    ASSERT(colStart <= colEnd && "Column start index should not be after column end index.");
  }

  SourceReference(unsigned line)
  : SourceReference(line, 0u, std::nullopt)
  { }

  std::string
  resolve(const ProgramLines &lines) const
  {
    // Note we could return a string view but we want to add adornments so better
    // to just return a fresh string.
    std::string lineText(lines[line_ - 1]);
    ASSERT(lineText.back() == '\n' && "Lines from program should end with newline.");
    if (colStart_ > 0 && colEnd_) {
      // Point out the specific part of the line that we are referring to.
      lineText += std::string("-", std::max(0u, colStart_ - 1));
      lineText += std::string("^", colStart_ - *colEnd_ + 1);
      lineText += '\n';
    }
    return lineText;
  }

  unsigned
  getLineNumber() const
  {
    return line_;
  }

private:
  SourceReference(unsigned line, unsigned colStart, std::optional<unsigned> colEnd)
  : line_(line), colStart_(colStart), colEnd_(colEnd)
  { }

  unsigned line_;
  unsigned colStart_;
  std::optional<unsigned> colEnd_;

};

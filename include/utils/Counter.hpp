#pragma once

#include <cstddef>

class Counter {
public:
  static size_t next() { return counter_++; }

private:
  // This is ok because counter_ will be initialized
  // as zero across all translation units where it's used.
  static inline size_t counter_{0};

};

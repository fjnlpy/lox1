#pragma once

#include <cassert>

// Cast ensures the variable is used even if assertions are turned off.
#define ASSERT(expr) assert(expr); (void)(expr)

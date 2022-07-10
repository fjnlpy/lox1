#pragma once

#include <iostream>

namespace {
  constexpr auto DEBUG = true;
}

template <class T>
constexpr void
LOGI(const T &string)
{
  std::cout << string << std::endl;
}

template <class T>
constexpr void
LOGE(const T &string)
{
  std::cerr << "[E] " << string << std::endl;
}

template <class T>
constexpr void
LOGD(const T &string)
{
  if constexpr (DEBUG) {
    std::cerr << "[D] " << string << std::endl;
  }
}

#pragma once

#include <iostream>

namespace {

constexpr auto DEBUG = true;

template <class O, class... T>
constexpr void
LOG(O &out, const std::string &tag, T&&... strings)
{
  out << tag;
  (out << ... << std::forward<T>(strings));
  out << std::endl;
}

}

template <class... T>
constexpr void
LOGI(T&&... strings)
{
  LOG(std::cout, "", std::forward<T>(strings)...);
}

template <class... T>
constexpr void
LOGE(T&&... strings)
{
  LOG(std::cerr, "[E] ", std::forward<T>(strings)...);
}

template <class... T>
constexpr void
LOGD(T&&... strings)
{
  if constexpr (DEBUG) {
    LOG(std::cerr, "[D] ", std::forward<T>(strings)...);
  }
}

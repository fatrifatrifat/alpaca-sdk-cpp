#pragma once
#include <chrono>
#include <print>
#include <thread>

#define ENABLE_LOGGING

#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_ERROR 4

#ifndef LOG_LEVEL_MINIMUM
#define LOG_LEVEL_MINIMUM LOG_LEVEL_DEBUG
#endif

#if defined(ENABLE_LOGGING)
#define LOG_MSG(level, fmt, ...)                                               \
  do {                                                                         \
    if ((level) >= LOG_LEVEL_MINIMUM)                                          \
      std::println((fmt)__VA_OPT__(, ) __VA_ARGS__);                           \
  } while (false)
#else
#define LOG_MSG(level, fmt, ...)                                               \
  do {                                                                         \
  } while (false)
#endif

namespace alpaca::utils {

inline bool IsSuccess(int s) { return 200 <= s && s < 300; }

inline auto ToIsoz(std::chrono::sys_time<std::chrono::seconds> t) {
  return std::format("{:%FT%T}Z", t);
};

inline auto SleepToNextBoundary(int minutes) {
  using namespace std::chrono;
  auto now = system_clock::now();
  auto now_min = time_point_cast<std::chrono::minutes>(now);
  auto mins = now_min.time_since_epoch().count();
  auto next =
      now_min + std::chrono::minutes{(minutes - (mins % minutes)) % minutes};
  std::this_thread::sleep_until(next + seconds{2});
};

}; // namespace alpaca::utils

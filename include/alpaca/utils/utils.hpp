#pragma once
#include <chrono>
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

inline bool IsSuccess(int s) noexcept { return 200 <= s && s < 300; }

inline auto ToIsoz(std::chrono::sys_time<std::chrono::seconds> t) noexcept {
  return std::format("{:%FT%T}Z", t);
};

inline auto SleepToNextBoundary(int minutes) noexcept {
  using namespace std::chrono;
  auto now = system_clock::now();
  auto now_min = time_point_cast<std::chrono::minutes>(now);
  auto mins = now_min.time_since_epoch().count();
  auto next =
      now_min + std::chrono::minutes{(minutes - (mins % minutes)) % minutes};
  std::this_thread::sleep_until(next + seconds{2});
};

inline std::string SymbolsEncode(const std::vector<std::string> &v) noexcept {
  std::string symbols;
  for (const auto &s : v) {
    symbols += s;
    symbols += "%2C";
  }
  symbols.erase(symbols.length() - 3, 3);
  return symbols;
}

struct QueryBuilder {
  std::string q;
  bool first{true};

  void add(std::string_view k, std::string_view v) noexcept {
    if (v.empty()) {
      return;
    }
    if (!first) {
      q.push_back('&');
    }
    first = false;
    q.append(k);
    q.push_back('=');
    q.append(v);
  }

  void add(std::string_view k, const std::string &v) noexcept {
    add(k, std::string_view(v));
  }
  void add(std::string_view k, const char *v) noexcept {
    if (!v || *v == '\0') {
      return;
    }
    add(k, std::string_view(v));
  }
  void add(std::string_view k, std::optional<std::string_view> v) noexcept {
    if (v) {
      add(k, v.value());
    }
  }
};

}; // namespace alpaca::utils

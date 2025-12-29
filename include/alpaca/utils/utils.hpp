#pragma once
#include <chrono>
#include <thread>

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

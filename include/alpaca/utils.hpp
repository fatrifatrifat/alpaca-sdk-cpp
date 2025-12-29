#pragma once
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <optional>
#include <thread>
#include <vector>

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

struct BacktestResult {
  double initial_cash{};
  double final_equity{};
  double total_return{};
  int trades{};
  int buys{};
  int sells{};
};

class EmaSmaSeeded {
public:
  explicit EmaSmaSeeded(int period)
      : N_(period), alpha_(2.0 / (period + 1.0)) {}

  std::optional<double> Update(double x) {
    if (!seeded_) {
      seed_sum_ += x;
      seed_count_++;

      if (seed_count_ == N_) {
        value_ = seed_sum_ / static_cast<double>(N_);
        seeded_ = true;
        return value_;
      }
      return std::nullopt;
    }

    value_ = alpha_ * x + (1.0 - alpha_) * value_;
    return value_;
  }

  bool Ready() const { return seeded_; }
  double Value() const { return value_; }

private:
  int N_{};
  double alpha_{};
  bool seeded_{false};

  int seed_count_{0};
  double seed_sum_{0.0};

  double value_{0.0};
};

struct MacdOutput {
  bool ready{false};
  double macd{};
  double signal{};
  double hist{};
  bool crossedAbove{false};
  bool crossedBelow{false};
};

class MACD_12_26_9 {
public:
  MacdOutput Update(double close) {
    auto e12 = ema12_.Update(close);
    auto e26 = ema26_.Update(close);

    if (!e12 || !e26) {
      return {};
    }

    double macd = *e12 - *e26;

    auto sig = signal_.Update(macd);
    if (!sig) {
      prev_macd_ = macd;
      prev_signal_.reset();
      return {};
    }

    MacdOutput out;
    out.ready = true;
    out.macd = macd;
    out.signal = *sig;
    out.hist = out.macd - out.signal;

    if (prev_signal_ && prev_macd_) {
      out.crossedAbove =
          (*prev_macd_ <= *prev_signal_) && (out.macd > out.signal);
      out.crossedBelow =
          (*prev_macd_ >= *prev_signal_) && (out.macd < out.signal);
    }

    prev_macd_ = out.macd;
    prev_signal_ = out.signal;
    return out;
  }

private:
  EmaSmaSeeded ema12_{12};
  EmaSmaSeeded ema26_{26};
  EmaSmaSeeded signal_{9};

  std::optional<double> prev_macd_;
  std::optional<double> prev_signal_;
};

inline BacktestResult BacktestBuyHoldAllIn(const std::vector<double> &closes,
                                           double initial_cash = 10'000.0) {
  BacktestResult r{};
  r.initial_cash = initial_cash;
  r.final_equity = initial_cash;
  r.total_return = 0.0;
  r.trades = r.buys = r.sells = 0;

  auto good = [](double x) { return std::isfinite(x) && x > 0.0; };

  if (!std::isfinite(initial_cash) || initial_cash <= 0.0)
    return r;

  auto it_first = std::find_if(closes.begin(), closes.end(), good);
  auto it_last = std::find_if(closes.rbegin(), closes.rend(), good);

  if (it_first == closes.end() || it_last == closes.rend())
    return r;

  const std::ptrdiff_t i_first = std::distance(closes.begin(), it_first);
  const std::ptrdiff_t i_last = (std::ptrdiff_t)closes.size() - 1 -
                                std::distance(closes.rbegin(), it_last);

  if (i_last <= i_first)
    return r;

  const double first = *it_first;
  const double last = *it_last;

  const long double shares = (long double)initial_cash / (long double)first;

  const long double final_equity = shares * (long double)last;

  r.final_equity = (double)final_equity;
  r.total_return = (r.final_equity / r.initial_cash) - 1.0;
  r.trades = 1;
  r.buys = 1;
  r.sells = 0;
  return r;
}

inline BacktestResult BacktestMACDAllIn(const std::vector<double> &closes,
                                        double initial_cash = 10'000.0) {
  BacktestResult r;
  r.initial_cash = initial_cash;

  if (closes.size() < 2) {
    r.final_equity = initial_cash;
    r.total_return = 0.0;
    return r;
  }

  MACD_12_26_9 macd;

  double cash = initial_cash;
  long long shares = 0;

  for (std::size_t i = 0; i < closes.size(); ++i) {
    const double close = closes[i];
    auto m = macd.Update(close);

    if (!m.ready)
      continue;

    if (m.crossedAbove && shares == 0) {
      long long buy_shares = static_cast<long long>(std::floor(cash / close));
      if (buy_shares > 0) {
        shares += buy_shares;
        cash -= buy_shares * close;
        r.trades++;
        r.buys++;
      }
    }

    if (m.crossedBelow && shares > 0) {
      cash += shares * close;
      shares = 0;
      r.trades++;
      r.sells++;
    }
  }

  const double last = closes.back();
  r.final_equity = cash + shares * last;
  r.total_return = (r.final_equity / initial_cash) - 1.0;
  return r;
}

}; // namespace alpaca::utils

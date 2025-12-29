#pragma once
#include <optional>

namespace alpaca::macd {

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

}; // namespace alpaca::macd

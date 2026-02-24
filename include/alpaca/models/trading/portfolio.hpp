#include <glaze/glaze.hpp>

namespace alpaca {

enum class IntradayReporting {
  market_hours,
  extended_hours,
  continuous,
};

enum class PNLReset {
  no_reset,
  per_day,
};

struct PortfolioParam {
  std::optional<std::string> period = std::nullopt;
  std::optional<std::string> timeframe = std::nullopt;
  std::optional<IntradayReporting> intradayReporting = std::nullopt;
  std::optional<std::string> start = std::nullopt;
  std::optional<PNLReset> pnlReset = std::nullopt;
  std::optional<std::string> end = std::nullopt;
  std::optional<std::string> extendedHours = std::nullopt;
  std::optional<std::string> cashflowTypes = std::nullopt;
};

struct Portfolio {
  std::vector<double> timestamp{};
  std::vector<double> equity{};
  std::vector<double> profitLoss{};
  std::vector<double> profitLossPCT{};
  double baseValue{};
  std::optional<std::string> baseValueAsof = std::nullopt;
  std::string timeframe{};
  std::optional<glz::generic> cashflow = std::nullopt;
};

constexpr std::optional<std::string_view>
ToString(std::optional<IntradayReporting> i) noexcept {
  if (!i) {
    return std::nullopt;
  }

  switch (*i) {
  case IntradayReporting::continuous:
    return "continuous";
  case IntradayReporting::market_hours:
    return "market_hours";
  case IntradayReporting::extended_hours:
    return "extended_hours";
  default:
    return std::nullopt;
  }
}

constexpr std::optional<std::string_view>
ToString(std::optional<PNLReset> i) noexcept {
  if (!i) {
    return std::nullopt;
  }

  switch (*i) {
  case PNLReset::no_reset:
    return "no_reset";
  case PNLReset::per_day:
    return "per_day";
  default:
    return std::nullopt;
  }
}

}; // namespace alpaca

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
  std::optional<std::string> period;
  std::optional<std::string> timeframe;
  std::optional<IntradayReporting> intradayReporting;
  std::optional<std::string> start;
  std::optional<PNLReset> pnlReset;
  std::optional<std::string> end;
  std::optional<std::string> extendedHours;
  std::optional<std::string> cashflowTypes;
};

struct Portfolio {
  std::vector<double> timestamp;
  std::vector<double> equity;
  std::vector<double> profitLoss;
  std::vector<double> profitLossPCT;
  double baseValue;
  std::optional<std::string> baseValueAsof;
  std::string timeframe;
  std::optional<glz::generic> cashflow;
};

constexpr std::optional<std::string_view>
ToString(std::optional<IntradayReporting> i) {
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

constexpr std::optional<std::string_view> ToString(std::optional<PNLReset> i) {
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

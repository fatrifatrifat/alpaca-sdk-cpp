#pragma once
#include <glaze/glaze.hpp>

namespace alpaca {

enum class BarFeed {
  SIP,
  IEX,
  Boats,
  OTC,
};

enum class BarAdjustment {
  Raw,
  Split,
  Dividend,
  Spinoff,
  All,
};

enum class BarSort {
  Asc,
  Desc,
};

struct Bar {
  double close{}, high{}, low{};
  long long number_of_trades{};
  double open{};
  std::string timestamp;
  long long volume{};
  double volume_weigted_price{};
};

struct Bars {
  std::map<std::string, std::vector<Bar>> bars;
  std::optional<std::string> next_page_token;
};

struct BarParams {
  std::vector<std::string> symbols;
  std::string timeframe{"1D"};
  std::string start{"2024-01-03T00:00:00Z"};
  std::string end{"2024-01-04T00:00:00Z"};
  std::optional<int> limit;
  std::optional<BarFeed> feed;
  std::optional<std::string> page_token;
  std::optional<BarAdjustment> adjustment;
  std::optional<std::string> asof;
  std::optional<std::string> currency;
  std::optional<BarSort> sort;
};

struct LatestBarParam {
  std::vector<std::string> symbols;
  std::optional<BarFeed> feed;
};

struct LatestBars {
  std::map<std::string, Bar> bars;
};

constexpr std::optional<std::string_view>
ToString(std::optional<BarFeed> f) noexcept {
  if (!f) {
    return std::nullopt;
  }

  switch (*f) {
  case BarFeed::SIP:
    return "sip";
  case BarFeed::IEX:
    return "iex";
  case BarFeed::Boats:
    return "boats";
  case BarFeed::OTC:
    return "otc";
  default:
    return std::nullopt;
  }
}

constexpr std::optional<std::string_view>
ToString(std::optional<BarAdjustment> a) noexcept {
  if (!a) {
    return std::nullopt;
  }

  switch (*a) {
  case BarAdjustment::Raw:
    return "raw";
  case BarAdjustment::Split:
    return "split";
  case BarAdjustment::Dividend:
    return "dividend";
  case BarAdjustment::Spinoff:
    return "spinoff";
  case BarAdjustment::All:
    return "all";
  default:
    return std::nullopt;
  }
}

constexpr std::optional<std::string_view>
ToString(std::optional<BarSort> s) noexcept {
  if (!s) {
    return std::nullopt;
  }

  switch (*s) {
  case BarSort::Asc:
    return "asc";
  case BarSort::Desc:
    return "desc";
  default:
    return std::nullopt;
  }
}

}; // namespace alpaca

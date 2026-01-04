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
  std::string currency;
};

struct BarParams {
  std::vector<std::string> symbols;
  std::string timeframe{"1D"};
  std::string start{"2024-01-03T00:00:00Z"};
  std::string end{"2024-01-04T00:00:00Z"};
  std::optional<int> limit;
  BarFeed feed{BarFeed::IEX};
  std::optional<std::string> page_token;
  std::optional<BarAdjustment> adjustment;
  std::optional<std::string> asof;
  std::optional<std::string> currency;
  std::optional<BarSort> sort;
};

struct LatestBarParam {
  std::vector<std::string> symbols;
  BarFeed feed{BarFeed::IEX};
};

struct LatestBars {
  std::map<std::string, Bar> bars;
};

}; // namespace alpaca

namespace glz {

template <> struct meta<alpaca::BarFeed> {
  using enum alpaca::BarFeed;
  static constexpr auto value = glz::enumerate(SIP, IEX, Boats, OTC);
};

template <> struct meta<alpaca::BarAdjustment> {
  using enum alpaca::BarAdjustment;
  static constexpr auto value =
      glz::enumerate(Raw, Split, Dividend, Spinoff, All);
};

template <> struct meta<alpaca::BarSort> {
  using enum alpaca::BarSort;
  static constexpr auto value = glz::enumerate(Asc, Desc);
};

template <> struct meta<alpaca::Bar> {
  using T = alpaca::Bar;
  static constexpr auto value =
      object("c", &T::close, "h", &T::high, "l", &T::low, "n",
             &T::number_of_trades, "o", &T::open, "t", &T::timestamp, "v",
             &T::volume, "vw", &T::volume_weigted_price);
};

template <> struct meta<alpaca::Bars> {
  using T = alpaca::Bars;
  static constexpr auto value =
      object("bars", &T::bars, "next_page_token", &T::next_page_token,
             "currency", &T::currency);
};

template <> struct meta<alpaca::LatestBars> {
  using T = alpaca::LatestBars;
  static constexpr auto value = object("bars", &T::bars);
};

}; // namespace glz

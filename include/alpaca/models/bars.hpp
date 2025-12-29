#pragma once
#include <glaze/glaze.hpp>

namespace alpaca {

struct Bar {
  double close{}, high{}, low{};
  long long number_of_trades{};
  double open{};
  std::string timestamp;
  long long volume{};
  double volume_weigted_price{};
};

struct BarParams {
  std::vector<std::string> symbols;
  std::string timeframe{"1D"};
  std::string start{"2024-01-03T00:00:00Z"};
  std::string end{"2024-01-04T00:00:00Z"};
  int limit{1000};
  std::string feed{"iex"};
  std::optional<std::string> page_token;
};

struct LatestBarParam {
  std::vector<std::string> symbols;
  std::string feed{"iex"};
};

struct Bars {
  std::map<std::string, std::vector<Bar>> bars;
  std::optional<std::string> next_page_token;
};

struct LatestBars {
  std::map<std::string, Bar> bars;
};

}; // namespace alpaca

namespace glz {

template <> struct meta<alpaca::Bars> {
  using T = alpaca::Bars;
  static constexpr auto value =
      object("bars", &T::bars, "next_page_token", &T::next_page_token);
};

template <> struct meta<alpaca::Bar> {
  using T = alpaca::Bar;
  static constexpr auto value =
      object("c", &T::close, "h", &T::high, "l", &T::low, "n",
             &T::number_of_trades, "o", &T::open, "t", &T::timestamp, "v",
             &T::volume, "vw", &T::volume_weigted_price);
};

template <> struct meta<alpaca::LatestBars> {
  using T = alpaca::LatestBars;
  static constexpr auto value = object("bars", &T::bars);
};

}; // namespace glz

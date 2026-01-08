#pragma once
#include <alpaca/models/marketdata/bars.hpp>
#include <glaze/glaze.hpp>

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
      object("bars", &T::bars, "next_page_token", &T::next_page_token);
};

template <> struct meta<alpaca::LatestBars> {
  using T = alpaca::LatestBars;
  static constexpr auto value = object("bars", &T::bars);
};

}; // namespace glz

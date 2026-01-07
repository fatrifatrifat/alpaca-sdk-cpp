#pragma once
#include <alpaca/models/positions/positions.hpp>

namespace glz {

template <> struct meta<alpaca::PositionExchange> {
  using enum alpaca::PositionExchange;
  static constexpr auto value =
      glz::enumerate(AMEX, ARCA, BATS, NYSE, NASDAQ, NYSEARCA, OTC);
};

template <> struct meta<alpaca::Position> {
  using T = alpaca::Position;
  static constexpr auto value = object(
      "asset_id", &T::assetID, "symbol", &T::symbol, "exchange", &T::exchange,
      "asset_class", &T::assetClass, "asset_marginable", &T::assetMarginable,
      "qty", &T::qty, "avg_entry_price", &T::avgEntryPrice, "side", &T::side,
      "market_value", &T::marketValue, "cost_basis", &T::costBasis,
      "unrealized_pl", &T::unrealizedPL, "unrealized_plpc", &T::unrealizedPLPC,
      "unrealized_intraday_pl", &T::unrealizedIntradayPL,
      "unrealized_intraday_plpc", &T::unrealizedIntradayPLPC, "current_price",
      &T::currentPrice, "lastday_price", &T::lastDayPrice, "change_today",
      &T::changeToday, "qty_available", &T::qtyAvailable);
};

} // namespace glz

#pragma once
#include <glaze/glaze.hpp>

namespace alpaca {

struct Position {
  std::string asset_id;
  std::string symbol;
  std::string exchange;
  std::string asset_class;
  bool asset_marginable{};

  std::string qty;
  std::string avg_entry_price;
  std::string side;
  std::string market_value;
  std::string cost_basis;

  std::string unrealized_pl;
  std::string unrealized_plpc;
  std::string unrealized_intraday_pl;
  std::string unrealized_intraday_plpc;

  std::string current_price;
  std::string lastday_price;
  std::string change_today;

  std::string qty_available;
};

using Positions = std::vector<Position>;

struct Shares {
  long double value{};
};

struct Percent {
  long double value{};
};

using LiquidationAmount = std::variant<Shares, Percent>;

struct ClosePositionParams {
  std::string symbol_or_asset_id;
  LiquidationAmount amt;
};

}; // namespace alpaca

namespace glz {

template <> struct meta<alpaca::Position> {
  using T = alpaca::Position;
  static constexpr auto value = object(
      "asset_id", &T::asset_id, "symbol", &T::symbol, "exchange", &T::exchange,
      "asset_class", &T::asset_class, "asset_marginable", &T::asset_marginable,
      "qty", &T::qty, "avg_entry_price", &T::avg_entry_price, "side", &T::side,
      "market_value", &T::market_value, "cost_basis", &T::cost_basis,
      "unrealized_pl", &T::unrealized_pl, "unrealized_plpc",
      &T::unrealized_plpc, "unrealized_intraday_pl", &T::unrealized_intraday_pl,
      "unrealized_intraday_plpc", &T::unrealized_intraday_plpc, "current_price",
      &T::current_price, "lastday_price", &T::lastday_price, "change_today",
      &T::change_today, "qty_available", &T::qty_available);
};

} // namespace glz

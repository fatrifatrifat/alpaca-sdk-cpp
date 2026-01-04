#pragma once
#include <glaze/glaze.hpp>

namespace alpaca {

enum class OrderSide {
  buy,
  sell,
};

enum class OrderType {
  market,
  limit,
  stop,
  stop_limit,
  trailing_stop,
};

enum class OrderTimeInForce {
  day,
  gtc,
  opg,
  cls,
  ioc,
  fok,
};

enum class OrderClass {
  simple,
  bracket,
  oco,
  oto,
  mleg,
  crypto,
};

enum class PositionIntent {
  buy_to_open,
  buy_to_close,
  sell_to_open,
  sell_to_close,
};

struct Leg {
  std::string symbol;
  long double ratio_qty;
  std::optional<OrderSide> side;
  std::optional<PositionIntent> positionIntent;
};

struct LegsResponse {
  std::string id;
  std::string client_order_id;
};

struct TakeProfit {
  long double limitPrice;
};

struct StopLoss {
  long double stopPrice;
  long double limitPrice;
};

struct Quantity {
  long double qty;
};

struct Notional {
  long double notional;
};

struct TrailPrice {
  long double trail_price;
};

struct TrailPercent {
  long double trail_percent;
};

using Legs = std::vector<Leg>;
using ShareAmount = std::variant<Quantity, Notional>;
using TrailAmount = std::variant<TrailPrice, TrailPercent>;

struct OrderRequest {
  std::string symbol;
  ShareAmount amt;
  OrderSide side;
  OrderType type;
  OrderTimeInForce time_in_force;
  std::optional<long double> limitPrice;
  std::optional<long double> stopPrice;
  std::optional<TrailAmount> trailAmt;
  std::optional<bool> extended_hours;
  std::optional<std::string> clientOrderID;
  std::optional<OrderClass> orderClass;
  std::optional<Legs> legs;
  std::optional<TakeProfit> takeProfit;
  std::optional<StopLoss> stopLoss;
  std::optional<PositionIntent> positionIntent;
  // TODO: Advanced insutrctions object
};

struct OrderResponse {
  std::string id;
  std::string client_order_id;

  std::string created_at;
  std::string updated_at;
  std::string submitted_at;

  std::optional<std::string> filled_at;
  std::optional<std::string> expired_at;
  std::optional<std::string> canceled_at;
  std::optional<std::string> failed_at;
  std::optional<std::string> replaced_at;

  std::optional<std::string> replaced_by;
  std::optional<std::string> replaces;

  std::string asset_id;
  std::string symbol;
  std::string asset_class;

  std::optional<std::string> notional;

  std::string qty;
  std::string filled_qty;
  std::optional<std::string> filled_avg_price;

  std::string order_class;
  std::string order_type;
  std::string type;
  std::string side;

  std::optional<std::string> position_intent;

  std::string time_in_force;

  std::optional<std::string> limit_price;
  std::optional<std::string> stop_price;

  std::string status;

  bool extended_hours{false};

  std::optional<glz::generic> legs;
  std::optional<std::string> trail_percent;
  std::optional<std::string> trail_price;
  std::optional<std::string> hwm;

  std::optional<std::string> subtag;
  std::optional<std::string> source;

  std::optional<std::string> expires_at;
};

}; // namespace alpaca

namespace glz {

template <> struct meta<alpaca::OrderResponse> {
  using T = alpaca::OrderResponse;
  static constexpr auto value = object(
      "id", &T::id, "client_order_id", &T::client_order_id, "created_at",
      &T::created_at, "updated_at", &T::updated_at, "submitted_at",
      &T::submitted_at, "filled_at", &T::filled_at, "expired_at",
      &T::expired_at, "canceled_at", &T::canceled_at, "failed_at",
      &T::failed_at, "replaced_at", &T::replaced_at, "replaced_by",
      &T::replaced_by, "replaces", &T::replaces, "asset_id", &T::asset_id,
      "symbol", &T::symbol, "asset_class", &T::asset_class, "notional",
      &T::notional, "qty", &T::qty, "filled_qty", &T::filled_qty,
      "filled_avg_price", &T::filled_avg_price, "order_class", &T::order_class,
      "order_type", &T::order_type, "type", &T::type, "side", &T::side,
      "position_intent", &T::position_intent, "time_in_force",
      &T::time_in_force, "limit_price", &T::limit_price, "stop_price",
      &T::stop_price, "status", &T::status, "extended_hours",
      &T::extended_hours, "legs", &T::legs, "trail_percent", &T::trail_percent,
      "trail_price", &T::trail_price, "hwm", &T::hwm, "subtag", &T::subtag,
      "source", &T::source, "expires_at", &T::expires_at);
};

template <> struct meta<alpaca::OrderRequest> {
  using T = alpaca::OrderRequest;
  static constexpr auto value =
      object("symbol", &T::symbol, "qty", &T::qty, "side", &T::side, "type",
             &T::type, "time_in_force", &T::time_in_force, "extended_hours",
             &T::extended_hours, "limit_price", &T::limit_price, "stop_price",
             &T::stop_price, "client_order_id", &T::client_order_id);
};

}; // namespace glz

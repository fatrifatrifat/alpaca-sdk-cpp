#pragma once
#include <glaze/glaze.hpp>

namespace alpaca {

struct Quantity {
  long double v{};
};
struct Notional {
  long double v{};
};
using ShareAmount = std::variant<Quantity, Notional>;

struct TrailPrice {
  long double v{};
};
struct TrailPercent {
  long double v{};
};
using TrailAmount = std::variant<TrailPrice, TrailPercent>;

enum class OrderSide { buy, sell };
enum class OrderType { market, limit, stop, stop_limit, trailing_stop };
enum class OrderTimeInForce { day, gtc, opg, cls, ioc, fok };
enum class PositionIntent {
  buy_to_open,
  buy_to_close,
  sell_to_open,
  sell_to_close
};
enum class OrderClass { simple, bracket, oco, oto, mleg, crypto };

struct TakeProfit {
  long double stop_price{};
};

struct StopLoss {
  long double stop_price{};
  long double limit_price{};
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

using Legs = std::vector<Leg>;
using LimitPrice = long double;
using StopPrice = long double;

struct OrderRequest {
  std::string symbol;
  ShareAmount amt;
  OrderSide side;
  OrderType type;
  OrderTimeInForce time_in_force;

  std::optional<LimitPrice> limitPrice;
  std::optional<StopPrice> stopPrice;

  std::optional<TrailAmount> trailAmt;

  std::optional<bool> extended_hours;
  std::optional<std::string> clientOrderID;
  std::optional<OrderClass> orderClass;
  std::optional<Legs> legs;

  std::optional<TakeProfit> takeProfit;
  std::optional<StopLoss> stopLoss;
  std::optional<PositionIntent> position_intent;
};

struct OrderRequestWire {
  std::string symbol;

  std::optional<long double> qty;
  std::optional<long double> notional;

  OrderSide side;
  OrderType type;
  OrderTimeInForce time_in_force;

  std::optional<long double> limit_price;
  std::optional<long double> stop_price;

  std::optional<bool> extended_hours;
  std::optional<std::string> clientOrderID;
  std::optional<OrderClass> orderClass;
  std::optional<Legs> legs;

  std::optional<long double> trail_price;
  std::optional<long double> trail_percent;

  std::optional<TakeProfit> take_profit;
  std::optional<StopLoss> stop_loss;
  std::optional<PositionIntent> position_intent;
};

inline OrderRequestWire toWire(const OrderRequest &r) {
  OrderRequestWire w{};
  w.symbol = r.symbol;
  w.side = r.side;
  w.type = r.type;
  w.time_in_force = r.time_in_force;

  w.limit_price = r.limitPrice;
  w.stop_price = r.stopPrice;
  w.extended_hours = r.extended_hours;
  w.clientOrderID = r.clientOrderID;
  w.orderClass = r.orderClass;
  w.legs = r.legs;
  w.take_profit = r.takeProfit;
  w.stop_loss = r.stopLoss;
  w.position_intent = r.position_intent;

  std::visit(
      [&](const auto &x) {
        using X = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<X, Quantity>)
          w.qty = x.v;
        if constexpr (std::is_same_v<X, Notional>)
          w.notional = x.v;
      },
      r.amt);

  if (r.trailAmt) {
    std::visit(
        [&](const auto &x) {
          using X = std::decay_t<decltype(x)>;
          if constexpr (std::is_same_v<X, TrailPrice>)
            w.trail_price = x.v;
          if constexpr (std::is_same_v<X, TrailPercent>)
            w.trail_percent = x.v;
        },
        *r.trailAmt);
  }

  return w;
}

inline std::optional<std::string> fromWire(OrderRequest &r,
                                           const OrderRequestWire &w) {
  r.symbol = w.symbol;
  r.side = w.side;
  r.type = w.type;
  r.time_in_force = w.time_in_force;

  r.limitPrice = w.limit_price;
  r.stopPrice = w.stop_price;
  r.extended_hours = w.extended_hours;
  r.clientOrderID = w.clientOrderID;
  r.orderClass = w.orderClass;
  r.legs = w.legs;
  r.takeProfit = w.take_profit;
  r.stopLoss = w.stop_loss;
  r.position_intent = w.position_intent;

  if (w.qty && w.notional) {
    return std::make_optional("qty and notional are mutually exclusive");
  }
  if (w.qty)
    r.amt = Quantity{*w.qty};
  else if (w.notional)
    r.amt = Notional{*w.notional};
  else
    return std::make_optional("either qty or notional is required");

  if (w.trail_price && w.trail_percent) {
    return std::make_optional(
        "trail_price and trail_percent are mutually exclusive");
  }
  if (w.trail_price)
    r.trailAmt = TrailAmount{TrailPrice{*w.trail_price}};
  else if (w.trail_percent)
    r.trailAmt = TrailAmount{TrailPercent{*w.trail_percent}};
  else
    r.trailAmt.reset();

  return std::nullopt;
}

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

// Enums
template <> struct meta<alpaca::OrderSide> {
  using enum alpaca::OrderSide;
  static constexpr auto value = glz::enumerate(buy, sell);
};

template <> struct meta<alpaca::OrderType> {
  using enum alpaca::OrderType;
  static constexpr auto value =
      glz::enumerate(market, limit, stop, stop_limit, trailing_stop);
};

template <> struct meta<alpaca::OrderTimeInForce> {
  using enum alpaca::OrderTimeInForce;
  static constexpr auto value = glz::enumerate(day, gtc, opg, cls, ioc, fok);
};

template <> struct meta<alpaca::OrderClass> {
  using enum alpaca::OrderClass;
  static constexpr auto value =
      glz::enumerate(simple, bracket, oco, oto, mleg, crypto);
};

template <> struct meta<alpaca::PositionIntent> {
  using enum alpaca::PositionIntent;
  static constexpr auto value =
      glz::enumerate(buy_to_open, buy_to_close, sell_to_open, sell_to_close);
};

// Structs
template <> struct meta<alpaca::Leg> {
  using T = alpaca::Leg;
  static constexpr auto value =
      object("symbol", &T::symbol, "ratio_qty", &T::ratio_qty, "side", &T::side,
             "position_intent", &T::positionIntent);
};

template <> struct meta<alpaca::LegsResponse> {
  using T = alpaca::LegsResponse;
  static constexpr auto value =
      object("id", &T::id, "client_order_id", &T::client_order_id);
};

template <> struct meta<alpaca::StopLoss> {
  using T = alpaca::StopLoss;
  static constexpr auto value =
      object("stop_price", &T::stop_price, "limit_price", &T::limit_price);
};

template <> struct meta<alpaca::OrderRequest> {
  using T = alpaca::OrderRequest;
  static constexpr auto value = object(
      "symbol", &T::symbol, "amt", &T::amt, "side", &T::side, "type", &T::type,
      "time_in_force", &T::time_in_force, "limit_price", &T::limitPrice,
      "stop_price", &T::stopPrice, "trail_amt", &T::trailAmt, "extended_hours",
      &T::extended_hours, "client_order_id", &T::clientOrderID, "order_class",
      &T::orderClass, "legs", &T::legs, "take_profit", &T::takeProfit,
      "stop_loss", &T::stopLoss, "position_intent", &T::position_intent);
};

template <> struct meta<alpaca::OrderRequestWire> {
  using T = alpaca::OrderRequestWire;
  static constexpr auto value = glz::object(
      "symbol", &T::symbol, "qty", &T::qty, "notional", &T::notional, "side",
      &T::side, "type", &T::type, "time_in_force", &T::time_in_force,
      "limit_price", &T::limit_price, "stop_price", &T::stop_price,
      "extended_hours", &T::extended_hours, "client_order_id",
      &T::clientOrderID, "order_class", &T::orderClass, "legs", &T::legs,
      "trail_price", &T::trail_price, "trail_percent", &T::trail_percent,
      "take_profit", &T::take_profit, "stop_loss", &T::stop_loss,
      "position_intent", &T::position_intent);
};

template <> struct to<JSON, alpaca::OrderRequest> {
  template <auto Opts>
  static void op(const alpaca::OrderRequest &value, is_context auto &&ctx,
                 auto &&b, auto &&ix) noexcept {
    auto wire = toWire(value);
    serialize<JSON>::op<Opts>(wire, ctx, b, ix);
  }
};

template <> struct from<JSON, alpaca::OrderRequest> {
  template <auto Opts>
  static void op(alpaca::OrderRequest &value, is_context auto &&ctx, auto &&it,
                 auto &&end) {
    alpaca::OrderRequestWire wire{};
    parse<JSON>::op<Opts>(wire, ctx, it, end);
    auto error = fromWire(value, wire);
    if (error) {
      throw std::runtime_error(error.value());
    }
  }
};

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

}; // namespace glz

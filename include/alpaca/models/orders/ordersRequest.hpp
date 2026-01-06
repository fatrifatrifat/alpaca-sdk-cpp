#pragma once
#include <alpaca/models/orders/common.hpp>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>

namespace alpaca {

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

}; // namespace alpaca

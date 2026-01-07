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
  OrderTimeInForce timeInForce;

  std::optional<LimitPrice> limitPrice;
  std::optional<StopPrice> stopPrice;

  std::optional<TrailAmount> trailAmt;

  std::optional<bool> extendedHours;
  std::optional<std::string> clientOrderID;
  std::optional<OrderClass> orderClass;
  std::optional<Legs> legs;

  std::optional<TakeProfit> takeProfit;
  std::optional<StopLoss> stopLoss;
  std::optional<PositionIntent> positionIntent;
};

struct OrderRequestWire {
  std::string symbol;

  std::optional<long double> qty;
  std::optional<long double> notional;

  OrderSide side;
  OrderType type;
  OrderTimeInForce timeInForce;

  std::optional<LimitPrice> limitPrice;
  std::optional<StopPrice> stopPrice;

  std::optional<bool> extendedHours;
  std::optional<std::string> clientOrderID;
  std::optional<OrderClass> orderClass;
  std::optional<Legs> legs;

  std::optional<long double> trailPrice;
  std::optional<long double> trailPercent;

  std::optional<TakeProfit> takeProfit;
  std::optional<StopLoss> stopLoss;
  std::optional<PositionIntent> positionIntent;
};

inline OrderRequestWire toWire(const OrderRequest &r) {
  OrderRequestWire w{};
  w.symbol = r.symbol;
  w.side = r.side;
  w.type = r.type;
  w.timeInForce = r.timeInForce;

  w.limitPrice = r.limitPrice;
  w.stopPrice = r.stopPrice;
  w.extendedHours = r.extendedHours;
  w.clientOrderID = r.clientOrderID;
  w.orderClass = r.orderClass;
  w.legs = r.legs;
  w.takeProfit = r.takeProfit;
  w.stopLoss = r.stopLoss;
  w.positionIntent = r.positionIntent;

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
            w.trailPrice = x.v;
          if constexpr (std::is_same_v<X, TrailPercent>)
            w.trailPercent = x.v;
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
  r.timeInForce = w.timeInForce;

  r.limitPrice = w.limitPrice;
  r.stopPrice = w.stopPrice;
  r.extendedHours = w.extendedHours;
  r.clientOrderID = w.clientOrderID;
  r.orderClass = w.orderClass;
  r.legs = w.legs;
  r.takeProfit = w.takeProfit;
  r.stopLoss = w.stopLoss;
  r.positionIntent = w.positionIntent;

  if (w.qty && w.notional) {
    return std::make_optional("qty and notional are mutually exclusive");
  }
  if (w.qty)
    r.amt = Quantity{*w.qty};
  else if (w.notional)
    r.amt = Notional{*w.notional};
  else
    return std::make_optional("either qty or notional is required");

  if (w.trailPrice && w.trailPercent) {
    return std::make_optional(
        "trail_price and trail_percent are mutually exclusive");
  }
  if (w.trailPrice)
    r.trailAmt = TrailAmount{TrailPrice{*w.trailPrice}};
  else if (w.trailPercent)
    r.trailAmt = TrailAmount{TrailPercent{*w.trailPercent}};
  else
    r.trailAmt.reset();

  return std::nullopt;
}

}; // namespace alpaca

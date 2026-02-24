#pragma once
#include <glaze/glaze.hpp>

namespace alpaca {

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
enum class OrderStatus { open, closed, all };
enum class OrderDirection { asc, desc };
enum class OrderAssetClass { us_equity, us_option, crypto, all };

struct Leg {
  std::string symbol{};
  long double ratioQty{};
  std::optional<OrderSide> side = std::nullopt;
  std::optional<PositionIntent> positionIntent = std::nullopt;
  auto operator<=>(const Leg &) const = default;
};

struct LegsResponse {
  std::string id{};
  std::string clientOrderID{};
  auto operator<=>(const LegsResponse &) const = default;
};

using Legs = std::vector<Leg>;
using LimitPrice = long double;
using StopPrice = long double;

struct Quantity {
  long double v{};
  auto operator<=>(const Quantity &) const = default;
};
struct Notional {
  long double v{};
  auto operator<=>(const Notional &) const = default;
};
using ShareAmount = std::variant<Quantity, Notional>;

struct TrailPrice {
  long double v{};
  auto operator<=>(const TrailPrice &) const = default;
};
struct TrailPercent {
  long double v{};
  auto operator<=>(const TrailPercent &) const = default;
};
using TrailAmount = std::variant<TrailPrice, TrailPercent>;

struct TakeProfit {
  StopPrice stopPrice{};
  auto operator<=>(const TakeProfit &) const = default;
};

struct StopLoss {
  StopPrice stopPrice{};
  LimitPrice limitPrice{};
  auto operator<=>(const StopLoss &) const = default;
};

struct OrderRequestParam {
  std::string symbol{};
  ShareAmount amt{};
  OrderSide side{};
  OrderType type{};
  OrderTimeInForce timeInForce{};

  std::optional<LimitPrice> limitPrice = std::nullopt;
  std::optional<StopPrice> stopPrice = std::nullopt;

  std::optional<TrailAmount> trailAmt = std::nullopt;

  std::optional<bool> extendedHours = std::nullopt;
  std::optional<std::string> clientOrderID = std::nullopt;
  std::optional<OrderClass> orderClass = std::nullopt;
  std::optional<Legs> legs = std::nullopt;

  std::optional<TakeProfit> takeProfit = std::nullopt;
  std::optional<StopLoss> stopLoss = std::nullopt;
  std::optional<PositionIntent> positionIntent = std::nullopt;
};

struct OrderRequestWire {
  std::string symbol{};

  std::optional<long double> qty = std::nullopt;
  std::optional<long double> notional = std::nullopt;

  OrderSide side{};
  OrderType type{};
  OrderTimeInForce timeInForce{};

  std::optional<LimitPrice> limitPrice = std::nullopt;
  std::optional<StopPrice> stopPrice = std::nullopt;

  std::optional<bool> extendedHours = std::nullopt;
  std::optional<std::string> clientOrderID = std::nullopt;
  std::optional<OrderClass> orderClass = std::nullopt;
  std::optional<Legs> legs = std::nullopt;

  std::optional<long double> trailPrice = std::nullopt;
  std::optional<long double> trailPercent = std::nullopt;

  std::optional<TakeProfit> takeProfit = std::nullopt;
  std::optional<StopLoss> stopLoss = std::nullopt;
  std::optional<PositionIntent> positionIntent = std::nullopt;
};

inline OrderRequestWire toWire(const OrderRequestParam &r) noexcept {
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

inline std::optional<std::string> fromWire(OrderRequestParam &r,
                                           const OrderRequestWire &w) noexcept {
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

struct OrderResponse {
  std::string id;
  std::string clientOrderID;

  std::string createdAt;
  std::string updatedAt;
  std::string submittedAt;

  std::optional<std::string> filledAt = std::nullopt;
  std::optional<std::string> expiredAt = std::nullopt;
  std::optional<std::string> canceledAt = std::nullopt;
  std::optional<std::string> failedAt = std::nullopt;
  std::optional<std::string> replacedAt = std::nullopt;

  std::optional<std::string> replacedBy = std::nullopt;
  std::optional<std::string> replaces = std::nullopt;

  std::optional<std::string> assetID = std::nullopt;
  std::optional<std::string> symbol = std::nullopt;
  std::optional<std::string> assetClass = std::nullopt;

  std::optional<std::string> notional = std::nullopt;
  std::optional<std::string> qty = std::nullopt;

  std::string filledQty;
  std::optional<std::string> filledAvgPrice = std::nullopt;

  std::optional<std::string> orderClass = std::nullopt;
  std::string orderType;
  OrderType type;
  OrderSide side;

  OrderTimeInForce timeInForce;
  std::optional<std::string> limitPrice = std::nullopt;
  std::optional<std::string> stopPrice = std::nullopt;

  std::string status;

  std::optional<std::string> position_intent = std::nullopt;

  bool extendedHours{false};

  std::optional<glz::generic> legs = std::nullopt;
  std::optional<std::string> trailPercent = std::nullopt;
  std::optional<std::string> trailPrice = std::nullopt;
  std::optional<std::string> hwm = std::nullopt;

  std::optional<std::string> subtag = std::nullopt;
  std::optional<std::string> source = std::nullopt;
};

struct OrderListParam {
  std::optional<OrderStatus> status = std::nullopt;
  std::optional<uint32_t> limit = std::nullopt;
  std::optional<std::string> after = std::nullopt;
  std::optional<std::string> until = std::nullopt;
  std::optional<OrderDirection> direction = std::nullopt;
  std::optional<bool> nested = std::nullopt;
  std::optional<std::vector<std::string>> symbols = std::nullopt;
  std::optional<OrderSide> side = std::nullopt;
  std::optional<std::vector<OrderAssetClass>> assetClass = std::nullopt;
  std::optional<std::string> beforeOrderID = std::nullopt;
  std::optional<std::string> afterOrderID = std::nullopt;
};

struct OrderID {
  std::string id;
  int status;
};

struct ReplaceOrderParam {
  std::optional<long double> qty = std::nullopt;
  std::optional<OrderTimeInForce> timeInForce = std::nullopt;
  std::optional<long double> limitPrice = std::nullopt;
  std::optional<long double> stopPrice = std::nullopt;
  std::optional<long double> trail = std::nullopt;
  std::optional<std::string> clientOrderID = std::nullopt;
};

constexpr std::optional<std::string_view>
ToString(std::optional<OrderStatus> d) noexcept {
  if (!d) {
    return std::nullopt;
  }

  switch (*d) {
  case OrderStatus::all:
    return "all";
  case OrderStatus::closed:
    return "closed";
  case OrderStatus::open:
    return "open";
  default:
    return std::nullopt;
  }
}

constexpr std::optional<std::string_view>
ToString(std::optional<OrderDirection> d) noexcept {
  if (!d) {
    return std::nullopt;
  }

  switch (*d) {
  case OrderDirection::asc:
    return "asc";
  case OrderDirection::desc:
    return "desc";
  default:
    return std::nullopt;
  }
}

constexpr std::optional<std::string_view>
ToString(std::optional<OrderSide> d) noexcept {
  if (!d) {
    return std::nullopt;
  }

  switch (*d) {
  case OrderSide::buy:
    return "buy";
  case OrderSide::sell:
    return "sell";
  default:
    return std::nullopt;
  }
}

constexpr std::optional<std::string_view>
ToString(std::optional<OrderAssetClass> d) noexcept {
  if (!d) {
    return std::nullopt;
  }

  switch (*d) {
  case OrderAssetClass::us_equity:
    return "us_equity";
  case OrderAssetClass::us_option:
    return "us_option";
  case OrderAssetClass::crypto:
    return "crypto";
  case OrderAssetClass::all:
    return "all";
  default:
    return std::nullopt;
  }
}

}; // namespace alpaca

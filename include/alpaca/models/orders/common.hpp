#pragma once
#include <optional>
#include <string>
#include <variant>
#include <vector>

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

struct Leg {
  std::string symbol;
  long double ratioQty;
  std::optional<OrderSide> side;
  std::optional<PositionIntent> positionIntent;
  auto operator<=>(const Leg &) const = default;
};

struct LegsResponse {
  std::string id;
  std::string clientOrderID;
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

}; // namespace alpaca

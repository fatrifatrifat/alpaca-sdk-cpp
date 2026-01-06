#pragma once
#include <optional>
#include <string>
#include <variant>
#include <vector>

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

}; // namespace alpaca

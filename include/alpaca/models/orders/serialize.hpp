#pragma once
#include <alpaca/models/orders/ordersRequest.hpp>
#include <alpaca/models/orders/ordersResponse.hpp>
#include <glaze/glaze.hpp>

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

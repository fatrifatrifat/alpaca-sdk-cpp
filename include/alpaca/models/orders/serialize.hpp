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

template <> struct meta<alpaca::OrderAssetClass> {
  using enum alpaca::OrderAssetClass;
  static constexpr auto value = glz::enumerate(us_equity, us_option, crypto);
};

// Structs
template <> struct meta<alpaca::Leg> {
  using T = alpaca::Leg;
  static constexpr auto value =
      object("symbol", &T::symbol, "ratio_qty", &T::ratioQty, "side", &T::side,
             "position_intent", &T::positionIntent);
};

template <> struct meta<alpaca::LegsResponse> {
  using T = alpaca::LegsResponse;
  static constexpr auto value =
      object("id", &T::id, "client_order_id", &T::clientOrderID);
};

template <> struct meta<alpaca::StopLoss> {
  using T = alpaca::StopLoss;
  static constexpr auto value =
      object("stop_price", &T::stopPrice, "limit_price", &T::limitPrice);
};

template <> struct meta<alpaca::OrderRequest> {
  using T = alpaca::OrderRequest;
  static constexpr auto value = object(
      "symbol", &T::symbol, "amt", &T::amt, "side", &T::side, "type", &T::type,
      "time_in_force", &T::timeInForce, "limit_price", &T::limitPrice,
      "stop_price", &T::stopPrice, "trail_amt", &T::trailAmt, "extended_hours",
      &T::extendedHours, "client_order_id", &T::clientOrderID, "order_class",
      &T::orderClass, "legs", &T::legs, "take_profit", &T::takeProfit,
      "stop_loss", &T::stopLoss, "position_intent", &T::positionIntent);
};

template <> struct meta<alpaca::OrderRequestWire> {
  using T = alpaca::OrderRequestWire;
  static constexpr auto value = glz::object(
      "symbol", &T::symbol, "qty", &T::qty, "notional", &T::notional, "side",
      &T::side, "type", &T::type, "time_in_force", &T::timeInForce,
      "limit_price", &T::limitPrice, "stop_price", &T::stopPrice,
      "extended_hours", &T::extendedHours, "client_order_id", &T::clientOrderID,
      "order_class", &T::orderClass, "legs", &T::legs, "trail_price",
      &T::trailPrice, "trail_percent", &T::trailPercent, "take_profit",
      &T::takeProfit, "stop_loss", &T::stopLoss, "position_intent",
      &T::positionIntent);
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
      "id", &T::id, "client_order_id", &T::clientOrderID, "created_at",
      &T::createdAt, "updated_at", &T::updatedAt, "submitted_at",
      &T::submittedAt, "filled_at", &T::filledAt, "expired_at", &T::expiredAt,
      "canceled_at", &T::canceledAt, "failed_at", &T::failedAt, "replaced_at",
      &T::replacedAt, "replaced_by", &T::replacedBy, "replaces", &T::replaces,
      "asset_id", &T::assetID, "symbol", &T::symbol, "asset_class",
      &T::assetClass, "notional", &T::notional, "qty", &T::qty, "filled_qty",
      &T::filledQty, "filled_avg_price", &T::filledAvgPrice, "order_class",
      &T::orderClass, "order_type", &T::orderType, "type", &T::type, "side",
      &T::side, "time_in_force", &T::timeInForce, "limit_price", &T::limitPrice,
      "stop_price", &T::stopPrice, "status", &T::status, "position_intent",
      &T::position_intent, "extended_hours", &T::extendedHours, "legs",
      &T::legs, "trail_percent", &T::trailPercent, "trail_price",
      &T::trailPrice, "hwm", &T::hwm, "subtag", &T::subtag, "source",
      &T::source, "expires_at", &T::expiredAt);
};

}; // namespace glz

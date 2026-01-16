#pragma once
#include <alpaca/models/trading/account.hpp>
#include <alpaca/models/trading/calendar.hpp>
#include <alpaca/models/trading/clock.hpp>
#include <alpaca/models/trading/order.hpp>
#include <alpaca/models/trading/position.hpp>
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

template <> struct meta<alpaca::PositionExchange> {
  using enum alpaca::PositionExchange;
  static constexpr auto value =
      glz::enumerate(AMEX, ARCA, BATS, NYSE, NASDAQ, NYSEARCA, OTC);
};

template <> struct meta<alpaca::Position> {
  using T = alpaca::Position;
  static constexpr auto value = object(
      "asset_id", &T::assetID, "symbol", &T::symbol, "exchange", &T::exchange,
      "asset_class", &T::assetClass, "asset_marginable", &T::assetMarginable,
      "qty", &T::qty, "avg_entry_price", &T::avgEntryPrice, "side", &T::side,
      "market_value", &T::marketValue, "cost_basis", &T::costBasis,
      "unrealized_pl", &T::unrealizedPL, "unrealized_plpc", &T::unrealizedPLPC,
      "unrealized_intraday_pl", &T::unrealizedIntradayPL,
      "unrealized_intraday_plpc", &T::unrealizedIntradayPLPC, "current_price",
      &T::currentPrice, "lastday_price", &T::lastDayPrice, "change_today",
      &T::changeToday, "qty_available", &T::qtyAvailable);
};

template <> struct meta<alpaca::AccountStatus> {
  using enum alpaca::AccountStatus;
  static constexpr auto value =
      glz::enumerate(ONBOARDING, SUBMISSION_FAILED, SUBMITTED, ACCOUNT_UPDATED,
                     APPROVAL_PENDING, ACTIVE, REJECTED);
};

template <> struct meta<alpaca::Account> {
  using T = alpaca::Account;
  static constexpr auto value = object(
      "id", &T::id, "admin_configurations", &T::admin_configurations,
      "user_configurations", &T::user_configurations, "account_number",
      &T::account_number, "status", &T::status, "crypto_status",
      &T::crypto_status, "options_approved_level", &T::options_approved_level,
      "options_trading_level", &T::options_trading_level, "currency",
      &T::currency, "buying_power", &T::buying_power, "regt_buying_power",
      &T::regt_buying_power, "daytrading_buying_power",
      &T::daytrading_buying_power, "effective_buying_power",
      &T::effective_buying_power, "non_marginable_buying_power",
      &T::non_marginable_buying_power, "options_buying_power",
      &T::options_buying_power, "bod_dtbp", &T::bod_dtbp, "cash", &T::cash,
      "accrued_fees", &T::accrued_fees, "portfolio_value", &T::portfolio_value,
      "pattern_day_trader", &T::pattern_day_trader, "trading_blocked",
      &T::trading_blocked, "transfers_blocked", &T::transfers_blocked,
      "account_blocked", &T::account_blocked, "created_at", &T::created_at,
      "trade_suspended_by_user", &T::trade_suspended_by_user, "multiplier",
      &T::multiplier, "shorting_enabled", &T::shorting_enabled, "equity",
      &T::equity, "last_equity", &T::last_equity, "long_market_value",
      &T::long_market_value, "short_market_value", &T::short_market_value,
      "position_market_value", &T::position_market_value, "initial_margin",
      &T::initial_margin, "maintenance_margin", &T::maintenance_margin,
      "last_maintenance_margin", &T::last_maintenance_margin, "sma", &T::sma,
      "daytrade_count", &T::daytrade_count, "balance_asof", &T::balance_asof,
      "crypto_tier", &T::crypto_tier, "intraday_adjustments",
      &T::intraday_adjustments, "pending_reg_taf_fees",
      &T::pending_reg_taf_fees);
};

template <> struct meta<alpaca::Clock> {
  using T = alpaca::Clock;
  static constexpr auto value =
      object("timestamp", &T::timestamp, "is_open", &T::isOpen, "next_open",
             &T::nextOpen, "next_close", &T::nextClose);
};

template <> struct meta<alpaca::CalendarRequest> {
  using T = alpaca::CalendarRequest;
  static constexpr auto value =
      object("start", &T::start, "end", &T::end, "date_type", &T::dateType);
};

template <> struct meta<alpaca::Calendar> {
  using T = alpaca::Calendar;
  static constexpr auto value =
      object("close", &T::close, "date", &T::date, "open", &T::open,
             "session_close", &T::session_close, "session_open",
             &T::session_open, "settlement_date", &T::settlementDate);
};

}; // namespace glz

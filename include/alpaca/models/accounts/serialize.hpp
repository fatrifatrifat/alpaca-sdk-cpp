#pragma once
#include <alpaca/models/accounts/accounts.hpp>

namespace glz {

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

}; // namespace glz

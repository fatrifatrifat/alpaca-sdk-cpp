#pragma once
#include <glaze/glaze.hpp>

namespace alpaca {

enum class AccountStatus {
  ONBOARDING,
  SUBMISSION_FAILED,
  SUBMITTED,
  ACCOUNT_UPDATED,
  APPROVAL_PENDING,
  ACTIVE,
  REJECTED,
};

enum OptionLevel {
  Disabled,
  CoveredCall,
  LogCall,
  Spreads,
};

struct Account {
  std::string id;

  glz::generic admin_configurations;
  std::optional<glz::generic> user_configurations;

  std::string account_number;
  AccountStatus status;
  std::string crypto_status;

  OptionLevel options_approved_level{};
  OptionLevel options_trading_level{};

  std::string currency;

  std::string buying_power;
  std::string regt_buying_power;
  std::string daytrading_buying_power;
  std::string effective_buying_power;
  std::string non_marginable_buying_power;
  std::string options_buying_power;
  std::string bod_dtbp;
  std::string cash;
  std::string accrued_fees;
  std::string portfolio_value;

  bool pattern_day_trader{};
  bool trading_blocked{};
  bool transfers_blocked{};
  bool account_blocked{};

  std::string created_at;
  bool trade_suspended_by_user{};

  std::string multiplier;
  bool shorting_enabled{};

  std::string equity;
  std::string last_equity;
  std::string long_market_value;
  std::string short_market_value;
  std::string position_market_value;
  std::string initial_margin;
  std::string maintenance_margin;
  std::string last_maintenance_margin;
  std::string sma;

  int daytrade_count{};
  std::string balance_asof;

  int crypto_tier{};
  std::string intraday_adjustments;
  std::string pending_reg_taf_fees;
};

}; // namespace alpaca

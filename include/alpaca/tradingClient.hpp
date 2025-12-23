#include "environment.hpp"
#include "httpClient.hpp"
#include <expected>
#include <print>

namespace alpaca {

struct Account {
  std::string id;

  glz::generic admin_configurations;
  std::optional<glz::generic> user_configurations;

  std::string account_number;
  std::string status;
  std::string crypto_status;

  int options_approved_level{};
  int options_trading_level{};

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

class TradingClient {
public:
  TradingClient(Environment &env) : env_(env), cli_(env_.GetBaseUrl()) {}

  std::expected<Account, std::string> GetAccount() {
    auto resp = cli_.Get(ACCOUNT_ENTRYPOINT, env_.GetAuthHeaders());
    if (!resp) {
      return std::unexpected(std::format("Error: {}", resp.error()));
    }

    if (resp->status != 200) {
      return std::unexpected(std::format("Error Code: {}", resp->status));
    }

    std::println("{}", resp->body);
    Account account;
    auto error = glz::read_json(account, resp->body);
    if (error) {
      return std::unexpected(
          std::format("Error Code: {}", glz::format_error(error, resp->body)));
    }

    return account;
  }

private:
  Environment &env_;
  HttpClient cli_;

  static constexpr const char *ACCOUNT_ENTRYPOINT = "/v2/account";
};

}; // namespace alpaca

namespace glz {
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
} // namespace glz

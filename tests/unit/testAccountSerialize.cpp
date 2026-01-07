#include <alpaca/alpaca.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <glaze/glaze.hpp>

#include <string>

namespace {

glz::generic generic_from_json(const std::string &json) {
  glz::generic g{};
  auto ec = glz::read_json(g, json);
  REQUIRE(!ec);
  return g;
}

alpaca::Account make_account_full() {
  alpaca::Account a{};

  a.id = "acct_123";
  a.admin_configurations = generic_from_json(R"json({"admin":"ok","x":1})json");
  a.user_configurations = generic_from_json(R"json({"theme":"dark"})json");

  a.account_number = "ABC123";
  a.status = alpaca::AccountStatus::ACTIVE;
  a.crypto_status = "ACTIVE";

  a.options_approved_level = alpaca::OptionLevel::CoveredCall;
  a.options_trading_level = alpaca::OptionLevel::Spreads;

  a.currency = "USD";

  a.buying_power = "10000";
  a.regt_buying_power = "9000";
  a.daytrading_buying_power = "8000";
  a.effective_buying_power = "7000";
  a.non_marginable_buying_power = "6000";
  a.options_buying_power = "5000";
  a.bod_dtbp = "4000";
  a.cash = "3000";
  a.accrued_fees = "0";
  a.portfolio_value = "12345.67";

  a.pattern_day_trader = false;
  a.trading_blocked = false;
  a.transfers_blocked = false;
  a.account_blocked = false;

  a.created_at = "2026-01-07T10:00:00Z";
  a.trade_suspended_by_user = false;

  a.multiplier = "2";
  a.shorting_enabled = true;

  a.equity = "12345.67";
  a.last_equity = "12000.00";
  a.long_market_value = "10000.00";
  a.short_market_value = "0.00";
  a.position_market_value = "10000.00";
  a.initial_margin = "0.00";
  a.maintenance_margin = "0.00";
  a.last_maintenance_margin = "0.00";
  a.sma = "0.00";

  a.daytrade_count = 0;
  a.balance_asof = "2026-01-07";

  a.crypto_tier = 1;
  a.intraday_adjustments = "0";
  a.pending_reg_taf_fees = "0";

  return a;
}

} // namespace

TEST_CASE(
    "Glaze Account: parse minimal subset JSON populates provided fields") {
  const std::string json = R"json(
  {
    "id": "acct_123",
    "admin_configurations": {"admin":"ok"},
    "account_number": "ABC123",
    "status": "ACTIVE",
    "crypto_status": "ACTIVE",
    "options_approved_level": 1,
    "options_trading_level": 3,
    "currency": "USD"
  }
  )json";

  alpaca::Account a{};
  auto ec = glz::read_json(a, json);
  REQUIRE(!ec);

  REQUIRE(a.id == "acct_123");
  REQUIRE(a.account_number == "ABC123");
  REQUIRE(a.status == alpaca::AccountStatus::ACTIVE);
  REQUIRE(a.crypto_status == "ACTIVE");
  REQUIRE(a.currency == "USD");

  REQUIRE(a.options_approved_level == alpaca::OptionLevel::CoveredCall);
  REQUIRE(a.options_trading_level == alpaca::OptionLevel::Spreads);

  REQUIRE_FALSE(a.user_configurations.has_value());
}

TEST_CASE(
    "Glaze Account: serialize -> deserialize roundtrip preserves key fields") {
  const auto in = make_account_full();

  std::string json;
  auto w = glz::write_json(in, json);
  REQUIRE(!w);
  REQUIRE(!json.empty());

  REQUIRE(json.find("\"account_number\"") != std::string::npos);
  REQUIRE(json.find("\"admin_configurations\"") != std::string::npos);
  REQUIRE(json.find("\"user_configurations\"") != std::string::npos);
  REQUIRE(json.find("\"status\"") != std::string::npos);
  REQUIRE(json.find("ACTIVE") != std::string::npos);

  alpaca::Account out{};
  auto r = glz::read_json(out, json);
  REQUIRE(!r);

  REQUIRE(out.id == in.id);
  REQUIRE(out.account_number == in.account_number);
  REQUIRE(out.status == in.status);
  REQUIRE(out.crypto_status == in.crypto_status);

  REQUIRE(out.options_approved_level == in.options_approved_level);
  REQUIRE(out.options_trading_level == in.options_trading_level);

  REQUIRE(out.currency == in.currency);

  REQUIRE(out.buying_power == in.buying_power);
  REQUIRE(out.portfolio_value == in.portfolio_value);

  REQUIRE(out.shorting_enabled == in.shorting_enabled);
  REQUIRE(out.daytrade_count == in.daytrade_count);
  REQUIRE(out.crypto_tier == in.crypto_tier);

  REQUIRE(out.user_configurations.has_value());
}

TEST_CASE("Glaze Account: invalid AccountStatus value fails parsing") {
  const std::string bad = R"json(
  {
    "id": "acct_123",
    "admin_configurations": {},
    "account_number": "ABC123",
    "status": "NOT_A_REAL_STATUS",
    "crypto_status": "ACTIVE",
    "options_approved_level": 1,
    "options_trading_level": 1,
    "currency": "USD"
  }
  )json";

  alpaca::Account a{};
  auto ec = glz::read_json(a, bad);
  REQUIRE(ec);
}

TEST_CASE("Glaze Account: missing required fields fails parsing (strict "
          "missing keys)") {
  const std::string incomplete = R"json({ "id": "acct_123" })json";

  alpaca::Account a{};
  auto ec = glz::read<glz::opts{.error_on_missing_keys = true}>(a, incomplete);

  REQUIRE(ec);
  REQUIRE(ec.ec == glz::error_code::missing_key);
}

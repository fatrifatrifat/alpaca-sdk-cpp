#include "environment.hpp"
#include "httpClient.hpp"
#include "utils.hpp"
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

struct OrderRequest {
  std::string symbol;
  std::string qty;
  std::string side;
  std::string type{"market"};
  std::string time_in_force{"day"};
  bool extended_hours{false};

  std::optional<std::string> limit_price;
  std::optional<std::string> stop_price;
  std::optional<std::string> client_order_id;
};

struct OrderResponse {
  std::string id;
  std::string client_order_id;

  std::string created_at;
  std::string updated_at;
  std::string submitted_at;

  std::optional<std::string> filled_at;
  std::optional<std::string> expired_at;
  std::optional<std::string> canceled_at;
  std::optional<std::string> failed_at;
  std::optional<std::string> replaced_at;

  std::optional<std::string> replaced_by;
  std::optional<std::string> replaces;

  std::string asset_id;
  std::string symbol;
  std::string asset_class;

  std::optional<std::string> notional;

  std::string qty;
  std::string filled_qty;
  std::optional<std::string> filled_avg_price;

  std::string order_class;
  std::string order_type;
  std::string type;
  std::string side;

  std::optional<std::string> position_intent;

  std::string time_in_force;

  std::optional<std::string> limit_price;
  std::optional<std::string> stop_price;

  std::string status;

  bool extended_hours{false};

  std::optional<glz::generic> legs;
  std::optional<std::string> trail_percent;
  std::optional<std::string> trail_price;
  std::optional<std::string> hwm;

  std::optional<std::string> subtag;
  std::optional<std::string> source;

  std::optional<std::string> expires_at;
};

struct Position {
  std::string asset_id;
  std::string symbol;
  std::string exchange;
  std::string asset_class;
  bool asset_marginable{};

  std::string qty;
  std::string avg_entry_price;
  std::string side;
  std::string market_value;
  std::string cost_basis;

  std::string unrealized_pl;
  std::string unrealized_plpc;
  std::string unrealized_intraday_pl;
  std::string unrealized_intraday_plpc;

  std::string current_price;
  std::string lastday_price;
  std::string change_today;

  std::string qty_available;
};

using Positions = std::vector<Position>;

struct Shares {
  long double value{};
};

struct Percent {
  long double value{};
};

using LiquidationAmount = std::variant<Shares, Percent>;

struct ClosePositionParams {
  std::string symbol_or_asset_id;
  LiquidationAmount amt;
};

class TradingClient {
private:
  static std::expected<std::string, std::string>
  build_close_position_query(const LiquidationAmount &a) {
    return std::visit(
        [](auto &&x) -> std::expected<std::string, std::string> {
          using X = std::decay_t<decltype(x)>;
          if constexpr (std::is_same_v<X, Shares>) {
            if (x.value <= 0)
              return std::unexpected("qty must be > 0");
            return "qty=" + std::format("{:.9f}", (long double)x.value);
          } else {
            if (x.value <= 0 || x.value > 100)
              return std::unexpected("percentage must be (0,100]");
            return "percentage=" + std::format("{:.9f}", (long double)x.value);
          }
        },
        a);
  }

public:
  explicit TradingClient(const Environment &env) : env_(env), cli_(env_.GetBaseUrl()) {}

  std::expected<Account, std::string> GetAccount() {
    auto resp = cli_.Get(ACCOUNT_ENDPOINT, env_.GetAuthHeaders());
    if (!resp) {
      return std::unexpected(std::format("Error: {}", resp.error()));
    }

    if (!utils::is_success(resp->status)) {
      return std::unexpected(std::format("HTTP {}: {}", resp->status, resp->body));
    }

    std::println("{}", resp->body);
    Account account;
    auto error = glz::read_json(account, resp->body);
    if (error) {
      return std::unexpected(
          std::format("Error: {}", glz::format_error(error, resp->body)));
    }

    return account;
  }

  std::expected<OrderResponse, std::string>
  SubmitOrder(const OrderRequest &request) {
    auto order_request = glz::write_json(request);
    if (!order_request) {
      return std::unexpected(
          std::format("Error: {}", glz::format_error(order_request.error())));
    }

    auto resp = cli_.Post(ORDERS_ENDPOINT, env_.GetAuthHeaders(),
                          order_request.value(), "application/json");

    if (!resp) {
      return std::unexpected(std::format("Error: {}", resp.error()));
    }

    if (!utils::is_success(resp->status)) {
      return std::unexpected(std::format("HTTP {}: {}", resp->status, resp->body));
    }

    std::println("{}", resp->body);
    OrderResponse response;
    auto error = glz::read_json(response, resp->body);
    if (error) {
      return std::unexpected(
          std::format("Error: {}", glz::format_error(error, resp->body)));
    }

    return response;
  }

  std::expected<Positions, std::string> GetAllOpenPositions() {
    auto resp = cli_.Get(POSITIONS_ENDPOINT, env_.GetAuthHeaders());
    if (!resp) {
      return std::unexpected(std::format("Error: {}", resp.error()));
    }

    if (!utils::is_success(resp->status)) {
      return std::unexpected(std::format("HTTP {}: {}", resp->status, resp->body));
    }

    std::println("{}", resp->body);
    Positions positions;
    auto error = glz::read_json(positions, resp->body);
    if (error) {
      return std::unexpected(
          std::format("Error: {}", glz::format_error(error, resp->body)));
    }

    return positions;
  }

  std::expected<Position, std::string>
  GetOpenPosition(const std::string &symbol) {
    std::println("{}/{}", POSITIONS_ENDPOINT, symbol);
    auto resp = cli_.Get(std::format("{}/{}", POSITIONS_ENDPOINT, symbol),
                         env_.GetAuthHeaders());
    if (!resp) {
      return std::unexpected(std::format("Error: {}", resp.error()));
    }

    if (!utils::is_success(resp->status)) {
      return std::unexpected(std::format("HTTP {}: {}", resp->status, resp->body));
    }

    std::println("{}", resp->body);
    Position position;
    auto error = glz::read_json(position, resp->body);
    if (error) {
      return std::unexpected(
          std::format("Error: {}", glz::format_error(error, resp->body)));
    }

    return position;
  }

  std::expected<OrderResponse, std::string>
  ClosePosition(const ClosePositionParams &cpp) {
    if (cpp.symbol_or_asset_id.empty()) {
      return std::unexpected("Error: Unvalid empty symbol parameter");
    }
    const auto qty_query = build_close_position_query(cpp.amt);
    if (!qty_query) {
      return std::unexpected("Error: Unvalid liquidation amount query");
    }

    auto resp =
        cli_.Delete(std::format("{}/{}?{}", POSITIONS_ENDPOINT,
                                cpp.symbol_or_asset_id, qty_query.value()),
                    env_.GetAuthHeaders());
    if (!resp) {
      return std::unexpected(std::format("Error: {}", resp.error()));
    }

    if (!utils::is_success(resp->status)) {
      return std::unexpected(std::format("HTTP {}: {}", resp->status, resp->body));
    }

    std::println("{}", resp->body);
    OrderResponse response;
    auto error = glz::read_json(response, resp->body);
    if (error) {
      return std::unexpected(
          std::format("Error: {}", glz::format_error(error, resp->body)));
    }

    return response;
  }

private:
  const Environment &env_;
  HttpClient cli_;

  static constexpr const char *ACCOUNT_ENDPOINT = "/v2/account";
  static constexpr const char *ORDERS_ENDPOINT = "/v2/orders";
  static constexpr const char *POSITIONS_ENDPOINT = "/v2/positions";
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

template <> struct meta<alpaca::OrderRequest> {
  using T = alpaca::OrderRequest;
  static constexpr auto value =
      object("symbol", &T::symbol, "qty", &T::qty, "side", &T::side, "type",
             &T::type, "time_in_force", &T::time_in_force, "extended_hours",
             &T::extended_hours, "limit_price", &T::limit_price, "stop_price",
             &T::stop_price, "client_order_id", &T::client_order_id);
};

template <> struct meta<alpaca::Position> {
  using T = alpaca::Position;
  static constexpr auto value = object(
      "asset_id", &T::asset_id, "symbol", &T::symbol, "exchange", &T::exchange,
      "asset_class", &T::asset_class, "asset_marginable", &T::asset_marginable,
      "qty", &T::qty, "avg_entry_price", &T::avg_entry_price, "side", &T::side,
      "market_value", &T::market_value, "cost_basis", &T::cost_basis,
      "unrealized_pl", &T::unrealized_pl, "unrealized_plpc",
      &T::unrealized_plpc, "unrealized_intraday_pl", &T::unrealized_intraday_pl,
      "unrealized_intraday_plpc", &T::unrealized_intraday_plpc, "current_price",
      &T::current_price, "lastday_price", &T::lastday_price, "change_today",
      &T::change_today, "qty_available", &T::qty_available);
};

} // namespace glz

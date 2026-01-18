#pragma once
#include <alpaca/client/environment.hpp>
#include <alpaca/client/httpClient.hpp>
#include <alpaca/models/trading/serialize.hpp>
#include <alpaca/utils/utils.hpp>
#include <expected>

namespace alpaca {

template <class Env = Environment, class Http = HttpClient>
class TradingClientT {
private:
  static std::expected<std::string, std::string>
  BuildClosePositionQuery(const LiquidationAmount &a) {
    return std::visit(
        [](auto &&x) -> std::expected<std::string, std::string> {
          using X = std::decay_t<decltype(x)>;
          if constexpr (std::is_same_v<X, Shares>) {
            if (x.value <= 0) {
              return std::unexpected("qty must be > 0");
            }
            return "qty=" + std::format("{:.9f}", (long double)x.value);
          } else {
            if (x.value <= 0 || x.value > 100) {
              return std::unexpected("percentage must be (0,100]");
            }
            return "percentage=" + std::format("{:.9f}", (long double)x.value);
          }
        },
        a);
  }

public:
  explicit TradingClientT(const Env &env)
      : env_(env), cli_(env_.GetBaseUrl(), env_.GetAuthHeaders()) {}

  TradingClientT(const Env &env, Http cli) : env_(env), cli_(std::move(cli)) {}

  std::expected<Account, APIError> GetAccount() {
    const auto &query = ACCOUNT_ENDPOINT;
    return cli_.template Request<Account>(Req::GET, query);
  }

  std::expected<OrderResponse, APIError>
  SubmitOrder(const OrderRequest &request) {
    std::string json;
    auto order_request = glz::write_json(request);
    if (!order_request) {
      return std::unexpected(APIError{
          ErrorCode::JSONParsing, glz::format_error(order_request.error())});
    }

    const auto &query = ORDERS_ENDPOINT;
    return cli_.template Request<OrderResponse>(
        Req::POST, query, order_request.value(), "application/json");
  }

  std::expected<Positions, APIError> GetAllOpenPositions() {
    const auto &query = POSITIONS_ENDPOINT;
    return cli_.template Request<Positions>(Req::GET, query);
  }

  std::expected<Position, APIError> GetOpenPosition(const std::string &symbol) {
    const auto query = std::format("{}/{}", POSITIONS_ENDPOINT, symbol);
    return cli_.template Request<Position>(Req::GET, query);
  }

  std::expected<OrderResponse, APIError>
  ClosePosition(const ClosePositionParams &cpp) {
    if (cpp.symbol_or_asset_id.empty()) {
      return std::unexpected(
          APIError{ErrorCode::IllArgument, "Unvalid empty symbol parameter"});
    }
    const auto qty_query = BuildClosePositionQuery(cpp.amt);
    if (!qty_query) {
      return std::unexpected(
          APIError{ErrorCode::IllArgument, "Unvalid liquidation amount query"});
    }

    const auto query = std::format("{}/{}?{}", POSITIONS_ENDPOINT,
                                   cpp.symbol_or_asset_id, qty_query.value());
    return cli_.template Request<OrderResponse>(Req::DELETE, query);
  }

  std::expected<Clock, APIError> GetMarketClockInfo() {
    const auto &query = CLOCK_ENDPOINT;
    return cli_.template Request<Clock>(Req::GET, query);
  }

  std::expected<CalendarResponse, APIError>
  GetMarketCalendarInfo(const CalendarRequest &c = {}) {
    utils::QueryBuilder qb;
    qb.add("start", c.start);
    qb.add("end", c.end);
    qb.add("date_type", c.dateType);
    const auto &query = std::format("{}?{}", CALENDAR_ENDPOINT, qb.q);
    return cli_.template Request<CalendarResponse>(Req::GET, query);
  }

  std::expected<std::vector<OrderResponse>, APIError>
  GetAllOrders(const OrderListParam &o = {}) {
    auto status = o.status ? ToString(*o.status) : std::nullopt;
    auto limit =
        o.limit ? std::make_optional(std::to_string(*o.limit)) : std::nullopt;
    auto direction = o.direction ? ToString(*o.direction) : std::nullopt;
    auto nested = o.nested ? std::make_optional(*o.nested ? "true" : "false")
                           : std::nullopt;
    auto symbols = o.symbols
                       ? std::make_optional(utils::SymbolsEncode(*o.symbols))
                       : std::nullopt;
    auto side = o.side ? ToString(*o.side) : std::nullopt;

    utils::QueryBuilder qb;
    qb.add("status", status);
    qb.add("limit", limit);
    qb.add("after", o.after);
    qb.add("until", o.until);
    qb.add("direction", direction);
    qb.add("nested", nested);
    qb.add("symbols", symbols);
    qb.add("side", side);
    if (o.assetClass) {
      for (const auto &a : *o.assetClass) {
        qb.add("asset_class", ToString(a));
      }
    }
    qb.add("before_order_id", o.beforeOrderID);
    qb.add("after_order_id", o.afterOrderID);
    const auto query = std::format("{}?{}", ORDERS_ENDPOINT, qb.q);
    std::println("Query: {}", query);
    return cli_.template Request<std::vector<OrderResponse>>(Req::GET, query);
  }

  std::expected<OrderID, APIError> DeleteAllOrders() {
    const auto &query = ORDERS_ENDPOINT;
    return cli_.template Request<OrderID>(Req::DELETE, query);
  }

  std::expected<OrderResponse, APIError>
  GetOrderByClientID(std::string_view id) {
    utils::QueryBuilder qb;
    qb.add("client_order_id", id);
    const auto query =
        std::format("{}:by_client_order_id?{}", ORDERS_ENDPOINT, qb.q);
    return cli_.template Request<OrderResponse>(Req::GET, query);
  }

  std::expected<OrderResponse, APIError>
  GetOrderByID(std::string_view orderID,
               std::optional<bool> nstd = std::nullopt) {
    auto nested =
        nstd ? std::make_optional((*nstd ? "true" : "false")) : std::nullopt;

    utils::QueryBuilder qb;
    qb.add("nested", nested);
    const auto query = std::format("{}/{}?{}", ORDERS_ENDPOINT, orderID, qb.q);
    return cli_.template Request<OrderResponse>(Req::GET, query);
  }

  std::expected<OrderResponse, APIError>
  ReplaceOrderByID(std::string_view orderID, const ReplaceOrderParam &r) {
    std::string json;
    auto order_request = glz::write_json(r);
    if (!order_request) {
      return std::unexpected(APIError{
          ErrorCode::JSONParsing, glz::format_error(order_request.error())});
    }

    const auto query = std::format("{}/{}", ORDERS_ENDPOINT, orderID);
    return cli_.template Request<OrderResponse>(
        Req::PATCH, query, order_request.value(), "application/json");
  }

  std::expected<std::monostate, APIError>
  DeleteOrderByID(std::string_view orderID) {
    const auto query = std::format("{}/{}", ORDERS_ENDPOINT, orderID);
    return cli_.template Request<std::monostate>(Req::DELETE, query);
  }

  std::expected<Portfolio, APIError>
  GetPortfolioHistory(const PortfolioParam &p) {
    utils::QueryBuilder qb;
    qb.add("period", p.period);
    qb.add("timeframe", p.timeframe);
    qb.add("intraday_reporting", ToString(p.intradayReporting));
    qb.add("start", p.start);
    qb.add("pnl_reset", ToString(p.pnlReset));
    qb.add("end", p.end);
    qb.add("extendedHours", p.extendedHours);
    qb.add("cashflowTypes", p.cashflowTypes);
    const auto query =
        std::format("{}/{}?{}", ACCOUNT_ENDPOINT, PORTFOLIO_ENDPOINT, qb.q);
    std::println("{}", query);
    return cli_.template Request<Portfolio>(Req::GET, query);
  }

private:
  const Env &env_;
  Http cli_;

  static constexpr const char *ACCOUNT_ENDPOINT = "/v2/account";
  static constexpr const char *ORDERS_ENDPOINT = "/v2/orders";
  static constexpr const char *POSITIONS_ENDPOINT = "/v2/positions";
  static constexpr const char *CLOCK_ENDPOINT = "/v2/clock";
  static constexpr const char *CALENDAR_ENDPOINT = "/v2/calendar";
  static constexpr const char *PORTFOLIO_ENDPOINT = "portfolio/history";
};

using TradingClient = TradingClientT<Environment, HttpClient>;

}; // namespace alpaca

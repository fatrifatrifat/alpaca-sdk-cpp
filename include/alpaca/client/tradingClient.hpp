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

  std::expected<CalendarResponse, APIError> GetMarketCalendarInfo() {
    const auto &query = CALENDAR_ENDPOINT;
    return cli_.template Request<CalendarResponse>(Req::GET, query);
  }

private:
  const Env &env_;
  Http cli_;

  static constexpr const char *ACCOUNT_ENDPOINT = "/v2/account";
  static constexpr const char *ORDERS_ENDPOINT = "/v2/orders";
  static constexpr const char *POSITIONS_ENDPOINT = "/v2/positions";
  static constexpr const char *CLOCK_ENDPOINT = "/v2/clock";
  static constexpr const char *CALENDAR_ENDPOINT = "/v2/calendar";
};

using TradingClient = TradingClientT<Environment, HttpClient>;

}; // namespace alpaca

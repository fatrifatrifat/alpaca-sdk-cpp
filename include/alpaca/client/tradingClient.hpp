#pragma once
#include <alpaca/client/environment.hpp>
#include <alpaca/client/httpClient.hpp>
#include <alpaca/models/accounts/serialize.hpp>
#include <alpaca/models/orders/serialize.hpp>
#include <alpaca/models/positions.hpp>
#include <alpaca/utils/utils.hpp>
#include <expected>
#include <print>

namespace alpaca {

class TradingClient {
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
  explicit TradingClient(const Environment &env)
      : env_(env), cli_(env_.GetBaseUrl()) {}

  std::expected<Account, std::string> GetAccount() {
    auto resp = cli_.Get(ACCOUNT_ENDPOINT, env_.GetAuthHeaders());
    if (!resp) {
      return std::unexpected(std::format("Error: {}", resp.error()));
    }

    if (!utils::IsSuccess(resp->status)) {
      return std::unexpected(
          std::format("HTTP {}: {}", resp->status, resp->body));
    }

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
    std::string json;
    auto order_request = glz::write_json(request);
    if (!order_request) {
      return std::unexpected(
          std::format("Error: {}", glz::format_error(order_request.error())));
    }
    std::println("Order request: {}", order_request.value());

    auto resp = cli_.Post(ORDERS_ENDPOINT, env_.GetAuthHeaders(),
                          order_request.value(), "application/json");

    if (!resp) {
      return std::unexpected(std::format("Error: {}", resp.error()));
    }

    if (!utils::IsSuccess(resp->status)) {
      return std::unexpected(
          std::format("HTTP {}: {}", resp->status, resp->body));
    }

    OrderResponse response;
    auto error = glz::read_json(response, resp->body);
    if (error) {
      return std::unexpected(
          std::format("Error: {}", glz::format_error(error, resp->body)));
    }
    std::println("Final Order req: {}", resp->body);

    return response;
  }

  std::expected<Positions, std::string> GetAllOpenPositions() {
    auto resp = cli_.Get(POSITIONS_ENDPOINT, env_.GetAuthHeaders());
    if (!resp) {
      return std::unexpected(std::format("Error: {}", resp.error()));
    }

    if (!utils::IsSuccess(resp->status)) {
      return std::unexpected(
          std::format("HTTP {}: {}", resp->status, resp->body));
    }

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
    auto resp = cli_.Get(std::format("{}/{}", POSITIONS_ENDPOINT, symbol),
                         env_.GetAuthHeaders());
    if (!resp) {
      return std::unexpected(std::format("Error: {}", resp.error()));
    }

    if (!utils::IsSuccess(resp->status)) {
      return std::unexpected(
          std::format("HTTP {}: {}", resp->status, resp->body));
    }

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
    const auto qty_query = BuildClosePositionQuery(cpp.amt);
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

    if (!utils::IsSuccess(resp->status)) {
      return std::unexpected(
          std::format("HTTP {}: {}", resp->status, resp->body));
    }

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

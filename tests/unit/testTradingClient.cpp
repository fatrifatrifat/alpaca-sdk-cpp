#include <alpaca/alpaca.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <expected>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace {

struct TestEnvironment {
  using Headers = std::vector<std::pair<std::string, std::string>>;

  std::string baseUrl = "http://unit.test";
  Headers headers = {{"APCA-API-KEY-ID", "k"}, {"APCA-API-SECRET-KEY", "s"}};

  std::string GetBaseUrl() const { return baseUrl; }
  Headers GetAuthHeaders() const { return headers; }
};

struct FakeHttpClient {
  struct Response {
    int status{};
    std::string body;
  };

  using Headers = TestEnvironment::Headers;

  struct Call {
    enum class Method { Get, Post, Delete } method;
    std::string path;
    Headers headers;
    std::string body;
    std::string contentType;
  };

  std::vector<Call> calls;

  std::function<std::expected<Response, std::string>(std::string_view,
                                                     const Headers &)>
      onGet;
  std::function<std::expected<Response, std::string>(
      std::string_view, const Headers &, std::string_view, std::string_view)>
      onPost;
  std::function<std::expected<Response, std::string>(std::string_view,
                                                     const Headers &)>
      onDelete;

  explicit FakeHttpClient(std::string = {}) {}

  std::expected<Response, std::string> Get(std::string_view path,
                                           const Headers &h) {
    calls.push_back({Call::Method::Get, std::string(path), h, {}, {}});
    if (onGet)
      return onGet(path, h);
    return Response{200, "{}"};
  }

  std::expected<Response, std::string> Post(std::string_view path,
                                            const Headers &h,
                                            std::string_view body,
                                            std::string_view contentType) {
    calls.push_back({Call::Method::Post, std::string(path), h,
                     std::string(body), std::string(contentType)});
    if (onPost)
      return onPost(path, h, body, contentType);
    return Response{200, "{}"};
  }

  std::expected<Response, std::string> Delete(std::string_view path,
                                              const Headers &h) {
    calls.push_back({Call::Method::Delete, std::string(path), h, {}, {}});
    if (onDelete)
      return onDelete(path, h);
    return Response{200, "{}"};
  }
};

std::string account_json_minimal() {
  return R"json(
  {
    "id": "acct_123",
    "admin_configurations": {},
    "account_number": "ABC123",
    "status": "ACTIVE",
    "crypto_status": "ACTIVE",
    "options_approved_level": 1,
    "options_trading_level": 3,
    "currency": "USD"
  }
  )json";
}

std::string order_response_json_minimal() {
  return R"json(
  {
    "id": "order_1",
    "client_order_id": "client_1",
    "created_at": "2026-01-07T10:00:00Z",
    "updated_at": "2026-01-07T10:00:01Z",
    "submitted_at": "2026-01-07T10:00:02Z",
    "filled_qty": "0",
    "order_type": "market",
    "type": "market",
    "side": "buy",
    "time_in_force": "day",
    "status": "new",
    "extended_hours": false
  }
  )json";
}

std::string positions_json_one() {
  return R"json(
  [
    {
      "asset_id": "asset_1",
      "symbol": "AAPL",
      "exchange": "NASDAQ",
      "asset_class": "us_equity",
      "asset_marginable": true,
      "qty": "10",
      "avg_entry_price": "150.00",
      "side": "long",
      "market_value": "1550.00",
      "cost_basis": "1500.00",
      "unrealized_pl": "50.00",
      "unrealized_plpc": "0.0333",
      "unrealized_intraday_pl": "10.00",
      "unrealized_intraday_plpc": "0.0066",
      "current_price": "155.00",
      "lastday_price": "154.00",
      "change_today": "0.0065",
      "qty_available": "10"
    }
  ]
  )json";
}

} // namespace

TEST_CASE("TradingClient.GetAccount: success parses Account via glaze") {
  TestEnvironment env{};
  FakeHttpClient http{env.GetBaseUrl()};

  http.onGet = [&](std::string_view path, const TestEnvironment::Headers &) {
    REQUIRE(path == "/v2/account");
    return FakeHttpClient::Response{200, account_json_minimal()};
  };

  alpaca::TradingClientT<TestEnvironment, FakeHttpClient> cli(env,
                                                              std::move(http));

  auto acc = cli.GetAccount();
  REQUIRE(acc.has_value());
  REQUIRE(acc->id == "acct_123");
  REQUIRE(acc->account_number == "ABC123");
  REQUIRE(acc->status == alpaca::AccountStatus::ACTIVE);
}

TEST_CASE("TradingClient.GetAccount: non-2xx returns HTTP error") {
  TestEnvironment env{};
  FakeHttpClient http{env.GetBaseUrl()};

  http.onGet = [&](std::string_view, const TestEnvironment::Headers &) {
    return FakeHttpClient::Response{401, "nope"};
  };

  alpaca::TradingClientT<TestEnvironment, FakeHttpClient> cli(env,
                                                              std::move(http));

  auto acc = cli.GetAccount();
  REQUIRE_FALSE(acc.has_value());
  REQUIRE_THAT(acc.error(), Catch::Matchers::ContainsSubstring("HTTP 401"));
}

TEST_CASE("TradingClient.SubmitOrder: sends wire JSON (qty/notional), not "
          "internal fields") {
  TestEnvironment env{};
  FakeHttpClient http{env.GetBaseUrl()};

  http.onPost = [&](std::string_view path, const TestEnvironment::Headers &,
                    std::string_view body, std::string_view contentType) {
    REQUIRE(path == "/v2/orders");
    REQUIRE(contentType == "application/json");

    REQUIRE(std::string(body).find("\"qty\"") != std::string::npos);
    REQUIRE(std::string(body).find("\"amt\"") == std::string::npos);
    REQUIRE(std::string(body).find("\"trail_price\"") == std::string::npos);

    return FakeHttpClient::Response{200, order_response_json_minimal()};
  };

  alpaca::TradingClientT<TestEnvironment, FakeHttpClient> cli(env,
                                                              std::move(http));

  alpaca::OrderRequest req{};
  req.symbol = "AAPL";
  req.amt = alpaca::Quantity{10};
  req.side = alpaca::OrderSide::buy;
  req.type = alpaca::OrderType::market;
  req.timeInForce = alpaca::OrderTimeInForce::day;

  auto res = cli.SubmitOrder(req);
  REQUIRE(res.has_value());
  REQUIRE(res->id == "order_1");
  REQUIRE(res->clientOrderID == "client_1");
}

TEST_CASE(
    "TradingClient.GetAllOpenPositions: success parses Positions via glaze") {
  TestEnvironment env{};
  FakeHttpClient http{env.GetBaseUrl()};

  http.onGet = [&](std::string_view path, const TestEnvironment::Headers &) {
    REQUIRE(path == "/v2/positions");
    return FakeHttpClient::Response{200, positions_json_one()};
  };

  alpaca::TradingClientT<TestEnvironment, FakeHttpClient> cli(env,
                                                              std::move(http));

  auto ps = cli.GetAllOpenPositions();
  REQUIRE(ps.has_value());
  REQUIRE(ps->size() == 1);
  REQUIRE((*ps)[0].symbol == "AAPL");
  REQUIRE((*ps)[0].exchange == alpaca::PositionExchange::NASDAQ);
}

TEST_CASE("TradingClient.GetOpenPosition: hits /v2/positions/{symbol} and "
          "parses Position") {
  TestEnvironment env{};
  FakeHttpClient http{env.GetBaseUrl()};

  http.onGet = [&](std::string_view path, const TestEnvironment::Headers &) {
    REQUIRE(path == "/v2/positions/AAPL");
    std::string one = positions_json_one();
    auto l = one.find('{');
    auto r = one.rfind('}');
    return FakeHttpClient::Response{200, one.substr(l, r - l + 1)};
  };

  alpaca::TradingClientT<TestEnvironment, FakeHttpClient> cli(env,
                                                              std::move(http));

  auto p = cli.GetOpenPosition("AAPL");
  REQUIRE(p.has_value());
  REQUIRE(p->symbol == "AAPL");
}

TEST_CASE(
    "TradingClient.ClosePosition: builds qty/percentage query correctly") {
  TestEnvironment env{};
  FakeHttpClient http{env.GetBaseUrl()};

  http.onDelete = [&](std::string_view path, const TestEnvironment::Headers &) {
    return FakeHttpClient::Response{200, order_response_json_minimal()};
  };

  alpaca::TradingClientT<TestEnvironment, FakeHttpClient> cli(env,
                                                              std::move(http));

  SECTION("Shares -> qty=... with 9 decimals") {
    TestEnvironment env2{};
    FakeHttpClient http2{env2.GetBaseUrl()};

    http2.onDelete = [&](std::string_view path,
                         const TestEnvironment::Headers &) {
      REQUIRE(std::string(path).find("/v2/positions/AAPL?qty=1.500000000") !=
              std::string::npos);
      return FakeHttpClient::Response{200, order_response_json_minimal()};
    };

    alpaca::TradingClientT<TestEnvironment, FakeHttpClient> cli2(
        env2, std::move(http2));

    alpaca::ClosePositionParams p{};
    p.symbol_or_asset_id = "AAPL";
    p.amt = alpaca::LiquidationAmount{alpaca::Shares{1.5L}};

    auto res = cli2.ClosePosition(p);
    REQUIRE(res.has_value());
  }

  SECTION("Percent -> percentage=... with 9 decimals") {
    TestEnvironment env2{};
    FakeHttpClient http2{env2.GetBaseUrl()};

    http2.onDelete = [&](std::string_view path,
                         const TestEnvironment::Headers &) {
      REQUIRE(std::string(path).find(
                  "/v2/positions/AAPL?percentage=25.000000000") !=
              std::string::npos);
      return FakeHttpClient::Response{200, order_response_json_minimal()};
    };

    alpaca::TradingClientT<TestEnvironment, FakeHttpClient> cli2(
        env2, std::move(http2));

    alpaca::ClosePositionParams p{};
    p.symbol_or_asset_id = "AAPL";
    p.amt = alpaca::LiquidationAmount{alpaca::Percent{25.0L}};

    auto res = cli2.ClosePosition(p);
    REQUIRE(res.has_value());
  }

  SECTION("Invalid percent -> returns expected error before HTTP call") {
    alpaca::ClosePositionParams p{};
    p.symbol_or_asset_id = "AAPL";
    p.amt = alpaca::LiquidationAmount{alpaca::Percent{200.0L}};

    auto res = cli.ClosePosition(p);
    REQUIRE_FALSE(res.has_value());
    REQUIRE_THAT(res.error(), Catch::Matchers::ContainsSubstring(
                                  "Unvalid liquidation amount query"));
  }
}

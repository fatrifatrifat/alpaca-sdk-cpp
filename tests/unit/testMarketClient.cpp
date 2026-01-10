#include <alpaca/alpaca.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <expected>
#include <print>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

struct TestEnvironment {
  using Headers = std::vector<std::pair<std::string, std::string>>;

  std::string dataUrl = "http://unit.test.data";
  Headers headers = {{"APCA-API-KEY-ID", "k"}, {"APCA-API-SECRET-KEY", "s"}};

  std::string GetDataUrl() const { return dataUrl; }
  Headers GetAuthHeaders() const { return headers; }
};

struct FakeHttpClient {
  struct Response {
    int status{};
    std::string body;
  };

  using Headers = TestEnvironment::Headers;

  struct Call {
    std::string path;
    Headers headers;
  };

  Headers headers;
  std::vector<Call> calls;

  std::unordered_map<std::string, std::expected<Response, std::string>>
      getRoutes;

  explicit FakeHttpClient(std::string url, Headers hdrs) : headers(hdrs) {}

  std::expected<Response, std::string> Get(std::string_view path,
                                           const Headers &h) {
    calls.push_back({std::string(path), h});
    auto it = getRoutes.find(std::string(path));
    if (it == getRoutes.end()) {
      return std::unexpected(std::string("no fake route for: ") +
                             std::string(path));
    }
    return it->second;
  }

  template <class T>
  std::expected<T, alpaca::APIError>
  Request(alpaca::Req type, const std::string &path,
          std::optional<std::string> body = std::nullopt,
          std::optional<std::string> content_type = std::nullopt) {
    (void)body;
    (void)content_type;

    if (type != alpaca::Req::GET) {
      return std::unexpected(alpaca::APIError{
          alpaca::ErrorCode::Unknown,
          "FakeHttpClient only supports GET in this test harness"});
    }

    auto raw = Get(path, headers);
    if (!raw) {
      return std::unexpected(
          alpaca::APIError{alpaca::ErrorCode::Transport, raw->body});
    }

    if (!alpaca::utils::IsSuccess(raw->status)) {
      return std::unexpected(alpaca::APIError{alpaca::ErrorCode::HTTPCode,
                                              raw->body, raw->status});
    }

    T obj{};
    auto err = glz::read_json(obj, raw->body);
    if (err) {
      return std::unexpected(alpaca::APIError{
          alpaca::ErrorCode::JSONParsing, glz::format_error(err, raw->body)});
    }

    return obj;
  }
};

std::string bars_page_1_json() {
  return R"json(
  {
    "bars": {
      "AAPL": [
        { "c": 1.0, "h": 1.1, "l": 0.9, "n": 10, "o": 1.0, "t": "2024-01-03T00:00:00Z", "v": 100, "vw": 1.01 }
      ]
    },
    "next_page_token": "TOKEN_1"
  }
  )json";
}

std::string bars_page_2_json() {
  return R"json(
  {
    "bars": {
      "AAPL": [
        { "c": 2.0, "h": 2.1, "l": 1.9, "n": 20, "o": 2.0, "t": "2024-01-03T00:05:00Z", "v": 200, "vw": 2.01 }
      ],
      "MSFT": [
        { "c": 3.0, "h": 3.1, "l": 2.9, "n": 30, "o": 3.0, "t": "2024-01-03T00:00:00Z", "v": 300, "vw": 3.01 }
      ]
    },
    "next_page_token": ""
  }
  )json";
}

std::string latest_bars_json() {
  return R"json(
  {
    "bars": {
      "AAPL": { "c": 10.0, "h": 10.1, "l": 9.9, "n": 1, "o": 10.0, "t": "2024-01-03T00:00:00Z", "v": 1, "vw": 10.0 },
      "MSFT": { "c": 20.0, "h": 20.1, "l": 19.9, "n": 2, "o": 20.0, "t": "2024-01-03T00:00:00Z", "v": 2, "vw": 20.0 }
    }
  }
  )json";
}

} // namespace

TEST_CASE("MarketDataClient.GetBars: builds query and merges paginated pages") {
  TestEnvironment env{};
  FakeHttpClient http{env.GetDataUrl(), env.GetAuthHeaders()};

  const std::string page1 = "/v2/stocks/bars?"
                            "symbols=AAPL%2CMSFT&"
                            "timeframe=1D&"
                            "start=2024-01-03T00:00:00Z&"
                            "end=2024-01-04T00:00:00Z";

  const std::string page2 = page1 + "&page_token=TOKEN_1";

  http.getRoutes[page1] = FakeHttpClient::Response{200, bars_page_1_json()};
  http.getRoutes[page2] = FakeHttpClient::Response{200, bars_page_2_json()};

  alpaca::MarketDataClientT<TestEnvironment, FakeHttpClient> cli(
      env, std::move(http));

  alpaca::BarParams p{};
  p.symbols = {"AAPL", "MSFT"};
  p.timeframe = "1D";
  p.start = "2024-01-03T00:00:00Z";
  p.end = "2024-01-04T00:00:00Z";

  auto res = cli.GetBars(p);
  REQUIRE(res.has_value());

  REQUIRE(res->bars.count("AAPL") == 1);
  REQUIRE(res->bars.count("MSFT") == 1);
  REQUIRE(res->bars.at("AAPL").size() == 2);
  REQUIRE(res->bars.at("MSFT").size() == 1);

  REQUIRE(res->bars.at("AAPL")[0].close == Catch::Approx(1.0));
  REQUIRE(res->bars.at("AAPL")[1].close == Catch::Approx(2.0));
  REQUIRE(res->bars.at("MSFT")[0].close == Catch::Approx(3.0));
}

TEST_CASE("MarketDataClient.GetBars: repeated next_page_token triggers "
          "pagination guard") {
  TestEnvironment env{};
  FakeHttpClient http{env.GetDataUrl(), env.GetAuthHeaders()};

  const std::string page1 = "/v2/stocks/bars?"
                            "symbols=AAPL&"
                            "timeframe=1D&"
                            "start=2024-01-03T00:00:00Z&"
                            "end=2024-01-04T00:00:00Z";

  const std::string page2 = page1 + "&page_token=TOKEN_1";

  http.getRoutes[page1] = FakeHttpClient::Response{
      200, R"json({"bars":{"AAPL":[]},"next_page_token":"TOKEN_1"})json"};
  http.getRoutes[page2] = FakeHttpClient::Response{
      200, R"json({"bars":{"AAPL":[]},"next_page_token":"TOKEN_1"})json"};

  alpaca::MarketDataClientT<TestEnvironment, FakeHttpClient> cli(
      env, std::move(http));

  alpaca::BarParams p{};
  p.symbols = {"AAPL"};
  p.timeframe = "1D";
  p.start = "2024-01-03T00:00:00Z";
  p.end = "2024-01-04T00:00:00Z";

  auto res = cli.GetBars(p);
  REQUIRE_FALSE(res.has_value());
  REQUIRE(res.error().message == "Pagination error: next_page_token repeated");
}

TEST_CASE("MarketDataClient.GetBars: input validation errors") {
  TestEnvironment env{};
  FakeHttpClient http{env.GetDataUrl(), env.GetAuthHeaders()};
  alpaca::MarketDataClientT<TestEnvironment, FakeHttpClient> cli(
      env, std::move(http));

  SECTION("empty symbols") {
    alpaca::BarParams p{};
    p.symbols = {};
    auto res = cli.GetBars(p);
    REQUIRE_FALSE(res.has_value());
    REQUIRE(res.error().message == "Empty symbol");
  }

  SECTION("empty timeframe") {
    alpaca::BarParams p{};
    p.symbols = {"AAPL"};
    p.timeframe = "";
    auto res = cli.GetBars(p);
    REQUIRE_FALSE(res.has_value());
    REQUIRE(res.error().message == "Empty timeframe");
  }

  SECTION("limit <= 0") {
    alpaca::BarParams p{};
    p.symbols = {"AAPL"};
    p.limit = 0;
    auto res = cli.GetBars(p);
    REQUIRE_FALSE(res.has_value());
    REQUIRE(res.error().message == "Empty limit");
  }
}

TEST_CASE("MarketDataClient.GetBars: HTTP non-2xx returns HTTP error") {
  TestEnvironment env{};
  FakeHttpClient http{env.GetDataUrl(), env.GetAuthHeaders()};

  const std::string page1 = "/v2/stocks/bars?"
                            "symbols=AAPL&"
                            "timeframe=1D&"
                            "start=2024-01-03T00:00:00Z&"
                            "end=2024-01-04T00:00:00Z";

  http.getRoutes[page1] = FakeHttpClient::Response{429, "rate_limited"};

  alpaca::MarketDataClientT<TestEnvironment, FakeHttpClient> cli(
      env, std::move(http));

  alpaca::BarParams p{};
  p.symbols = {"AAPL"};
  p.timeframe = "1D";
  p.start = "2024-01-03T00:00:00Z";
  p.end = "2024-01-04T00:00:00Z";

  auto res = cli.GetBars(p);
  REQUIRE_FALSE(res.has_value());
  REQUIRE(res.error().code == alpaca::ErrorCode::HTTPCode);
  REQUIRE(res.error().status.value() == 429);
  REQUIRE_THAT(res.error().message,
               Catch::Matchers::ContainsSubstring("rate_limited"));
}

TEST_CASE(
    "MarketDataClient.GetBars: glaze parse error returns formatted error") {
  TestEnvironment env{};
  FakeHttpClient http{env.GetDataUrl(), env.GetAuthHeaders()};

  const std::string page1 = "/v2/stocks/bars?"
                            "symbols=AAPL&"
                            "timeframe=1D&"
                            "start=2024-01-03T00:00:00Z&"
                            "end=2024-01-04T00:00:00Z";

  http.getRoutes[page1] =
      FakeHttpClient::Response{200, R"json({ "bars": "not a map" })json"};

  alpaca::MarketDataClientT<TestEnvironment, FakeHttpClient> cli(
      env, std::move(http));

  alpaca::BarParams p{};
  p.symbols = {"AAPL"};
  p.timeframe = "1D";
  p.start = "2024-01-03T00:00:00Z";
  p.end = "2024-01-04T00:00:00Z";

  auto res = cli.GetBars(p);
  REQUIRE_FALSE(res.has_value());
  REQUIRE(res.error().code == alpaca::ErrorCode::JSONParsing);
}

TEST_CASE("MarketDataClient.GetLatestBar: success parses LatestBars and query "
          "contains feed when provided") {
  TestEnvironment env{};
  FakeHttpClient http{env.GetDataUrl(), env.GetAuthHeaders()};

  const std::string path = "/v2/stocks/bars/latest?"
                           "symbols=AAPL%2CMSFT&"
                           "feed=sip";

  http.getRoutes[path] = FakeHttpClient::Response{200, latest_bars_json()};

  alpaca::MarketDataClientT<TestEnvironment, FakeHttpClient> cli(
      env, std::move(http));

  alpaca::LatestBarParam p{};
  p.symbols = {"AAPL", "MSFT"};
  p.feed = alpaca::BarFeed::SIP;

  auto res = cli.GetLatestBar(p);
  REQUIRE(res.has_value());

  REQUIRE(res->bars.count("AAPL") == 1);
  REQUIRE(res->bars.count("MSFT") == 1);
  REQUIRE(res->bars.at("AAPL").close == Catch::Approx(10.0));
  REQUIRE(res->bars.at("MSFT").close == Catch::Approx(20.0));
}

TEST_CASE("MarketDataClient.GetLatestBar: empty symbols returns error") {
  TestEnvironment env{};
  FakeHttpClient http{env.GetDataUrl(), env.GetAuthHeaders()};
  alpaca::MarketDataClientT<TestEnvironment, FakeHttpClient> cli(
      env, std::move(http));

  alpaca::LatestBarParam p{};
  p.symbols = {};

  auto res = cli.GetLatestBar(p);
  REQUIRE_FALSE(res.has_value());
  REQUIRE(res.error().message == "Empty symbol");
}

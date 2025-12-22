#pragma once
#include "environment.hpp"
#include "httpClient.hpp"
#include <glaze/glaze.hpp>
#include <print>
#include <string>

namespace alpaca {

struct Bar {
  size_t c{}, h{}, l{}, n{}, o{};
  std::string t;
  size_t v{}, vw{};
};

class MarketDataClient {
public:
  explicit MarketDataClient(Environment &env)
      : env_(env), cli_(env_.GetDataUrl()) {}

  std::vector<Bar> GetBars(const std::string &symbol,
                           const std::string &timeframe = "1D",
                           const std::string &start = "2024-01-03T00:00:00Z",
                           const std::string &end = "2024-01-04T00:00:00Z",
                           const std::string &limit = "1000",
                           const std::string &feed = "sip") {
    std::string query;
    if (symbol.empty()) {
      std::println("Error: Empty symbol");
      return {};
    }
    query += "symbols=" + symbol + "&";

    if (timeframe.empty()) {
      std::println("Error: Empty timeframe");
      return {};
    }
    query += "timeframe=" + timeframe;

    if (start.empty()) {
      std::println("Error: Empty start");
      return {};
    }
    query += "&start=" + start;

    if (end.empty()) {
      std::println("Error: Empty end");
      return {};
    }
    query += "&end=" + end;

    if (limit.empty()) {
      std::println("Error: Empty limit");
      return {};
    }
    query += "&limit=" + limit;

    if (feed.empty()) {
      std::println("Error: Empty feed");
      return {};
    }
    query += "&feed=" + feed;

    std::println("{}", query);
    auto resp = cli_.Get(BARS_ENDPOINT + query, env_.GetAuthHeaders());
    if (!resp) {
      std::println("{}", resp.error());
      return {};
    }

    if (resp->status != 200) {
      std::println("Error Code: {}", resp->status);
      return {};
    }

    std::println("{}", resp->body);
    return {};
  }

private:
  Environment &env_;
  HttpClient cli_;

  static constexpr const char *BARS_ENDPOINT = "/v2/stocks/bars?";
};

}; // namespace alpaca

namespace glz {
template <> struct meta<alpaca::Bar> {
  using T = alpaca::Bar;
  static constexpr auto value =
      object("c", &T::c, "h", &T::h, "l", &T::l, "n", &T::n, "o", &T::o, "t",
             &T::t, "v", &T::v, "vw", &T::vw);
};
}; // namespace glz

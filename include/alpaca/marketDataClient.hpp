#pragma once
#include "environment.hpp"
#include "httpClient.hpp"
#include <glaze/glaze.hpp>
#include <print>
#include <string>

namespace alpaca {

class MarketDataClient {
public:
  struct Bar {
    double c{}, h{}, l{};
    long long n;
    double o{};
    std::string t;
    long long v{};
    double vw;
  };

  struct BarParams {
    std::string symbol;
    std::string timeframe = "1D";
    std::string start = "2024-01-03T00:00:00Z";
    std::string end = "2024-01-04T00:00:00Z";
    int limit = 1000;
    std::string feed = "sip";
  };

  struct Bars {
    std::map<std::string, std::vector<Bar>> bars;
  };

private:
  static std::string build_query(
      std::initializer_list<std::pair<std::string_view, std::string>> kvs) {
    std::string q;
    bool first = true;

    for (auto &[k, v] : kvs) {
      if (v.empty())
        continue;
      if (!first)
        q += "&";
      first = false;
      q += k;
      q += "=";
      q += v;
    }
    return q;
  }

public:
  explicit MarketDataClient(Environment &env)
      : env_(env), cli_(env_.GetDataUrl()) {}

  Bars GetBars(const BarParams &p) {
    if (p.symbol.empty()) {
      std::println("Error: Empty symbol");
      return {};
    }
    if (p.timeframe.empty()) {
      std::println("Error: Empty timeframe");
      return {};
    }
    if (p.start.empty() || p.end.empty()) {
      std::println("Error: Empty start/end");
      return {};
    }
    if (p.limit <= 0) {
      std::println("Error: limit must be > 0");
      return {};
    }
    if (p.feed.empty()) {
      std::println("Error: Empty feed");
      return {};
    }

    const std::string query = build_query({
        {"symbols", p.symbol},
        {"timeframe", p.timeframe},
        {"start", p.start},
        {"end", p.end},
        {"limit", std::to_string(p.limit)},
        {"feed", p.feed},
    });

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
    Bars bars;
    auto error = glz::read_json(bars, resp->body);
    if (error) {
      std::println("Error JsonParsing: {}",
                   glz::format_error(error, resp->body));
      return {};
    }
    return bars;
  }

private:
  Environment &env_;
  HttpClient cli_;

  static constexpr const char *BARS_ENDPOINT = "/v2/stocks/bars?";
};

}; // namespace alpaca

namespace glz {
template <> struct meta<alpaca::MarketDataClient::Bar> {
  using T = alpaca::MarketDataClient::Bar;
  static constexpr auto value =
      object("c", &T::c, "h", &T::h, "l", &T::l, "n", &T::n, "o", &T::o, "t",
             &T::t, "v", &T::v, "n", &T::n);
};

template <> struct meta<alpaca::MarketDataClient::Bars> {
  using T = alpaca::MarketDataClient::Bars;
  static constexpr auto value = object("bars", &T::bars);
};
}; // namespace glz

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
    std::vector<std::string> symbols;
    std::string timeframe = "1D";
    std::string start = "2024-01-03T00:00:00Z";
    std::string end = "2024-01-04T00:00:00Z";
    int limit = 1000;
    std::string feed = "iex";
  };

  struct LatestBarParam {
    std::vector<std::string> symbols;
    std::string feed = "iex";
  };

  struct Bars {
    std::map<std::string, std::vector<Bar>> bars;
    std::optional<std::string> next_page_token;
  };

  struct LatestBars {
    std::map<std::string, Bar> bars;
  };

private:
  static std::string symbols_encode(const std::vector<std::string>& v) {
    std::string symbols;
    for(const auto& s : v) {
      symbols += s;
      symbols += "%2C";
    }
    symbols.erase(symbols.length() - 3, 3);
    return symbols;
  }

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

  std::expected<Bars, std::string> GetBars(const BarParams &p) {
    if (p.symbols.empty()) {
      return std::unexpected("Error: Empty symbol");
    }
    if (p.timeframe.empty()) {
      return std::unexpected("Error: Empty timeframe");
    }
    if (p.start.empty() || p.end.empty()) {
      return std::unexpected("Error: Empty start/end");
    }
    if (p.limit <= 0) {
      return std::unexpected("Error: Empty limit");
    }
    if (p.feed.empty()) {
      return std::unexpected("Error: Empty feed");
    }

    const std::string query = build_query({
        {"symbols", symbols_encode(p.symbols)},
        {"timeframe", p.timeframe},
        {"start", p.start},
        {"end", p.end},
        {"limit", std::to_string(p.limit)},
        {"feed", p.feed},
    });

    std::println("{}", query);
    auto resp = cli_.Get(BARS_ENDPOINT + query, env_.GetAuthHeaders());
    if (!resp) {
      return std::unexpected(std::format("Error: {}", resp.error()));
    }

    if (resp->status != 200) {
      return std::unexpected(std::format("Error Code: {}", resp->status));
    }

    std::println("{}", resp->body);
    Bars bars;
    auto error = glz::read_json(bars, resp->body);
    if (error) {
      return std::unexpected(
          std::format("Error Code: {}", glz::format_error(error, resp->body)));
    }

    return bars;
  }

  std::expected<LatestBars, std::string> GetLatestBar(const LatestBarParam &p) {
    if (p.symbols.empty()) {
      return std::unexpected("Error: Empty symbol");
    }
    if (p.feed.empty()) {
      return std::unexpected("Error: Empty feed");
    }

    const std::string query = build_query({
        {"symbols", symbols_encode(p.symbols)},
        {"feed", p.feed},
    });

    std::println("{}", query);
    auto resp = cli_.Get(LATEST_BARS_ENDPOINT + query, env_.GetAuthHeaders());
    if (!resp) {
      return std::unexpected(std::format("Error: {}", resp.error()));
    }

    if (resp->status != 200) {
      return std::unexpected(std::format("Error Code: {}", resp->status));
    }

    std::println("{}", resp->body);
    LatestBars bars;
    auto error = glz::read_json(bars, resp->body);
    if (error) {
      return std::unexpected(
          std::format("Error Code: {}", glz::format_error(error, resp->body)));
    }

    return {};
  }

private:
  Environment &env_;
  HttpClient cli_;

  static constexpr const char *BARS_ENDPOINT = "/v2/stocks/bars?";
  static constexpr const char *LATEST_BARS_ENDPOINT = "/v2/stocks/bars/latest?";
};

}; // namespace alpaca

namespace glz {

template <> struct meta<alpaca::MarketDataClient::Bars> {
  using T = alpaca::MarketDataClient::Bars;
  static constexpr auto value =
      object("bars", &T::bars, "next_page_token", &T::next_page_token);
};

template <> struct meta<alpaca::MarketDataClient::Bar> {
  using T = alpaca::MarketDataClient::Bar;
  static constexpr auto value =
      object("c", &T::c, "h", &T::h, "l", &T::l, "n", &T::n, "o", &T::o, "t",
             &T::t, "v", &T::v, "vw", &T::vw);
};

template <> struct meta<alpaca::MarketDataClient::LatestBars> {
  using T = alpaca::MarketDataClient::LatestBars;
  static constexpr auto value = object("bars", &T::bars);
};

}; // namespace glz

#pragma once
#include <alpaca/environment.hpp>
#include <alpaca/httpClient.hpp>
#include <alpaca/utils/utils.hpp>
#include <glaze/glaze.hpp>
#include <string>

namespace alpaca {

class MarketDataClient {
private:
  static std::string SymbolsEncode(const std::vector<std::string> &v) {
    std::string symbols;
    for (const auto &s : v) {
      symbols += s;
      symbols += "%2C";
    }
    symbols.erase(symbols.length() - 3, 3);
    return symbols;
  }

  static std::string BuildQuery(
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

  std::expected<Bars, std::string> GetBarsPimpl(const BarParams &p) {
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

    const std::string query = BuildQuery({
        {"symbols", SymbolsEncode(p.symbols)},
        {"timeframe", p.timeframe},
        {"start", p.start},
        {"end", p.end},
        {"limit", std::to_string(p.limit)},
        {"feed", p.feed},
        {"page_token", p.page_token.value_or("")},
    });

    auto resp = cli_.Get(BARS_ENDPOINT + query, env_.GetAuthHeaders());
    if (!resp) {
      return std::unexpected(std::format("Error: {}", resp.error()));
    }

    if (!utils::IsSuccess(resp->status)) {
      return std::unexpected(std::format("Error Code: {}", resp->status));
    }

    Bars bars;
    auto error = glz::read_json(bars, resp->body);
    if (error) {
      return std::unexpected(
          std::format("Error Code: {}", glz::format_error(error, resp->body)));
    }

    return bars;
  }

public:
  explicit MarketDataClient(const Environment &env)
      : env_(env), cli_(env_.GetDataUrl()) {}

  std::expected<Bars, std::string> GetBars(const BarParams &p) {
    std::map<std::string, std::vector<Bar>> barsBySymbol;
    std::optional<std::string> page;

    std::unordered_set<std::string> seen;
    auto params = p;

    while (true) {
      params.page_token = page;
      auto resp = GetBarsPimpl(params);
      if (!resp) {
        return std::unexpected(resp.error());
      }

      for (auto &[sym, bars] : resp->bars) {
        auto &dst = barsBySymbol[sym];
        dst.insert(dst.end(), bars.begin(), bars.end());
      }

      if (!resp->next_page_token || resp->next_page_token->empty()) {
        break;
      }

      if (!seen.insert(*resp->next_page_token).second) {
        return std::unexpected("Pagination error: next_page_token repeated");
      }

      page = resp->next_page_token;
    }

    return Bars{barsBySymbol};
  }

  std::expected<LatestBars, std::string> GetLatestBar(const LatestBarParam &p) {
    if (p.symbols.empty()) {
      return std::unexpected("Error: Empty symbol");
    }
    if (p.feed.empty()) {
      return std::unexpected("Error: Empty feed");
    }

    const std::string query = BuildQuery({
        {"symbols", SymbolsEncode(p.symbols)},
        {"feed", p.feed},
    });

    auto resp = cli_.Get(LATEST_BARS_ENDPOINT + query, env_.GetAuthHeaders());
    if (!resp) {
      return std::unexpected(std::format("Error: {}", resp.error()));
    }

    if (!utils::IsSuccess(resp->status)) {
      return std::unexpected(std::format("Error Code: {}", resp->status));
    }

    LatestBars bars;
    auto error = glz::read_json(bars, resp->body);
    if (error) {
      return std::unexpected(
          std::format("Error Code: {}", glz::format_error(error, resp->body)));
    }

    return bars;
  }

private:
  const Environment &env_;
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
      object("c", &T::close, "h", &T::high, "l", &T::low, "n",
             &T::number_of_trades, "o", &T::open, "t", &T::timestamp, "v",
             &T::volume, "vw", &T::volume_weigted_price);
};

template <> struct meta<alpaca::MarketDataClient::LatestBars> {
  using T = alpaca::MarketDataClient::LatestBars;
  static constexpr auto value = object("bars", &T::bars);
};

}; // namespace glz

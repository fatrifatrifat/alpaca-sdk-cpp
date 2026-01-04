#pragma once
#include <alpaca/client/environment.hpp>
#include <alpaca/client/httpClient.hpp>
#include <alpaca/models/bars.hpp>
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
      return std::unexpected("Empty symbol");
    }
    if (p.timeframe.empty()) {
      return std::unexpected("Empty timeframe");
    }
    if (p.start.empty() || p.end.empty()) {
      return std::unexpected("Empty start/end");
    }
    if (p.limit.has_value() && p.limit.value() <= 0) {
      return std::unexpected("Empty limit");
    }

    std::string feed;
    switch (p.feed) {
    case alpaca::BarFeed::SIP:
      feed = "sip";
      break;
    case alpaca::BarFeed::IEX:
      feed = "iex";
      break;
    case alpaca::BarFeed::Boats:
      feed = "boats";
      break;
    case alpaca::BarFeed::OTC:
      feed = "otc";
      break;
    default:
      return std::unexpected("Empty/Invalid feed");
    }

    std::optional<std::string> adjustment;
    if (p.adjustment.has_value()) {
      switch (p.adjustment.value()) {
      case alpaca::BarAdjustment::Raw:
        adjustment = std::make_optional("raw");
        break;
      case alpaca::BarAdjustment::Split:
        adjustment = std::make_optional("split");
        break;
      case alpaca::BarAdjustment::Dividend:
        adjustment = std::make_optional("divided");
        break;
      case alpaca::BarAdjustment::Spinoff:
        adjustment = std::make_optional("spin-off");
        break;
      case alpaca::BarAdjustment::All:
        adjustment = std::make_optional("all");
        break;
      }
    }

    std::optional<std::string> sort;
    if (p.sort.has_value()) {
      switch (p.sort.value()) {
      case alpaca::BarSort::Asc:
        sort = std::make_optional("asc");
        break;
      case alpaca::BarSort::Desc:
        sort = std::make_optional("desc");
        break;
      }
    }

    const std::string query = BuildQuery({
        {"symbols", SymbolsEncode(p.symbols)},
        {"timeframe", p.timeframe},
        {"start", p.start},
        {"end", p.end},
        {"limit", std::to_string(p.limit.value_or(1000))},
        {"adjustment", adjustment.value_or("")},
        {"asof", p.asof.value_or("")},
        {"feed", feed},
        {"currency", p.currency.value_or("")},
        {"page_token", p.page_token.value_or("")},
        {"sort", sort.value_or("")},
    });

    auto resp = cli_.Get(BARS_ENDPOINT + query, env_.GetAuthHeaders());
    if (!resp) {
      return std::unexpected(std::format("Error: {}", resp.error()));
    }

    if (!utils::IsSuccess(resp->status)) {
      return std::unexpected(
          std::format("HTTP {}: {}", resp->status, resp->body));
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

    std::string feed;
    switch (p.feed) {
    case alpaca::BarFeed::SIP:
      feed = "sip";
      break;
    case alpaca::BarFeed::IEX:
      feed = "iex";
      break;
    case alpaca::BarFeed::Boats:
      feed = "boats";
      break;
    case alpaca::BarFeed::OTC:
      feed = "otc";
      break;
    default:
      return std::unexpected("Empty/Invalid feed");
    }

    const std::string query = BuildQuery({
        {"symbols", SymbolsEncode(p.symbols)},
        {"feed", feed},
    });

    auto resp = cli_.Get(LATEST_BARS_ENDPOINT + query, env_.GetAuthHeaders());
    if (!resp) {
      return std::unexpected(std::format("Error: {}", resp.error()));
    }

    if (!utils::IsSuccess(resp->status)) {
      return std::unexpected(
          std::format("HTTP {}: {}", resp->status, resp->body));
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

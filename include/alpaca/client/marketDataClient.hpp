#pragma once
#include <alpaca/client/environment.hpp>
#include <alpaca/client/httpClient.hpp>
#include <alpaca/models/bars/serialize.hpp>
#include <alpaca/utils/utils.hpp>
#include <glaze/glaze.hpp>
#include <string>

namespace alpaca {

template <class Env = Environment, class Http = HttpClient>
class MarketDataClientT {
private:
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

    auto feed = p.feed ? ToString(p.feed.value()) : std::nullopt;
    auto adjustment =
        p.adjustment ? ToString(p.adjustment.value()) : std::nullopt;
    auto sort = p.sort ? ToString(p.sort.value()) : std::nullopt;
    auto limit = p.limit ? std::make_optional(std::to_string(p.limit.value()))
                         : std::nullopt;

    utils::QueryBuilder qb;
    qb.add("symbols", utils::SymbolsEncode(p.symbols));
    qb.add("timeframe", p.timeframe);
    qb.add("start", p.start);
    qb.add("end", p.end);
    qb.add("limit", limit);
    qb.add("adjustment", adjustment);
    qb.add("asof", p.asof);
    qb.add("feed", feed);
    qb.add("currency", p.currency);
    qb.add("page_token", p.page_token);
    qb.add("sort", sort);

    auto resp = cli_.Get(BARS_ENDPOINT + qb.q, env_.GetAuthHeaders());
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
  explicit MarketDataClientT(const Env &env)
      : env_(env), cli_(env_.GetDataUrl()) {}

  MarketDataClientT(const Env &env, Http cli)
      : env_(env), cli_(std::move(cli)) {}

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

      if (!seen.insert(resp->next_page_token.value()).second) {
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

    auto feed = p.feed ? ToString(p.feed.value()) : std::nullopt;

    utils::QueryBuilder qb;
    qb.add("symbols", utils::SymbolsEncode(p.symbols));
    qb.add("feed", feed);

    auto resp = cli_.Get(LATEST_BARS_ENDPOINT + qb.q, env_.GetAuthHeaders());
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
  const Env &env_;
  Http cli_;

  static constexpr const char *BARS_ENDPOINT = "/v2/stocks/bars?";
  static constexpr const char *LATEST_BARS_ENDPOINT = "/v2/stocks/bars/latest?";
};

using MarketDataClient = MarketDataClientT<Environment, HttpClient>;

}; // namespace alpaca

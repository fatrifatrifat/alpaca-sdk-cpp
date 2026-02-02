#pragma once
#include <alpaca/client/environment.hpp>
#include <alpaca/client/httpClient.hpp>
#include <alpaca/models/marketdata/serialize.hpp>
#include <alpaca/utils/utils.hpp>
#include <glaze/glaze.hpp>
#include <string>

namespace alpaca {

template <class Env = Environment, class Http = HttpClient>
class MarketDataClientT {
private:
  std::expected<Bars, APIError> GetBarsPimpl(const BarParams &p) {
    const auto limit =
        p.limit ? std::make_optional(std::to_string(*p.limit)) : std::nullopt;

    utils::QueryBuilder qb;
    qb.add("symbols", utils::SymbolsEncode(p.symbols));
    qb.add("timeframe", p.timeframe);
    qb.add("start", p.start);
    qb.add("end", p.end);
    qb.add("limit", limit);
    qb.add("adjustment", ToString(p.adjustment));
    qb.add("asof", p.asof);
    qb.add("feed", ToString(p.feed));
    qb.add("currency", p.currency);
    qb.add("page_token", p.page_token);
    qb.add("sort", ToString(p.sort));
    const std::string query = BARS_ENDPOINT + qb.q;
    return cli_.template Request<Bars>(Req::GET, query);
  }

public:
  explicit MarketDataClientT(const Env &env)
      : env_(env), cli_(env_.GetDataUrl(), env_.GetAuthHeaders()) {}

  MarketDataClientT(const Env &env, Http cli)
      : env_(env), cli_(std::move(cli)) {}

  std::expected<Bars, APIError> GetBars(const BarParams &p) {
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
        return std::unexpected(APIError{
            ErrorCode::Unknown, "Pagination error: next_page_token repeated"});
      }

      page = resp->next_page_token;
    }

    return Bars{barsBySymbol};
  }

  std::expected<LatestBars, APIError> GetLatestBar(const LatestBarParam &p) {
    utils::QueryBuilder qb;
    qb.add("symbols", utils::SymbolsEncode(p.symbols));
    qb.add("feed", ToString(p.feed));
    const std::string query = LATEST_BARS_ENDPOINT + qb.q;
    return cli_.template Request<LatestBars>(Req::GET, query);
  }

private:
  const Env &env_;
  Http cli_;

  static constexpr const char *BARS_ENDPOINT = "/v2/stocks/bars?";
  static constexpr const char *LATEST_BARS_ENDPOINT = "/v2/stocks/bars/latest?";
};

using MarketDataClient = MarketDataClientT<Environment, HttpClient>;

}; // namespace alpaca

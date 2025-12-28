#define ENABLE_LOGGING

#include <algorithm>
#include <alpaca/environment.hpp>
#include <alpaca/httpClient.hpp>
#include <alpaca/marketDataClient.hpp>
#include <alpaca/tradingClient.hpp>
#include <chrono>
#include <thread>

using namespace std::chrono;
namespace au = alpaca::utils;

#ifdef ENABLE_LOGGING
#define LOG_MSG(s) std::println("{}", s);
#else
#define LOG_MSG(s)
#endif

struct SymState {
  au::MACD_12_26_9 macd{};
  std::optional<std::string> last_ts{};
  bool in_position{false};
};

int main(int argc, char **argv) {
  if (argc < 2) {
    LOG_MSG("Wrong arguments. Intended arguments <symbol> <day lookup "
            "buffer count>")
    return 1;
  }

  std::vector<std::string> symbols;
  symbols.reserve(argc - 1);
  for (auto i{1uz}; i < argc; i++) {
    symbols.push_back(argv[i]);
  }

  constexpr int WARMUP_BARS = 600;
  constexpr int PERIOD_MIN = 15;
  constexpr int LIVE_FETCH_BARS = 12;
  constexpr seconds API_LAG = seconds{3};

  const std::string timeframe = "15Min";
  const auto barDur = minutes{PERIOD_MIN};

  auto env = alpaca::Environment();
  auto market = alpaca::MarketDataClient(env);
  auto trade = alpaca::TradingClient(env);

  auto warm_end_s = time_point_cast<seconds>(system_clock::now());
  auto warm_start_s = warm_end_s - days{45};

  const std::string warm_start = au::to_isoz(warm_start_s);
  const std::string warm_end = au::to_isoz(warm_end_s);
  LOG_MSG(std::format("Start: {}, End: {}", warm_start, warm_end));

  auto warm_resp =
      market.GetBars({symbols, timeframe, warm_start, warm_end, WARMUP_BARS});
  if (!warm_resp) {
    LOG_MSG(
        std::format("Couldn't get warmup bars. Error: {}", warm_resp.error()));
    return 1;
  }

  std::unordered_map<std::string, SymState> st;
  st.reserve(symbols.size());

  for (auto &s : symbols) {
    auto it = warm_resp->bars.find(s);
    if (it == warm_resp->bars.end() || it->second.empty()) {
      LOG_MSG(
          std::format("[{}] No warmup bars return (check window/symbol)", s));
      continue;
    }

    auto vec = it->second;
    std::sort(vec.begin(), vec.end(),
              [](auto &a, auto &b) { return a.timestamp < b.timestamp; });

    if (vec.size() > WARMUP_BARS) {
      vec.erase(vec.begin(), vec.end() - WARMUP_BARS);
    }

    auto &state = st[s];
    for (auto &b : vec) {
      state.macd.Update(b.close);
    }

    state.last_ts = vec.back().timestamp;
    LOG_MSG(std::format("[{}] Warmed with {} bars. last_ts={}", s, vec.size(),
                        *state.last_ts));
  }

  if (st.empty()) {
    LOG_MSG("No symbols warmed up successfully. Exiting.");
    return 1;
  }

  auto get_cash = [&trade] -> double {
    auto acc = trade.GetAccount();
    return std::stod(acc->cash);
  };

  while (true) {
    std::this_thread::sleep_for(seconds{15});

    auto end_tp = time_point_cast<seconds>(system_clock::now()) - API_LAG;
    auto start_tp = end_tp - duration_cast<seconds>(barDur * LIVE_FETCH_BARS);

    int limit = static_cast<int>(symbols.size()) * LIVE_FETCH_BARS * 2;

    auto resp = market.GetBars({symbols, timeframe, au::to_isoz(start_tp),
                                au::to_isoz(end_tp), limit});
    if (!resp) {
      LOG_MSG(std::format("Live GetBars failed: {}", resp.error()));
      continue;
    }

    for (auto &s : symbols) {
      auto sit = st.find(s);
      if (sit == st.end()) {
        continue;
      }

      auto it = resp->bars.find(s);
      if (it == resp->bars.end() || it->second.empty()) {
        LOG_MSG(std::format("[{}] Market close AND/OR no recent bars", s));
        continue;
      }

      auto vec = it->second;
      std::sort(vec.begin(), vec.end(),
                [](auto &a, auto &b) { return a.timestamp < b.timestamp; });

      auto &state = sit->second;

      for (auto &bar : vec) {
        if (state.last_ts && bar.timestamp <= *state.last_ts) {
          LOG_MSG("Deduplicate, only process brand new bars");
          continue;
        }

        auto m = state.macd.Update(bar.close);
        state.last_ts = bar.timestamp;

        if (!m.ready) {
          LOG_MSG("Not ready");
          continue;
        }

        if (m.crossedAbove && !state.in_position) {
          double cash = get_cash();
          long long qty = static_cast<long long>(std::floor(cash / bar.close));
          if (qty > 0) {
            auto o = trade.SubmitOrder({s, std::to_string(qty), "buy"});
            if (o) {
              state.in_position = true;
              LOG_MSG(std::format("[{}], BUY qty={} @ close={} ts={}", s, qty,
                                  bar.close, bar.timestamp));
            } else {
              LOG_MSG(std::format("[{}], BUY failed: {}", s, o.error()));
            }
          }
        }

        if (m.crossedBelow && state.in_position) {
          auto c = trade.ClosePosition({s, alpaca::Percent{100.0L}});
          if (c) {
            state.in_position = false;
            std::println("[{}] SELL (close position) @ close={} ts={}", s,
                         bar.close, bar.timestamp);
          } else {
            std::println("[{}] SELL failed: {}", s, c.error());
          }
        }
      }
    }
  }
}

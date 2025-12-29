
#include <algorithm>
#include <alpaca/environment.hpp>
#include <alpaca/httpClient.hpp>
#include <alpaca/marketDataClient.hpp>
#include <alpaca/tradingClient.hpp>
#include <alpaca/utils/macd.hpp>
#include <alpaca/utils/utils.hpp>
#include <chrono>
#include <print>
#include <thread>

using namespace std::chrono;
namespace au = alpaca::utils;
namespace am = alpaca::macd;

#define ENABLE_LOGGING

#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_ERROR 4

#ifndef LOG_LEVEL_MINIMUM
#define LOG_LEVEL_MINIMUM LOG_LEVEL_DEBUG
#endif

#if defined(ENABLE_LOGGING)
#define LOG_MSG(level, fmt, ...)                                               \
  do {                                                                         \
    if ((level) >= LOG_LEVEL_MINIMUM)                                          \
      std::println((fmt)__VA_OPT__(, ) __VA_ARGS__);                           \
  } while (false)
#else
#define LOG_MSG(level, fmt, ...)                                               \
  do {                                                                         \
  } while (false)
#endif

struct SymState {
  am::MACD_12_26_9 macd{};
  std::optional<std::string> last_ts{};
  bool in_position{false};
};

int main(int argc, char **argv) {
  if (argc < 2) {
    LOG_MSG(LOG_LEVEL_ERROR, "Wrong arguments. Intended arguments <symbols>");
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

  const std::string warm_start = au::ToIsoz(warm_start_s);
  const std::string warm_end = au::ToIsoz(warm_end_s);
  LOG_MSG(LOG_LEVEL_DEBUG, "Start: {}, End: {}", warm_start, warm_end);

  auto warm_resp =
      market.GetBars({symbols, timeframe, warm_start, warm_end, WARMUP_BARS});
  if (!warm_resp) {
    LOG_MSG(LOG_LEVEL_ERROR, "Couldn't get warmup bars. {}", warm_resp.error());
    return 1;
  }

  std::unordered_map<std::string, SymState> st;
  st.reserve(symbols.size());

  for (auto &s : symbols) {
    auto it = warm_resp->bars.find(s);
    if (it == warm_resp->bars.end() || it->second.empty()) {
      LOG_MSG(LOG_LEVEL_INFO,
              "[{}] No warmup bars return (check window/symbol)", s);
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
    LOG_MSG(LOG_LEVEL_INFO, "[{}] Warmed with {} bars. last_ts={}", s,
            vec.size(), *state.last_ts);
  }

  if (st.empty()) {
    LOG_MSG(LOG_LEVEL_ERROR, "No symbols warmed up successfully. Exiting.");
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
    LOG_MSG(LOG_LEVEL_DEBUG, "Start: {}, End: {}", au::ToIsoz(start_tp),
            au::ToIsoz(end_tp));

    int limit = static_cast<int>(symbols.size()) * LIVE_FETCH_BARS * 2;

    auto resp = market.GetBars(
        {symbols, timeframe, au::ToIsoz(start_tp), au::ToIsoz(end_tp), limit});
    if (!resp) {
      LOG_MSG(LOG_LEVEL_INFO, "Live GetBars failed: {}", resp.error());
      continue;
    }

    for (auto &s : symbols) {
      auto sit = st.find(s);
      if (sit == st.end()) {
        continue;
      }

      auto it = resp->bars.find(s);
      if (it == resp->bars.end() || it->second.empty()) {
        LOG_MSG(LOG_LEVEL_INFO, "[{}] Market close AND/OR no recent bars", s);
        continue;
      }

      auto vec = it->second;
      std::sort(vec.begin(), vec.end(),
                [](auto &a, auto &b) { return a.timestamp < b.timestamp; });

      auto &state = sit->second;

      for (auto &bar : vec) {
        if (state.last_ts && bar.timestamp <= *state.last_ts) {
          continue;
        }

        auto m = state.macd.Update(bar.close);
        state.last_ts = bar.timestamp;

        if (!m.ready) {
          LOG_MSG(LOG_LEVEL_DEBUG, "Not ready");
          continue;
        }

        if (m.crossedAbove && !state.in_position) {
          double buy_power = std::floor(get_cash() / symbols.size());
          long long qty =
              static_cast<long long>(std::floor(buy_power / bar.close));
          if (qty > 0) {
            auto o = trade.SubmitOrder({s, std::to_string(qty), "buy"});
            if (o) {
              state.in_position = true;
              LOG_MSG(LOG_LEVEL_INFO, "[{}], BUY qty={} @ close={} ts={}", s,
                      qty, bar.close, bar.timestamp);
            } else {
              LOG_MSG(LOG_LEVEL_INFO, "[{}], BUY failed: {}", s, o.error());
            }
          }
        }

        if (m.crossedBelow && state.in_position) {
          auto c = trade.ClosePosition({s, alpaca::Percent{100.0L}});
          if (c) {
            state.in_position = false;
            LOG_MSG(LOG_LEVEL_INFO,
                    "[{}] SELL (close position) @ close={} ts={}", s, bar.close,
                    bar.timestamp);
          } else {
            LOG_MSG(LOG_LEVEL_INFO, "[{}] SELL failed: {}", s, c.error());
          }
        }
      }
    }
  }
}

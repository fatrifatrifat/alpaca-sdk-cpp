// #define ENABLE_LOGGING

#include <algorithm>
#include <alpaca/environment.hpp>
#include <alpaca/httpClient.hpp>
#include <alpaca/marketDataClient.hpp>
#include <alpaca/tradingClient.hpp>
#include <chrono>

using namespace std::chrono;

#ifdef ENABLE_LOGGING
#define LOG_MSG(s)                                                             \
  do {                                                                         \
    std::println("{}", s);                                                     \
  } while (false)
#else
#define LOG_MSG(s)                                                             \
  do {                                                                         \
  } while (false)
#endif

#define WARM_UP_BARS 600

int main(int argc, char **argv) {
  auto env = alpaca::Environment();
  auto market = alpaca::MarketDataClient(env);
  auto trade = alpaca::TradingClient(env);

  auto end_s = time_point_cast<seconds>(system_clock::now());
  auto start_s = end_s - days{std::stoi(argv[argc - 1])};

  const std::string start = alpaca::utils::to_isoz(start_s);
  const std::string end = alpaca::utils::to_isoz(end_s);

  std::vector<std::string> sp500;
  sp500.reserve(argc - 2);
  for (auto i{1uz}; i < argc - 1; i++) {
    sp500.push_back(argv[i]);
  }

  double initial_cash = std::stod(trade.GetAccount()->cash);

  auto resp = market.GetBars({sp500, "15Min", start, end, WARM_UP_BARS});

  if (!resp) {
    std::println("{}", resp.error());
    return 1;
  }

  for (const auto &s : sp500) {
    LOG_MSG(std::format("Stock: {}", s));
    auto it = resp->bars.find(s);
    if (it == resp->bars.end() || it->second.empty()) {
      LOG_MSG(std::format("No bars returned for {} in that window", s));
      continue;
    }

    auto &vec = it->second;
    std::sort(vec.begin(), vec.end(), [](const auto &a, const auto &b) {
      return a.timestamp < b.timestamp;
    });

    if (vec.size() > WARM_UP_BARS) {
      vec.erase(vec.begin(), vec.end() - WARM_UP_BARS);
    }

    std::vector<double> closes;
    closes.reserve(vec.size());
    for (auto &b : vec) {
      closes.push_back(b.close);
    }
    LOG_MSG(std::format("Vec size: {}", vec.size()));

    auto macdRes = alpaca::utils::BacktestMACDAllIn(closes, initial_cash);
    auto bhRes = alpaca::utils::BacktestBuyHoldAllIn(closes, initial_cash);

    LOG_MSG(std::format(
        "MACD final equity: {:.2f}  return: {:.2f}%  trades: {} (buys "
        "{}, sells {})",
        macdRes.final_equity, macdRes.total_return * 100.0, macdRes.trades,
        macdRes.buys, macdRes.sells));

    LOG_MSG(std::format(
        "B&H  final equity: {:.2f}  return: {:.2f}%  trades: {}\n",
        bhRes.final_equity, bhRes.total_return * 100.0, bhRes.trades));
  }
}

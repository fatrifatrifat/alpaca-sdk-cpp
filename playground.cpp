#include <algorithm>
#include <alpaca/environment.hpp>
#include <alpaca/httpClient.hpp>
#include <alpaca/marketDataClient.hpp>
#include <alpaca/tradingClient.hpp>
#include <chrono>
#include <thread>

using namespace std::chrono;

#define WARM_UP_BARS 500
#define PERIOD 5

int main(int argc, char **argv) {
  auto env = alpaca::Environment();
  auto market = alpaca::MarketDataClient(env);
  auto trade = alpaca::TradingClient(env);

  auto now = time_point_cast<seconds>(system_clock::now());
  auto end_s = now;

  auto lookback = minutes{PERIOD * WARM_UP_BARS};

  auto start_s = end_s - duration_cast<seconds>(lookback);

  auto to_isoz = [](sys_time<seconds> t) {
    return std::format("{:%FT%T}Z", t);
  };

  const std::string start = to_isoz(start_s);
  const std::string end = to_isoz(end_s);

  std::println("Start: {}", start);
  std::println("End: {}", end);

  std::vector<std::string> sp500;
  sp500.reserve(argc - 1);
  for (auto i{1uz}; i < argc; i++) {
    sp500.push_back(argv[i]);
  }

  double initial_cash = std::stod(trade.GetAccount()->cash);

  auto resp = market.GetBars({sp500, "5Min", start, end, 10'000});
  if (!resp) {
    std::println("{}", resp.error());
    return 1;
  }

  for (const auto &s : sp500) {
    std::println("Stock: {}", s);
    auto it = resp->bars.find(s);
    if (it == resp->bars.end() || it->second.empty()) {
      std::println("No bars returned for {} in that window", s);
      continue;
    }

    auto &vec = it->second;
    std::sort(vec.begin(), vec.end(), [](const auto &a, const auto &b) {
      return a.timestamp < b.timestamp;
    });

    std::vector<double> closes;
    closes.reserve(vec.size());
    for (auto &b : vec) {
      closes.push_back(b.close);
    }

    auto macdRes = alpaca::utils::BacktestMACDAllIn(closes, initial_cash);
    auto bhRes = alpaca::utils::BacktestBuyHoldAllIn(closes, initial_cash);

    std::println("MACD final equity: {:.2f}  return: {:.2f}%  trades: {} (buys "
                 "{}, sells {})",
                 macdRes.final_equity, macdRes.total_return * 100.0,
                 macdRes.trades, macdRes.buys, macdRes.sells);

    std::println("B&H  final equity: {:.2f}  return: {:.2f}%  trades: {}\n",
                 bhRes.final_equity, bhRes.total_return * 100.0, bhRes.trades);
  }

  // std::println("Latest Bar: ");

  // {
  //   auto resp = market.GetLatestBar({{"AAPL", "TSLA"}});
  //   if (!resp) {
  //     std::println("{}", resp.error());
  //     return 1;
  //   }
  // }

  // std::println("Account Info: ");

  // {
  //   auto resp = trade.GetAccount();
  //   if (!resp) {
  //     std::println("{}", resp.error());
  //     return 1;
  //   }
  // }

  // std::println("Order Request: ");

  // {
  //   auto resp = trade.SubmitOrder({"AAPL", "16", "buy"});
  //   if (!resp) {
  //     std::println("{}", resp.error());
  //     return 1;
  //   }
  // }

  // std::println("All Open Positions: ");

  // {
  //   auto resp = trade.GetAllOpenPositions();
  //   if (!resp) {
  //     std::println("{}", resp.error());
  //     return 1;
  //   }
  // }

  // std::println("Open Positions: ");

  // {
  //   auto resp = trade.GetOpenPosition("AAPL");
  //   if (!resp) {
  //     std::println("{}", resp.error());
  //     return 1;
  //   }
  // }

  // std::println("Close Position: ");

  // {
  //   auto resp = trade.ClosePosition({"AAPL", alpaca::Shares{1.0L}});
  //   if (!resp) {
  //     std::println("{}", resp.error());
  //     return 1;
  //   }
  // }
}

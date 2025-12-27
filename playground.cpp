#include <algorithm>
#include <alpaca/environment.hpp>
#include <alpaca/httpClient.hpp>
#include <alpaca/marketDataClient.hpp>
#include <alpaca/tradingClient.hpp>

int main(int argc, char **argv) {
  auto env = alpaca::Environment();
  auto market = alpaca::MarketDataClient(env);
  auto trade = alpaca::TradingClient(env);

  std::vector<std::string> sp500;
  sp500.reserve(argc - 1);
  for (auto i{1uz}; i < argc; i++) {
    sp500.push_back(argv[i]);
  }

  for (const auto &s : sp500) {
    std::println("Stock: {}", s);
    auto resp = market.GetBars(
        {{s}, "1Min", "2023-01-01T00:00:00Z", "2025-12-01T00:00:00Z", 10'000});
    if (!resp) {
      std::println("{}", resp.error());
      return 1;
    }

    auto vec = resp->bars.at(s);
    std::sort(vec.begin(), vec.end(), [](const auto &a, const auto &b) {
      return a.timestamp < b.timestamp;
    });

    std::vector<double> closes(vec.size());
    for (auto &b : vec) {
      closes.push_back(b.close);
    }

    auto macdRes = alpaca::utils::BacktestMACDAllIn(closes, 100'000);
    auto bhRes = alpaca::utils::BacktestBuyHoldAllIn(closes, 100'000);

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

#include <alpaca/alpaca.hpp>

using namespace alpaca;

int main() {
  auto env = Environment{};
  auto trade = TradingClient{env};

  auto resp = trade.GetPortfolioHistory({});
  if (!resp) {
    std::println("{}", resp.error());
    return 1;
  }

  std::println("Portfolio history:");
  std::println("  baseValue={}", resp->baseValue);
  std::println("  timeframe={}", resp->timeframe);

  if (resp->baseValueAsof) {
    std::println("  baseValueAsof={}", *resp->baseValueAsof);
  }

  std::println("  points={}", resp->timestamp.size());
  if (!resp->timestamp.empty()) {
    const auto i = resp->timestamp.size() - 1;
    std::println("  last: t={} equity={} pnl={} pnl%={}",
                 resp->timestamp[i], resp->equity[i], resp->profitLoss[i],
                 resp->profitLossPCT[i]);
  }

  return 0;
}

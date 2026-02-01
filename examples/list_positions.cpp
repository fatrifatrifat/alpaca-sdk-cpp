#include <alpaca/alpaca.hpp>

using namespace alpaca;

int main() {
  auto env = Environment{};
  auto trade = TradingClient{env};

  auto pos = trade.GetAllOpenPositions();
  if (!pos) {
    std::println("{}", pos.error());
    return 1;
  }

  std::println("Open positions count={}", pos->size());
  for (const auto& p : *pos) {
    std::println("{} qty={} avgEntry={} current={} mv={} upl={}",
                 p.symbol, p.qty, p.avgEntryPrice, p.currentPrice,
                 p.marketValue, p.unrealizedPL);
  }

  return 0;
}

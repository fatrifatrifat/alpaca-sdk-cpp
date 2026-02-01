#include <alpaca/alpaca.hpp>

using namespace alpaca;

int main() {
  auto env = Environment{};
  auto trade = TradingClient{env};

  auto clk = trade.GetMarketClockInfo();
  if (!clk) {
    std::println("{}", clk.error());
    return 1;
  }

  std::println("Clock:");
  std::println("  timestamp={}", clk->timestamp);
  std::println("  isOpen={}", clk->isOpen);
  std::println("  nextOpen={}", clk->nextOpen);
  std::println("  nextClose={}", clk->nextClose);

  return 0;
}

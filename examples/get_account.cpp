#include <alpaca/alpaca.hpp>

using namespace alpaca;

int main() {
  auto env = Environment{};
  auto trade = TradingClient{env};

  auto acc = trade.GetAccount();
  if (!acc) {
    std::println("{}", acc.error());
    return 1;
  }

  std::println("Account:");
  std::println("  id={}", acc->id);
  std::println("  account_number={}", acc->account_number);
  std::println("  currency={}", acc->currency);
  std::println("  cash={}", acc->cash);
  std::println("  buying_power={}", acc->buying_power);
  std::println("  equity={}", acc->equity);
  std::println("  portfolio_value={}", acc->portfolio_value);
  std::println("  pattern_day_trader={}", acc->pattern_day_trader);
  std::println("  trading_blocked={}", acc->trading_blocked);

  return 0;
}

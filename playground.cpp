#include <alpaca/alpaca.hpp>
#include <alpaca/utils/macd.hpp>

using namespace alpaca;

int main(int argc, char **argv) {
  auto env = Environment();
  auto market = MarketDataClient(env);
  auto trade = TradingClient(env);

  auto resp = trade.GetMarketCalendarInfo();
  if (!resp) {
    std::println("{}, {}, {}", static_cast<int>(resp.error().code),
                 resp.error().message, *resp.error().status);
    return 1;
  }

  for (const auto &r : *resp) {
    std::println("{}, {}, {}, {}", r.date, r.open, r.close, r.settlementData);
  }
}

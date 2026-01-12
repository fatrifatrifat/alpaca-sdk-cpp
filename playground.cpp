#include <alpaca/alpaca.hpp>
#include <alpaca/utils/macd.hpp>

using namespace alpaca;

int main(int argc, char **argv) {
  auto env = Environment();
  auto market = MarketDataClient(env);
  auto trade = TradingClient(env);

  // macd::run({"AAPL"});

  auto resp = trade.GetMarketCalendarInfo();
  if (!resp) {
    std::println("{}, {}, {}", static_cast<int>(resp.error().code),
                 resp.error().message, *resp.error().status);
    return 1;
  }

  const auto &r = resp.value();
  std::println("{}, {}, {}, {}", r.front().date, r.front().open,
               r.front().close, r.front().settlementDate);

  std::println("{}, {}, {}, {}", r.back().date, r.back().open, r.back().close,
               r.back().settlementDate);
}

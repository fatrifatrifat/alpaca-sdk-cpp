#include <alpaca/alpaca.hpp>
#include <alpaca/utils/macd.hpp>

using namespace alpaca;

int main(int argc, char **argv) {
  auto env = Environment();
  auto market = MarketDataClient(env);
  auto trade = TradingClient(env);

  auto resp = trade.SubmitOrder({{"AAPL"},
                                 Quantity{2},
                                 OrderSide::buy,
                                 OrderType::market,
                                 OrderTimeInForce::day});
  if (!resp) {
    std::println("{}, {}, {}", static_cast<int>(resp.error().code),
                 resp.error().message, *resp.error().status);
    return 1;
  }

  std::println("{}, {}, {}, {}", *resp->qty, resp->createdAt, resp->id,
               *resp->expiredAt);
}

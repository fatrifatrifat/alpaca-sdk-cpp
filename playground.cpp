#include <alpaca/alpaca.hpp>
#include <alpaca/utils/macd.hpp>

using namespace alpaca;

int main(int argc, char **argv) {
  auto env = Environment();
  auto market = MarketDataClient(env);
  auto trade = TradingClient(env);

  const OrderListParam o = {
      .status = OrderStatus::open,
      .symbols = {{"AAPL"}},
  };
  auto resp = trade.GetOrderByClientID("fc762f2e-d531-48fc-915f-5e61d2d20993");
  if (!resp) {
    if (resp.error().status.has_value()) {
      std::println("{}, {}, {}", static_cast<int>(resp.error().code),
                   resp.error().message, *resp.error().status);
    } else {
      std::println("{}, {}", static_cast<int>(resp.error().code),
                   resp.error().message);
    }
    return 1;
  }

  const auto &r = resp.value();
  std::println("{}, {}, {}", r.id, r.createdAt, r.clientOrderID);
}

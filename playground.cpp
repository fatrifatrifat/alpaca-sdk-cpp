#include <alpaca/alpaca.hpp>
#include <alpaca/utils/macd.hpp>

using namespace alpaca;

int main(int argc, char **argv) {
  auto env = Environment();
  auto market = MarketDataClient(env);
  auto trade = TradingClient(env);

  const OrderListParam o = {
      .status = OrderStatus::all,
      .symbols = {{"AAPL"}},
  };
  auto resp = trade.GetAllOrders(o);
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
  std::println("Size: {}", r.size());
  for (const auto &e : r) {
    std::println("{}", e.id);
  }
}

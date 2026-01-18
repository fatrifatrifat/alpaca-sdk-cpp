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
  auto resp = trade.GetAllOrders(o);

  if (!resp) {
    std::println("{}", resp.error());
    return 1;
  }

  const auto &r = resp.value();
  for (const auto &o : r) {
    std::println("Delete order: {}", o.id);
    auto resp_delete = trade.DeleteOrderByID(o.id);

    if (!resp_delete) {
      std::println("{}", resp_delete.error());
      return 1;
    }
  }
}

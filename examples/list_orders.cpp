#include <alpaca/alpaca.hpp>

using namespace alpaca;

int main() {
  auto env = Environment{};
  auto trade = TradingClient{env};

  OrderListParam p = {
      .status = OrderStatus::all,
      .limit = 25,
      .direction = OrderDirection::desc,
      .nested = false,
  };

  auto orders = trade.GetAllOrders(p);
  if (!orders) {
    std::println("{}", orders.error());
    return 1;
  }

  std::println("Orders count={}", orders->size());
  for (const auto& o : *orders) {
    const auto sym = o.symbol ? *o.symbol : std::string{"<none>"};

    std::println("id={} symbol={} status={} type={} side={} filledQty={}",
                 o.id, sym, o.status, o.orderType, ToString(o.side).value_or("?"),
                 o.filledQty);
  }

  return 0;
}

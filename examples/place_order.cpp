#include <alpaca/alpaca.hpp>

using namespace alpaca;

static bool has_yes_flag(int argc, char** argv) {
  for (int i = 1; i < argc; ++i) {
    if (std::string_view{argv[i]} == "--yes") return true;
  }
  return false;
}

int main(int argc, char** argv) {
  if (argc < 4) {
    std::println("Usage: {} SYMBOL buy|sell QTY [--yes]", argv[0]);
    std::println("Example: {} AAPL buy 1 --yes", argv[0]);
    return 2;
  }

  const std::string symbol = argv[1];
  const std::string side_s = argv[2];
  const long double qty = std::stold(argv[3]);

  OrderSide side{};
  if (side_s == "buy")
    side = OrderSide::buy;
  else if (side_s == "sell")
    side = OrderSide::sell;
  else {
    std::println("Invalid side '{}'. Use buy or sell.", side_s);
    return 2;
  }

  OrderRequest r{};
  r.symbol = symbol;
  r.amt = Quantity{qty};
  r.side = side;
  r.type = OrderType::market;
  r.timeInForce = OrderTimeInForce::day;

  auto env = Environment{};
  auto trade = TradingClient{env};

  if (!has_yes_flag(argc, argv)) {
    std::println("Dry-run (no order sent). Add --yes to submit.");
    std::println("Would submit: symbol={} side={} qty={}",
                 r.symbol, ToString(r.side).value_or("?"), qty);
    return 0;
  }

  auto resp = trade.SubmitOrder(r);
  if (!resp) {
    std::println("{}", resp.error());
    return 1;
  }

  std::println("Order submitted:");
  std::println("  id={}", resp->id);
  std::println("  status={}", resp->status);
  std::println("  filledQty={}", resp->filledQty);

  return 0;
}

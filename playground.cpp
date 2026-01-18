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

  auto resp = trade.GetPortfolioHistory({});

  if (!resp) {
    std::println("{}", resp.error());
    return 1;
  }

  std::println("{}, {}, {}, {}, {}, {}, {}", resp->timestamp, resp->equity,
               resp->profitLoss, resp->profitLossPCT, resp->baseValue,
               resp->baseValueAsof.value(), resp->timeframe);
}

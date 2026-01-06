#include <alpaca/alpaca.hpp>
#include <alpaca/utils/macd.hpp>

int main(int argc, char **argv) {
  auto env = alpaca::Environment();
  auto market = alpaca::MarketDataClient(env);
  auto trade = alpaca::TradingClient(env);

  auto resp =
      trade.SubmitOrder({"AAPL", alpaca::Quantity{10}, alpaca::OrderSide::buy,
                         alpaca::OrderType::market,
                         alpaca::OrderTimeInForce::day, alpaca::LimitPrice{0.5},
                         alpaca::StopPrice{0.3f}, alpaca::TrailPercent{1.5f}});
  if (!resp) {
    std::println("{}", resp.error());
  }
}

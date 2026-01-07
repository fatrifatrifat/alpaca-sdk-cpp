#include <alpaca/alpaca.hpp>
#include <alpaca/utils/macd.hpp>

int main(int argc, char **argv) {
  auto env = alpaca::Environment();
  auto market = alpaca::MarketDataClient(env);
  auto trade = alpaca::TradingClient(env);

  auto resp = trade.GetAllOpenPositions();
  if (!resp) {
    std::println("{}", resp.error());
  }
}

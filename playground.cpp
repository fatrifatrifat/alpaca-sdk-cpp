#include <alpaca/environment.hpp>
#include <alpaca/httpClient.hpp>
#include <alpaca/marketDataClient.hpp>

int main() {
  auto env = alpaca::Environment();
  auto market = alpaca::MarketDataClient(env);
  market.GetBars("AAPL");
}

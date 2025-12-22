#include <alpaca/environment.hpp>
#include <alpaca/httpClient.hpp>
#include <alpaca/marketDataClient.hpp>

int main() {
  auto env = alpaca::Environment();
  auto market = alpaca::MarketDataClient(env);
  auto resp = market.GetBars({"AAPL"});
  if (!resp) {
    std::println("{}", resp.error());
    return 1;
  }
}

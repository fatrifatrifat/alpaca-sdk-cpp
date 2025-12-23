#include <alpaca/environment.hpp>
#include <alpaca/httpClient.hpp>
#include <alpaca/marketDataClient.hpp>
#include <alpaca/tradingClient.hpp>

int main() {
  auto env = alpaca::Environment();
  auto market = alpaca::MarketDataClient(env);
  auto trade = alpaca::TradingClient(env);

  {
    auto resp = market.GetBars({{"AAPL"}});
    if (!resp) {
      std::println("{}", resp.error());
      return 1;
    }
  }

  std::println("Latest Bar: ");

  {
    auto resp = market.GetLatestBar({{"AAPL", "TSLA"}});
    if (!resp) {
      std::println("{}", resp.error());
      return 1;
    }
  }

  std::println("Account Info: ");

  {
    auto resp = trade.GetAccount();
    if (!resp) {
      std::println("{}", resp.error());
      return 1;
    }
  }

  std::println("Order Request: ");

  {
    auto resp = trade.SubmitOrder({"TSLA", "15", "buy"});
    if (!resp) {
      std::println("{}", resp.error());
      return 1;
    }
  }

  std::println("All Open Positions: ");

  {
    auto resp = trade.GetAllOpenPositions();
    if (!resp) {
      std::println("{}", resp.error());
      return 1;
    }
  }

  std::println("Open Positions: ");

  {
    auto resp = trade.GetOpenPosition("AAPL");
    if (!resp) {
      std::println("{}", resp.error());
      return 1;
    }
  }
}

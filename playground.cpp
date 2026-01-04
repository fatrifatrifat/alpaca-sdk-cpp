#include <alpaca/alpaca.hpp>
#include <alpaca/utils/macd.hpp>

int main(int argc, char **argv) {
  // std::vector<std::string> symbols;
  // symbols.reserve(argc - 1);
  // for (auto i{1uz}; i < argc; i++) {
  //   symbols.push_back(argv[i]);
  // }

  // alpaca::macd::run(symbols);

  auto env = alpaca::Environment();
  auto market = alpaca::MarketDataClient(env);
  auto trade = alpaca::TradingClient(env);

  auto bars = market.GetBars({{"AAPL"},
                              "1H",
                              "2024-01-03T00:00:00Z",
                              "2024-01-04T00:00:00Z",
                              std::nullopt,
                              alpaca::BarFeed::IEX,
                              std::nullopt,
                              std::nullopt,
                              std::nullopt,
                              "USD",
                              std::nullopt});
  if (!bars) {
    LOG_MSG(LOG_LEVEL_ERROR, "{}", bars.error());
    return 1;
  }

  std::println("Currency: {}", bars->currency);
  for (const auto &stock : bars->bars) {
    for (const auto &b : stock.second) {
      std::println("[{}], {}", stock.first, b.timestamp);
    }
  }
}

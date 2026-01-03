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

  auto acc_resp = trade.GetAccount();
  if (!acc_resp) {
    LOG_MSG(LOG_LEVEL_ERROR, "{}", acc_resp.error());
    return 1;
  }

  LOG_MSG(LOG_LEVEL_DEBUG, "{}, {}",
          static_cast<int>(acc_resp->options_approved_level),
          static_cast<int>(acc_resp->status));
}

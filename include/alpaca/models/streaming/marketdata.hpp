#pragma once
#include <alpaca/client/httpClient.hpp>
#include <functional>
#include <string>
#include <vector>

namespace alpaca {

struct StreamTrade {
  std::string symbol;
  std::string timestamp;
  std::string exchange;
  std::string tape;
  double price{};
  int64_t size{};
};

struct StreamQuote {
  std::string symbol;
  std::string timestamp;
  double askPrice{};
  double bidPrice{};
  int64_t askSize{};
  int64_t bidSize{};
};

struct StreamBar {
  std::string symbol;
  std::string timestamp;
  double open{};
  double high{};
  double low{};
  double close{};
  double vwap{};
  int64_t volume{};
  int64_t numTrades{};
};

struct MarketDataSubscription {
  std::vector<std::string> trades;
  std::vector<std::string> quotes;
  std::vector<std::string> bars;
  std::string feed{"iex"};
};

struct MarketDataCallbacks {
  std::function<void(StreamTrade)> onTrade;
  std::function<void(StreamQuote)> onQuote;
  std::function<void(StreamBar)> onBar;
  std::function<void(APIError)> onError;
  std::function<void()> onConnected;
  std::function<void()> onDisconnected;
};

} // namespace alpaca

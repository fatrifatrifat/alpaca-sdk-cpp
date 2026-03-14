#include <alpaca/alpaca.hpp>
#include <alpaca/client/marketDataStream.hpp>

#include <chrono>
#include <print>
#include <thread>

int main() {
    alpaca::Environment env;

    alpaca::MarketDataSubscription sub;
    sub.trades = {"AAPL"};
    sub.bars   = {"AAPL"};
    sub.feed   = "iex";

    alpaca::MarketDataCallbacks cbs;

    cbs.onConnected = [] {
        std::println("Connected and authenticated.");
    };

    cbs.onDisconnected = [] {
        std::println("Disconnected.");
    };

    cbs.onTrade = [](alpaca::StreamTrade t) {
        std::println("TRADE  {} @ {} x {}", t.symbol, t.price, t.size);
    };

    cbs.onBar = [](alpaca::StreamBar b) {
        std::println("BAR    {} O={} H={} L={} C={} V={}",
                     b.symbol, b.open, b.high, b.low, b.close, b.volume);
    };

    cbs.onError = [](alpaca::APIError e) {
        std::println("ERROR  [{}] {}", static_cast<int>(e.code), e.message);
    };

    alpaca::MarketDataStream stream(env);
    stream.Connect(sub, std::move(cbs));

    std::println("Streaming for 30 seconds...");
    std::this_thread::sleep_for(std::chrono::seconds(30));

    stream.Disconnect();
    std::println("Done.");
}

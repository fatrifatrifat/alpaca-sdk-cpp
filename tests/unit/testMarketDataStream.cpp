#include <alpaca/client/marketDataStream.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <memory>
#include <string>
#include <vector>

// ── Test doubles ──────────────────────────────────────────────────────────────

namespace {

struct TestEnvironment {
    std::string GetID()     const { return "TEST_KEY"; }
    std::string GetSecret() const { return "TEST_SECRET"; }
    std::string GetStreamDataUrl(const std::string& feed) const {
        return "wss://stream.test/" + feed;
    }
};

// Shared mutable state so the test can observe sends and inject messages
// even after FakeWebSocket is moved into the stream.
struct FakeWsState {
    std::vector<std::string> sent;
    alpaca::WsCallbacks      cbs;
    bool                     connected = false;

    void Inject(const std::string& msg) {
        if (cbs.onMessage) cbs.onMessage(msg);
    }
};

struct FakeWebSocket {
    std::shared_ptr<FakeWsState> state = std::make_shared<FakeWsState>();

    void Connect(const std::string& /*url*/, alpaca::WsCallbacks c) {
        state->cbs       = std::move(c);
        state->connected = true;
        // market-data protocol: server sends "connected" first; no auto-open
    }
    void Disconnect() {
        state->connected = false;
        if (state->cbs.onClose) state->cbs.onClose();
    }
    void Send(const std::string& m) { state->sent.push_back(m); }
    bool IsConnected() const { return state->connected; }
};

using TestStream = alpaca::MarketDataStreamT<TestEnvironment, FakeWebSocket>;

// MarketDataStreamT contains std::atomic<State> so it is not movable.
// Return by unique_ptr to avoid any copy/move of the stream object.
struct StreamHandle {
    std::unique_ptr<TestStream>  stream;
    std::shared_ptr<FakeWsState> ws;
};

StreamHandle MakeStream(const TestEnvironment& env,
                        alpaca::MarketDataSubscription sub,
                        alpaca::MarketDataCallbacks    cbs) {
    FakeWebSocket fakeWs;
    auto s = fakeWs.state;
    auto stream = std::make_unique<TestStream>(env, std::move(fakeWs));
    stream->Connect(std::move(sub), std::move(cbs));
    return {std::move(stream), std::move(s)};
}

} // namespace

// ── Tests ─────────────────────────────────────────────────────────────────────

TEST_CASE("[MarketDataStream] auth is first message after server connected", "[MarketDataStream]") {
    TestEnvironment env;
    alpaca::MarketDataSubscription sub;
    sub.trades = {"AAPL"};
    sub.feed   = "iex";

    auto [stream, ws] = MakeStream(env, sub, {});

    ws->Inject(R"([{"T":"success","msg":"connected"}])");

    REQUIRE(!ws->sent.empty());
    REQUIRE_THAT(ws->sent[0],
                 Catch::Matchers::ContainsSubstring(R"("action":"auth")"));
    REQUIRE_THAT(ws->sent[0],
                 Catch::Matchers::ContainsSubstring("TEST_KEY"));
}

TEST_CASE("[MarketDataStream] subscribe sent after authenticated and onConnected fires", "[MarketDataStream]") {
    TestEnvironment env;
    alpaca::MarketDataSubscription sub;
    sub.trades = {"AAPL"};
    sub.quotes = {"AAPL"};
    sub.bars   = {"AAPL"};

    bool connected = false;
    alpaca::MarketDataCallbacks cbs;
    cbs.onConnected = [&] { connected = true; };

    auto [stream, ws] = MakeStream(env, sub, cbs);
    ws->Inject(R"([{"T":"success","msg":"connected"}])");
    ws->Inject(R"([{"T":"success","msg":"authenticated"}])");

    REQUIRE(connected);
    REQUIRE(ws->sent.size() >= 2);
    REQUIRE_THAT(ws->sent[1],
                 Catch::Matchers::ContainsSubstring(R"("action":"subscribe")"));
    REQUIRE_THAT(ws->sent[1],
                 Catch::Matchers::ContainsSubstring("\"AAPL\""));
}

TEST_CASE("[MarketDataStream] trade message fires onTrade", "[MarketDataStream]") {
    TestEnvironment env;
    alpaca::MarketDataSubscription sub;
    sub.trades = {"AAPL"};

    alpaca::StreamTrade received;
    bool fired = false;
    alpaca::MarketDataCallbacks cbs;
    cbs.onTrade = [&](alpaca::StreamTrade t) { received = std::move(t); fired = true; };

    auto [stream, ws] = MakeStream(env, sub, cbs);
    ws->Inject(R"([{"T":"success","msg":"connected"}])");
    ws->Inject(R"([{"T":"success","msg":"authenticated"}])");
    ws->Inject(R"([{"T":"subscription","trades":["AAPL"]}])");
    ws->Inject(
        R"([{"T":"t","S":"AAPL","p":150.0,"s":100,"t":"2024-01-02T10:00:00Z","x":"C","z":"A"}])");

    REQUIRE(fired);
    REQUIRE(received.symbol   == "AAPL");
    REQUIRE(received.price    == 150.0);
    REQUIRE(received.size     == 100);
    REQUIRE(received.exchange == "C");
    REQUIRE(received.tape     == "A");
}

TEST_CASE("[MarketDataStream] bar message fires onBar", "[MarketDataStream]") {
    TestEnvironment env;
    alpaca::MarketDataSubscription sub;
    sub.bars = {"AAPL"};

    alpaca::StreamBar received;
    bool fired = false;
    alpaca::MarketDataCallbacks cbs;
    cbs.onBar = [&](alpaca::StreamBar b) { received = std::move(b); fired = true; };

    auto [stream, ws] = MakeStream(env, sub, cbs);
    ws->Inject(R"([{"T":"success","msg":"connected"}])");
    ws->Inject(R"([{"T":"success","msg":"authenticated"}])");
    ws->Inject(R"([{"T":"subscription","bars":["AAPL"]}])");
    ws->Inject(
        R"([{"T":"b","S":"AAPL","o":148.0,"h":151.0,"l":147.5,"c":150.5,)"
        R"("v":12000,"n":300,"vw":149.8,"t":"2024-01-02T10:00:00Z"}])");

    REQUIRE(fired);
    REQUIRE(received.symbol    == "AAPL");
    REQUIRE(received.open      == 148.0);
    REQUIRE(received.high      == 151.0);
    REQUIRE(received.volume    == 12000);
    REQUIRE(received.numTrades == 300);
    REQUIRE(received.vwap      == 149.8);
}

TEST_CASE("[MarketDataStream] error message fires onError", "[MarketDataStream]") {
    TestEnvironment env;
    alpaca::MarketDataSubscription sub;

    alpaca::APIError received{alpaca::ErrorCode::Unknown, ""};
    bool fired = false;
    alpaca::MarketDataCallbacks cbs;
    cbs.onError = [&](alpaca::APIError e) { received = e; fired = true; };

    auto [stream, ws] = MakeStream(env, sub, cbs);
    ws->Inject(R"([{"T":"success","msg":"connected"}])");
    ws->Inject(R"([{"T":"error","code":406,"msg":"not authenticated"}])");

    REQUIRE(fired);
    REQUIRE(received.code == alpaca::ErrorCode::Connection);
    REQUIRE_THAT(received.message,
                 Catch::Matchers::ContainsSubstring("not authenticated"));
}

TEST_CASE("[MarketDataStream] unknown T value is silently ignored", "[MarketDataStream]") {
    TestEnvironment env;
    alpaca::MarketDataSubscription sub;

    int errors = 0;
    alpaca::MarketDataCallbacks cbs;
    cbs.onError = [&](alpaca::APIError) { ++errors; };

    auto [stream, ws] = MakeStream(env, sub, cbs);
    ws->Inject(R"([{"T":"success","msg":"connected"}])");
    ws->Inject(R"([{"T":"unknown_future_type","foo":"bar"}])");

    REQUIRE(errors == 0);
}

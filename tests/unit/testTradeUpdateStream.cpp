#include <alpaca/client/tradeUpdateStream.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <memory>
#include <string>
#include <vector>

// ── Test doubles ──────────────────────────────────────────────────────────────

namespace {

struct TestEnvironment {
    std::string GetID()             const { return "TEST_KEY"; }
    std::string GetSecret()         const { return "TEST_SECRET"; }
    std::string GetTradeStreamUrl() const { return "wss://trade.test/stream"; }
};

struct FakeWsState {
    std::vector<std::string> sent;
    alpaca::WsCallbacks      cbs;
    bool                     connected = false;

    void Inject(const std::string& msg) {
        if (cbs.onMessage) cbs.onMessage(msg);
    }
    void SimulateOpen() {
        if (cbs.onOpen) cbs.onOpen();
    }
};

struct FakeWebSocket {
    std::shared_ptr<FakeWsState> state = std::make_shared<FakeWsState>();

    void Connect(const std::string& /*url*/, alpaca::WsCallbacks c) {
        state->cbs       = std::move(c);
        state->connected = true;
        state->SimulateOpen(); // trade stream sends auth on open
    }
    void Disconnect() {
        state->connected = false;
        if (state->cbs.onClose) state->cbs.onClose();
    }
    void Send(const std::string& m) { state->sent.push_back(m); }
    bool IsConnected() const { return state->connected; }
};

using TestStream = alpaca::TradeUpdateStreamT<TestEnvironment, FakeWebSocket>;

// TradeUpdateStreamT contains std::atomic so it is not movable.
// Return via unique_ptr.
struct StreamHandle {
    std::unique_ptr<TestStream>  stream;
    std::shared_ptr<FakeWsState> ws;
};

StreamHandle MakeStream(const TestEnvironment& env, alpaca::TradeUpdateCallbacks cbs) {
    FakeWebSocket fakeWs;
    auto s = fakeWs.state;
    auto stream = std::make_unique<TestStream>(env, std::move(fakeWs));
    stream->Connect(std::move(cbs));
    return {std::move(stream), std::move(s)};
}

// Minimal order JSON for embedding in trade_updates messages
constexpr std::string_view kMinimalOrder = R"({
    "id":"order_1","client_order_id":"c_1","created_at":"2024-01-02T10:00:00Z",
    "updated_at":"2024-01-02T10:00:01Z","submitted_at":"2024-01-02T10:00:02Z",
    "filled_qty":"0","order_type":"market","type":"market",
    "side":"buy","time_in_force":"day","status":"filled","extended_hours":false
})";

} // namespace

// ── Tests ─────────────────────────────────────────────────────────────────────

TEST_CASE("[TradeUpdateStream] auth sent on connect", "[TradeUpdateStream]") {
    TestEnvironment env;
    auto [stream, ws] = MakeStream(env, {});

    REQUIRE(!ws->sent.empty());
    REQUIRE_THAT(ws->sent[0],
                 Catch::Matchers::ContainsSubstring(R"("action":"auth")"));
    REQUIRE_THAT(ws->sent[0],
                 Catch::Matchers::ContainsSubstring("TEST_KEY"));
}

TEST_CASE("[TradeUpdateStream] listen sent after authorized and onConnected fires", "[TradeUpdateStream]") {
    TestEnvironment env;
    bool connected = false;
    alpaca::TradeUpdateCallbacks cbs;
    cbs.onConnected = [&] { connected = true; };

    auto [stream, ws] = MakeStream(env, cbs);
    ws->Inject(R"({"stream":"authorization","data":{"status":"authorized"}})");

    REQUIRE(connected);
    REQUIRE(ws->sent.size() >= 2);
    REQUIRE_THAT(ws->sent[1],
                 Catch::Matchers::ContainsSubstring(R"("action":"listen")"));
    REQUIRE_THAT(ws->sent[1],
                 Catch::Matchers::ContainsSubstring("trade_updates"));
}

TEST_CASE("[TradeUpdateStream] fill event fires onUpdate", "[TradeUpdateStream]") {
    TestEnvironment env;
    alpaca::TradeUpdate received;
    bool fired = false;
    alpaca::TradeUpdateCallbacks cbs;
    cbs.onUpdate = [&](alpaca::TradeUpdate u) { received = std::move(u); fired = true; };

    auto [stream, ws] = MakeStream(env, cbs);
    ws->Inject(R"({"stream":"authorization","data":{"status":"authorized"}})");
    ws->Inject(std::string(R"({"stream":"trade_updates","data":{"event":"fill","at":"2024-01-02T10:00:03Z","order":)")
               + std::string(kMinimalOrder) + "}}");

    REQUIRE(fired);
    REQUIRE(received.event == alpaca::TradeUpdateEvent::fill);
    REQUIRE(received.at    == "2024-01-02T10:00:03Z");
    REQUIRE(received.order.id == "order_1");
}

TEST_CASE("[TradeUpdateStream] partial_fill event fires correctly", "[TradeUpdateStream]") {
    TestEnvironment env;
    alpaca::TradeUpdate received;
    alpaca::TradeUpdateCallbacks cbs;
    cbs.onUpdate = [&](alpaca::TradeUpdate u) { received = std::move(u); };

    auto [stream, ws] = MakeStream(env, cbs);
    ws->Inject(R"({"stream":"authorization","data":{"status":"authorized"}})");
    ws->Inject(std::string(R"({"stream":"trade_updates","data":{"event":"partial_fill","at":"2024-01-02T10:00:04Z","order":)")
               + std::string(kMinimalOrder) + "}}");

    REQUIRE(received.event == alpaca::TradeUpdateEvent::partial_fill);
}

TEST_CASE("[TradeUpdateStream] new maps to new_order", "[TradeUpdateStream]") {
    TestEnvironment env;
    alpaca::TradeUpdate received;
    alpaca::TradeUpdateCallbacks cbs;
    cbs.onUpdate = [&](alpaca::TradeUpdate u) { received = std::move(u); };

    auto [stream, ws] = MakeStream(env, cbs);
    ws->Inject(R"({"stream":"authorization","data":{"status":"authorized"}})");
    ws->Inject(std::string(R"({"stream":"trade_updates","data":{"event":"new","at":"2024-01-02T10:00:00Z","order":)")
               + std::string(kMinimalOrder) + "}}");

    REQUIRE(received.event == alpaca::TradeUpdateEvent::new_order);
}

TEST_CASE("[TradeUpdateStream] unknown event string maps to unknown", "[TradeUpdateStream]") {
    TestEnvironment env;
    alpaca::TradeUpdate received;
    alpaca::TradeUpdateCallbacks cbs;
    cbs.onUpdate = [&](alpaca::TradeUpdate u) { received = std::move(u); };

    auto [stream, ws] = MakeStream(env, cbs);
    ws->Inject(R"({"stream":"authorization","data":{"status":"authorized"}})");
    ws->Inject(std::string(
        R"({"stream":"trade_updates","data":{"event":"some_future_event","at":"2024-01-02T10:00:00Z","order":)")
               + std::string(kMinimalOrder) + "}}");

    REQUIRE(received.event == alpaca::TradeUpdateEvent::unknown);
}

TEST_CASE("[TradeUpdateStream] unauthorized auth fires onError", "[TradeUpdateStream]") {
    TestEnvironment env;
    alpaca::APIError received{alpaca::ErrorCode::Unknown, ""};
    bool fired = false;
    alpaca::TradeUpdateCallbacks cbs;
    cbs.onError = [&](alpaca::APIError e) { received = e; fired = true; };

    auto [stream, ws] = MakeStream(env, cbs);
    ws->Inject(R"({"stream":"authorization","data":{"status":"unauthorized"}})");

    REQUIRE(fired);
    REQUIRE(received.code == alpaca::ErrorCode::Connection);
}

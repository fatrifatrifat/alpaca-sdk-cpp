# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Configure (tests and examples disabled by default)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build with tests
cmake -S . -B build -DALPACA_BUILD_TESTS=ON
cmake --build build -j

# Build with examples
cmake -S . -B build -DALPACA_BUILD_EXAMPLES=ON
cmake --build build -j

# Run all tests
cd build && ctest --output-on-failure

# Run a single test by name (Catch2)
cd build && ./tests/alpaca_tests "[TradingClient.GetAccount]"
```

**Prerequisites:** C++23, OpenSSL, CMake ≥ 3.23. Dependencies (`glaze`, `cpp-httplib`, `ixwebsocket`, `Catch2`) are fetched automatically via CMake FetchContent.

## Architecture

This is a **header-only** C++23 SDK for Alpaca's REST APIs. All source lives under `include/alpaca/`.

### Layer structure

```
Environment  →  HttpClient      →  TradingClient / MarketDataClient
Environment  →  WebSocketClient →  MarketDataStream / TradeUpdateStream
```

- **`Environment`** (`client/environment.hpp`): reads credentials from env vars (`APCA_API_KEY_ID`, `APCA_API_SECRET_KEY`, `APCA_API_BASE_URL`, `APCA_API_DATA_URL`) and produces auth headers. Also exposes `GetTradeStreamUrl()` and `GetStreamDataUrl(feed)`.
- **`HttpClient`** (`client/httpClient.hpp`): wraps `httplib::SSLClient`; exposes a single templated `Request<T>()` that handles HTTP dispatch and deserializes the JSON body into `T` via `glaze`.
- **`TradingClient`** / **`MarketDataClient`** (`client/tradingClient.hpp`, `client/marketDataClient.hpp`): domain clients that compose `Environment` + `HttpClient`. All methods return `std::expected<T, APIError>`—no exceptions.
- **`WebSocketClient`** (`client/websocketClient.hpp`): thin wrapper around `ix::WebSocket` (stored as `unique_ptr` for movability). Exposes `Connect(url, WsCallbacks)` / `Send` / `Disconnect`.
- **`MarketDataStream`** (`client/marketDataStream.hpp`): template `MarketDataStreamT<Env, Ws>`. Non-blocking; runs auth handshake then dispatches `StreamTrade` / `StreamQuote` / `StreamBar` via `MarketDataCallbacks`.
- **`TradeUpdateStream`** (`client/tradeUpdateStream.hpp`): template `TradeUpdateStreamT<Env, Ws>`. Fires `TradeUpdate` (with `TradeUpdateEvent` enum) via `TradeUpdateCallbacks`.

### Testability via templates

Both clients are class templates `TradingClientT<Env, Http>` with default args `Environment` and `HttpClient`. Tests inject `TestEnvironment` and `FakeHttpClient` to avoid real network calls. See `tests/unit/testTradingClient.cpp` for the pattern.

### Serialization

`glaze` is used for JSON. All `glz::meta<>` specializations mapping C++ struct fields to JSON keys live in:
- `include/alpaca/models/trading/serialize.hpp`
- `include/alpaca/models/marketdata/serialize.hpp`

The `OrderRequestParam` → wire format conversion (`OrderRequestWire`) is handled by a custom `glz::to<JSON>` / `glz::from<JSON>` specialization in `trading/serialize.hpp` — this translates the user-facing `amt` (variant of `Quantity`/`Notional`) into `qty`/`notional` fields on the wire.

### Error handling

`APIError` (defined in `httpClient.hpp`) carries an `ErrorCode` enum, a message string, and an optional HTTP status code. Use `std::expected`'s `.has_value()` / `.error()` at call sites.

### Pagination

`MarketDataClient::GetBars()` automatically follows `next_page_token` until exhausted, accumulating results across pages. The private `GetBarsPimpl()` handles a single page request.

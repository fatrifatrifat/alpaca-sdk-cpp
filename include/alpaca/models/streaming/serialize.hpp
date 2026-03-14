#pragma once
#include <alpaca/models/streaming/marketdata.hpp>
#include <alpaca/models/streaming/tradeupdate.hpp>
#include <alpaca/models/trading/serialize.hpp>
#include <glaze/glaze.hpp>
#include <string>

namespace alpaca {

// ── Market data wire ────────────────────────────────────
// Single fat struct
// that covers all market-data message types. Fields not present in a given
// message are left at their zero-initialized defaults; unknown JSON keys are
// ignored via error_on_unknown_keys = false.

struct MarketDataMsgWire {
  std::string type;      // "T"  – type discriminator
  std::string msg;       // "msg" – auth success/error message
  int code{};            // "code" – error code
  std::string symbol;    // "S"
  std::string timestamp; // "t"
  // trade
  double price{};       // "p"
  int64_t tradeSize{};  // "s"
  std::string exchange; // "x"
  std::string tape;     // "z"
  // quote
  double askPrice{};       // "ap"
  double bidPrice{};       // "bp"
  int64_t askSize{};       // "as"
  int64_t bidSize{};       // "bs"
  std::string askExchange; // "ax"
  std::string bidExchange; // "bx"
  // bar
  double barOpen{};       // "o"
  double barHigh{};       // "h"
  double barLow{};        // "l"
  double barClose{};      // "c"
  double barVwap{};       // "vw"
  int64_t barVolume{};    // "v"
  int64_t barNumTrades{}; // "n"
};

// ── Trade-update stream wire
// ──────────────────────────────────────────────────

struct TradingAuthDataWire {
  std::string status;
};

struct TradingStreamBaseWire {
  std::string stream;
  TradingAuthDataWire data;
};

struct TradeUpdateDataWire {
  std::string event;
  std::string at;
  OrderResponse order{};
};

struct TradeUpdateEnvelopeWire {
  std::string stream;
  TradeUpdateDataWire data;
};

// ── Helper: event string → enum
// ───────────────────────────────────────────────
inline TradeUpdateEvent ParseTradeUpdateEvent(const std::string &s) noexcept {
  if (s == "new")
    return TradeUpdateEvent::new_order;
  if (s == "fill")
    return TradeUpdateEvent::fill;
  if (s == "partial_fill")
    return TradeUpdateEvent::partial_fill;
  if (s == "canceled")
    return TradeUpdateEvent::canceled;
  if (s == "replaced")
    return TradeUpdateEvent::replaced;
  if (s == "rejected")
    return TradeUpdateEvent::rejected;
  if (s == "pending_new")
    return TradeUpdateEvent::pending_new;
  if (s == "pending_cancel")
    return TradeUpdateEvent::pending_cancel;
  if (s == "expired")
    return TradeUpdateEvent::expired;
  if (s == "suspended")
    return TradeUpdateEvent::suspended;
  return TradeUpdateEvent::unknown;
}

} // namespace alpaca

// ── Glaze meta
// ────────────────────────────────────────────────────────────────
namespace glz {

template <> struct meta<alpaca::MarketDataMsgWire> {
  using T = alpaca::MarketDataMsgWire;
  static constexpr auto value = object(
      "T", &T::type, "msg", &T::msg, "code", &T::code, "S", &T::symbol, "t",
      &T::timestamp, "p", &T::price, "s", &T::tradeSize, "x", &T::exchange, "z",
      &T::tape, "ap", &T::askPrice, "bp", &T::bidPrice, "as", &T::askSize, "bs",
      &T::bidSize, "ax", &T::askExchange, "bx", &T::bidExchange, "o",
      &T::barOpen, "h", &T::barHigh, "l", &T::barLow, "c", &T::barClose, "vw",
      &T::barVwap, "v", &T::barVolume, "n", &T::barNumTrades);
};

template <> struct meta<alpaca::TradingAuthDataWire> {
  using T = alpaca::TradingAuthDataWire;
  static constexpr auto value = object("status", &T::status);
};

template <> struct meta<alpaca::TradingStreamBaseWire> {
  using T = alpaca::TradingStreamBaseWire;
  static constexpr auto value = object("stream", &T::stream, "data", &T::data);
};

template <> struct meta<alpaca::TradeUpdateDataWire> {
  using T = alpaca::TradeUpdateDataWire;
  static constexpr auto value =
      object("event", &T::event, "at", &T::at, "order", &T::order);
};

template <> struct meta<alpaca::TradeUpdateEnvelopeWire> {
  using T = alpaca::TradeUpdateEnvelopeWire;
  static constexpr auto value = object("stream", &T::stream, "data", &T::data);
};

} // namespace glz

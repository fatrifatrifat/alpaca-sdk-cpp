#pragma once
#include <alpaca/client/environment.hpp>
#include <alpaca/client/websocketClient.hpp>
#include <alpaca/models/streaming/marketdata.hpp>
#include <alpaca/models/streaming/serialize.hpp>
#include <atomic>
#include <format>
#include <string>
#include <vector>

namespace alpaca {

template <class Env = Environment, class Ws = WebSocketClient>
class MarketDataStreamT {
public:
  explicit MarketDataStreamT(const Env &env, Ws ws = Ws{})
      : env_(env), ws_(std::move(ws)) {}

  void Connect(MarketDataSubscription sub, MarketDataCallbacks cbs) {
    sub_ = std::move(sub);
    cbs_ = std::move(cbs);
    state_ = State::Connecting;

    std::string url = env_.GetStreamDataUrl(sub_.feed);

    WsCallbacks wsCbs;
    wsCbs.onMessage = [this](const std::string &raw) { OnMessage(raw); };
    wsCbs.onClose = [this]() {
      state_ = State::Disconnected;
      if (cbs_.onDisconnected)
        cbs_.onDisconnected();
    };
    wsCbs.onError = [this](const std::string &reason) {
      if (cbs_.onError)
        cbs_.onError(APIError{ErrorCode::Connection, reason});
    };
    ws_.Connect(url, std::move(wsCbs));
  }

  void Disconnect() { ws_.Disconnect(); }

  bool IsConnected() const { return ws_.IsConnected(); }

private:
  enum class State { Disconnected, Connecting, Authenticated, Subscribed };

  void OnMessage(const std::string &raw) {
    std::vector<MarketDataMsgWire> msgs;
    auto err = glz::read<glz::opts{.error_on_unknown_keys = false}>(msgs, raw);
    if (err) {
      if (cbs_.onError)
        cbs_.onError(
            APIError{ErrorCode::JSONParsing, glz::format_error(err, raw)});
      return;
    }

    for (auto &wire : msgs) {
      if (wire.type == "success") {
        if (wire.msg == "connected") {
          SendAuth();
        } else if (wire.msg == "authenticated") {
          state_ = State::Authenticated;
          if (cbs_.onConnected)
            cbs_.onConnected();
          SendSubscribe();
        }
      } else if (wire.type == "subscription") {
        state_ = State::Subscribed;
      } else if (wire.type == "error") {
        if (cbs_.onError)
          cbs_.onError(APIError{ErrorCode::Connection, wire.msg, wire.code});
      } else if (wire.type == "t") {
        if (cbs_.onTrade) {
          StreamTrade t;
          t.symbol = wire.symbol;
          t.timestamp = wire.timestamp;
          t.price = wire.price;
          t.size = wire.tradeSize;
          t.exchange = wire.exchange;
          t.tape = wire.tape;
          cbs_.onTrade(std::move(t));
        }
      } else if (wire.type == "q") {
        if (cbs_.onQuote) {
          StreamQuote q;
          q.symbol = wire.symbol;
          q.timestamp = wire.timestamp;
          q.askPrice = wire.askPrice;
          q.bidPrice = wire.bidPrice;
          q.askSize = wire.askSize;
          q.bidSize = wire.bidSize;
          cbs_.onQuote(std::move(q));
        }
      } else if (wire.type == "b") {
        if (cbs_.onBar) {
          StreamBar b;
          b.symbol = wire.symbol;
          b.timestamp = wire.timestamp;
          b.open = wire.barOpen;
          b.high = wire.barHigh;
          b.low = wire.barLow;
          b.close = wire.barClose;
          b.vwap = wire.barVwap;
          b.volume = wire.barVolume;
          b.numTrades = wire.barNumTrades;
          cbs_.onBar(std::move(b));
        }
      }
      // Unknown T values are silently ignored
    }
  }

  void SendAuth() {
    std::string msg =
        std::format(R"({{"action":"auth","key":"{}","secret":"{}"}})",
                    env_.GetID(), env_.GetSecret());
    ws_.Send(msg);
  }

  void SendSubscribe() {
    ws_.Send(std::format(
        R"({{"action":"subscribe","trades":{},"quotes":{},"bars":{}}})",
        FormatList(sub_.trades), FormatList(sub_.quotes),
        FormatList(sub_.bars)));
  }

  static std::string FormatList(const std::vector<std::string> &syms) {
    std::string result = "[";
    for (std::size_t i = 0; i < syms.size(); ++i) {
      result += '"';
      result += syms[i];
      result += '"';
      if (i + 1 < syms.size())
        result += ',';
    }
    result += ']';
    return result;
  }

  Env env_;
  Ws ws_;
  MarketDataSubscription sub_;
  MarketDataCallbacks cbs_;
  std::atomic<State> state_{State::Disconnected};
};

using MarketDataStream = MarketDataStreamT<Environment, WebSocketClient>;

} // namespace alpaca

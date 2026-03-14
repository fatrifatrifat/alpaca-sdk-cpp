#pragma once
#include <alpaca/client/environment.hpp>
#include <alpaca/client/websocketClient.hpp>
#include <alpaca/models/streaming/serialize.hpp>
#include <alpaca/models/streaming/tradeupdate.hpp>
#include <atomic>
#include <format>
#include <glaze/glaze.hpp>
#include <string>

namespace alpaca {

template <class Env = Environment, class Ws = WebSocketClient>
class TradeUpdateStreamT {
public:
  explicit TradeUpdateStreamT(const Env &env, Ws ws = Ws{})
      : env_(env), ws_(std::move(ws)) {}

  void Connect(TradeUpdateCallbacks cbs) {
    cbs_ = std::move(cbs);
    state_ = State::Connecting;

    WsCallbacks wsCbs;
    wsCbs.onOpen = [this]() { SendAuth(); };
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
    ws_.Connect(env_.GetTradeStreamUrl(), std::move(wsCbs));
  }

  void Disconnect() { ws_.Disconnect(); }

  bool IsConnected() const { return ws_.IsConnected(); }

private:
  enum class State { Disconnected, Connecting, Authenticated, Subscribed };

  void OnMessage(const std::string &raw) {
    // Step 1: determine the stream type
    TradingStreamBaseWire base;
    auto err = glz::read<glz::opts{.error_on_unknown_keys = false}>(base, raw);
    if (err) {
      if (cbs_.onError)
        cbs_.onError(
            APIError{ErrorCode::JSONParsing, glz::format_error(err, raw)});
      return;
    }

    if (base.stream == "authorization") {
      if (base.data.status == "authorized") {
        state_ = State::Authenticated;
        if (cbs_.onConnected)
          cbs_.onConnected();
        SendListen();
      } else {
        if (cbs_.onError)
          cbs_.onError(APIError{ErrorCode::Connection,
                                "Trade stream authorization failed"});
      }
    } else if (base.stream == "listening") {
      state_ = State::Subscribed;
    } else if (base.stream == "trade_updates") {
      // Step 2: full parse for trade update payload
      TradeUpdateEnvelopeWire envelope;
      auto err2 =
          glz::read<glz::opts{.error_on_unknown_keys = false}>(envelope, raw);
      if (err2) {
        if (cbs_.onError)
          cbs_.onError(
              APIError{ErrorCode::JSONParsing, glz::format_error(err2, raw)});
        return;
      }

      if (cbs_.onUpdate) {
        TradeUpdate update;
        update.event = ParseTradeUpdateEvent(envelope.data.event);
        update.at = envelope.data.at;
        update.order = envelope.data.order;
        cbs_.onUpdate(std::move(update));
      }
    }
    // Other stream types are silently ignored
  }

  void SendAuth() {
    ws_.Send(std::format(R"({{"action":"auth","key":"{}","secret":"{}"}})",
                         env_.GetID(), env_.GetSecret()));
  }

  void SendListen() {
    ws_.Send(R"({"action":"listen","data":{"streams":["trade_updates"]}})");
  }

  Env env_;
  Ws ws_;
  TradeUpdateCallbacks cbs_;
  std::atomic<State> state_{State::Disconnected};
};

using TradeUpdateStream = TradeUpdateStreamT<Environment, WebSocketClient>;

} // namespace alpaca

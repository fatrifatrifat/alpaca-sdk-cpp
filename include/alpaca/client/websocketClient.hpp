#pragma once
#include <functional>
#include <ixwebsocket/IXWebSocket.h>
#include <memory>
#include <string>

namespace alpaca {

struct WsCallbacks {
  std::function<void(const std::string &)> onMessage;
  std::function<void()> onOpen;
  std::function<void()> onClose;
  std::function<void(const std::string &)> onError;
};

// ix::WebSocket contains non-movable members (std::atomic, std::mutex, etc.).
// We wrap it in a unique_ptr so WebSocketClient itself is move-constructible.
// The lambda in setOnMessageCallback captures `this` only after Connect() is
// called, which is always after the client has reached its final address inside
// the stream object — so there is no dangling-pointer risk.
class WebSocketClient {
public:
  WebSocketClient() : ws_(std::make_unique<ix::WebSocket>()) {}

  // Move-constructible/assignable (unique_ptr is movable); no copies.
  WebSocketClient(WebSocketClient &&) = default;
  WebSocketClient &operator=(WebSocketClient &&) = default;
  WebSocketClient(const WebSocketClient &) = delete;
  WebSocketClient &operator=(const WebSocketClient &) = delete;

  void Connect(const std::string &url, WsCallbacks cbs) {
    cbs_ = std::move(cbs);
    ws_->setUrl(url);
    ws_->setOnMessageCallback([this](const ix::WebSocketMessagePtr &msg) {
      switch (msg->type) {
      case ix::WebSocketMessageType::Message:
        if (cbs_.onMessage)
          cbs_.onMessage(msg->str);
        break;
      case ix::WebSocketMessageType::Open:
        if (cbs_.onOpen)
          cbs_.onOpen();
        break;
      case ix::WebSocketMessageType::Close:
        if (cbs_.onClose)
          cbs_.onClose();
        break;
      case ix::WebSocketMessageType::Error:
        if (cbs_.onError)
          cbs_.onError(msg->errorInfo.reason);
        break;
      default:
        break;
      }
    });
    ws_->start();
  }

  void Disconnect() { ws_->stop(); }

  void Send(const std::string &msg) { ws_->send(msg); }

  bool IsConnected() const {
    return ws_->getReadyState() == ix::ReadyState::Open;
  }

private:
  std::unique_ptr<ix::WebSocket> ws_;
  WsCallbacks cbs_;
};

} // namespace alpaca

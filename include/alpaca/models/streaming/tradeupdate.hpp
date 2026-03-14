#pragma once
#include <alpaca/client/httpClient.hpp>
#include <alpaca/models/trading/order.hpp>
#include <functional>
#include <string>

namespace alpaca {

enum class TradeUpdateEvent {
  new_order,
  fill,
  partial_fill,
  canceled,
  replaced,
  rejected,
  pending_new,
  pending_cancel,
  expired,
  suspended,
  unknown
};

struct TradeUpdate {
  TradeUpdateEvent event{TradeUpdateEvent::unknown};
  std::string at;
  OrderResponse order{};
};

struct TradeUpdateCallbacks {
  std::function<void(TradeUpdate)> onUpdate;
  std::function<void(APIError)> onError;
  std::function<void()> onConnected;
  std::function<void()> onDisconnected;
};

} // namespace alpaca

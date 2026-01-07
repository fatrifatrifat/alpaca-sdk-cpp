#pragma once
#include <alpaca/models/orders/common.hpp>
#include <glaze/glaze.hpp>

namespace alpaca {

enum class OrderAssetClass { us_equity, us_option, crypto };
enum class OrderStatus {
  new_,
  partially_filled,
  filled,
  done_for_day,
  canceled,
  expired,
  replaced,
  pending_cancel,
  pending_replace,
  accepted,
  pending_new,
  accepted_for_bidding,
  stopped,
  rejected,
  suspended,
  calculated
};

struct OrderResponse {
  std::string id;
  std::string clientOrderID;

  std::string createdAt;
  std::string updatedAt;
  std::string submittedAt;

  std::optional<std::string> filledAt;
  std::optional<std::string> expiredAt;
  std::optional<std::string> canceledAt;
  std::optional<std::string> failedAt;
  std::optional<std::string> replacedAt;

  std::optional<std::string> replacedBy;
  std::optional<std::string> replaces;

  std::optional<std::string> assetID;
  std::optional<std::string> symbol;
  std::optional<std::string> assetClass;

  std::optional<std::string> notional;
  std::optional<std::string> qty;

  std::string filledQty;
  std::optional<std::string> filledAvgPrice;

  std::optional<std::string> orderClass;
  std::string orderType;
  OrderType type;
  OrderSide side;

  OrderTimeInForce timeInForce;
  std::optional<std::string> limitPrice;
  std::optional<std::string> stopPrice;

  std::string status;

  std::optional<std::string> position_intent;

  bool extendedHours{false};

  std::optional<glz::generic> legs;
  std::optional<std::string> trailPercent;
  std::optional<std::string> trailPrice;
  std::optional<std::string> hwm;

  std::optional<std::string> subtag;
  std::optional<std::string> source;
};

}; // namespace alpaca

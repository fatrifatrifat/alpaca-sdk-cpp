#pragma once
#include <alpaca/models/orders/common.hpp>
#include <glaze/glaze.hpp>

namespace alpaca {

struct OrderResponse {
  std::string id;
  std::string client_order_id;

  std::string created_at;
  std::string updated_at;
  std::string submitted_at;

  std::optional<std::string> filled_at;
  std::optional<std::string> expired_at;
  std::optional<std::string> canceled_at;
  std::optional<std::string> failed_at;
  std::optional<std::string> replaced_at;

  std::optional<std::string> replaced_by;
  std::optional<std::string> replaces;

  std::string asset_id;
  std::string symbol;
  std::string asset_class;

  std::optional<std::string> notional;

  std::string qty;
  std::string filled_qty;
  std::optional<std::string> filled_avg_price;

  std::string order_class;
  std::string order_type;
  std::string type;
  std::string side;

  std::optional<std::string> position_intent;

  std::string time_in_force;

  std::optional<std::string> limit_price;
  std::optional<std::string> stop_price;

  std::string status;

  bool extended_hours{false};

  std::optional<glz::generic> legs;
  std::optional<std::string> trail_percent;
  std::optional<std::string> trail_price;
  std::optional<std::string> hwm;

  std::optional<std::string> subtag;
  std::optional<std::string> source;

  std::optional<std::string> expires_at;
};

}; // namespace alpaca

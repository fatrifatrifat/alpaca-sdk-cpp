#pragma once
#include <glaze/glaze.hpp>

namespace alpaca {

struct OrderRequest {
  std::string symbol;
  std::string qty;
  std::string side;
  std::string type{"market"};
  std::string time_in_force{"day"};
  bool extended_hours{false};

  std::optional<std::string> limit_price;
  std::optional<std::string> stop_price;
  std::optional<std::string> client_order_id;
};

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

namespace glz {

template <> struct meta<alpaca::OrderResponse> {
  using T = alpaca::OrderResponse;
  static constexpr auto value = object(
      "id", &T::id, "client_order_id", &T::client_order_id, "created_at",
      &T::created_at, "updated_at", &T::updated_at, "submitted_at",
      &T::submitted_at, "filled_at", &T::filled_at, "expired_at",
      &T::expired_at, "canceled_at", &T::canceled_at, "failed_at",
      &T::failed_at, "replaced_at", &T::replaced_at, "replaced_by",
      &T::replaced_by, "replaces", &T::replaces, "asset_id", &T::asset_id,
      "symbol", &T::symbol, "asset_class", &T::asset_class, "notional",
      &T::notional, "qty", &T::qty, "filled_qty", &T::filled_qty,
      "filled_avg_price", &T::filled_avg_price, "order_class", &T::order_class,
      "order_type", &T::order_type, "type", &T::type, "side", &T::side,
      "position_intent", &T::position_intent, "time_in_force",
      &T::time_in_force, "limit_price", &T::limit_price, "stop_price",
      &T::stop_price, "status", &T::status, "extended_hours",
      &T::extended_hours, "legs", &T::legs, "trail_percent", &T::trail_percent,
      "trail_price", &T::trail_price, "hwm", &T::hwm, "subtag", &T::subtag,
      "source", &T::source, "expires_at", &T::expires_at);
};

template <> struct meta<alpaca::OrderRequest> {
  using T = alpaca::OrderRequest;
  static constexpr auto value =
      object("symbol", &T::symbol, "qty", &T::qty, "side", &T::side, "type",
             &T::type, "time_in_force", &T::time_in_force, "extended_hours",
             &T::extended_hours, "limit_price", &T::limit_price, "stop_price",
             &T::stop_price, "client_order_id", &T::client_order_id);
};

}; // namespace glz

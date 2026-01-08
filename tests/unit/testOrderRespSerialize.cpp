#include <alpaca/alpaca.hpp>

#include <catch2/catch_test_macros.hpp>
#include <glaze/glaze.hpp>

#include <string>

namespace {

std::string minimal_order_response_json() {
  return R"json(
  {
    "id": "order_123",
    "client_order_id": "client_abc",
    "created_at": "2026-01-07T10:00:00Z",
    "updated_at": "2026-01-07T10:00:01Z",
    "submitted_at": "2026-01-07T10:00:02Z",

    "filled_qty": "0",
    "order_type": "market",
    "type": "market",
    "side": "buy",
    "time_in_force": "day",
    "status": "new",

    "extended_hours": false
  }
  )json";
}

} // namespace

TEST_CASE(
    "Glaze OrderResponse: minimal JSON parses and populates required fields") {
  const auto json = minimal_order_response_json();

  alpaca::OrderResponse r{};
  auto ec = glz::read_json(r, json);
  REQUIRE(!ec);

  REQUIRE(r.id == "order_123");
  REQUIRE(r.clientOrderID == "client_abc");
  REQUIRE(r.createdAt == "2026-01-07T10:00:00Z");
  REQUIRE(r.updatedAt == "2026-01-07T10:00:01Z");
  REQUIRE(r.submittedAt == "2026-01-07T10:00:02Z");

  REQUIRE(r.filledQty == "0");
  REQUIRE(r.orderType == "market");
  REQUIRE(r.type == alpaca::OrderType::market);
  REQUIRE(r.side == alpaca::OrderSide::buy);
  REQUIRE(r.timeInForce == alpaca::OrderTimeInForce::day);
  REQUIRE(r.status == "new");

  REQUIRE(r.extendedHours == false);

  REQUIRE_FALSE(r.filledAt.has_value());
  REQUIRE_FALSE(r.symbol.has_value());
  REQUIRE_FALSE(r.qty.has_value());
  REQUIRE_FALSE(r.notional.has_value());
  REQUIRE_FALSE(r.legs.has_value());
}

TEST_CASE("Glaze OrderResponse: optionals parse when present") {
  const std::string json = R"json(
  {
    "id": "order_999",
    "client_order_id": "client_xyz",
    "created_at": "2026-01-07T10:00:00Z",
    "updated_at": "2026-01-07T10:00:01Z",
    "submitted_at": "2026-01-07T10:00:02Z",

    "asset_id": "asset_1",
    "symbol": "AAPL",
    "asset_class": "us_equity",

    "notional": "250.50",
    "qty": "10",
    "filled_qty": "10",
    "filled_avg_price": "251.01",

    "order_class": "simple",
    "order_type": "market",
    "type": "market",
    "side": "buy",
    "time_in_force": "day",

    "limit_price": "0",
    "stop_price": "0",

    "status": "filled",
    "position_intent": "buy_to_open",

    "extended_hours": true,

    "trail_percent": "1.5",
    "trail_price": "0.25",
    "hwm": "123.45",

    "subtag": "test",
    "source": "access_key",

    "legs": [{"id":"leg1","client_order_id":"c1"}]
  }
  )json";

  alpaca::OrderResponse r{};
  auto ec = glz::read_json(r, json);
  REQUIRE(!ec);

  REQUIRE(r.id == "order_999");
  REQUIRE(r.clientOrderID == "client_xyz");

  REQUIRE(r.assetID.has_value());
  REQUIRE(*r.assetID == "asset_1");
  REQUIRE(r.symbol.has_value());
  REQUIRE(*r.symbol == "AAPL");
  REQUIRE(r.assetClass.has_value());
  REQUIRE(*r.assetClass == "us_equity");

  REQUIRE(r.notional.has_value());
  REQUIRE(*r.notional == "250.50");
  REQUIRE(r.qty.has_value());
  REQUIRE(*r.qty == "10");

  REQUIRE(r.filledQty == "10");
  REQUIRE(r.filledAvgPrice.has_value());
  REQUIRE(*r.filledAvgPrice == "251.01");

  REQUIRE(r.orderClass.has_value());
  REQUIRE(*r.orderClass == "simple");

  REQUIRE(r.orderType == "market");
  REQUIRE(r.type == alpaca::OrderType::market);
  REQUIRE(r.side == alpaca::OrderSide::buy);
  REQUIRE(r.timeInForce == alpaca::OrderTimeInForce::day);

  REQUIRE(r.limitPrice.has_value());
  REQUIRE(*r.limitPrice == "0");
  REQUIRE(r.stopPrice.has_value());
  REQUIRE(*r.stopPrice == "0");

  REQUIRE(r.status == "filled");

  REQUIRE(r.position_intent.has_value());
  REQUIRE(*r.position_intent == "buy_to_open");

  REQUIRE(r.extendedHours == true);

  REQUIRE(r.trailPercent.has_value());
  REQUIRE(*r.trailPercent == "1.5");
  REQUIRE(r.trailPrice.has_value());
  REQUIRE(*r.trailPrice == "0.25");
  REQUIRE(r.hwm.has_value());
  REQUIRE(*r.hwm == "123.45");

  REQUIRE(r.subtag.has_value());
  REQUIRE(*r.subtag == "test");
  REQUIRE(r.source.has_value());
  REQUIRE(*r.source == "access_key");

  REQUIRE(r.legs.has_value());
}

TEST_CASE("Glaze OrderResponse: serialize then deserialize preserves fields "
          "(roundtrip)") {
  alpaca::OrderResponse in{};
  in.id = "order_roundtrip";
  in.clientOrderID = "client_roundtrip";

  in.createdAt = "2026-01-07T10:00:00Z";
  in.updatedAt = "2026-01-07T10:00:01Z";
  in.submittedAt = "2026-01-07T10:00:02Z";

  in.symbol = "AAPL";
  in.qty = "10";
  in.filledQty = "10";

  in.orderType = "market";
  in.type = alpaca::OrderType::market;
  in.side = alpaca::OrderSide::buy;

  in.timeInForce = alpaca::OrderTimeInForce::day;
  in.status = "filled";
  in.extendedHours = false;

  std::string json;
  auto w = glz::write_json(in, json);
  REQUIRE(!w);
  REQUIRE(!json.empty());

  alpaca::OrderResponse out{};
  auto r = glz::read_json(out, json);
  REQUIRE(!r);

  REQUIRE(out.id == in.id);
  REQUIRE(out.clientOrderID == in.clientOrderID);
  REQUIRE(out.createdAt == in.createdAt);
  REQUIRE(out.updatedAt == in.updatedAt);
  REQUIRE(out.submittedAt == in.submittedAt);

  REQUIRE(out.symbol == in.symbol);
  REQUIRE(out.qty == in.qty);
  REQUIRE(out.filledQty == in.filledQty);

  REQUIRE(out.orderType == in.orderType);
  REQUIRE(out.type == in.type);
  REQUIRE(out.side == in.side);

  REQUIRE(out.timeInForce == in.timeInForce);
  REQUIRE(out.status == in.status);
  REQUIRE(out.extendedHours == in.extendedHours);

  REQUIRE(json.find("\"client_order_id\"") != std::string::npos);
  REQUIRE(json.find("\"time_in_force\"") != std::string::npos);
  REQUIRE(json.find("\"filled_qty\"") != std::string::npos);
  REQUIRE(json.find("\"order_type\"") != std::string::npos);
}

TEST_CASE("Glaze OrderResponse: missing required fields fails parsing") {
  const std::string bad = R"json(
  {
    "client_order_id": "x"
  }
  )json";

  alpaca::OrderResponse r{};
  auto ec = glz::read<glz::opts{.error_on_missing_keys = true}>(r, bad);

  REQUIRE(ec);
  REQUIRE(ec.ec == glz::error_code::missing_key);
}

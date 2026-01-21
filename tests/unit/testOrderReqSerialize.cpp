#include <alpaca/models/trading/serialize.hpp>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glaze/glaze.hpp>

#include <string>
#include <variant>

namespace {

alpaca::OrderRequestParam base_req() {
  alpaca::OrderRequestParam r{};
  r.symbol = "AAPL";
  r.side = alpaca::OrderSide::buy;
  r.type = alpaca::OrderType::market;
  r.timeInForce = alpaca::OrderTimeInForce::day;
  return r;
}

std::string write_json_or_fail(const alpaca::OrderRequestParam &req) {
  std::string json;
  auto ec = glz::write_json(req, json);
  REQUIRE(!ec);
  REQUIRE(!json.empty());
  return json;
}

alpaca::OrderRequestParam read_json_or_fail(const std::string &json) {
  alpaca::OrderRequestParam out{};
  auto ec = glz::read_json(out, json);
  REQUIRE(!ec);
  return out;
}

} // namespace

TEST_CASE(
    "Glaze OrderRequestParam serialization uses WIRE schema (no amt/trail_amt "
    "keys)") {
  auto req = base_req();
  req.amt = alpaca::Quantity{10};
  req.trailAmt = alpaca::TrailAmount{alpaca::TrailPrice{0.25L}};

  const auto json = write_json_or_fail(req);

  REQUIRE(json.find("\"qty\"") != std::string::npos);
  REQUIRE(json.find("\"trail_price\"") != std::string::npos);
  REQUIRE(json.find("\"amt\"") == std::string::npos);
  REQUIRE(json.find("\"trail_amt\"") == std::string::npos);
  REQUIRE(json.find("\"time_in_force\"") != std::string::npos);
}

TEST_CASE("Glaze boundary: Quantity -> JSON -> OrderRequestParam roundtrip") {
  auto req = base_req();
  req.amt = alpaca::Quantity{10};

  const auto json = write_json_or_fail(req);
  const auto back = read_json_or_fail(json);

  REQUIRE(back.symbol == req.symbol);
  REQUIRE(back.side == req.side);
  REQUIRE(back.type == req.type);
  REQUIRE(back.timeInForce == req.timeInForce);

  REQUIRE(std::holds_alternative<alpaca::Quantity>(back.amt));
  REQUIRE(static_cast<double>(std::get<alpaca::Quantity>(back.amt).v) ==
          Catch::Approx(10.0));

  REQUIRE(json.find("\"qty\"") != std::string::npos);
  REQUIRE(json.find("\"notional\"") == std::string::npos);
}

TEST_CASE("Glaze boundary: Notional -> JSON -> OrderRequestParam roundtrip") {
  auto req = base_req();
  req.amt = alpaca::Notional{250.50L};

  const auto json = write_json_or_fail(req);
  const auto back = read_json_or_fail(json);

  REQUIRE(std::holds_alternative<alpaca::Notional>(back.amt));
  REQUIRE(static_cast<double>(std::get<alpaca::Notional>(back.amt).v) ==
          Catch::Approx(250.50));

  REQUIRE(json.find("\"notional\"") != std::string::npos);
  REQUIRE(json.find("\"qty\"") == std::string::npos);
}

TEST_CASE("Glaze boundary: optionals serialize + roundtrip correctly") {
  auto req = base_req();
  req.amt = alpaca::Quantity{1};

  req.extendedHours = false;
  req.clientOrderID = std::string{"client-123"};
  req.orderClass = alpaca::OrderClass::simple;

  const auto json = write_json_or_fail(req);
  const auto back = read_json_or_fail(json);

  REQUIRE(back.extendedHours == req.extendedHours);
  REQUIRE(back.clientOrderID == req.clientOrderID);
  REQUIRE(back.orderClass == req.orderClass);

  REQUIRE(json.find("\"extended_hours\"") != std::string::npos);
  REQUIRE(json.find("\"client_order_id\"") != std::string::npos);
  REQUIRE(json.find("\"order_class\"") != std::string::npos);
}

TEST_CASE("Glaze boundary: trail_price and trail_percent mapping roundtrips") {
  SECTION("trail_price") {
    auto req = base_req();
    req.amt = alpaca::Quantity{1};
    req.trailAmt = alpaca::TrailAmount{alpaca::TrailPrice{0.25L}};

    const auto json = write_json_or_fail(req);
    const auto back = read_json_or_fail(json);

    REQUIRE(back.trailAmt.has_value());
    REQUIRE(std::holds_alternative<alpaca::TrailPrice>(*back.trailAmt));
    REQUIRE(
        static_cast<double>(std::get<alpaca::TrailPrice>(*back.trailAmt).v) ==
        Catch::Approx(0.25));

    REQUIRE(json.find("\"trail_price\"") != std::string::npos);
    REQUIRE(json.find("\"trail_percent\"") == std::string::npos);
  }

  SECTION("trail_percent") {
    auto req = base_req();
    req.amt = alpaca::Quantity{1};
    req.trailAmt = alpaca::TrailAmount{alpaca::TrailPercent{1.5L}};

    const auto json = write_json_or_fail(req);
    const auto back = read_json_or_fail(json);

    REQUIRE(back.trailAmt.has_value());
    REQUIRE(std::holds_alternative<alpaca::TrailPercent>(*back.trailAmt));
    REQUIRE(
        static_cast<double>(std::get<alpaca::TrailPercent>(*back.trailAmt).v) ==
        Catch::Approx(1.5));

    REQUIRE(json.find("\"trail_percent\"") != std::string::npos);
    REQUIRE(json.find("\"trail_price\"") == std::string::npos);
  }
}

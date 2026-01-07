#include <alpaca/alpaca.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glaze/glaze.hpp>

#include <map>
#include <string>
#include <vector>

namespace {

std::string bar_json_one() {
  return R"json(
  {
    "c": 155.25,
    "h": 156.00,
    "l": 154.10,
    "n": 123,
    "o": 154.50,
    "t": "2024-01-03T00:00:00Z",
    "v": 99999,
    "vw": 155.12
  }
  )json";
}

alpaca::Bar make_bar() {
  alpaca::Bar b{};
  b.close = 155.25;
  b.high = 156.00;
  b.low = 154.10;
  b.number_of_trades = 123;
  b.open = 154.50;
  b.timestamp = "2024-01-03T00:00:00Z";
  b.volume = 99999;
  b.volume_weigted_price = 155.12;
  return b;
}

std::string bars_json_map() {
  return R"json(
  {
    "bars": {
      "AAPL": [
        { "c": 155.25, "h": 156.00, "l": 154.10, "n": 123, "o": 154.50, "t": "2024-01-03T00:00:00Z", "v": 99999, "vw": 155.12 },
        { "c": 156.10, "h": 157.00, "l": 155.00, "n": 200, "o": 155.50, "t": "2024-01-03T00:05:00Z", "v": 50000, "vw": 156.00 }
      ],
      "MSFT": [
        { "c": 300.00, "h": 301.00, "l": 299.00, "n": 50, "o": 299.50, "t": "2024-01-03T00:00:00Z", "v": 1000, "vw": 300.10 }
      ]
    },
    "next_page_token": "TOKEN_123"
  }
  )json";
}

std::string latest_bars_json() {
  return R"json(
  {
    "bars": {
      "AAPL": { "c": 155.25, "h": 156.00, "l": 154.10, "n": 123, "o": 154.50, "t": "2024-01-03T00:00:00Z", "v": 99999, "vw": 155.12 },
      "MSFT": { "c": 300.00, "h": 301.00, "l": 299.00, "n": 50,  "o": 299.50, "t": "2024-01-03T00:00:00Z", "v": 1000,  "vw": 300.10 }
    }
  }
  )json";
}

} // namespace

TEST_CASE("Glaze Bar: parse compact bar JSON (c/h/l/n/o/t/v/vw)") {
  const auto json = bar_json_one();

  alpaca::Bar b{};
  auto ec = glz::read_json(b, json);
  REQUIRE(!ec);

  REQUIRE(b.close == Catch::Approx(155.25));
  REQUIRE(b.high == Catch::Approx(156.00));
  REQUIRE(b.low == Catch::Approx(154.10));
  REQUIRE(b.number_of_trades == 123);
  REQUIRE(b.open == Catch::Approx(154.50));
  REQUIRE(b.timestamp == "2024-01-03T00:00:00Z");
  REQUIRE(b.volume == 99999);
  REQUIRE(b.volume_weigted_price == Catch::Approx(155.12));
}

TEST_CASE("Glaze Bar: serialize -> deserialize roundtrip preserves fields") {
  const auto in = make_bar();

  std::string json;
  auto w = glz::write_json(in, json);
  REQUIRE(!w);
  REQUIRE(!json.empty());

  REQUIRE(json.find("\"c\"") != std::string::npos);
  REQUIRE(json.find("\"vw\"") != std::string::npos);
  REQUIRE(json.find("\"t\"") != std::string::npos);

  alpaca::Bar out{};
  auto r = glz::read_json(out, json);
  REQUIRE(!r);

  REQUIRE(out.close == Catch::Approx(in.close));
  REQUIRE(out.high == Catch::Approx(in.high));
  REQUIRE(out.low == Catch::Approx(in.low));
  REQUIRE(out.number_of_trades == in.number_of_trades);
  REQUIRE(out.open == Catch::Approx(in.open));
  REQUIRE(out.timestamp == in.timestamp);
  REQUIRE(out.volume == in.volume);
  REQUIRE(out.volume_weigted_price == Catch::Approx(in.volume_weigted_price));
}

TEST_CASE("Glaze Bars: parses bars map + next_page_token") {
  const auto json = bars_json_map();

  alpaca::Bars bs{};
  auto ec = glz::read_json(bs, json);
  REQUIRE(!ec);

  REQUIRE(bs.bars.size() == 2);
  REQUIRE(bs.bars.count("AAPL") == 1);
  REQUIRE(bs.bars.count("MSFT") == 1);

  REQUIRE(bs.bars.at("AAPL").size() == 2);
  REQUIRE(bs.bars.at("MSFT").size() == 1);

  REQUIRE(bs.bars.at("AAPL")[0].timestamp == "2024-01-03T00:00:00Z");
  REQUIRE(bs.bars.at("AAPL")[1].timestamp == "2024-01-03T00:05:00Z");

  REQUIRE(bs.next_page_token.has_value());
  REQUIRE(*bs.next_page_token == "TOKEN_123");
}

TEST_CASE("Glaze Bars: serialize -> deserialize roundtrip preserves map keys "
          "and token") {
  alpaca::Bars in{};
  in.bars["AAPL"] = {make_bar()};
  in.next_page_token = std::string{"TOKEN_ABC"};

  std::string json;
  auto w = glz::write_json(in, json);
  REQUIRE(!w);
  REQUIRE(!json.empty());

  REQUIRE(json.find("\"bars\"") != std::string::npos);
  REQUIRE(json.find("\"next_page_token\"") != std::string::npos);
  REQUIRE(json.find("AAPL") != std::string::npos);

  alpaca::Bars out{};
  auto r = glz::read_json(out, json);
  REQUIRE(!r);

  REQUIRE(out.bars.count("AAPL") == 1);
  REQUIRE(out.bars.at("AAPL").size() == 1);
  REQUIRE(out.next_page_token == in.next_page_token);
}

TEST_CASE("Glaze LatestBars: parses symbol->Bar map") {
  const auto json = latest_bars_json();

  alpaca::LatestBars lb{};
  auto ec = glz::read_json(lb, json);
  REQUIRE(!ec);

  REQUIRE(lb.bars.size() == 2);
  REQUIRE(lb.bars.count("AAPL") == 1);
  REQUIRE(lb.bars.count("MSFT") == 1);

  REQUIRE(lb.bars.at("AAPL").close == Catch::Approx(155.25));
  REQUIRE(lb.bars.at("MSFT").close == Catch::Approx(300.00));
}

TEST_CASE("Glaze enums: invalid BarFeed/BarAdjustment/BarSort fail parsing") {
  SECTION("BarFeed invalid") {
    alpaca::BarFeed f{};
    auto ec = glz::read_json(f, R"json("NOT_REAL")json");
    REQUIRE(ec);
  }

  SECTION("BarAdjustment invalid") {
    alpaca::BarAdjustment a{};
    auto ec = glz::read_json(a, R"json("NOT_REAL")json");
    REQUIRE(ec);
  }

  SECTION("BarSort invalid") {
    alpaca::BarSort s{};
    auto ec = glz::read_json(s, R"json("NOT_REAL")json");
    REQUIRE(ec);
  }
}

TEST_CASE("Glaze Bar: missing keys fails parsing (strict missing keys)") {
  const std::string incomplete = R"json({ "c": 1.0 })json";

  alpaca::Bar b{};
  auto ec = glz::read<glz::opts{.error_on_missing_keys = true}>(b, incomplete);
  REQUIRE(ec);
  REQUIRE(ec.ec == glz::error_code::missing_key);
}

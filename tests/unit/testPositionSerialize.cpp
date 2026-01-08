#include <alpaca/alpaca.hpp>

#include <catch2/catch_test_macros.hpp>
#include <glaze/glaze.hpp>

#include <string>

namespace {

std::string minimal_position_json() {
  return R"json(
  {
    "asset_id": "asset_1",
    "symbol": "AAPL",
    "exchange": "NASDAQ",
    "asset_class": "us_equity",
    "asset_marginable": true,

    "qty": "10",
    "avg_entry_price": "150.00",
    "side": "long",
    "market_value": "1550.00",
    "cost_basis": "1500.00",

    "unrealized_pl": "50.00",
    "unrealized_plpc": "0.0333",
    "unrealized_intraday_pl": "10.00",
    "unrealized_intraday_plpc": "0.0066",

    "current_price": "155.00",
    "lastday_price": "154.00",
    "change_today": "0.0065",

    "qty_available": "10"
  }
  )json";
}

alpaca::Position make_position_full() {
  alpaca::Position p{};
  p.assetID = "asset_1";
  p.symbol = "AAPL";
  p.exchange = alpaca::PositionExchange::NASDAQ;
  p.assetClass = "us_equity";
  p.assetMarginable = true;

  p.qty = "10";
  p.avgEntryPrice = "150.00";
  p.side = "long";
  p.marketValue = "1550.00";
  p.costBasis = "1500.00";

  p.unrealizedPL = "50.00";
  p.unrealizedPLPC = "0.0333";
  p.unrealizedIntradayPL = "10.00";
  p.unrealizedIntradayPLPC = "0.0066";

  p.currentPrice = "155.00";
  p.lastDayPrice = "154.00";
  p.changeToday = "0.0065";

  p.qtyAvailable = "10";
  return p;
}

} // namespace

TEST_CASE("Glaze Position: minimal JSON parses and populates fields") {
  const auto json = minimal_position_json();

  alpaca::Position p{};
  auto ec = glz::read_json(p, json);
  REQUIRE(!ec);

  REQUIRE(p.assetID == "asset_1");
  REQUIRE(p.symbol == "AAPL");
  REQUIRE(p.exchange == alpaca::PositionExchange::NASDAQ);
  REQUIRE(p.assetClass == "us_equity");
  REQUIRE(p.assetMarginable == true);

  REQUIRE(p.qty == "10");
  REQUIRE(p.avgEntryPrice == "150.00");
  REQUIRE(p.side == "long");
  REQUIRE(p.marketValue == "1550.00");
  REQUIRE(p.costBasis == "1500.00");

  REQUIRE(p.unrealizedPL == "50.00");
  REQUIRE(p.unrealizedPLPC == "0.0333");
  REQUIRE(p.unrealizedIntradayPL == "10.00");
  REQUIRE(p.unrealizedIntradayPLPC == "0.0066");

  REQUIRE(p.currentPrice == "155.00");
  REQUIRE(p.lastDayPrice == "154.00");
  REQUIRE(p.changeToday == "0.0065");

  REQUIRE(p.qtyAvailable == "10");
}

TEST_CASE(
    "Glaze Position: serialize -> deserialize roundtrip preserves fields") {
  const auto in = make_position_full();

  std::string json;
  auto w = glz::write_json(in, json);
  REQUIRE(!w);
  REQUIRE(!json.empty());

  REQUIRE(json.find("\"asset_id\"") != std::string::npos);
  REQUIRE(json.find("\"avg_entry_price\"") != std::string::npos);
  REQUIRE(json.find("\"lastday_price\"") != std::string::npos);
  REQUIRE(json.find("\"qty_available\"") != std::string::npos);
  REQUIRE(json.find("NASDAQ") != std::string::npos);

  alpaca::Position out{};
  auto r = glz::read_json(out, json);
  REQUIRE(!r);

  REQUIRE(out.assetID == in.assetID);
  REQUIRE(out.symbol == in.symbol);
  REQUIRE(out.exchange == in.exchange);
  REQUIRE(out.assetClass == in.assetClass);
  REQUIRE(out.assetMarginable == in.assetMarginable);

  REQUIRE(out.qty == in.qty);
  REQUIRE(out.avgEntryPrice == in.avgEntryPrice);
  REQUIRE(out.side == in.side);
  REQUIRE(out.marketValue == in.marketValue);
  REQUIRE(out.costBasis == in.costBasis);

  REQUIRE(out.unrealizedPL == in.unrealizedPL);
  REQUIRE(out.unrealizedPLPC == in.unrealizedPLPC);
  REQUIRE(out.unrealizedIntradayPL == in.unrealizedIntradayPL);
  REQUIRE(out.unrealizedIntradayPLPC == in.unrealizedIntradayPLPC);

  REQUIRE(out.currentPrice == in.currentPrice);
  REQUIRE(out.lastDayPrice == in.lastDayPrice);
  REQUIRE(out.changeToday == in.changeToday);

  REQUIRE(out.qtyAvailable == in.qtyAvailable);
}

TEST_CASE("Glaze Position: invalid PositionExchange fails parsing") {
  const std::string bad = R"json(
  {
    "asset_id": "asset_1",
    "symbol": "AAPL",
    "exchange": "NOT_REAL",
    "asset_class": "us_equity",
    "asset_marginable": true,

    "qty": "10",
    "avg_entry_price": "150.00",
    "side": "long",
    "market_value": "1550.00",
    "cost_basis": "1500.00",

    "unrealized_pl": "50.00",
    "unrealized_plpc": "0.0333",
    "unrealized_intraday_pl": "10.00",
    "unrealized_intraday_plpc": "0.0066",

    "current_price": "155.00",
    "lastday_price": "154.00",
    "change_today": "0.0065",

    "qty_available": "10"
  }
  )json";

  alpaca::Position p{};
  auto ec = glz::read_json(p, bad);
  REQUIRE(ec);
}

TEST_CASE("Glaze Positions (vector): parses array of positions") {
  const std::string json = R"json(
  [
    {
      "asset_id": "asset_1",
      "symbol": "AAPL",
      "exchange": "NASDAQ",
      "asset_class": "us_equity",
      "asset_marginable": true,
      "qty": "10",
      "avg_entry_price": "150.00",
      "side": "long",
      "market_value": "1550.00",
      "cost_basis": "1500.00",
      "unrealized_pl": "50.00",
      "unrealized_plpc": "0.0333",
      "unrealized_intraday_pl": "10.00",
      "unrealized_intraday_plpc": "0.0066",
      "current_price": "155.00",
      "lastday_price": "154.00",
      "change_today": "0.0065",
      "qty_available": "10"
    },
    {
      "asset_id": "asset_2",
      "symbol": "MSFT",
      "exchange": "NYSE",
      "asset_class": "us_equity",
      "asset_marginable": true,
      "qty": "5",
      "avg_entry_price": "300.00",
      "side": "long",
      "market_value": "1520.00",
      "cost_basis": "1500.00",
      "unrealized_pl": "20.00",
      "unrealized_plpc": "0.0133",
      "unrealized_intraday_pl": "5.00",
      "unrealized_intraday_plpc": "0.0033",
      "current_price": "304.00",
      "lastday_price": "303.00",
      "change_today": "0.0033",
      "qty_available": "5"
    }
  ]
  )json";

  alpaca::Positions ps{};
  auto ec = glz::read_json(ps, json);
  REQUIRE(!ec);

  REQUIRE(ps.size() == 2);
  REQUIRE(ps[0].symbol == "AAPL");
  REQUIRE(ps[0].exchange == alpaca::PositionExchange::NASDAQ);
  REQUIRE(ps[1].symbol == "MSFT");
  REQUIRE(ps[1].exchange == alpaca::PositionExchange::NYSE);
}

TEST_CASE("Glaze Position: missing required fields fails parsing (strict "
          "missing keys)") {
  const std::string incomplete = R"json({ "symbol": "AAPL" })json";

  alpaca::Position p{};
  auto ec = glz::read<glz::opts{.error_on_missing_keys = true}>(p, incomplete);

  REQUIRE(ec);
  REQUIRE(ec.ec == glz::error_code::missing_key);
}

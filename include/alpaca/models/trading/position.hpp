#pragma once
#include <glaze/glaze.hpp>

namespace alpaca {

enum class PositionExchange { AMEX, ARCA, BATS, NYSE, NASDAQ, NYSEARCA, OTC };

struct Position {
  std::string assetID;
  std::string symbol;
  PositionExchange exchange;
  std::string assetClass;
  bool assetMarginable{};

  std::string qty;
  std::string avgEntryPrice;
  std::string side;
  std::string marketValue;
  std::string costBasis;

  std::string unrealizedPL;
  std::string unrealizedPLPC;
  std::string unrealizedIntradayPL;
  std::string unrealizedIntradayPLPC;

  std::string currentPrice;
  std::string lastDayPrice;
  std::string changeToday;

  std::string qtyAvailable;
};

using Positions = std::vector<Position>;

struct Shares {
  long double value{};
};

struct Percent {
  long double value{};
};

using LiquidationAmount = std::variant<Shares, Percent>;

struct ClosePositionParams {
  std::string symbol_or_asset_id;
  LiquidationAmount amt;
};

}; // namespace alpaca

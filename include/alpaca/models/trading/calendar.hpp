#pragma once
#include <glaze/glaze.hpp>

namespace alpaca {

struct CalendarRequest {
  std::string start{};
  std::string end{};
  std::string dateType{};
};

struct Calendar {
  std::string close;
  std::string date;
  std::string open;
  std::optional<std::string> session_close;
  std::optional<std::string> session_open;
  std::string settlementDate;
};

using CalendarResponse = std::vector<Calendar>;

}; // namespace alpaca

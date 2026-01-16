#pragma once
#include <glaze/glaze.hpp>

namespace alpaca {

struct Clock {
  std::string timestamp;
  bool isOpen;
  std::string nextOpen;
  std::string nextClose;
};

}; // namespace alpaca

#pragma once

namespace alpaca::utils {

inline bool is_success(int s) {
  return 200 <= s &&s < 300;
}

};

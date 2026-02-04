# Alpaca C++ SDK (Modern)

A modern, strongly-typed C++ SDK for the Alpaca Trading API and Market Data API, built with clarity, correctness, and modern C++ design in mind.

This project is intentionally small, explicit, and predictable. This project was originally designed to be used for [QUARCC](https://quarcc.com/) as a more modern and maintained replacement to the already existing [community SDK](https://github.com/marpaia/alpaca-trade-api-cpp) 

---

## Key Features

- **Modern C++ (C++23+)**
  - `std::expected` for explicit error handling
  - `std::optional`, `std::variant`, `std::format`, `std::print`
- **Strong typing**
  - Typed request / response models
  - No stringly-typed JSON at call sites
- **Explicit architecture**
  - `Environment` for configuration
  - `HttpClient` for transport
  - `TradingClient` and `MarketDataClient` for domain logic
- **Deterministic behavior**
  - No hidden retries
  - No global state
  - No exceptions thrown by the SDK
- **Automatic pagination**
  - Market-data endpoints aggregate results safely

---

## Authentication

The SDK reads credentials and endpoints from environment variables:

```bash
export APCA_API_KEY_ID="your_key"
export APCA_API_SECRET_KEY="your_secret"
export APCA_API_BASE_URL="paper-api.alpaca.markets"
export APCA_API_DATA_URL="data.alpaca.markets"
```

## Usage

Include the main header:
```cpp
#include <alpaca/alpaca.hpp>
```

Main entry points:

- `alpaca::TradingClient`
- `alpaca::MarketDataClient`

All API calls return:

```cpp
std::expected<T, alpaca::APIError>
```

No exceptions are thrown by the SDK.

## Examples

Get Account:
```cpp
#include <alpaca/alpaca.hpp>
using namespace alpaca;

int main() {
  Environment env{};
  TradingClient trade{env};

  auto acc = trade.GetAccount();
  if (!acc) {
    std::println("{}", acc.error());
    return 1;
  }

  std::println("id={} cash={} buying_power={}",
               acc->id, acc->cash, acc->buying_power);
}
```

---

Place a Market Order:

```cpp
#include <alpaca/alpaca.hpp>
using namespace alpaca;

int main() {
  OrderRequestParam r{};
  r.symbol = "AAPL";
  r.amt = Quantity{1};
  r.side = OrderSide::buy;
  r.type = OrderType::market;
  r.timeInForce = OrderTimeInForce::day;

  Environment env{};
  TradingClient trade{env};

  auto resp = trade.SubmitOrder(r);
  if (!resp) {
    std::println("{}", resp.error());
    return 1;
  }

  std::println("Order submitted: id={} status={}",
               resp->id, resp->status);
}

```

Other examples can be found in the [examples](./examples) folder. For extensive documentation on the Alpaca API itself, check the [official Alpaca API Reference](https://docs.alpaca.markets/reference)

---

## Goals

The future plans for this SDK is to add:

- Streaming / WebSocket APIs
- Async or coroutine interfaces
- Automatic retry / backoff logic

## Install

**Prerequesites**
- C++ 23
- OpenSSL
- CMake 3.23 at least

**Linux:**

Installing the library

```bash
git clone https://github.com/fatrifatrifat/alpaca-sdk-cpp"
cd alpaca-sdk-cpp/

cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$HOME/.local \
  -DALPACA_BUILD_EXAMPLES=OFF \
  -DALPACA_BUILD_TESTS=OFF \
  -DALPACA_ENABLE_SSL=ON

cmake --build build -j
cmake  --install build 
```

After install, the package config will typically be located at:

```bash
$HOME/.local/lib/cmake/alpaca_sdk/
```

Create your CMakeLists.txt file:

```cmake
cmake_minimum_required(VERSION 3.23)
project(my_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# If you installed into a custom prefix (like ~/.local),
# add it so CMake can locate alpaca_sdkConfig.cmake
list(APPEND CMAKE_PREFIX_PATH "$ENV{HOME}/.local")

find_package(alpaca_sdk CONFIG REQUIRED)

add_executable(my_app src/main.cpp)
target_link_libraries(my_app PRIVATE alpaca::alpaca_sdk)
```

Include in your main file:

```cpp
#include <alpaca/alpaca.hpp>

int main() {
  // ...
}
```

Build and run your project:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

./build/my_app
```

## Used Dependencies

  - [`cpp-httplib`](https://github.com/yhirose/cpp-httplib)
  - [`glaze`](https://github.com/stephenberry/glaze)

## License

[MIT](https://choosealicense.com/licenses/mit/)

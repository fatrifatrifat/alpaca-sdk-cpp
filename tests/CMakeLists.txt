include(CTest)
include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.2
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Catch2)

add_executable(alpaca_tests
  unit/testOrderReqSerialize.cpp
  unit/testOrderRespSerialize.cpp
  unit/testAccountSerialize.cpp
  unit/testBarsSerialize.cpp
  unit/testPositionSerialize.cpp
  unit/testTradingClient.cpp
  unit/testMarketClient.cpp
)

target_link_libraries(alpaca_tests
  PRIVATE
    alpaca_sdk
    Catch2::Catch2WithMain
)

target_compile_features(alpaca_tests PRIVATE cxx_std_23)

include(Catch)
catch_discover_tests(alpaca_tests)

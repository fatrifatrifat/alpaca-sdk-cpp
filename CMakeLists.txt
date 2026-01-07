cmake_minimum_required(VERSION 3.23)

project(alpaca_sdk VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
  add_compile_options(/Zc:preprocessor)
endif()

option(ALPACA_ENABLE_SSL "Enable HTTPS support (OpenSSL) for cpp-httplib" ON)
option(ALPACA_BUILD_PLAYGROUND "Build playground.cpp executable" ON)
option(ALPACA_BUILD_EXAMPLES "Build examples/ executables" ON)
option(ALPACA_BUILD_TESTS "Build unit/integration tests" ON)
if (ALPACA_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

include(FetchContent)

FetchContent_Declare(
  glaze
  GIT_REPOSITORY https://github.com/stephenberry/glaze.git
  GIT_TAG        main
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(glaze)

FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG        master
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(httplib)

if (ALPACA_ENABLE_SSL)
  find_package(OpenSSL REQUIRED)
endif()

add_library(alpaca_sdk INTERFACE)

target_include_directories(alpaca_sdk
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(alpaca_sdk INTERFACE cxx_std_23)

target_link_libraries(alpaca_sdk
  INTERFACE
    glaze::glaze
    httplib::httplib
)

if (ALPACA_ENABLE_SSL)
  target_compile_definitions(alpaca_sdk INTERFACE CPPHTTPLIB_OPENSSL_SUPPORT)
  target_link_libraries(alpaca_sdk INTERFACE OpenSSL::SSL OpenSSL::Crypto)
endif()

if (ALPACA_BUILD_PLAYGROUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/playground.cpp")
  add_executable(playground playground.cpp)
  target_link_libraries(playground PRIVATE alpaca_sdk)
  target_compile_features(playground PRIVATE cxx_std_23)
endif()

if (ALPACA_BUILD_EXAMPLES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
  add_subdirectory(examples)
endif()

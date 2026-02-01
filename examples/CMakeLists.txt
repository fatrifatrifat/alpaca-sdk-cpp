function(alpaca_add_example name)
  add_executable(${name} ${ARGN})
  target_link_libraries(${name} PRIVATE alpaca_sdk)
  target_compile_features(${name} PRIVATE cxx_std_23)
endfunction()

alpaca_add_example(example_place_order place_order.cpp)

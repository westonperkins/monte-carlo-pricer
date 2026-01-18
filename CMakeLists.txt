cmake_minimum_required(VERSION 3.14)
project(monte_carlo_pricer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create the pricing library
add_library(mc_pricer
    mc_pricer.cpp
)

target_include_directories(mc_pricer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Create executable
add_executable(mc_test
    main.cpp
)

target_link_libraries(mc_test
    mc_pricer
)

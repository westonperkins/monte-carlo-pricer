set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.14)
project(monte_carlo_pricer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(pybind11 REQUIRED)

# Core pricing library
add_library(mc_pricer
    mc_pricer.cpp
)

target_include_directories(mc_pricer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Python module
pybind11_add_module(mc_pricer_py
    bindings.cpp
    mc_pricer.cpp
)

target_include_directories(mc_pricer_py PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Test executable (optional, but we keep it)
add_executable(mc_test
    main.cpp
)

target_link_libraries(mc_test
    mc_pricer
)

cmake_minimum_required(VERSION 3.14)

project(monte_carlo_pricer LANGUAGES CXX)

# ---------------------------------------
# General C++ settings
# ---------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile_commands.json (for VS Code / clangd)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------
# Python / pybind11 configuration (IMPORTANT)
# ---------------------------------------

# Force pybind11 to use modern FindPython
set(PYBIND11_FINDPYTHON ON)

# Find the same Python that `python3` refers to
find_package(Python REQUIRED COMPONENTS Interpreter Development)

# Find pybind11 (installed via Homebrew)
find_package(pybind11 REQUIRED)

# ---------------------------------------
# Core pricing library (C++)
# ---------------------------------------
add_library(mc_pricer
    mc_pricer.cpp
)

target_include_directories(mc_pricer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ---------------------------------------
# Python module (pybind11)
# ---------------------------------------
pybind11_add_module(mc_pricer_py
    bindings.cpp
    mc_pricer.cpp
)

target_include_directories(mc_pricer_py PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ---------------------------------------
# Test executable (C++)
# ---------------------------------------
add_executable(mc_test
    main.cpp
)

target_link_libraries(mc_test
    mc_pricer
)

# Monte Carlo Options Analyzer

A Monte Carlo options pricing engine built with C++ (pybind11) and a Flask web GUI. Fetches live market data via yfinance, prices options using Monte Carlo simulation with antithetic variates, and computes risk metrics.

## Prerequisites

- Python 3.10+
- CMake 3.14+
- A C++17 compiler (clang or gcc)
- pybind11

### macOS (Homebrew)

```bash
brew install cmake pybind11
```

### Linux (apt)

```bash
sudo apt-get install build-essential cmake git
pip install pybind11[global]
```

## Setup

### 1. Install Python dependencies

```bash
pip install -r requirements.txt
```

### 2. Build the C++ pricing engine

```bash
mkdir -p build && cd build
cmake ..
make
cd ..
```

### 3. Run the web app

```bash
python web/app.py
```

Open **http://localhost:5050** in your browser.

## Docker

```bash
docker build -t monte-carlo .
docker run -p 5050:5050 monte-carlo
```

Then open **http://localhost:5050**.

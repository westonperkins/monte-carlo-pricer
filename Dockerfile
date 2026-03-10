FROM python:3.13-slim

# install build tools and cmake
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git \
    && rm -rf /var/lib/apt/lists/*

# install pybind11
RUN pip install --no-cache-dir pybind11[global]

WORKDIR /app

# copy source
COPY mc_pricer.h mc_pricer.cpp payoff.h payoffs.h bindings.cpp main.cpp CMakeLists.txt ./

# build C++ engine
RUN mkdir build && cd build && \
    cmake .. -DPYBIND11_FINDPYTHON=ON && \
    cmake --build . --parallel

# install python deps
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# copy python code and web app
COPY python/ ./python/
COPY web/ ./web/

EXPOSE 5050

CMD ["python3", "web/app.py"]

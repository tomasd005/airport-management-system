FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    pkg-config \
    libglib2.0-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . /app

RUN make programa-principal programa-testes

ENTRYPOINT ["bash", "/app/docker/run-benchmarks.sh"]

FROM ubuntu:xenial

RUN apt-get update && apt-get install -y \
    libglib2.0-dev                       \
    libjson-glib-dev                     \
    libsoup2.4-dev                       \
    libssl-dev                           \
    python3-pip                          \
    unzip                                \
    valac                                \
    && rm -rf /var/lib/apt/lists/*

# Meson
RUN pip3 install meson

# Ninja
ADD https://github.com/ninja-build/ninja/releases/download/v1.7.2/ninja-linux.zip /tmp
RUN unzip /tmp/ninja-linux.zip -d /usr/local/bin

ADD . .

RUN mkdir build && CFLAGS='-march=native -Ofast' meson --buildtype=release build && ninja -C build

ENTRYPOINT build/cscoin-miner https://cscoins.2017.csgames.org:8989/client

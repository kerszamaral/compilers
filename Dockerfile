# Dockerfile made by Ian Kersz
# This file may be distributed as it doesnt impact the work done.
FROM ubuntu:22.04 AS builder
RUN apt update && apt-get install --no-install-recommends -y make flex bison clang-14
RUN rm -rf /var/lib/apt/lists/*
VOLUME /project
WORKDIR /project
RUN ln -s $(which clang-14) /usr/bin/g++
CMD ["make", "run"]


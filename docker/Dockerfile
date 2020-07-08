# syntax=docker/dockerfile:experimental

FROM alpine:3 as alpine

WORKDIR /root

RUN apk add --no-cache binutils patch xz

ADD http://ftp.debian.org/debian/pool/main/o/opencv/libopencv-dev_3.2.0+dfsg-6_armel.deb libopencv-dev.deb
COPY opencv.patch .

# Unpack deb file
RUN ar x libopencv-dev.deb && \
    tar -xJf control.tar.xz && \
    tar -xJf data.tar.xz && \

# Apply patch
    patch -p1 < opencv.patch && \

# Repack deb files
    tar -cvJf control.tar.xz md5sums control && \
    tar -cvJf data.tar.xz usr && \
    ar rcs libopencv-dev-modified.deb debian-binary control.tar.xz data.tar.xz

FROM ev3dev/debian-buster-cross:latest as debian

USER root


RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install --yes --no-install-recommends \
        ninja-build \
        libboost-log1.67-dev:armel

COPY --from=alpine /root/libopencv-dev-modified.deb /var/cache/apt/libopencv-dev.deb

RUN DEBIAN_FRONTEND=noninteractive apt-get install --yes --no-install-recommends \
        /var/cache/apt/libopencv-dev.deb

USER compiler

COPY build_docker.sh .
CMD ~/build_docker.sh

FROM ubuntu:xenial
MAINTAINER Thomas Karl Pietrowski <thopiekar@googlemail.com>

RUN DEBIAN_FRONTEND=noninteractive apt-get -y update
RUN DEBIAN_FRONTEND=noninteractive apt-get -y dist-upgrade
RUN DEBIAN_FRONTEND=noninteractive apt-get -y install software-properties-common
RUN DEBIAN_FRONTEND=noninteractive add-apt-repository --enable-source -y ppa:thopiekar/cura
RUN DEBIAN_FRONTEND=noninteractive apt-get -y update

RUN mkdir -p /build
COPY [".", "/build"]
WORKDIR /build

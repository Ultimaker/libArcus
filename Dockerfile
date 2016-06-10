FROM ubuntu:xenial
MAINTAINER Thomas Karl Pietrowski <thopiekar@googlemail.com>

RUN add-apt-repository ppa:thopiekar/cura
RUN apt-get update

FROM ubuntu:xenial
MAINTAINER Thomas Karl Pietrowski <thopiekar@googlemail.com>

RUN apt-get install software-properties-common
RUN add-apt-repository ppa:thopiekar/cura
RUN apt-get update

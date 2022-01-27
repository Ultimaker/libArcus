Arcus
=====

This library contains C++ code and Python3 bindings for creating a socket in a thread
and using this socket to send and receive messages based on the Protocol Buffers
library. It is designed to facilitate the communication between Cura and its
backend and similar code.

Building
========

To build the library, the following packages are needed:
* [Protobuf 3](https://github.com/google/protobuf) (3.0+)
* [CMake](https://www.cmake.org)
To build the python bindings (default on, disable with -DBUILD_PYTHON=OFF) these additional libries are needed:
* python3-dev (3.4+)
* python3-sip-dev (4.16+)

On Ubuntu 20.04 this can be achieved with:

```
sudo apt install build-essential cmake python3-dev python3-sip-dev protobuf-compiler libprotoc-dev libprotobuf-dev
```

Building the library can be done with:

- ```$ mkdir build && cd build```
- ```$ cmake ..```
- ```$ make```
- ```# make install```

This will install to CMake's default install prefix, ```/usr/local```. To change the
prefix, set ```CMAKE_INSTALL_PREFIX```. By default, the examples directory is also built.
To disable this, set BUILD_EXAMPLES to off.

To disable building the Python bindings, set BUILD_PYTHON to OFF. They will be
installed into ```$prefix/lib/python3/dist-packages``` on Debian-based systems
and into ```$prefix/lib/python3.4/site-packages``` on other computers.

Building the Python bindings on 64-bit Windows requires you to build with Microsoft Visual
C++ since the module will fail to import if built with MinGW.

Using the Socket
================

The socket assumes a very simple and strict wire protocol: one 32-bit integer with
a header, one 32-bit integer with the message size, one 32-bit integer with a type id
then a byte array containing the message as serialized by Protobuf. The receiving side
checks for these fields and will deserialize the message, after which it can be processed 
by the application.

To send or receive messages, the message first needs to be registered on both sides with 
a call to `registerMessageType()`. You can also register all messages from a Protobuf 
 .proto file with a call to `registerAllMessageTypes()`. For the Python bindings, this 
is the only supported way of registering since there are no Python classses for 
individual message types.

The Python bindings expose the same API as the Public C++ API, except for the missing
`registerMessageType()` and the individual messages. The Python bindings wrap the
messages in a class that exposes the message's properties as Python properties, and
can thus be set the same way you would set any other Python property. 

The exception is repeated fields. Currently, only repeated messages are supported, which
can be created through the `addRepeatedMessage()` method. `repeatedMessageCount()` will
return the number of repeated messages on an object and `getRepeatedMessage()` will get
a certain instance of a repeated message. See python/PythonMessage.h for more details.

Origin of the Name
==================

The name Arcus is from the Roman god Arcus. This god is the roman equivalent of
the goddess Iris, who is the personification of the rainbow and the messenger
of the gods.

Java
====
There is a Java port of libArcus, which can be found [here](https://github.com/Ocarthon/libArcus-Java).

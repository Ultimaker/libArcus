Arcus
=====

This library contains a C++ and Python3 class for creating a socket in a thread
and using this socket to send and receive messages based on the Protocol Buffers
library. It is designed to facilitate the communication between Cura and its
backend and similar code.

Installing Protobuf
-------------------
C++

1. Be sure to have libtool installed.
2. Download protobuf from https://github.com/google/protobuf/ (download ZIP and unZIP at desired location, or clone the repo) The protocol buffer is used for communication between the CuraEngine and the GUI.
3. Before installing protobuf, change autogen.sh : comment line 18 to line 38 using '#'s. This removes the dependency on gtest-1.7.0.
4. Run autogen.sh from the protobuf directory: 
   $ ./autogen.sh
5. $ ./configure
6. $ make
7. $ make install     # Requires superused priviliges.
8. (In case the shared library cannot be loaded, you can try "sudo ldconfig" on Linux systems)

Python

1. Navigate to protobuf-master/python
2. sudo apt-get install python3-setuptools
3. python3 setup.py build 
4. sudo python3 setup.py install

Installing Protobuf on Windows
------------------------------
C++

(Make sure to use the latest MinGW stable version, e.g. MinGW 4.8.1)

1. Download and install MinGW-get from http://sourceforge.net/projects/mingw/files/Installer/mingw-get/
2. With MinGW-get, install the MSYS package for MinGW
3. With MinGW-get, install msys-autogen, msys-automake, msys-libtool
4. Download ProtoBuf from https://github.com/google/protobuf (tested with version 3.0.0-alpha-1)
5. Extract ProtoBuf to .../MinGW/msys/1.0/local
6. Launch .../MinGW/msys/1.0/msys.bat (run as administrator!)
7. Open a terminal and navigate to .../MinGW/msys/1.0/local/protobuf-3.0.0-alpha-1
8. $ ./autogen.sh
	8.1. If at this point you are getting errors of missing AM_PROG_AR, you must make sure the ar.exe binary is installed and the newest stable version.
9. $ ./configure
10. $ mingw32-make
11. $ mingw32-make install

Python

(Make sure to use the latest Python-3 version, e.g. Python 3.4.1)
12. $ cd python
13. $ python setup.py build
14. $ python setup.py install

Building
========

To build the library, you need CMake and Protobuf installed (see below). In addition, if the
Python module should be installed, you need a python interpreter available. Only
Python 3 is supported.

Building the library can be done with:

- mkdir build && cd build
- cmake ..
- make
- make install

This will install to CMake's default install prefix, /usr/local. To change the
prefix, set CMAKE_INSTALL_PREFIX. By default, it will install the Python module
to $prefix/$libdir/python$version/site-packages, where $libdir is an architecture
dependant library directory and $version is the major and minor Python version.
To change this destination, set PYTHON_SITE_PACKAGES_DIR. To disable installing
the Python module completely, set INSTALL_PYTHON_PACKAGE to off. By default, the
examples directory is also built. To disable this, set BUILD_EXAMPLES to off.

Using the Socket
================

The socket assumes a very simple and strict wire protocol: one 32-bit integer with
a type ID, one 32-bit integer with the message size, then a byte array containing
the message as serialized by Protobuf. The receiving side checks for these fields
and will deserialize the message, after which it can be processed by the
application.

To send or receive messages, the message first needs to be registered on both sides
with `registerMessageType()`. Most importantly, the ID passed to this method is
needed since it is part of the wire protocol. This ID should be the same both on the
sender and on the receiver side, otherwise the message type cannot be properly
determined. The message type ID can be freely chosen, except that `0` is reserved as
it is used for keep alive messages.

Origin of the Name
==================

The name Arcus is from the Roman god Arcus. This god is the roman equivalent of
the goddess Iris, who is the personification of the rainbow and the messenger
of the gods.

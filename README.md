# Arcus

<p align="center">
    <a href="https://github.com/Ultimaker/libArcus/actions/workflows/conan-package.yml" alt="Conan Package">
        <img src="https://github.com/Ultimaker/libarcus/actions/workflows/conan-package.yml/badge.svg" /></a>
    <a href="https://github.com/Ultimaker/libArcus/issues" alt="Open Issues">
        <img src="https://img.shields.io/github/issues/ultimaker/libarcus" /></a>
    <a href="https://github.com/Ultimaker/libArcus/issues?q=is%3Aissue+is%3Aclosed" alt="Closed Issues">
        <img src="https://img.shields.io/github/issues-closed/ultimaker/libarcus?color=g" /></a>
    <a href="https://github.com/Ultimaker/libArcus/pulls" alt="Pull Requests">
        <img src="https://img.shields.io/github/issues-pr/ultimaker/libarcus" /></a>
    <a href="https://github.com/Ultimaker/libArcus/graphs/contributors" alt="Contributors">
        <img src="https://img.shields.io/github/contributors/ultimaker/libarcus" /></a>
    <a href="https://github.com/Ultimaker/libArcus" alt="Repo Size">
        <img src="https://img.shields.io/github/repo-size/ultimaker/libarcus?style=flat" /></a>
    <a href="https://github.com/Ultimaker/libArcus/blob/master/LICENSE" alt="License">
        <img src="https://img.shields.io/github/license/ultimaker/libarcus?style=flat" /></a>
</p>

This library contains C++ code for creating a socket in a thread and using this socket to send and receive messages
based on the Protocol Buffers library. It is designed to facilitate the communication between Cura and its backend and similar code.

<br>

[![OpenSSF Scorecard](https://api.securityscorecards.dev/projects/github.com/Ultimaker/libArcus/badge)](https://api.securityscorecards.dev/projects/github.com/Ultimaker/libArcus)

<br>

## License

![License](https://img.shields.io/github/license/ultimaker/libarcus?style=flat)  
Arcus is released under terms of the AGPLv3 License. Terms of the license can be found in the LICENSE file. Or at
http://www.gnu.org/licenses/agpl.html

> But in general it boils down to:  
> **You need to share the source of any Arcus modifications if you make an application with Arcus.**

## System Requirements

### Windows
- Python 3.6 or higher
- Ninja 1.10 or higher
- VS2022 or higher
- CMake 3.23 or higher
- nmake
- protobuf
- zlib

### MacOs
- Python 3.6 or higher
- Ninja 1.10 or higher
- apply clang 11 or higher
- CMake 3.23 or higher
- make
- protobuf
- zlib

### Linux
- Python 3.6 or higher
- Ninja 1.10 or higher
- gcc 12 or higher
- CMake 3.23 or higher
- make
- protobuf
- zlib


## How To Build

> **Note:**  
> We are currently in the process of switch our builds and pipelines to an approach which uses [Conan](https://conan.io/)
> and pip to manage our dependencies, which are stored on our JFrog Artifactory server and in the pypi.org.
> At the moment not everything is fully ported yet, so bare with us.

If you want to develop Cura with libArcus see the Cura Wiki: [Running Cura from source](https://github.com/Ultimaker/Cura/wiki/Running-Cura-from-Source)

If you have never used [Conan](https://conan.io/) read their [documentation](https://docs.conan.io/en/latest/index.html)
which is quite extensive and well maintained. Conan is a Python program and can be installed using pip

### 1. Configure Conan

```bash
pip install conan --upgrade
conan config install https://github.com/ultimaker/conan-config.git
conan profile new default --detect --force
```

Community developers would have to remove the Conan cura repository because it requires credentials. 

Ultimaker developers need to request an account for our JFrog Artifactory server at IT
```bash
conan remote remove cura
```

### 2. Clone libArcus
```bash
git clone https://github.com/Ultimaker/libArcus.git
cd libArcus
```

### 3. Install & Build libArcus (Release OR Debug)

#### Release
```bash
conan install . --build=missing --update
# optional for a specific version: conan install . arcus/<version>@<user>/<channel> --build=missing --update
cmake --preset release
cmake --build --preset release
```

#### Debug

```bash
conan install . --build=missing --update build_type=Debug
cmake --preset debug
cmake --build --preset debug
```

## Creating a new Arcus Conan package

To create a new Arcus Conan package such that it can be used in Cura and Uranium, run the following command:

```shell
conan create . arcus/<version>@<username>/<channel> --build=missing --update
```

This package will be stored in the local Conan cache (`~/.conan/data` or `C:\Users\username\.conan\data` ) and can be used in downstream
projects, such as Cura and Uranium by adding it as a requirement in the `conanfile.py` or in `conandata.yml`.

Note: Make sure that the used `<version>` is present in the conandata.yml in the libArcus root

You can also specify the override at the commandline, to use the newly created package, when you execute the `conan install`
command in the root of the consuming project, with:


```shell
conan install . -build=missing --update --require-override=arcus/<version>@<username>/<channel>
```

## Developing libArcus In Editable Mode

You can use your local development repository downsteam by adding it as an editable mode package.
This means you can test this in a consuming project without creating a new package for this project every time.

```bash
    conan editable add . arcus/<version>@<username>/<channel>
```

Then in your downsteam projects (Cura) root directory override the package with your editable mode package.  

```shell
conan install . -build=missing --update --require-override=arcus/<version>@<username>/<channel>
```

## Using the Socket


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

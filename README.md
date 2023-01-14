# Message Passing Toolkit

A collection of interfaces and routines for C-based generic types,
config abstraction and message encoding.

Extended functionality includes graphic primitives and simple buffer/queue
implementations.

## Build

Running `make` will create a local environment in the `build` subdirectory.
The target location can be changed by setting a different `PREFIX` variable.

Includes configurations for alternative (real out-of-tree) build systems:
- [meson](https://mesonbuild.com)
- [CMake](https://cmake.org)

The `examples` directory contains many small programs demonstrating
usage of various interfaces and routines.
Most of them will be run during `make test` and are also used in test
definitions for alternative build environments.

## C library components

To maximize reusability and minimize dependencies, functionality
is split into multiple (shared) libraries.

### Core
Most interface definitions and in-memory operations.

Dynamic type registry, basic conversion routines and generic shared COW array
with pluggable init/fini operations for each element.

Implementation of message coding, data buffers and event processing.

Definitions and operations for generic file parsing
and configuration interface.

### Plot
Types and operations for data plotting.

Definitions for generic graphic objects.

Additional operations for data output, value sources and
generators for various types of initial data.

### Input/Output
Socket operations and input loop.

Buffered data stream with integrated
data encoding/decoding and message send/receive.

Implementation of output interface with channel selection
(stdout/stderr, history file, remote process) depending
on MPT message type.

Notification system for registering and waiting for multiple inputs.

### Loader
Wrappers to load functionality from shared libraries.

Provide abstration for line input from user.

## C++
Implementation and extensions of basic (core, plot, I/O) elements.

Generic graphic display implementation.

## LUA bindings
Interface to use MPT stream and COBS encoding.

LUA implementation to load binary module.
Controller for client application.

## Python interface
Pure Python implemetation to interact with client and plot programs.

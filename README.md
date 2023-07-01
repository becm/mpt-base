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
Supplies implementation of most standard data types and shared functionality for
general in-memory operations. To guarantee consistent runtime behaviour,
the base library also hosts the dynamic type registry (single source of truth).

Most provided data types have interfaces for reference counting and are often
used along with the built-in generic copy-on-write array implementation.

Buffers and data streams support on-the-fly message coding and decoding.
The same messages spec is also used in the included event processing.

A configurable streaming parser for structured text does emit `path/value` pairs
and is used by default to feed the built-in configuration framework.

### Plot
Types and operations for data plotting and definitions of basic graphic objects.

Sets of numeric data can be read from different sources, split up and formatted
as text output.
Generators for various types of initial data are also included for convenience.

### Input/Output
Socket operations and input loop.

The implementation of a buffered I/O file data stream can also be used for
sending and receiving of messages if the configured methods for inline data
encoding and decoding support delimiter recognition and insertion.

Output objects may select channels (stdout/stderr, history file, remote process)
depending on passed MPT message type.

A notification sytem is used to register multiple inputs for event dispatching.
Abstractions are used to decouple data and dispatch layers, so it should also
be possible to use a different 3rd party notification solution to feed the
internal event framework.

### Loader
Wrappers to load functionality from shared libraries.

Also provides abstration for line input from user. Behaviour and features might
differ depending on the detected `readline` implementation and setup.

## C++
While most `C++` data types and template definitions are defined in headers,
more complex operations may still require methods from this library.

Doing generic metatype representation for graphic objects and large portions of
plotting transformations has far less boilerplate compared to pure `C`.
Most types however can be passed freely between both layers due to compatible
memory layout definitions.

## LUA bindings
To use MPT stream and COBS encoding of the core I/O library a generic 3rd party
library loader and some additional wrappers as supplied as a combination of
native and LUA code.

A basic implementation for controlling a MPT client application can be extended
by adding furter user command handlers to the created LUA object.

## Python interface
Pure Python implemetation to start and interact with client and plot programs.

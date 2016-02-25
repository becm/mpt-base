# Message Passing Toolkit

A collection of interfaces and routines
to transfer opaque binary messages via stream connections
and processing of received data.

Extended funtionality includes configuration handling
and simple buffer/queue implementations.

To maximize reusability and minimize dependencies, functionality
is split into multiple (shared) libraries.

## Core
Most interface definitions and in-memory operations.

Implementation of message coding, data buffers and event processing.

Definitions and operations for generic file parsing
and configuration interface.

## Plot
Types and operations for data plotting.

Definitions for generic graphic objects.

Additional operations for data output,
value source and profile generation.

## Input/Output
Socket operations and input loop.

Buffered data stream with integrated
data encoding/decoding and message send/receive.

Implementation of output interface with channel selection
(stdout/stderr, history file, remote process) depending
on MPT message type.

Notification system for registering and waiting for multiple inputs.

## Loader
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

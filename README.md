# Message Passing Toolkit

A collection of interfaces and routines originally built
to transfer opaque binary messages via stream connections
and processing of received messages.

Extensions to read/query configuration data,
some buffers and queues and additional interfaces
were added to extend functionality.

To maximize reusability and minimize dependencies, functionality
is split into multiple (shared) libraries.

## Core
Most interface definitions and in-memory operations.

Operations for coding, data stores, configuration, messages and configuration.

## Plot
Datatypes and operations for data plotting.

Additional operations for data output, value source and profile generation.

## Input/Output
Socket operations and input loop.

Own implementation of buffered data stream was deemed necessary
for control over buffers and additional data for encoding/decoding.

Notification system for registering and waiting for multiple inputs.

Interface for dynamic loading of shared library modules.

## LUA bindings
Interface to use MPT stream and COBS encoding.

LUA implementation to load binary module.
Controller for client application.

## Python interface
Pure Python implemetation to interact with client and plot programs.

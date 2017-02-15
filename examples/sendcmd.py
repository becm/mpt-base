#!/usr/bin/env python3
# send input lines to stream socket
from sys import argv
import socket

if len(argv) > 2:
    c = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
    c.connect((argv[1], int(argv[2])))
else:
    c = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    c.connect(argv[1])

while True:
    try:
        a = input()
        c.send(bytes(a + '\0', 'utf8'))
    except EOFError:
        break

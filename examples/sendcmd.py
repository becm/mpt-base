#!/usr/bin/env python3
import sys, socket

if len(sys.argv) > 2:
  c = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
  c.connect((sys.argv[1], int(sys.argv[2])))
else:
  c = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
  c.connect(sys.argv[1])

while True:
  try:
    a = input()
    c.send(bytes(a + '\0', 'utf8'))
  except EOFError:
    break

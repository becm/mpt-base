#!/usr/bin/env python3
# meson_gen.py: update special meson.build files
import sys, os, fnmatch, io

marker = b'meson.build.gen'

def config(line):
  line = line.split()
  
  suffix = []
  files = []
  dirs = []
  for i in line:
    i = i.split('=')
    if i[0] == 'suffix':
      suffix = i[1].split(',')
    if i[0] == 'files':
      files = i[1].split(',')
    if i[0] == 'directories' or i[0] == 'dirs':
      dirs = i[1].split(',')
  
  return (dirs, suffix, files)

def isgenerated(hdr):
  elem = hdr.split()
  
  if elem[0] == b'#' + marker + b':':
    return True
  
  if len(elem) < 2 or elem[0] != b'#':
    return False
  
  if elem[1] == marker + b':':
    return True
  
  if len(elem) < 3 or elem[1] != marker or elem[2] != b':':
    return False
  
  return True

def newline(f):
    mb.write(bytes(os.linesep, 'utf-8'))

def write_header(f, t, n):
  f.write(b' # ')
  f.write(bytes(t, 'utf-8'))
  f.write(b': ')
  f.write(bytes(n, 'utf-8'))
  newline(f)

def write_entry(f, n):
  f.write(b'  \'')
  f.write(bytes(n, 'utf-8'))
  f.write(b'\',')
  newline(f)

for root, dirs, files in os.walk('.'):
  for name in fnmatch.filter(files, 'meson.build'):
    name = os.path.join(root,name)
    mb = open(name, 'rb')
    header = mb.readline()
    
    if not isgenerated(header): continue;
    
    if len(sys.argv) < 2 or sys.argv[1] == 'gen':
      True
    elif sys.argv[1] == 'clean':
      open(name, 'wb').write(header)
      continue
    
    
    #mb = open(name, 'w')
    mb = io.BytesIO()
    
    tmp = str(header, 'utf-8').split(':')[1].strip()
    
    mb.write(header)
    mb.write(b'src = files(')
    newline(mb)
    
    dirs, suffixes, files = config(tmp)
    
    for d in dirs:
      write_header(mb, 'directory', d)
      d = os.path.join(root,d)
      for f in os.listdir(d):
        write_entry(mb, f)
    
    for s in suffixes:
      s = '*.' + s
      write_header(mb, 'match', s)
      for f in os.listdir(root):
        if fnmatch.fnmatch(f, s):
          write_entry(mb, f)
    
    if len(files):
      mb.write(b' # files')
      newline(mb)
      for f in files:
        write_entry(mb, f)
    
    mb.write(b')')
    newline(mb)
    
    open(name, 'wb').write(mb.getvalue())

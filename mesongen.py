#!/usr/bin/env python3
# mesongen.py: update special meson.build files
import sys, os, fnmatch, io

marker = b'meson.build.gen'
buildfile = 'meson.build'

def isgenerated(hdr):
  elem = hdr.split()
  
  if len(elem) == 0:
    return False
  
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
  f.write(bytes(os.linesep, 'utf-8'))

def write_header(f, t, n):
  f.write(b' # ')
  f.write(bytes(t, 'utf-8'))
  f.write(b': ')
  f.write(bytes(n, 'utf-8'))
  newline(f)

def write_entries(f, elem):
  for e in elem:
    f.write(b'  \'')
    f.write(bytes(e, 'utf-8'))
    f.write(b'\',')
    newline(f)

def content(pattern, path='.', match=None):
  buf = []
  for f in os.listdir(path):
    fn = os.path.join(path, f)
    if os.path.isfile(fn):
      m = './' + f if match == None else match + '/' + f
      for p in pattern:
        #print(m,p)
        if not p.count('/'):
          if fnmatch.fnmatch(f, p):
            #print(' +',m)
            buf.append(f if match == None else m)
            break
        else:
          if fnmatch.fnmatch(m, p):
            #print(' +',m)
            buf.append(f if match == None else m)
            break
        #print(' -',m)
    elif os.path.isdir(fn):
      m = f if match == None else match + '/' + f
      buf = buf + content(pattern, fn, m)
  return buf

def update(path=buildfile, clear=False):
  header = open(path, 'rb').readline()
  
  if not isgenerated(header):
    return False
  
  if clear == True:
    open(path, 'wb').write(header)
    return True
  
  tmp = str(header, 'utf-8').split(':')[1].strip()
  buf = content(tmp.split(), os.path.dirname(path))
  buf.sort()
  
  with open(path, 'wb') as out:
    out.write(header)
    out.write(b'src = files(')
    newline(out)
    write_entries(out, buf)
    out.write(b')')
    newline(out)
  
  return True

def update_recursive(path='.', clear=False):
  for root, dirs, files in os.walk(path):
    if files.count(buildfile):
      update(os.path.join(root, buildfile), clear)

if __name__ == "__main__":
  from sys import exit, stderr
  if len(sys.argv) < 2:
    update_recursive('.')
  elif sys.argv[1] == 'clean':
    if len(sys.argv) < 3:
        update_recursive('.', clear=True)
    else:
      for d in sys.argv[2:]:
        if not update(d, clear=True):
          exit(1)
  elif sys.argv[1] == 'gen':
    if len(sys.argv) < 3:
      update_recursive('.', clear=True)
    else:
      for d in sys.argv[2:]:
        if not update(d):
          exit(1)
  else:
    stderr.write('bad operation: ' + sys.argv[1] + os.linesep)
    exit(2)

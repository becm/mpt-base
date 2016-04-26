#!/usr/bin/env python3
# mesongen.py: update special meson.build files
import sys, os, fnmatch, io, re

# marker to process file
marker = b'meson.build.gen'
buildfile = 'meson.build'
newline = os.linesep.encode('utf-8')
matchexp = re.compile(b'#\s*' + marker + b'\s*')

def isgenerated(hdr):
  ''' get end of header marker '''
  match = matchexp.match(hdr)
  return match if match == None else match.end()

def write_entries(f, elem):
  ''' write file entries to output '''
  for e in elem:
    f.write(b'  \'')
    f.write(e.encode('utf-8'))
    f.write(b'\',' + newline)

def content(pattern, path='.', match=None):
  ''' collect matching filenames '''
  if type(pattern) == str:
    pattern = pattern.split()
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
  ''' rewrite build file '''
  header = open(path, 'rb').readline()
  pos = isgenerated(header)
  if pos == None:
    return False
  
  if clear == True:
    open(path, 'wb').write(header)
    return True
  
  pattern = header[pos:].decode('utf-8')
  buf = content(pattern.split(), os.path.dirname(path))
  buf.sort()
  
  with open(path, 'wb') as out:
    out.write(header)
    if not pattern.endswith(os.linesep):
      out.write(newline)
    out.write(b'src = files(' + newline)
    write_entries(out, buf)
    out.write(b')' + newline)
  
  return True

def update_recursive(path='.', clear=False):
  ''' rewrite all build files in path'''
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
      update_recursive('.')
    else:
      for d in sys.argv[2:]:
        if not update(d):
          exit(1)
  else:
    stderr.write('bad operation: ' + sys.argv[1] + os.linesep)
    exit(2)

#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Shell environment for MPT.

create connection to MPT plotting process

run Shell Interpreter for message passing
to attached user processes

"""

import cmd
import os
import sys
import socket

from struct import pack
from glob import glob
from array import array
from tempfile import gettempdir
from subprocess import Popen, PIPE

MESSAGE_OUTPUT = 0x0

MESSAGE_COMMAND = 0x4
MESSAGE_SET = 0x6

MESSAGE_VALUES = 0x8
MESSAGE_DEST = 0xd

VALUES_UNSIGNED = 0x20
VALUES_FLOAT = 0x40
VALUES_INTEGER = 0x60

if sys.byteorder == "little":
    VALUES_NATIVE = -0x80
else:
    VALUES_NATIVE = 0

SEARCH_TERMINAL = [
    ['x-terminal-emulator', '-e'],
    ['konsole', '-e'],
    ['xterm', '-e']
]


rundir = os.getenv('MPT_TMP')
if not rundir:
    if os.name != 'posix':
        rundir = os.path.join(gettempdir(), 'mpt')
    else:
        rundir = os.path.join(gettempdir(), 'mpt-' + str(os.getuid()))


def encode_command(msg):
    if isinstance(msg, str):
        msg = bytearray(msg, 'utf-8')
    
    try:
        msg.index(b"\x00")
        raise ValueError("inline zero byte")
    except:
        pass
    
    return msg + b"\x00"


def encode_cobs(msg):
    """ simple COBS encoder """
    code = 0x1
    ret = bytearray()
    
    ret.append(code)
    
    if isinstance(msg, str):
        msg = bytearray(msg, 'utf-8')
    
    for b in msg:
        if b != 0:
            if code >= 254:
                ret.append(b)
                ret[len(ret) - code] = code + 1
                code = 1
                continue
            ret.append(b)
            code = code + 1
        else:
            if code != 1:
                ret[len(ret) - code] = code
            code = 1
            ret.append(code)
    
    ret[len(ret) - code] = code
    ret.append(0)
    
    return ret


class Output(object):
    """ output buffer """
    _encode = encode_command
    _buf = None
    
    def __init__(self, name=None):
        """ append data to output """
        self._encode = Output._encode
        if name is None:
            return
        self._chan = open(name, 'wb')
    
    def push(self, msg):
        """ send data to output """
        if not self._buf:
            self._buf = bytearray()
        
        if isinstance(msg, tuple) or isinstance(msg, list):
            """ serialize message data """
            for t in msg:
                self._buf = self._buf + t
        elif isinstance(msg, bytes):
            self._buf = self._buf + msg
        elif isinstance(msg, str):
            self._buf = self._buf + bytes(bytearray(msg, 'utf-8'))
        else:
            raise TypeError("invalid output data")
    
    def flush(self):
        """ flush output data """
        if not self._chan or self._buf is None:
            return
        
        if self._encode:
            self._buf = self._encode(self._buf)
        
        if not len(self._buf):
            self._buf = None
            return
        
        if hasattr(self._chan, 'sendall'):
            ret = self._chan.sendall(self._buf)
        else:
            ret = self._chan.write(self._buf)
        
        if hasattr(self._chan, 'flush'):
            self._chan.flush()
        
        self._buf = None
        return ret


def make_header(cmd=MESSAGE_COMMAND, arg=0, _id=None):
    """ serialize header """
    if _id is None:
        return pack('>bb', cmd, arg)
    return pack('>Hbb', _id, cmd, arg)


def make_destination(dest):
    """ serialize destination encoding """
    lay = grf = wld = dim = cyc = off = None
    
    if isinstance(dest, tuple) or isinstance(dest, list):
        if len(dest) > 5:
            off = dest[5]
        if len(dest) > 4:
            cyc = dest[4]
        if len(dest) > 3:
            dim = dest[3]
        if len(dest) > 2:
            wld = dest[2]
        if len(dest) > 1:
            grf = dest[1]
        if len(dest) > 0:
            lay = dest[0]
    elif isinstance(dest, dict):
        lay = dest.get('lay')
        if not lay:
            lay = dest.get('layout')
        grf = dest.get('grf')
        if not grf:
            grf = dest.get('graph')
        wld = dest.get('wld')
        if not wld:
            wld = dest.get('world')
        dim = dest.get('dim')
        if not dim:
            dim = dest.get('dimension')
        cyc = dest.get('cyc')
        if not cyc:
            cyc = dest.get('cycle')
        off = dest.get('off')
        if not off:
            off = dest.get('offset')
    
    if lay is None:
        lay = 1
    if grf is None:
        grf = 1
    if wld is None:
        wld = 1
    if dim is None:
        dim = 0
    if cyc is None:
        cyc = 0
    if off is None:
        off = 0
    
    return pack('>BBBBII', lay, grf, wld, dim, cyc, off)


def make_message(msg, dest=None, messageID=None):
    """ create serialized message elements """
    if isinstance(msg, array):
        if 'df'.find(msg.typecode) >= 0:
            arg = VALUES_NATIVE + VALUES_FLOAT + (msg.itemsize - 1)
        elif 'LIHBC'.find(msg.typecode) >= 0:
            arg = VALUES_NATIVE + VALUES_UNSIGNED + (msg.itemsize - 1)
        elif 'lihbc'.find(msg.typecode) >= 0:
            arg = VALUES_NATIVE + VALUES_INTEGER + (msg.itemsize - 1)
        else:
            raise TypeError("invalid value type")
        hdr = make_header(MESSAGE_DEST, arg, messageID)
        return (hdr + make_destination(dest), msg.tostring())
    if isinstance(msg, str):
        msg = bytes(bytearray(msg, 'utf-8'))
        return (make_header(MESSAGE_COMMAND, ord(" "), messageID), msg)
    if isinstance(msg, bytes):
        if dest:
            fmt = VALUES_NATIVE + VALUES_FLOAT + (8 - 1)
            hdr = make_header(MESSAGE_VALUES, fmt, messageID)
        else:
            hdr = make_header(MESSAGE_OUTPUT, ord(" "), messageID)
        return (hdr, msg)


class Graphic(Output):
    """ start or connect to graphic output process """
    
    DEFAULT_PORT = 16565
    
    # instance is first argument
    def nextID(self):
        return None
    
    def __init__(self, dest=None, exe='mptplot'):
        """ create or connect to graphic process """
        super(Graphic, self).__init__()
        self._encode = encode_cobs
        
        if isinstance(dest, tuple):
            port = Graphic.DEFAULT_PORT
            if len(dest) > 1:
                dest, port = dest
            elif len(dest) > 0:
                dest = dest[0]
            else:
                dest = 'localhost'
            
            dest = (dest, port)
        
        # connect to existing graphic process
        if exe is None:
            if dest is None:
                return
            if isinstance(dest, str):
                chan = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            elif isinstance(dest, tuple):
                chan = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            else:
                raise TypeError("invalid destination")
            
            chan.connect(dest)
            self._chan = chan
            
            # default to non-return ID
            self.nextID = lambda: 0
            
            return
        
        # use global output process
        if exe[0] != '/':
            try:
                exe = os.path.join(os.environ['MPT_PREFIX'], 'bin', exe)
            except:
                exe = which(exe)
        
        # create graphic process arguments
        if dest is None:
            argv = [exe]
        elif isinstance(dest, str):
            self.name = 'Unix:' + dest
            argv = [exe, '-s', self.name]
        elif isinstance(dest, tuple):
            argv = [exe, '-s', 'Ip:' + dest[0] + ':' + str(dest[1])]
            self.name = 'Ip:localhost:' + str(dest[1])
        else:
            raise TypeError("invalid destination")
        
        # new environment for graphic process
        environ = os.environ.copy()
        environ['MPT_GRAPHIC_PIPE'] = 'cobs'
        
        # start new graphic process
        self.process = Popen(args=argv, executable=exe,
                             preexec_fn=os.setpgrp, stdin=PIPE, env=environ)
        self._chan = self.process.stdin
    
    def __del__(self):
        """ close graphic process if startet for this connection """
        if not hasattr(self, 'process'):
            return
        if self.process.poll() is not None:
            return
        self.send('close')
    
    def __str__(self):
        if not hasattr(self, 'name'):
            return ''
        return self.name
    
    def send(self, msg, dest=None, onreturn=None):
        """ send message data to graphic process """
        if onreturn:
            onreturn(None)
        self.push(make_message(msg, dest, self.nextID()))
        self.flush()


def which(program):
    """ resolve program name in $PATH """
    def is_exe(fpath):
        return os.path.exists(fpath) and os.access(fpath, os.X_OK)

    fpath, fname = os.path.split(program)
    if fpath:
        if is_exe(program):
            return program
    else:
        for path in os.environ['PATH'].split(os.pathsep):
            exe_file = os.path.join(path, program)
            if is_exe(exe_file):
                return exe_file
    return None


def error(*err):
    """ print message and newline to standard error """
    msg = ""
    for i in err:
        msg = msg + str(i) + " "
    sys.stderr.write(msg + os.linesep)


def tolist(arg):
    """ convert argument to list """
    if isinstance(arg, list):
        return arg
    if isinstance(arg, tuple):
        return list(arg)
    if isinstance(arg, str):
        return [arg]
    raise TypeError("require string, tuple or list")


def getterm():
    """ get terminal program """
    term = os.getenv('MPT_TERMINAL')
    if term is not None:
        exe = which(term)
        if exe:
            return [exe]
        error("invalid terminal: " + term)
        return None
    for term in SEARCH_TERMINAL:
        exe = which(term[0])
        if exe is not None:
            return [exe] + term[1:]
    return None


def setup(process, wpath='', cpath=None):
    """ setup MPT environment """
    control = []
    env = os.environ.copy()
    if not cpath:
        cpath = os.getcwd()
    env['MPT_CWD'] = cpath
    
    for idx, p in enumerate(process):
        sname = 'mpt.' + str(os.getpid()) + '.ctl' + str(idx)
        try:
            sname = os.path.join(wpath, sname)
            os.mkfifo(sname, 0o600)
            env['MPT_CONNECT'] = 'r:' + sname
            cl = p.get('client')
            if cl is not None:
                env['MPT_CLIENT'] = cl
            Popen(args=p['command'], env=env, stderr=PIPE,
                  close_fds=True, preexec_fn=os.setpgrp)
            control = control + [open(sname, 'wb')]
        except OSError:
            cleanup(control)
            raise
    return control


def cleanup(control):
    """ delete control pipes """
    for c in control:
        if not c:
            continue
        try:
            os.unlink(c.name)
        except OSError:
            pass


def close(control):
    """ send close command to control pipes """
    for c in control:
        if not c or c.closed:
            continue
        try:
            c.write(encode_command(b'close'))
            c.flush()
            c.close()
        except IOError:
            pass


class Shell(cmd.Cmd):
    """ MPT shell interpreter and user process controller
    
    Create shell object for user processes
    
    i=Shell()     # create shell instance
    i.add('ode')  # add client program
    i.run()       # start interpreter
    
    """
    intro = "MPT control shell"
    prompt = "(mpt) "
    
    terminal = None
    wrapper = [sys.executable, os.path.abspath(__file__)]
    
    def do_start(self, args):
        """ send start command to first user process """
        return self.send('start ' + args)
    
    def complete_start(self, text, line, begidx, endidx):
        return glob(text + '*')
    
    def do_cont(self, args):
        """ send continue command to first user process """
        return self.send('cont ' + args)
    
    def do_stop(self, args):
        """ send stop command to first user process """
        return self.send('stop ' + args)
    
    def do_read(self, args):
        """ send read command to first user process """
        return self.send('read ' + args)
    
    def complete_read(self, text, line, begidx, endidx):
        return glob(text + '*')
    
    def do_init(self, args):
        """ send initialisation command to first user process """
        return self.send('init ' + args)
    
    def do_prep(self, args):
        """ send prepare command to first user process """
        return self.send('prep ' + args)
    
    def do_step(self, args):
        """ send single step command to first user process """
        return self.send('step ' + args)
    
    def do_set(self, args):
        """ send assignment command to first user process """
        return self.send('set ' + args)
    
    def do_clear(self, args):
        """ send clear command to first user process """
        return self.send('clear ' + args)
    
    def do_graphic(self, args):
        """ graphic command for first user process """
        if hasattr(self, 'graphic') and hasattr(self.graphic, 'send'):
            return self.graphic.send(args)
        self.error('no graphic connection')
    
    def do_close(self, args):
        """ close shell and user processes """
        if hasattr(self, 'graphic') and hasattr(self.graphic, 'process'):
            del self.graphic.process
        raise SystemExit
    
    def do_EOF(self, args):
        """ clean exit on 'C-d' """
        sys.stdout.write(os.linesep)
        raise SystemExit
    
    def do_exit(self, line=""):
        """ exit command interpreter and clean environment """
        close(self.control)
        raise SystemExit
    
    def do_mesg(self, line):
        """ send command to (first) user process """
        s = line.split(' ')[0]
        try:
            chan = int(s)
            return self.send(line[len(s):], chan)
        except ValueError:
            return self.send(line)
    
    def do_shell(self, args):
        """ execute command via standard shell """
        return os.system(args)
    
    def complete_shell(self, text, line, begidx, endidx):
        return glob(text + '*')
    
    def default(self, line=""):
        """ default error message """
        self.error('command "' + str.split(line, ' ')[0] + '" not found')
    
    def send(self, args, chan=0):
        """ send command to user processes """
        if chan < 0:
            self.error('bad control slot: ' + str(chan))
            return
        if not hasattr(self, 'control') or chan >= len(self.control):
            self.error('control slot ' + str(chan) + ' not available')
            return
        
        c = self.control[chan]
        
        if not c or c.closed:
            self.error('control slot ' + str(chan) + ' inactive')
            return
        
        try:
            m = bytearray()
            for d in make_message(args):
                m = m + d
            c.write(encode_cobs(m))
            c.flush()
        except:
            c.close()
    
    def error(self, *args):
        error(*args)
    
    def add(self, cmd):
        """ append user process """
        if self.terminal is None:
            if Shell.terminal is None:
                Shell.terminal = getterm()
            self.terminal = Shell.terminal
        
        if not hasattr(self, 'process'):
            self.process = []
        
        # set new process parameters
        proc = {
            'command': tolist(self.terminal) + tolist(self.wrapper),
            'client': cmd
        }
        self.process = self.process + [proc]
    
    def run(self, wpath=rundir):
        """ setup and run shell interpreter """
        
        if wpath != '':
            try:
                os.mkdir(wpath, 0o700)
            except OSError:
                pass
        
        # create client processes
        self.control = setup(self.process, wpath)
        
        # connect clients to graphic
        if hasattr(self, 'control') and hasattr(self, 'graphic'):
            cmd = bytearray()
            cmd = cmd + make_header(MESSAGE_SET, 2)
            cmd = cmd + bytearray("mpt\x00graphic\x00", 'utf-8')
            cmd = cmd + bytearray(self.graphic.name, 'utf-8')
            cmd = encode_cobs(cmd)
            for p in self.control:
                p.write(cmd)
                p.flush()
        
        # run command interpreter
        while 1:
            try:
                self.cmdloop()
            except KeyboardInterrupt:
                self.intro = ' '
            except SystemExit:
                break
            except:
                # avoid graphic close on error
                if hasattr(self, 'graphic'):
                    if hasattr(self.graphic, 'process'):
                        del self.graphic.process
                raise
        
        # remove control pipes
        cleanup(self.control)
        del self.control
        # remove temporary path
        if wpath == '':
            return
        try:
            os.rmdir(wpath)
        except:
            pass


def client(args):
    """ wrap programs to avoid close of termial on error """
    p = None
    try:
        p = Popen(args).wait()
        if p == 0:
            return p
        if p < 0:
            error("Terminated by signal " + str(-p))
        else:
            error("Exited with code " + str(p))
    
    except KeyboardInterrupt:
        error("Keyboard Interrupt")
    
    except OSError:
        error("File not accessable")
    
    error("Hit Enter to continue")
    sys.stdin.readline()
    return p


def run(args):
    """ run Shell environment """
    for i in args:
        if not os.path.exists(i):
            raise SystemExit("client program not found: " + i)
        if not os.access(i, os.X_OK):
            raise SystemExit("client program not executable: " + i)
    i = Shell()
    for arg in args:
        i.add(arg)
    g = os.path.join(rundir, 'mpt.' + str(os.getpid()) + '.graphic')
    i.graphic = Graphic(dest=g)
    i.run()


if __name__ == '__main__':
    cl = os.getenv('MPT_CLIENT')
    if cl is not None:
        
        # change working directory
        wdir = os.getenv('MPT_CWD')
        if wdir is not None:
            os.chdir(wdir)
        
        exe = which(cl)
        if exe is None:
            exe = './' + cl
        
        os.unsetenv('MPT_CLIENT')
        os.environ['MPT_FLAGS'] = 'e'
        # start client under debug process
        if os.getenv('MPT_DEBUG') is not None:
            # enable client setup from environment
            dbg = os.getenv('MPT_DEBUG_COMMAND')
            if not dbg:
                dbg = 'gdb'
            client([dbg, exe])
        else:
            error("Start program '" + '\033[01m' + cl + '\033[00m' + "'")
            client(exe)
    else:
        if len(sys.argv) < 2:
            name = os.path.basename(sys.argv[0])
            raise SystemExit(sys.argv[0] + ": missing arguments" +
                             os.linesep + "  " + name +
                             " <client1> [<clientN>]")
        run(sys.argv[1:])

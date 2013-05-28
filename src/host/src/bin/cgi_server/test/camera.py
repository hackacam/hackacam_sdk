#!/usr/bin/env python
import sys
import httplib
import readline
import cmd
import socket
import threading
import select

class Receiver(threading.Thread):
    magic_bytes = "93d60153bbe87fc0ae09be1d8be26e84"
    port        = 8000
    def __init__(self, magic_bytes, port):
        threading.Thread.__init__(self)
        self.magic_bytes = magic_bytes
        self.port = port
        self.exit_ = False
        self.daemon = True
    def run(self):
        rx_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        rx_sock.bind(("0.0.0.0", self.port + 1))
        print "\nListing for broadcoast on port", self.port + 1
        while True:
            rlist, wlist, elist = select.select([rx_sock], [], [], 1.0)
            if len(rlist):
                data, addr = rx_sock.recvfrom(1024)
                print "\nReceived broadcast message from", addr
                print data
            elif self.exit_:
                rx_sock.close()
                return

class Cmd(cmd.Cmd):
    ip = ['192', '168', '6', '58']
    bcast = "255.255.255.255"
    def __init__(self, arg):
        cmd.Cmd.__init__(self)
        self.magic_bytes = Receiver.magic_bytes
        self.port = Receiver.port
        self.tx_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.tx_socket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        self.mac_address = None
        self.timeout = 60
        self.net_recovery = False
        self.receiver = None
        self.ip = []
        if arg is not None:
            for camera in arg.split(','):
                ip = camera.split('.')
                while len(ip) < 4:
                    ip.insert(0, -1)
                for n in range(0, 4):
                    if ip[n] < 0: ip[n] = Cmd.ip[n]
                self.ip.append('.'.join(ip))    
        if len(self.ip) == 0: self.ip.append('.'.join(Cmd.ip))
        if len(self.ip) == 1:
            self.prompt = self.ip[0] + ' > '
        else:
            prompt = []
            for p in self.ip:
                a = p.split('.')
                prompt.append(a[3])
            self.prompt = ','.join(prompt) + ' > '
    def do_help(self, command): 
        "print help message"
        if command == "":
            print """
Usage: camery.py <ip_address>,<ip_address>,...
Connects to the camera (default ip_address is 192.168.6.58) or list of cameras and issues commands.
See below list of commands script accepts.
You can hit <tab> for command completions. There is no argument completion yet.
This is why certain commands start uppercase (Reboot vs. reset)
Arguments must be given in name=value format and separated by spaces.
History and line editing works.
Shortcuts and special cases:
    1. stream id must be given as number and must be first argument. Stream id defaults to 0. 
    2. If 'action' is ommited, it will be automatically added when necessary. 
    3. If login has no arguments, it adds user=root&password=password
    4. If logout has no arguments, it adds user=root

Examples:
    image osd=true  => /cgi-bin/image?action=set&osd=true
    s<tab>          => /cgi-bin/stream?action=get&id=0
    s<tab> 1 encoder=mjpeg quality=50 =>
                       /cgi-bin/stream&action=set&id=1&encoder=mjpeg&quality=50
    Sn<tab>         => /cgi-bin/snapshot&action=get
"""
        cmd.Cmd.do_help(self, command)
    def emptyline(self):
        print
    def do_EOF(self, line):
        """ ctrl-D to exit the application """
        print
        return -1
    def do_login(self, line):
        "login user (without arguments supplies default ones)"
        if len(line) == 0: line = 'user=root&password=password'
        self.command('login', line, False) 
    def do_Logout(self, line):
        "logout user (without argument logouts root)"
        if len(line) == 0: line = 'user=root'
        self.command('logout', line, False)
    def do_user(self, line):
        "change user password"
        self.command('user', line)
    def do_reset(self, line):
        "reset camera"
        self.command('reset', line, False)
    def do_Reboot(self, line):
        "reboot camera"
        self.command('reboot', line, False)
    def do_Status(self, line):
        "print camera status"
        self.command('status', line, False)
    def do_device_info(self, line):
        "get or set device info"
        self.command('device_info', line)
    def do_Date(self, line):
        "get or set date"
        self.command('date', line, True)
    def do_image(self, line):
        "get or set image"
        self.command('image', line)
    def do_stream(self, line):
        "get or set stream - if stream number is not provided, it is 0"
        self.command('stream', line)
    def do_roi(self, line):
        "get or set roi"
        self.command('roi', line)
    def do_test(self, line):
        "execute test command"
        self.command('test', line, False)
    def do_Snapshot(self, line):
        "get or set snapshot - does not support snapshot send"
        self.command('snapshot', line, False)
    def do_net_recovery(self, line):
        """Changes the mode used to execute commands. If net_recovery is
on, IP broadcast is used to reach the camera specified by the mac_address.
If net_recovery is off, normal HTTP communication is used.
Syntax:
    net_recovery <on/off> [<mac_address>] [<port>] [<magic_bytes>]
    default port is 8000 and magic_bytes is 93d60153bbe87fc0ae09be1d8be26e84
"""
        params = line.split()
        if len(params) < 1 or not (params[0] == "on" or params[0] == "off"):
            cmd.Cmd.do_help(self, command)
            return
        if len(params) > 1 : self.mac_address = params[1]
        if len(params) > 2 : self.port        = params[2]
        if len(params) > 3 : self.magic_bytes = params[3]
        if params[0] == "off": 
            self.net_recovery = False
            self.kill_receiver()
        else:
            self.net_recovery = True
            self.start_receiver()
    def do_net_discovery(self, line):
        """send a net discovery packet (see also net_recovery). This will cause
all cameras with network recovery enabled to reply as if device_info?action=get
was executed. The reply is sent to port + 1.
"""
        self.tx_socket.sendto(self.magic_bytes + "\r\n", (Cmd.bcast, self.port))
        self.start_receiver()
    def kill_receiver(self):
        if self.receiver is not None:
            self.receiver.exit_ = True
            self.receiver.join()
    def start_receiver(self):
        self.kill_receiver()
        self.receiver = Receiver(self.magic_bytes, self.port)
        self.receiver.start()
    def do_raw_command(self, line):
        "send raw command, (in inp_file mode, does not support net recovery)"
        params = line.split()
        if params[0][0:4] == "cmd=":
            self.command('raw_command', line, False)
        elif params[0][0:9] == "inp_file=":
            self.send_file(params[1], 'raw_command?' + params[0])
        else:
            print "must have cmd= or inp_file="
    def do_firmware(self, line):
        "send firmare, rest of the line should be file name (does not support net recovery)"
        self.send_file(line, "firmware?file=" + line)
    def send_file(self, filename, command):
        for camera in self.ip:
            try:
                fp = open(filename, "r")
            except:
                print "Unable open", filename
                return
            conn = httplib.HTTPConnection(camera)
            conn.request("POST", "/cgi-bin/" + command, fp)
            conn.set_debuglevel(1)
            resp = conn.getresponse()
            if len(self.ip) == 1:
                print resp.read()
            else:
                print camera, ': ', resp.read()
    def do_Logfile(self, line):
        "Fetch camera logfile (does not support net_recovery, camera must be in test mode)"
        camera = self.ip[0]
        try:
            conn = httplib.HTTPConnection(camera)
            conn.request("GET", "/cgi-bin/test?logfile=1")
            resp = conn.getresponse()
            content = resp.getheader("Content-Disposition")
            content_list = content.split('"')
            filename = content_list[1]
            outf = open(filename, "w")
            while True:
                buffer = resp.read(64 * 1024 * 1024)
                if len(buffer) == 0: break
                print >> outf, buffer,
            outf.close()
            print "Written", filename
        except httplib.BadStatusLine:
            print "Error: Bad Status Line"
        except socket.error:
            print "Error: connection refused"
    def command(self, cmd, line, has_action = True):
        params = line.split()
        if has_action and self.find_action(params): has_action = False
        count = len(params)
        if cmd == 'stream':
            if count > 0 and params[0].isdigit(): 
                params[0] = 'id=' + params[0]
                count -= 1
            else:
                params.append('id=0')
        if has_action:
            params.append('action=get')
            if count > 0 : params[-1] = 'action=set'
        if len(params) > 0:
            cmd +=  '?' + '&'.join(params)
        if self.net_recovery:
            if self.mac_address is None:
                print "Error: missing mac_address"
                return
            request = self.magic_bytes + "\r\n" \
                    + self.mac_address + "\r\n" \
                    + cmd + "\r\n"
            print request
            self.tx_socket.sendto(request, (Cmd.bcast , self.port))
            return
        request = '/cgi-bin/' + cmd
        print "'" + request + "'"
        try:
            n = 0
            for camera in self.ip:
                conn = httplib.HTTPConnection(camera)
                conn.request("GET", request)
                if n == 0:
		            conn.set_debuglevel(1)
                resp = conn.getresponse()
                if len(self.ip) == 1:
                    print resp.read()
                else:
                    print camera, ': ', resp.read()
                n += 1
        except httplib.BadStatusLine:
            print "Error: Bad Status Line"
        except socket.error:
            print "Error: connection refused"
    def find_action(self, params):
        for param in params:
            elems = param.split('=')
            if len(elems) and elems[0] == 'action': return True
        return False       

if __name__ == "__main__":
    camera = None
    if len(sys.argv) > 1: camera = sys.argv[1]
    acmd = Cmd(camera)
    acmd.cmdloop()

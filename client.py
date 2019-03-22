#!/usr/bin/env python

import socket
import time
from cmd import Cmd
from string import atoi,atof

ip="127.0.0.1"
port=9999

DataRate=0.0
AcqRate=0.0
FileSize=0
AcqRun=0
FileName=""
RunNum=0


s=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
try:
    s.connect((ip,port))
except Exception as e:
    print("Connection refused")
    exit(-1)
else:
    pass
finally:
    pass

def PrintStatus(verb=1):
    global AcqRun,AcqDur,DataRate,AcqRate,FileSize,FileName
    global s
    if verb==1:
        if AcqRun==1:
            print("Status: Running")
        else:
            print("Status: Stopped")
        print("FileName: %s RunNum: %d"%(FileName,RunNum))
        print("DataRate: %-5.2f kB/s AcqRate: %-5.2f kHz FileSize:%.3f MB" %(DataRate,AcqRate,FileSize/1000.))

def Update(cli):
    global AcqRun,AcqDur,DataRate,AcqRate,FileSize,FileName
    global s
    Get('status',cli)
    Get('filename',cli)
    Get('runnum',cli)

def Send(msg,cli):
    global s
    try:
        s.send(b"%s"%msg)
    except Exception as e:
        print "Connection refused."
        cli.prompt='JUNA (Disconnected)>'
    else:
        pass
    finally:
        pass

def Get(com,cli,verb=1):
    global AcqRun,AcqDur,DataRate,AcqRate,FileSize,FileName,RunNum
    global s
    if com=="status":
        Send(b'get status',cli)
        fb_raw=s.recv(100)
        fb=fb_raw.split()
        AcqRun=atoi(fb[0])
        DataRate=atof(fb[1])
        AcqRate=atof(fb[2])
        FileSize=atoi(fb[3])
        AcqDur=atoi(fb[4])
        # PrintStatus(verb)
        # print AcqRun,AcqDur,DataRate,AcqRate,FileSize
    elif com=="filename":
        s.send(b'get filename')
        fb_raw=s.recv(100)
        fb=fb_raw.split()
        FileName=fb[0]
        # PrintStatus(verb)
        # print "FileName:", FileName
    elif com=="runnum":
        s.send(b'get runnum')
        fb_raw=s.recv(100)
        fb=fb_raw.split()
        RunNum=atoi(fb[0])
        # PrintStatus(verb)

def Set(com,arg,cli):
    global AcqRun,AcqDur,DataRate,AcqRate,FileSize,FileName
    global s
    if com=="filename":
        s.send(b'setfilename %s'%arg)
    elif com=="runnum":
        s.send(b'setrunnum %s' % arg)


class CLI(Cmd):

    prompt = 'JUNA (%s:%d)>' %(ip,port)
    # intro = 'FileName: %s RunNum: %s' %(FileName,RunNum)

    def __init(self):
        Cmd.__init__(self)

    def do_send(self,line):
        Send(b"%s"%line,self)

    def do_start(self,line):
        Send(b"start",self)
        time.sleep(1);
        PrintStatus()

    def do_stop(self,line):
        Send(b"stop",self)
        time.sleep(1);
        PrintStatus()

    def do_quit(self,line):
        Send(b"quit",self)

    def do_exit(self,line):
        s.close()
        exit(0)

    def do_get(self,line):
        Get(line,self)

    def do_set(self,line):
        line=line.split()
        print line[0],line[1]
        Set(line[0],line[1],self)
        
    def do_connect(self,line):
        global ip,port,s
        s.close()
        s=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        try:
            print("Connecting %s:%d"%(ip,port))
            s.connect((ip,port))
            self.prompt='JUNA (%s:%d)>'%(ip,port)
        except Exception as e:
            print("Connection refused")
            self.prompt='JUNA (Disconnected)>'
    def do_print(self,line):
        Update(self)
        PrintStatus()

    def default(self,line):
        print("Command not exist")

if __name__ == '__main__':
    cli = CLI()
    Update(cli)
    cli.intro='FileName: %s RunNum: %d' %(FileName,RunNum)
    cli.cmdloop()

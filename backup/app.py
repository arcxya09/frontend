#!/usr/bin/env python2
#
from Tkinter import *
import socket
import time
from string import atoi,atof
# from cmd import Cmd


class Application(Frame):
    """docstring for Application"""

    def __init__(self, master=None):
        Frame.__init__(self, master)
        self.window_init()
        self.var_init()
        self.status_panel_init()
        self.connect_panel_init()
        self.file_panel_init()

        self.autoupdate()

    def var_init(self):
        self.port = IntVar(value=9999)
        self.ip = StringVar(value='127.0.0.1')
        self.connect_status = StringVar(value='disconnect')

        self.file_status = StringVar(value='---')
        self.runnum_status = StringVar(value='---')
        self.connect_button_title = StringVar(value='connect')

        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        self.state = DISABLED

        self.margin = 30

        self.autoupdate_flag = True

    def window_init(self):
        self.master.title('DAQ')
        self.master.geometry('330x500')

    def status_panel_init(self):

        self.connect_status_label = Label(self.master, textvariable=self.connect_status, bg='red', width=10, justify='center')
        self.connect_status_label.place(x=5, y=5, anchor='nw')

        self.file_status_label = Label(self.master, textvariable=self.file_status, width=10, justify='center')
        self.file_status_label.place(x=105, y=5, anchor='nw')

        self.runnum_status_label = Label(self.master, textvariable=self.runnum_status, width=10, justify='center')
        self.runnum_status_label.place(x=205, y=5, anchor='nw')

    def connect_panel_init(self):
        def func_connect():
            if self.connect_status.get() == 'disconnect':
                try:
                    self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    self.s.connect((self.ip.get(), self.port.get()))
                    self.update()
                except Exception as e:
                    print("Connection refused")
                    return -1
                self.connect_button_title.set('disconnect')
                # self.connect_button_title.set('connect')
                self.connect_status.set('connect')
                self.connect_status_label['bg'] = 'green'
                self.state = NORMAL
                self.file_entry['state'] = self.state
                self.runn_entry['state'] = self.state
                self.file_set_button['state'] = self.state

                self.file_entry.delete(0, END)
                self.runn_entry.delete(0, END)
                self.file_entry.insert(0, self.file_status.get())
                self.runn_entry.insert(0, self.runnum_status.get())
            else:
                print(self.ip.get(), self.port.get())
                try:
                    self.s.shutdown(socket.SHUT_RDWR)
                except Exception as e:
                    print("Connection refused")
                    return -1
                self.connect_status.set('disconnect')
                self.connect_button_title.set('connect')
                self.connect_status_label['bg'] = 'red'
                self.state = DISABLED
                self.file_entry['state'] = self.state
                self.runn_entry['state'] = self.state
                self.file_set_button['state'] = self.state

            return 0

        self.ip_entry = Entry(self.master, width=10, textvariable=self.ip, justify='center')
        self.port_entry = Entry(self.master, width=10, textvariable=self.port, justify='center')

        self.ip_entry.place(x=2, y=5 + self.margin, anchor='nw')
        self.port_entry.place(x=105, y=5 + self.margin, anchor='nw')

        self.connect_button = Button(self.master, textvariable=self.connect_button_title, width=10,height=1, command=func_connect)
        self.connect_button.place(x=205, y=5 + self.margin, anchor='nw')

    def file_panel_init(self):
        def func_file_set():
            self.file_status.set(self.file_entry.get())
            self.runnum_status.set(self.runn_entry.get())
            self.set('filename',self.file_entry.get())
            self.set('runnum',self.runn_entry.get())

        self.file_entry = Entry(self.master, width=10, justify='center', state=DISABLED)
        self.runn_entry = Entry(self.master, width=10, justify='center', state=DISABLED)

        self.file_entry.place(x=2, y=5 + 2 * self.margin, anchor='nw')
        self.runn_entry.place(x=105, y=5 + 2 * self.margin, anchor='nw')

        self.file_set_button = Button(self.master, text='set', width=10, state=self.state, command=func_file_set)
        self.file_set_button.place(x=205, y=5 + 2 * self.margin, anchor='nw')

    def run_status_panel_init(self):

        self.connect_status_label = Label(self.master, textvariable=self.connect_status, bg='red', width=10, justify='center')
        self.connect_status_label.place(x=5, y=5, anchor='nw')

        self.file_status_label = Label(self.master, textvariable=self.file_status, width=10, justify='center')
        self.file_status_label.place(x=105, y=5, anchor='nw')

        self.runnum_status_label = Label(self.master, textvariable=self.runnum_status, width=10, justify='center')
        self.runnum_status_label.place(x=205, y=5, anchor='nw')


    def send(self, msg):
        try:
            self.s.send(b"%s" % msg)
        except Exception as e:
            print("Connection refused.")
            self.connect_button_title.set('disconnect')
            self.connect_status.set('disconnect')
            self.connect_status_label['bg'] = 'red'

    def get(self, com):
        if com == "status":
            self.send(b'get status')
            fb_raw = self.s.recv(100)
            fb = fb_raw.split()
            self.AcqRun = atoi(fb[0])
            self.DataRate = atof(fb[1])
            self.AcqRate = atof(fb[2])
            self.FileSize = atoi(fb[3])
            self.AcqDur = atoi(fb[4])
            # PrintStatus(verb)
            # print AcqRun,AcqDur,DataRate,AcqRate,FileSize
        elif com == "filename":
            self.s.send(b'get filename')
            fb_raw = self.s.recv(100)
            fb = fb_raw.split()
            self.file_status.set(fb[0])
            # PrintStatus(verb)
            # print "FileName:", FileName
        elif com == "runnum":
            self.s.send(b'get runnum')
            fb_raw = self.s.recv(100)
            fb = fb_raw.split()
            self.runnum_status.set(atoi(fb[0]))
            # PrintStatus(verb)
            # print RunNum

    def set(self, com, arg):
        if com == "filename":
            self.s.send(b'setfilename %s' % arg)
        elif com == "runnum":
            self.s.send(b'setrunnum %s' % arg)

    def update(self):
        # print('updating')
        self.get('status')
        self.get('filename')
        self.get('runnum')


    def autoupdate(self):
        if self.autoupdate_flag == True and self.connect_status.get() == 'connect':
            try:
                self.update()
            except Exception as e:
                print("Connection refused")

        self.after(1000, self.autoupdate)

if __name__ == '__main__':
    app = Application()
    app.mainloop()

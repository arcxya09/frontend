#!/usr/bin/env python2
#
from Tkinter import *
import socket
import time
from string import atoi,atof
# from cmd import Cmd
# from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
# from matplotlib.figure import Figure


class Application(Frame):
    """docstring for Application"""

    def __init__(self, master=None):
        Frame.__init__(self, master)
        self.window_init()
        self.var_init()
        self.status_panel_init()
        self.connect_panel_init()
        # self.file_panel_init()
        self.run_status_panel_init()
        # self.figure_init()
        self.run_control_panel_init()
        self.gui_pack()
        self.autoupdate()
        self.supervisor()

    def var_init(self):
        self.port = IntVar(value=9999)
        self.ip = StringVar(value='127.0.0.1')
        self.connect_status = StringVar(value='disconnect')

        self.FileName = '---'
        self.RunNum = 0

        self.DataRate = 0.0
        self.AcqRate = 0.0
        self.AcqRun=0
        self.FileSize = 0.0

        self.runnum_status = StringVar(value='Run Number: ---')
        self.file_status = StringVar(value='File Name: ---')
        self.connect_button_title = StringVar(value='connect')

        self.data_rate_status = StringVar(value='DataRate: --- kB/s')
        self.acq_rate_status = StringVar(value='AcqRate: --- kHz')
        self.file_size_status = StringVar(value='FileSize: --- MB')

        self.file_size_checkbutton=IntVar(value=1)

        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        self.state = DISABLED


        self.margin = 30

        self.autoupdate_flag = True

    def gui_pack(self):
        ##################
        self.status_panel.pack(side=TOP, fill=X)
        self.connect_status_label.pack(side=LEFT, padx=3)
        self.file_status_label.pack(side=LEFT,padx=1)
        self.runnum_status_label.pack(side=LEFT,padx=1)

        ##################
        self.connect_panel.pack(side=TOP, fill=X)
        self.ip_entry.pack(side=LEFT,padx=1)
        self.port_entry.pack(side=LEFT,padx=1)
        self.connect_button.pack(side=LEFT,padx=1)

        self.file_entry.pack(side=LEFT,padx=1)
        self.runn_entry.pack(side=LEFT,padx=1)
        self.file_set_button.pack(side=LEFT,padx=1)

        ##################
        self.run_status_panel.pack(side=TOP, fill=X,pady=10)
        self.run_status_label.pack(side=LEFT,padx=1)
        self.data_rate_label.pack(side=LEFT,padx=10)
        self.acq_rate_label.pack(side=LEFT,padx=1)
        self.file_size_label.pack(side=LEFT,padx=1)

        ##################
        self.run_control_panel.pack(side=TOP, fill=X,pady=10)
        self.run_start_button.pack(side=LEFT,padx=1)
        self.run_stop_button.pack(side=LEFT,padx=5)
        self.max_file_size_checkbutton.pack(side=LEFT,ipadx=10)
        self.max_file_size_label.pack(side=LEFT,padx=5)
        self.max_file_size_entry.pack(side=LEFT,padx=5)

    def window_init(self):
        self.master.title('DAQ')
        self.master.geometry('630x700')
        self['bd'] = 5
        self.pack()

    def status_panel_init(self):

        self.status_panel = Frame(self, bd=5)

        self.connect_status_label = Label(self.status_panel, textvariable=self.connect_status, bg='red', width=10, padx=0, pady=0)

        self.file_status_label = Label(self.status_panel, textvariable=self.file_status, width=20)

        self.runnum_status_label = Label(self.status_panel, textvariable=self.runnum_status, width=20)

    def connect_panel_init(self):
        def func_connect():
            if self.connect_status.get() == 'disconnect':
                try:
                    self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    self.s.connect((self.ip.get(), self.port.get()))
                    self.update()
                except Exception as e:
                    print("Connection refused 3")
                    return -1
                self.after_connect()

                self.file_entry.delete(0, END)
                self.runn_entry.delete(0, END)
                self.file_entry.insert(0, self.FileName)
                self.runn_entry.insert(0, self.RunNum)
            else:
                try:
                    self.s.shutdown(socket.SHUT_RDWR)
                except Exception as e:
                    print("Connection refused 4")
                    return -1
                self.after_disconnect()

            return 0

        self.connect_panel = Frame(self, bd=5)

        self.port_entry = Entry(self.connect_panel, width=8, textvariable=self.port, justify='center')
        self.ip_entry = Entry(self.connect_panel, width=15, textvariable=self.ip, justify='center')


        self.connect_button = Button(self.connect_panel, textvariable=self.connect_button_title, width=7,height=1, command=func_connect)

        def func_file_set():
            self.FileName=self.file_entry.get()
            self.RunNum=atoi(self.runn_entry.get())
            self.set('filename',self.file_entry.get())
            self.set('runnum',self.runn_entry.get())
            self.file_status.set("File Name: %s" % self.FileName)
            self.runnum_status.set("Run Number: %d" % self.RunNum)


        self.file_entry = Entry(self.connect_panel, width=15, justify='center', state=DISABLED)
        self.runn_entry = Entry(self.connect_panel, width=8, justify='center', state=DISABLED)


        self.file_set_button = Button(self.connect_panel, text='set', width=7, state=self.state, command=func_file_set)

    # def figure_init(self):
    #     self.figure_panel = Frame(self, bd=5)
    #     self.figure_panel.pack(side=TOP, fill=X, expand=True)

    #     self.fig = Figure()
    #     self.ax = self.fig.add_subplot(111)
    #     self.ax.set_xlabel("X axis")
    #     self.ax.set_ylabel("Y axis")
    #     # self.ax.grid()

    #     self.graph = FigureCanvasTkAgg(self.fig, master=self.figure_panel)
    #     self.graph.get_tk_widget().pack(fill=X, expand=True)

    def run_status_panel_init(self):

        self.run_status_panel=Frame(self,bd=5)

        self.run_status_label = Label(self.run_status_panel, text=" ", bg='red',width=2, justify=LEFT)

        self.data_rate_label = Label(self.run_status_panel, textvariable=self.data_rate_status, width=18, justify=LEFT)

        self.acq_rate_label = Label(self.run_status_panel, textvariable=self.acq_rate_status, width=18, justify=LEFT)

        self.file_size_label = Label(self.run_status_panel, textvariable=self.file_size_status, width=18, justify=LEFT)


    def run_control_panel_init(self):

        self.run_control_panel=Frame(self,bd=5)


        self.run_start_button = Button(self.run_control_panel, text='START', width=7, state=DISABLED, command=self.func_run_start)


        self.run_stop_button = Button(self.run_control_panel, text='STOP', width=7, state=DISABLED, command=self.func_run_stop)

        self.max_file_size_checkbutton=Checkbutton(self.run_control_panel,state=DISABLED,variable=self.file_size_checkbutton,onvalue=1,offvalue=0,width=1,anchor='e')
        self.max_file_size_label = Label(self.run_control_panel, text='Maximum File Size (MB)', width=18, anchor='w')
        self.max_file_size_entry = Entry(self.run_control_panel, width=10, justify=CENTER)
        self.max_file_size_entry.insert(0,2000);


    def send(self, msg):
        try:
            self.s.send(b"%s" % msg)
        except Exception as e:
            print("Connection refused 1.")
            self.connect_button_title.set('disconnect')
            self.connect_status.set('disconnect')
            self.connect_status_label['bg'] = 'red'

    def get(self, com):
        try:
            if com == "status":
                self.send(b'get status')
                fb_raw = self.s.recv(100)
                fb = fb_raw.split()
                self.AcqRun = atoi(fb[0])
                self.DataRate=atof(fb[1])
                self.AcqRate=atof(fb[2])
                self.FileSize=atof(fb[3])
                self.AcqDur = atoi(fb[4])


                self.data_rate_status.set("DataRate: %-5.2f kB/s" % self.DataRate)
                self.acq_rate_status.set("AcqRate: %-5.2f kHz" % self.AcqRate)
                self.file_size_status.set("FileSize: %-5.2f MB" % self.FileSize)
                # PrintStatus(verb)
                # print AcqRun,AcqDur,DataRate,AcqRate,FileSize
            elif com == "filename":
                self.s.send(b'get filename')
                fb_raw = self.s.recv(100)
                fb = fb_raw.split()
                self.file_status.set("File Name: %s" % fb[0])
                self.FileName=fb[0]
                # PrintStatus(verb)
                # print "FileName:", FileName
            elif com == "runnum":
                self.s.send(b'get runnum')
                fb_raw = self.s.recv(100)
                fb = fb_raw.split()
                self.RunNum=atoi(fb[0])
                self.runnum_status.set("Run Number: %d" % atoi(fb[0]))
                # PrintStatus(verb)
                # print RunNum
        except Exception as e:
            self.connect_status.set('disconnect')
            return
    

    def set(self, com, arg):
        try:
            if com == "filename":
                self.s.send(b'setfilename %s' % arg)
            elif com == "runnum":
                self.s.send(b'setrunnum %s' % arg)
        except Exception as e:
            self.connect_status.set('disconnect')
            return
        
        

    def update(self):
        self.get('status')
        print 'test'
        self.get('filename')
        self.get('runnum')

    def autoupdate(self):
        if self.autoupdate_flag==True and self.connect_status.get() == 'connect':
            try:
                self.update()
            except Exception as e:
                connect_status.set('disconnect')
                print("Connection refused 2")
        self.after(1000, self.autoupdate)

    def supervisor(self):
        # self.update()
        if self.connect_status.get()=='disconnect':
            self.after_disconnect()

        elif self.connect_status.get()=='connect':
            self.after_connect()

            if self.AcqRun==1:
                self.after_start()
                if self.file_size_checkbutton.get()==1:
                    if self.FileSize>atoi(self.max_file_size_entry.get()):
                        self.autoupdate_flag=False
                        self.FileSize=0
                        curr_runn=self.RunNum
                        self.func_run_stop()
                        while self.AcqRun != 0:
                            time.sleep(1)
                            self.update()
                        self.func_run_start()
                        self.autoupdate_flag=True
            else:
                self.after_stop()



        self.after(100,self.supervisor)



    def after_disconnect(self):
        self.connect_status.set('disconnect')
        self.connect_button_title.set('connect')
        self.connect_status_label['bg'] = 'red'
        self.connect_status_label['fg'] = 'black'
        self.state = DISABLED
        self.file_entry['state'] = self.state
        self.runn_entry['state'] = self.state
        self.file_set_button['state'] = self.state
        self.run_start_button['state']=self.state
        self.run_stop_button['state']=self.state
        self.max_file_size_entry['state']=self.state
        self.max_file_size_label['state']=self.state
        self.max_file_size_checkbutton['state'] = self.state
        self.AcqRun=0

        self.run_status_label['bg']='red'

        self.ip_entry['state']=NORMAL
        self.port_entry['state']=NORMAL

    def after_connect(self):
        self.connect_status.set('connect')
        self.connect_button_title.set('disconnect')
        self.connect_status_label['bg'] = 'green'
        self.connect_status_label['fg'] = 'white'
        self.state = NORMAL
        self.file_entry['state'] = self.state
        self.runn_entry['state'] = self.state
        self.file_set_button['state'] = self.state
        self.max_file_size_checkbutton['state'] = self.state

        self.ip_entry['state']=DISABLED
        self.port_entry['state']=DISABLED

    def after_start(self):
        self.file_entry['state'] = DISABLED
        self.runn_entry['state'] = DISABLED
        self.file_set_button['state'] = DISABLED
        self.run_start_button['state'] =DISABLED
        self.max_file_size_entry['state']= DISABLED
        self.max_file_size_label['state'] =DISABLED
        self.max_file_size_checkbutton['state'] = DISABLED

        self.run_stop_button['state']=NORMAL
        self.run_status_label['bg']='green'


    def after_stop(self):
        self.file_entry['state'] = NORMAL
        self.runn_entry['state'] = NORMAL
        self.file_set_button['state'] = NORMAL
        self.run_start_button['state']= NORMAL
        self.max_file_size_entry['state'] =NORMAL
        self.max_file_size_label['state']= NORMAL

        if self.file_size_checkbutton.get()==0:
            self.max_file_size_entry['state']=DISABLED
        else:
            self.max_file_size_checkbutton['state'] = NORMAL
        self.run_stop_button['state']=DISABLED
        self.run_status_label['bg']='red'


    def func_run_start(self):
        self.send(b"start")

    def func_run_stop(self):
        self.send(b"stop")

if __name__ == '__main__':
    app = Application()
    app.mainloop()

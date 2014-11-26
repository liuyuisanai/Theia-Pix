#!/usr/bin/env python
from Tkinter import *
import Tkinter as tk
import tkFileDialog as tkFile
import re
import subprocess
import os as os
from nsh import NSH
from sys import argv, exit
from datetime import datetime

class Application(tk.Frame):
    def __init__(self, master=None):
        tk.Frame.__init__(self, master)
        self.createWidgets()
        self.logs_dir = "/fs/microsd/log"
        self.dir_to_save = os.getcwd()
        master.columnconfigure(0, weight=1)
        master.rowconfigure(0,weight=1)


    def createWidgets(self):
        # == create objects ==
        scrollbar = tk.Scrollbar(self, orient=VERTICAL)
        self.loglist = tk.Listbox(self,
                selectmode=EXTENDED,
                yscrollcommand=scrollbar.set,
                height = 30,
                width = 40
                )
        scrollbar.config(command=self.loglist.yview)
        # == Buttons ==
        self.connectButton = tk.Button(self, text="Connect",
                command = self.connect)
        self.listButton = tk.Button(self, text="List logs",
                command=self.listlog, state=DISABLED)
        self.getButton  = tk.Button(self, text="Get selected",
                command=self.getlog, state=DISABLED)
        self.lastButton = tk.Button(self, text="Get last log",
                command=self.lastlog, state=DISABLED)
        self.saveDirButton = tk.Button(self, text="Select save path",
                command=self.savedir)
        self.flashButton = tk.Button(self, text="Flash firmware",
                command=self.flash)
        # == place objects in places ==
        self.connectButton.grid(row=0, column=0, sticky="nesw")
        self.flashButton.grid(  row=0, column=1, sticky="nesw")
        self.listButton.grid(   row=1, column=0, sticky="nesw")
        self.getButton.grid(    row=1, column=1, sticky="nesw")
        self.lastButton.grid(   row=2, column=0, sticky="nesw")
        self.saveDirButton.grid(row=2, column=1, sticky="nesw")
        self.loglist.grid(row=3, column=0, columnspan=2, sticky="nesw")
        self.grid(sticky="nesw")
        self.grid_rowconfigure(0, weight=1)
        self.grid_columnconfigure(0, weight=1)
        #scrollbar.grid()

    def flash(self):
        if os.name == 'nt':
            port="--port=COM32,COM31,COM30,COM29,COM28,COM27,COM26,COM25,COM24,COM23,COM22,COM21,COM20,COM19,COM18,COM17,COM16,COM15,COM14,COM13,COM12,COM11,COM10,COM9,COM8,COM7,COM6,COM5,COM4,COM3,COM2,COM1,COM0"
        else:
            port = "--port=/dev/ttyA*,/dev/tty.usbmodem*"
        px4fmu_file = tkFile.askopenfilename()
        print px4fmu_file, port
        subprocess.call(['../px_uploader.py', port, px4fmu_file])


    def connect(self):
        try:
            self.nsh = NSH()
            self.port = self.nsh.wait_and_open_nsh()
            self.listButton.config(state=ACTIVE)
            self.getButton.config(state=ACTIVE)
            self.lastButton.config(state=ACTIVE)
            self.flashButton.config(state=DISABLED)
        except:
            print "Cannot connect to device"

    def savedir(self):
        self.dir_to_save = tkFile.askdirectory()


    def listlog(self):
        self.loglist.delete(0,END)
        for file in self.nsh.get_all_files(self.logs_dir):
            self.loglist.insert(END, file)
            self.loglist.see(END)
            self.loglist.select_anchor(END)


    def getlog(self):
        selected = self.loglist.curselection()
        for i in selected:
            cur = self.logs_dir + self.loglist.get(i)
            if self.nsh.file_exists(cur):
                            data = self.nsh.download_file(cur)
                            with open(self.dir_to_save+'/'+cur[cur.rindex('/')+1:], "wb") as f:
                                f.write(data)
                            print("Success.")
            else:
                print("File not found")

    def lastlog(self):
        dirs = self.nsh.ls_dir(self.logs_dir)
        # Keep entries only "date/" entries
        dirs = filter(lambda d: re.sub("[0-9]{4}\-[0-9]{2}\-[0-9]{2}", '', d) == "/", dirs)
        if dirs:
            dirs.sort(key=lambda x: datetime.strptime(x, '%Y-%m-%d/'))

            last_dir = dirs.pop()
            print("Getting logs from {}/{}".format(self.logs_dir, last_dir))

            logs = self.nsh.ls_dir(self.logs_dir + '/' + last_dir)
            # Keep entries only which end with .bin
            logs = filter(lambda l: l.endswith('.bin'), logs)
            if logs:
                logs.sort(key=lambda x: datetime.strptime(x, "%H_%M_%S.bin"))

                last_log = logs.pop()
                data = self.nsh.download_file(self.logs_dir + '/' + last_dir + last_log)
                with open(self.dir_to_save+'/'+last_log, "wb") as f:
                    f.write(data)

                preflight = "preflight_perf" + last_log[:-3] + "txt"
                if self.nsh.file_exists(self.logs_dir + '/' + last_dir + preflight):
                    data = self.nsh.download_file(self.logs_dir + '/' + last_dir + preflight)
                    with open(self.dir_to_save+'/'+preflight, "wb") as f:
                        f.write(data)
                else:
                    print("Cannot find {} file (ignoring it).".format(preflight))

                print("Success.")
            else:
                print("Cannot find any log with timestamp!")
        else:
            print("Unable to find any log with date")

root = tk.Tk()
app = Application(root)
app.master.title('Sample application')
app.mainloop()

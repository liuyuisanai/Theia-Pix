#!/usr/bin/env python
from Tkinter import *
import Tkinter as tk
import re
from nsh import NSH
from sys import argv, exit
from datetime import datetime

class Application(tk.Frame):
    def __init__(self, master=None):
        tk.Frame.__init__(self, master)
        self.grid(sticky="nesw")
        self.createWidgets()
        self.nsh = NSH()
        self.nsh.wait_and_open_nsh()
        self.logs_dir = "/fs/microsd/log"
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
        self.quitButton = tk.Button(self, text='Quit',
            command=self.quit)
        self.listButton = tk.Button(self, text="List logs",
                command=self.listlog)
        self.getButton  = tk.Button(self, text="Get selected",
                command=self.getlog)
        self.lastButton = tk.Button(self, text="Get last log",
                command=self.lastlog)
        # == place objects in places ==
        self.listButton.grid(sticky="nesw")
        self.getButton.grid(sticky="nesw")
        self.lastButton.grid(sticky="nesw")
        self.quitButton.grid(sticky="nesw")
        self.loglist.grid(sticky="nesw")
        #scrollbar.grid()

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
                            with open(cur[cur.rindex('/')+1:], "wb") as f:
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
                with open(last_log, "wb") as f:
                    f.write(data)

                preflight = "preflight_perf" + last_log[:-3] + "txt"
                if self.nsh.file_exists(self.logs_dir + '/' + last_dir + preflight):
                    data = self.nsh.download_file(self.logs_dir + '/' + last_dir + preflight)
                    with open(preflight, "wb") as f:
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

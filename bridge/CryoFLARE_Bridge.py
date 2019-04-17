#------------------------------------------------------------------------------
#
# Author: Andreas Schenk
# Friedrich Miescher Institute, Basel, Switzerland
#
# This file is part of CryoFlare
#
# Copyright (C) 2017-2019 by the CryoFlare Authors
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 3.0 of the License.
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License
# along with CryoFlare.  If not, see <http:#www.gnu.org/licenses/>.
#
#------------------------------------------------------------------------------
# service code adapted from http:#www.chrisumbel.com/article/windows_services_in_python

import msvcrt
import comtypes
import comtypes.client as cc
import win32service  
import win32serviceutil  
import win32event  
import os   
import sys
import logging,logging.handlers

poll_time=1000
lock_file="E:/Data/CryoFLARE_Bridge/cryoflare_bridge.lock"
command_file="E:/Data/CryoFLARE_Bridge/cryoflare_bridge.cmd" 
lock_string="BRIDGE"


# set up logging 
log_file=os.path.splitext(__file__)[0]+".log"
log = logging.getLogger()
log.setLevel(logging.INFO)
f = logging.Formatter('%(asctime)s %(process)d:%(thread)d %(name)s %(levelname)-8s %(message)s')
h=logging.StreamHandler(sys.stdout)
h.setLevel(logging.NOTSET)
h.setFormatter(f)
log.addHandler(h)
h=logging.handlers.RotatingFileHandler(log_file,maxBytes=1024**2,backupCount=1)
h.setLevel(logging.NOTSET)
h.setFormatter(f)
log.addHandler(h)
del h,f
#hook to log unhandled exceptions
def excepthook(type,value,traceback):
    logging.error("Unhandled exception occured",exc_info=(type,value,traceback))
    #Don't need another copy of traceback on stderr
    if old_excepthook!=sys.__excepthook__:
        old_excepthook(type,value,traceback)
old_excepthook = sys.excepthook
sys.excepthook = excepthook
del log_file



class CryoFLAREBridge(win32serviceutil.ServiceFramework):  
    # you can NET START/STOP the service by the following name  
    _svc_name_ = "CryoFLAREBridge"  
    # this text shows up as the service name in the Service  
    # Control Manager (SCM)  
    _svc_display_name_ = "CryoFLARE TEM scripting Bridge"  
    # this text shows up as the description in the SCM  
    _svc_description_ = "This service provides a bridge between CryoFLARE and Advanced TEM scripting"
    # list of commands
    _commands_={}
    
        
    def __init__(self, args):  
        win32serviceutil.ServiceFramework.__init__(self,args)  
        # create an event to listen for stop requests on  
        self.hWaitStop = win32event.CreateEvent(None, 0, 0, None) 
        
    # core logic of the service     
    def SvcDoRun(self):
        log.info("Starting service")
        import servicemanager  
        advanced_instrument = cc.CreateObject("TEMAdvancedScripting.AdvancedInstrument")
        rc = None  
        while rc != win32event.WAIT_OBJECT_0:
            if os.path.exists(command_file) and os.path.getsize(command_file)>0 and not os.path.exists(lock_file):
                try:
                    with open(lock_file,"w") as f:
                            msvcrt.locking(f.fileno(), msvcrt.LK_NBLCK, len(lock_string))
                            f.write(lock_string)
                            with open(command_file,"r") as f_com:
                                for line in f_com.readlines():
                                    log.info("executing command: %s" % line.rstrip())
                                    self.ExecuteCommand(advanced_instrument,line.rstrip())
                            os.remove(command_file)
                            msvcrt.locking(f.fileno(), msvcrt.LK_UNLCK, len(lock_string))
                finally:
                    os.remove(lock_file)
            # block for 1 seconds and listen for a stop event  
            rc = win32event.WaitForSingleObject(self.hWaitStop, poll_time)  
                
        
    # called when we're being shut down      
    def SvcStop(self):  
        log.info("Stopping service")
        # tell the SCM we're shutting down  
        self.ReportServiceStatus(win32service.SERVICE_STOP_PENDING)  
        # fire the stop event  
        win32event.SetEvent(self.hWaitStop)  

    @staticmethod
    def RegisterCommand(name,c):
        log.info("Registering: %s" % (name))
        CryoFLAREBridge._commands_[name]=c

    def ExecuteCommand(self,advanced_instrument,line):
        sp=line.split()
        command=sp.pop(0)
        if command in CryoFLAREBridge._commands_:
            CryoFLAREBridge._commands_[command](advanced_instrument,sp)

def SkipPhasePlate(instrument,args):
    log.info("Moving to next phase plate position")
    pp = instrument.Phaseplate
    pp.SelectNextPresetPosition()


CryoFLAREBridge.RegisterCommand("SkipPhasePlate",SkipPhasePlate)

if __name__ == '__main__':
    win32serviceutil.HandleCommandLine(CryoFLAREBridge)  

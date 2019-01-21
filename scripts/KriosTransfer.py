#!/usr/prog/sb/em/sw/EMAN2/2.2/bin/python
import sys
from PyQt4.QtGui import *
from PyQt4.QtCore import *
import os
from itertools import chain


transfer_node="chbscl-1-2a-int.nibr.novartis.net"
class RSyncProcess(QObject):
	progress = pyqtSignal()
	finished = pyqtSignal()
	error = pyqtSignal()

	def __init__(self):
		QObject.__init__(self)
		self.process=QProcess()
		self.process.error.connect(self.onProcessError)
		self.process.finished.connect(self.onProcessFinished)
		self.process.readyReadStandardOutput.connect(self.onProcessStandardOutput)
		self.process.readyReadStandardError.connect(self.onProcessStandardError)
		self.percentage=0

	def start(self,username,password,items,destination_host,destination):
		filelist=list(chain.from_iterable(items))
                if len(filelist)==0:
                        self.onProcessFinished()
                        return
                arguments=QStringList()
		if password:
			program="sshpass"
			arguments.append("-e")
			arguments.append(os.path.join(sys.path[0],"rsync"))
			env = QProcessEnvironment.systemEnvironment()
			env.insert("SSHPASS", password)
			self.process.setProcessEnvironment(env)
		else:
			program=os.path.join(sys.path[0],"rsync")
		arguments.append("-R")
		arguments.append("--info=progress2")
		arguments.append("-rltuv")
		arguments.append("--perms")
		arguments.append("--chmod=Dg+ws,Fg+w")
		arguments.append("--no-i-r")
		arguments+=filelist
		if username:
			arguments.append(username+"@"+destination_host+":"+destination)
		else:
			arguments.append(destination_host+":"+destination)
		self.percentage=0
		self.process.start(program,arguments)
		self.process.waitForStarted(-1)

	def percentDone(self):
		return self.percentage

	def onProcessError(self):
		self.error.emit()

	def onProcessFinished(self):
		self.finished.emit()

	def onProcessStandardOutput(self):
		output=self.process.readAllStandardOutput()
		if output:
			lines=output.split('\n')
			for line in lines:
                                f=line.indexOf("%")
                                if f!=-1:
					percentage=float(line[:f].split(" ")[-1])
					if percentage>self.percentage:
						self.percentage=percentage
						self.progress.emit()


	def onProcessStandardError(self):
		#print "An error occured"
		print self.process.readAllStandardError()
		#self.process.terminate()

	def terminate(self):
		self.process.terminate()


class RSyncTransfer(QObject):
	progress = pyqtSignal()
	finished = pyqtSignal()
	error = pyqtSignal()
	def __init__(self,num_processes):
		QObject.__init__(self)
		self.num_processes=num_processes
		self.process=[]
		for i in range(self.num_processes):
			p=RSyncProcess()
			self.process.append(p)
			p.progress.connect(self.percentDone)
			p.finished.connect(self.processFinished)
			p.progress.connect(self.progress)
			p.error.connect(self.error)

	def processFinished(self):
                print "process finished"
		self.num_finished+=1
		if self.num_finished>=len(self.process):
			self.finished.emit()

	def percentDone(self):
		return sum([p.percentDone() for p in self.process])/len(self.process)

	def start(self,username,password,items,destination_host,destination):
		self.num_finished=0
		chunk=[items[i::self.num_processes] for i in range(self.num_processes)]
		for i in range(self.num_processes):
			self.process[i].start(username,password,chunk[i],destination_host,destination)
	def cancel(self):
		for p in self.process:
			p.terminate()

class LoginDialog(QDialog):
	def __init__(self,node):
		QDialog.__init__(self)
		self.setWindowTitle("Cluster login")
		self.node=node
		layout=QFormLayout(self)
		self.username=QLineEdit(self)
		self.password=QLineEdit(self)
		self.password.setEchoMode(QLineEdit.Password)
		layout.addRow(QLabel("User name"),self.username)
		layout.addRow(QLabel("Password"),self.password)
		self.buttonbox=QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel,Qt.Horizontal,self)
		layout.addRow(self.buttonbox)
		self.buttonbox.accepted.connect(self.accept)
		self.buttonbox.rejected.connect(self.reject)

	def getLogin(self):
		while True:
			if self.exec_() == QDialog.Accepted:
				if self.check_login():
					return self.username.text(),self.password.text()
			else:
				return None,None
	def check_login(self):
		p=QProcess()
		env = QProcessEnvironment.systemEnvironment()
		env.insert("SSHPASS", self.password.text())
		p.setProcessEnvironment(env)
		p.start("sshpass",["-e","ssh", self.username.text()+"@"+self.node,"echo"])
		p.waitForStarted(-1)
		p.waitForFinished(-1)
		return p.exitCode()==0
				

class Exporter(QObject):
	loginSuccess = pyqtSignal()
	dataRead = pyqtSignal()
	def __init__(self,source,num_processes):
		QObject.__init__(self)
		self.username=None
		self.password=None
		self.source=source
		self.rsync_transfer=RSyncTransfer(num_processes)
		self.rsync_transfer.progress.connect(self.updateProgress)
		self.machine=QStateMachine()
		get_login=QState()
		read_data=QState()
		copy_intermediate=QState()
		copy_raw=QState()
		cleanup=QFinalState()

		get_login.entered.connect(self.getLogin)
		get_login.addTransition(self.loginSuccess,read_data)
		read_data.entered.connect(self.readData)
		read_data.addTransition(self.dataRead,copy_intermediate)
		copy_intermediate.entered.connect(self.copyIntermediate)		
		copy_intermediate.addTransition(self.rsync_transfer.finished,copy_raw)
		copy_raw.entered.connect(self.copyRaw)
		copy_raw.addTransition(self.rsync_transfer.finished,cleanup)
		cleanup.entered.connect(self.cleanup)

		self.machine.addState(get_login)
		self.machine.setInitialState(get_login)
		self.machine.addState(read_data)
		self.machine.addState(copy_intermediate)
		self.machine.addState(copy_raw)
		self.machine.addState(cleanup)
		
		QTimer.singleShot(0,self.machine.start)
	
	def getLogin(self):
                print "start get login"
		login_dialog=LoginDialog(transfer_node)
		self.username,self.password=login_dialog.getLogin()
		if self.username:
                        print "login success"
			self.loginSuccess.emit()
		else:
			QApplication.exit()

	def readData(self):
                print "start reading data"
		in_data=sys.stdin.readlines()
		self.raw_data={}
		self.data={}
		for line in in_data:
			key,line_data=line.split("=")
			if "raw" in key:
				self.raw_data[key]=line_data.rstrip("\n").split(",")
			else:
				self.data[key]=line_data.rstrip("\n").split(",")
                print "data read"
		self.dataRead.emit()

	def copyIntermediate(self): 
                print "starting copy of intermediate data\n"
		project_dir=os.path.split(self.source.rstrip("/"))[1]
		self.progress_dialog=QProgressDialog("Copying intermediate files to lustre...", "Abort Copy", 0, 100)
		self.progress_dialog.setMinimumDuration(0)
		self.progress_dialog.setWindowModality(Qt.WindowModal)
		self.progress_dialog.canceled.connect(self.onCancel)
		self.progress_dialog.setValue(0)
		self.rsync_transfer.start(self.username,self.password,self.data.values(),transfer_node,"/dlab/em/user_archive/"+self.username+"/"+project_dir+"/")
                print "copy of intermediate data finished\n"

	def copyRaw(self):
                print "starting copy of raw data"
		self.progress_dialog.reset()
		project_dir=os.path.split(self.source.rstrip("/"))[1]
		if "FMI" in source:
			title="Copying raw files to FMI ..."
			destination_host="172.27.28.12"
			destination="gmicro_krios/"+project_dir+"/"
			username="scheandr"
			password=""
		else:
			import time
			title="Copying raw files to dlab ..."
			destination_host=transfer_node
			destination="/dlab/em/krios/"+time.strftime("%Y")+"/"+project_dir+"/"
			username=self.username
			password=self.password
		self.progress_dialog=QProgressDialog(title, "Abort Copy", 0, 100)
		self.progress_dialog.setMinimumDuration(0)
		self.progress_dialog.setWindowModality(Qt.WindowModal)
		self.progress_dialog.canceled.connect(self.onCancel)
		self.progress_dialog.setValue(0)
		self.rsync_transfer.start(username,password,self.raw_data.values(),destination_host,destination)
                print "copy of raw data finished"

	def cleanup(self):
		self.progress_dialog.reset()		
		QApplication.exit()

	def onCancel(self):
		self.rsync_transfer.cancel()
		QApplication.exit()

	def updateProgress(self):
		self.progress_dialog.setValue(self.rsync_transfer.percentDone())



app = QApplication(sys.argv)
app.setQuitOnLastWindowClosed(False)
num_processes=4
source=sys.argv[1]
exported=Exporter(source,num_processes)
sys.exit(app.exec_())

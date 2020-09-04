################################################################################
#
# Author: Andreas Schenk
# Friedrich Miescher Institute, Basel, Switzerland
#
# This file is part of CryoFLARE
#
# Copyright (C) 2020 by the CryoFLARE Authors
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
# along with CryoFLARE.  If not, see http://www.gnu.org/licenses/.
#
################################################################################
import sys,tempfile,os,atexit,shutil,fcntl,__main__
import relion_tools
def RELION_CREATE_JOB(jobtype,job_id,jobalias,starname,nodetype,inputstar):
	jobfolder="job%03d" % (jobid)
	rln_jobpath=os.path.join(jobtype,jobfolder)
	jobalias_link=os.path.join(jobtype,jobalias)
	default_pipeline="default_pipeline.star"
	gui_projectdir=".gui_projectdir"
	### create folders and aliases ###
	os.mkdir(rln_jobpath)
    with open(rln_jobpath, os.O_RDONLY) as dir_fd:
        fcntl.flock(dir_fd,fcntl.LOCK_EX)
            if not os.path.exists(jobalias_link):
                os.symlinke(os.path.join("..",rln_jobpath),jobalias_link)
        fcntl.flock(dir_fd,fcntl.LOCK_UN)
  	SHARED_FILES("jobalias_link")
	### create node files ###
    os.mkdirs(os.path.join(".Nodes",nodetype,jobalias_link),exist_ok=True)
    os.mkdirs(os.path.join(".Nodes",nodetype,ln_jobpath),exist_ok=True)
    with open(os.path.join(".Nodes",nodetype,jobalias_link,starname)):
        pass
    with open(os.path.join(".Nodes",nodetype,rln_jobpath,starname)):
        pass
    with open(".gui_projectdir,"a"):
        pass
    import relion_tools
    relion_tools.add_to_pipeline(default_pipeline,jobtype,job_id,jobalias,inputstar,output_nodes rln_jobpath+"/"+starname+":"+nodetype)
    SHARED_FILES("default_pipeline","gui_projectdir")




def RESULTS(*args):
    for name in args:
        if name in vars(__main__):
            print "RESULT_EXPORT:%s=%s" % (name,str(vars(__main__)[name]))
        else:
            print "WARNING: result %s not defined" % (name)

def _export_files_(*args):
        export_string=args[0]
        for name in args[1:]:
            if name in vars(__main__):
                print "%s:%s=%s" % (export_string,name,str(vars(__main__)[name]))
                _exported_files_.append(vars(__main__)[name])
            else:
                print "WARNING: file %s not defined" % (name)

def FILES(*args):
    RESULTS(*args)
    _export_files_("FILE_EXPORT",*args)

def RAW_FILES(*args):
    RESULTS(*args)
    _export_files_("RAW_FILE_EXPORT",*args)

def SHARED_FILES(*args):
    _export_files_("SHARED_FILE_EXPORT",*args)


def _cleanup_():
    sys.stderr.flush()
    sys.stdout.flush()
    shutil.rmtree(scratch)

def FILES_MISSING():
    for f in _exported_files_:
        if not os.exists(f):
            return True
    return False


def LIMIT_EXPORT(name,limit_low,limit_high):
    if not name in vars(__main__):
        export="false"
    else:
        val=float(globals[name])
        if val<limit_low or val>limit_high:
            export="false"
        else:
            export="true"
    RESULT("export")

(script_name,script_ext)=os.path.splitext(os.path.basename(sys.argv[0]))
scratch=tempfile.mkdtemp(script_ext,script_name,"/dev/shm/")
atexit.register(_cleanup_)
_exported_files_=[]

for line in sys.stdin:
	sp=line.rstrip().split("=")
        globals()[sp[0]]=sp[1]

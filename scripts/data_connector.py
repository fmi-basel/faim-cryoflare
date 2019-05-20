import sys,tempfile,os,atexit,shutil,__main__

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

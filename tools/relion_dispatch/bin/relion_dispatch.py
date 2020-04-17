#------------------------------------------------------------------------------
#
# Author: Andreas Schenk
# Friedrich Miescher Institute, Basel, Switzerland
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
# along with this file.  If not, see <http:#www.gnu.org/licenses/>.
#
#------------------------------------------------------------------------------
#!/usr/bin/python
import sys,re,json,os,shlex
from jinja2 import Environment, FileSystemLoader
from subprocess import check_output
from math import floor

def parse(parameter_file):
    job={}
    job["pre_commands"]=[]
    job["post_commands"]=[]
    job["parameters"]={}
    with open(parameter_file) as f:
        job["outfile"]=f.readline().rstrip()
        job["errfile"]=f.readline().rstrip()
        for line in f.xreadlines():
            if not line or line=="\n":
                continue
            if not "command" in job or job["command"]=="":
                if line.startswith("`which relion"):
                    sp=shlex.split(line)
                    while not sp[0].startswith("--") and len(sp)>0:
                        s=sp.pop(0)
			if s.startswith("relion"):
			    job["command"]=s.rstrip("`")
                            #strip of _mpi if present
                            if job["command"].endswith("_mpi"):
                                job["command"]=job["command"][:-4]
                    arguments={}
                    while len(sp)>0:
                        param=sp.pop(0)[2:]
                        if len(sp)>0 and not sp[0].startswith("--"):
                            value=sp.pop(0)
                        else:
                            value=""
                        arguments[param]=value
                    job["parameters"]=arguments
                else:
                    # parsing pre commands
                    job["pre_commands"].append(line.strip("\n"))
            else:
                #parse post command
                job["post_commands"].append(line.strip("\n"))
    return job

def strip_comments(d):

    for (key,value) in d.iteritems():
        if isinstance(value,dict):
            strip_comments(value)
    if "__comment__" in d:
        del d["__comment__"]

def read_job_types(filename):
    with open(filename) as json_file:
        job_types=json.load(json_file)
    strip_comments(job_types)
    for name,job_type in job_types.iteritems():
        job_type["name"]=name    
    return job_types

def match_parameter(parameters,matches):
    for mp in matches:
        if not mp in parameters:
            return False
    return True

def nomatch_parameter(parameters,nomatches):
    for nmp in nomatches:
        if nmp in parameters:
            return False
    return True

def find_job_type(job,job_types):
    job_type=None
    for (key,item) in job_types.iteritems():
            if "match_command" in item:
                if job["command"]!=item["match_command"]:
                    continue
            #print "matching command: %s" % (item["match_command"])
            if "match_parameter" in item:
                if not isinstance(item["match_parameter"],list):
                    match_param=[item["match_parameter"]]
                else:
                    match_param=item["match_parameter"]
                if not match_parameter(job["parameters"],match_param):
                    continue
            if "nomatch_parameter" in item:
                if not isinstance(item["nomatch_parameter"],list):
                    nomatch_param=[item["nomatch_parameter"]]
                else:
                    nomatch_param=item["nomatch_parameter"]
                if not nomatch_parameter(job["parameters"],nomatch_param):
                    continue
            job_type=item
            print "identified job type %s" % key
            break
    if not job_type:
        print " job type not found. Aborting"
        sys.exit(-1)
    return job_type

def nproc_max(ndim,box,pad,nclass,ngpu,mgpu):
    return int(ngpu*floor(mgpu/(84*box**ndim*pad*nclass))+1)

def get_boxsize(starfile):
    first_image=check_output("relion_star_printtable %s data_ _rlnImageName" % starfile, shell=True).split("\n")[0]
    return int(check_output("relion_image_handler --stats --i %s" % first_image,shell=True).split()[3])

def sanitize(job,job_type,job_types):
    if "inherits" in job_type:
        sanitize(job,job_types[job_type["inherits"]],job_types)
    if "job_parameters" in job_type:
        for key in job_type["job_parameters"].keys():
            job[key]=job_type["job_parameters"][key]
    if "relion_remove_parameter" in job_type:
        for param in job_type["relion_remove_parameter"]:
            if param in job["parameters"]:
                del job["parameters"][param]
    if "relion_add_parameter" in job_type:
        job["parameters"].update(job_type["relion_add_parameter"])
    if "adjust_GPU_nproc_to_mem" in job_type:
        # mem usage determination not yet supported for multy body runs and continue runs
        if not "multibody_masks" in job["parameters"]:
            if "K" in job["parameters"]:
                #classficiation
                nclass=int(job["parameters"]["K"])
                if "ref" in job["parameters"] or "denovo_3dref" in job["parameters"]:
                    ndim=3
                else:
                  ndim=2
            else:
                #refinement
                nclass=1
                ndim=3
            if "pad" in job["parameters"]:
                pad= int(job["parameters"]["pad"])
            else:
                pad=2 # relion default
            if "continue" in job["parameters"]:
                optimizer_file=job["parameters"]["continue"]
                star_file=optimizer_file.replace("optimiser","data")
            else:
                star_file=job["parameters"]["i"]
            box=get_boxsize(star_file)
            ngpu=job_type["adjust_GPU_nproc_to_mem"][0]
            mgpu=job_type["adjust_GPU_nproc_to_mem"][1]
            nproc_gpu_max=nproc_max(ndim,box,pad,nclass,ngpu,mgpu)
            if nproc_gpu_max<job["nproc"]:
                if nproc_gpu_max < 5:
                    nproc_gpu_max = 5
                job["nproc"]=nproc_gpu_max
                print "adjusting nproc to %d due to GPU memory constraints" % nproc_gpu_max


def get_job_template(job_types,job_type):
    jinja2_env = Environment(trim_blocks=True,loader=FileSystemLoader(os.path.join(os.path.dirname(__file__),"..","job_template")))
    while True:
        template_name="%s.sh.j2" % job_type["name"]
        if template_name in jinja2_env.list_templates():
            return jinja2_env.get_template(template_name)
        else:
            if "inherits" in job_type:
                job_type=job_types[job_type["inherits"]]
                continue
        print "no template found for job"
        break


def dispatch(job,job_template,parameter_file):
    args=["--%s %s" %i for i in job["parameters"].iteritems()]
    if job["nthread"]>1:
        args.append("--j %d" % (job["nthread"]))
    job["arguments"]=" ".join(args)
    run_file=parameter_file+".run"
    with open(run_file,"w") as f:
        f.write(job_template.render(job=job))
    os.system(job["submit_command"]+" "+run_file)


######### main ###########

parameter_file=sys.argv[1]
job=parse(parameter_file)
job_types=read_job_types(os.path.join(os.path.split(os.path.abspath(__file__))[0],"..","config","dispatch_config.json"))
job_type=find_job_type(job,job_types)
sanitize(job,job_type,job_types)
job_template=get_job_template(job_types,job_type)
dispatch(job,job_template,parameter_file)

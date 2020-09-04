################################################################################
#
# Author: Andreas Schenk
# Friedrich Miescher Institute, Basel, Switzerland
#
# This file is part of CryoFLARE
#
# Copyright (C) 2017-2020 by the CryoFLARE Authors
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
import sys,os,exceptions,fcntl

process_names={0:"Import",1:"MotionCorr",2:"CtfFind",3:"ManualPick",4:"AutoPick",5:"Extract",6:"Sort",7:"Select",8:"Class2D",9:"Class3D",10:"Refine3D",11:"Polish",12:"MaskCreate",13:"JoinStar",14:"Subtract",15:"PostProcess",16:"LocalRes",17:"MovieRefine",18:"InitialModel",19:"MultiBody",20:"Polish",21:"CtfRefine"}   
process_ids={y:x for x,y in process_names.iteritems()}
template="""data_pipeline_general

_rlnPipeLineJobCounter 0
 

data_pipeline_processes

loop_ 
_rlnPipeLineProcessName #1 
_rlnPipeLineProcessAlias #2 
_rlnPipeLineProcessType #3 
_rlnPipeLineProcessStatus #4 
 

data_pipeline_nodes

loop_ 
_rlnPipeLineNodeName #1 
_rlnPipeLineNodeType #2 
 

data_pipeline_input_edges

loop_ 
_rlnPipeLineEdgeFromNode #1 
_rlnPipeLineEdgeProcess #2 
 

data_pipeline_output_edges

loop_ 
_rlnPipeLineEdgeProcess #1 
_rlnPipeLineEdgeToNode #2 
"""


def add_to_pipeline(pipeline_name,jobtype,job_id,alias,input_nodes,output_nodes):
    process_type=process_ids[jobtype]
    process_dir="%s/job%03d/" % (process_names[process_type],job_id)
    process_dir_alias="%s/%s/" % (process_names[process_type],alias)
    lines=[]
    # ensure pipeline file exists
    with open(pipeline_name,"a"):
        pass
    with open(pipeline_name,"r+") as pfile:
        try:
            fcntl.flock(pfile,fcntl.LOCK_EX)
            lines=pfile.readlines()
            if not lines:
                lines=template.splitlines(True)
            lines.reverse()
            changed=False
            output=""
            while lines and not "_rlnPipeLineJobCounter" in lines[-1]:
                output+=lines.pop()
            counter=int(lines.pop().split()[1])
            if counter<=job_id:
                changed=True
            output+="_rlnPipeLineJobCounter %d\n" % (max(job_id+1,counter))
            while lines  and not "_rlnPipeLineProcessStatus" in lines[-1]:
                output+=lines.pop()
            output+=lines.pop()
            present=False
            while lines and "job" in lines[-1]:
                if "job%03d" % (job_id) in lines[-1]:
                    present=True
                output+=lines.pop()
            if present:
                while lines:
                    output+=lines.pop()
            else:
                changed=True
                output+="%s %s %d 2\n" % (process_dir,process_dir_alias,process_type)

                while lines  and not "_rlnPipeLineNodeType" in lines[-1]:
                    output+=lines.pop()
                output+=lines.pop()
                while lines and "job" in lines[-1]:
                    output+=lines.pop()
                for node in output_nodes.split(","):
                    output+="%s %s\n" % tuple(node.split(":"))
                while lines  and not "_rlnPipeLineEdgeProcess" in lines[-1]:
                    output+=lines.pop()
                output+=lines.pop()
                while lines and "job" in lines[-1]:
                    output+=lines.pop()
                for node in input_nodes.split(","):
                    if node and process_dir:
                        output+="%s %s\n" % (node,process_dir)
                while lines  and not "_rlnPipeLineEdgeToNode" in lines[-1]:
                    output+=lines.pop()
                output+=lines.pop()
                while lines and "job" in lines[-1]:
                    output+=lines.pop()
                for node in output_nodes.split(","):
                    output+="%s %s\n" % (process_dir,node.split(":")[0])
                while lines:
                    output+=lines.pop()
            if changed:
                pfile.seek(0)
                pfile.truncate()
                pfile.write(output)
        finally:
            fcntl.flock(pfile,fcntl.LOCK_UN)    


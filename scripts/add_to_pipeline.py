#!/usr/bin/env python
import sys,os

process_names={0:"Import",2:"CtfFind"}
process_ids={"Import":0,"CtfFind":2}

pipeline_name=sys.argv[1]
process_type=process_ids[sys.argv[2]]
job_id=int(sys.argv[3])
alias=sys.argv[4]
input_nodes=sys.argv[5]
output_nodes=sys.argv[6]

process_dir="%s/job%03d/" % (process_names[process_type],job_id)
process_dir_alias="%s/%s/" % (process_names[process_type],alias)


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


lines=[]
if  os.path.exists(pipeline_name):
    with open(pipeline_name) as f:
        lines=f.readlines()
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
    with open(pipeline_name,"w") as f:
        f.write(output)
    

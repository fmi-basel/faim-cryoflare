#!/bin/sh --noprofile
######################## get parameters from GUI ###############################

. data_connector.sh


######################## load modules ##########################################

module purge
module load eman2/2.2


######################## define output files ###################################

motioncor2_aligned_avg_dw_png=$destination_path/micrographs_mc2/${short_name}.png
motioncor2_aligned_avg_dw_fft_thumbnail=$destination_path/micrographs_mc2/${short_name}_fft.png
motioncor2_shift_plot=$destination_path/micrographs_mc2/${short_name}_shift.png
FILES motioncor2_aligned_avg_dw_png motioncor2_aligned_avg_dw_fft_thumbnail motioncor2_shift_plot


######################## define additional parameters ##########################

pixel_size=`CALCULATE "1e10*$apix_x"`


######################## run processing if files are missing ###################

if FILES_MISSING; then
  RUN e2proc2d.py --process math.realtofft  --meanshrink 7 --process mask.sharp:inner_radius=1 $motioncor2_aligned_avg_dw $motioncor2_aligned_avg_dw_fft_thumbnail  >> $motioncor2_log   2>&1

  python <<EOT
import matplotlib.pyplot as plt
from matplotlib import cm
import numpy as np
x=[]
y=[]

with open("$motioncor2_log") as f:
    for line in f.readlines():
        if line.startswith("...... Frame"):
            sp=line.split()
            x.append(float(sp[5])*$pixel_size)
            y.append(float(sp[6])*$pixel_size)
x=np.array(x)
y=np.array(y)
x-=x[len(x)/2]
y-=y[len(y)/2]
plt.figure(figsize=(5,5))
plt.plot(x,y,'k-')
plt.scatter(x,y, marker='o',c=range(len(x)),cmap=cm.jet,s=30, zorder=9)
plt.scatter(x[:1],y[:1], marker='D',c=range(1),cmap=cm.jet,s=40, zorder=10)
plt.xlabel('shift x (A)')
plt.ylabel('shift y (A)')
plt.tight_layout()
plt.savefig("$motioncor2_shift_plot",dpi=100)
EOT

  RUN e2proc2d.py  --meanshrink 7 ${motioncor2_aligned_avg_dw} ${motioncor2_aligned_avg_dw_png}  >> $motioncor2_log 2>&1
fi


######################## extract output parameters #############################

motioncor2_max_shift=`python <<EOT
from math import sqrt
x=[]
y=[]

with open("$motioncor2_log") as f:
    for line in f.readlines():
        if line.startswith("...... Frame"):
            sp=line.split()
            x.append(float(sp[5])*$pixel_size)
            y.append(float(sp[6])*$pixel_size)
dist2=0
for i in range(len(x)):
    for j in range(i+1,len(x)):
        dx=x[i]-x[j]
        dy=y[i]-y[j]
        dist2=max(dist2,dx*dx+dy*dy)
print sqrt(dist2)
EOT`


######################## decide if image should be excluded from export ########

LIMIT_EXPORT motioncor2_max_shift 0 $motioncor2_input_maxshift



######################## create Relion job #####################################

rln_jobtype=Import
rln_jobid=4
rln_alias=micrographs_mc2
rln_nodetype=1
rln_starname=micrographs.star
rln_inputstar=""
rln_header=(MicrographName)
RELION_WRITE "$destination_path" "$rln_jobtype" "$rln_jobid" "$rln_alias" "$rln_nodetype" "$rln_starname" "$rln_inputstar" "rln_header[@]" "micrographs_mc2/${short_name}_DW.mrc"


######################## export result parameters ##############################

RESULTS motioncor2_max_shift


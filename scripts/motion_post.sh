#!/bin/bash --noprofile 
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
set -e
set -u
######################## get parameters from GUI ###############################
. data_connector.sh
######################## load modules ##########################################
module purge
module load EMAN2/2.20
#CRYOFLARE_DEBUG=1
######################## plotting and shift calc function    ###################
create_shift_plot ()
{
  local logfile=$1
  local pix_x=$2
  local pix_y=$3
  local fig=$4
  local fig_amp=$5
  python <<EOT
import matplotlib.pyplot as plt
from math import sqrt
from matplotlib import cm
import numpy as np
x=[]
y=[]
amp=[]
with open("$logfile") as f:
  for line in f.readlines():
    sp=line.split()
    x.append(float(sp[0])*$pix_x)
    y.append(float(sp[1])*$pix_y)
    if len(x)>1:
      amp.append(sqrt((x[-1]-x[-2])**2+(y[-1]-y[-2])**2))

x=np.array(x)
y=np.array(y)
x-=x[len(x)/2]
y-=y[len(y)/2]
plt.figure(figsize=(5,5))
plt.plot(x,y,'k-')
x_min,x_max=plt.xlim()
y_min,y_max=plt.ylim()
x_hr=(x_max-x_min)/2.0
y_hr=(y_max-y_min)/2.0
halfrange=max(y_hr,x_hr)
plt.xlim(x_min+x_hr-halfrange,x_min+x_hr+halfrange)
plt.ylim(y_min+y_hr-halfrange,y_min+y_hr+halfrange)
plt.scatter(x,y, marker='o',c=range(len(x)),cmap=cm.jet,s=30, zorder=9)
plt.scatter(x[:1],y[:1], marker='D',c=range(1),cmap=cm.jet,s=40, zorder=10)
plt.xlabel('shift x (A)')
plt.ylabel('shift y (A)')
plt.tight_layout()
plt.savefig("$fig",dpi=100)
plt.figure(figsize=(5,5))
plt.plot(amp)
plt.xlabel('frame')
plt.ylabel('shift (A)')
plt.tight_layout()
plt.savefig("$fig_amp",dpi=100)
EOT
}

calc_max_shift()
{
  local logfile=$1
  local pix_x=$2
  local pix_y=$3
  python <<EOT
from math import sqrt
x=[]
y=[]

with open("$logfile") as f:
    for line in f.readlines():
            sp=line.split()
            x.append(float(sp[0])*$pix_x)
            y.append(float(sp[1])*$pix_y)
dist2=0
for i in range(len(x)):
    for j in range(i+1,len(x)):
        dx=x[i]-x[j]
        dy=y[i]-y[j]
        dist2=max(dist2,dx*dx+dy*dy)
print sqrt(dist2)
EOT
}
######################## define output files ###################################
aligned_micrograph_png=${aligned_micrograph/.mrc/.png}
aligned_micrograph_fft=${aligned_micrograph/.mrc/_fft.png}
motion_shift_plot=${aligned_micrograph/.mrc/_shift.png}
motion_shift_amp_plot=${aligned_micrograph/.mrc/_shift_amp.png}
FILES aligned_micrograph_png aligned_micrograph_fft motion_shift_plot motion_shift_amp_plot
echo compress raw: $motion_input_compress_raw
######################## run processing if files are missing ###################
if FILES_MISSING; then
  # create small image
  RUN e2proc2d.py --process math.realtofft  --meanshrink 7 --process mask.sharp:inner_radius=1 $aligned_micrograph $aligned_micrograph_fft  >> $motion_log   2>&1
  # create shift plot
  echo create_shift_plot "$shift_log" $apix_x $apix_y "$motion_shift_plot" "$motion_shift_amp_plot"
  create_shift_plot "$shift_log" $apix_x $apix_y "$motion_shift_plot" "$motion_shift_amp_plot"
  RUN e2proc2d.py  --meanshrink 7 ${aligned_micrograph} ${aligned_micrograph_png}  >> $motion_log 2>&1
  if [ "$motion_input_compress_raw" = "true" ]; then
    echo compressing movie
    echo bzip2 $raw_movie
    RUN bzip2 $raw_movie
    raw_movie=${raw_movie}.bz2
    RAW_FILES raw_movie
  else
    echo not compressing movie
  fi
fi
######################## extract output parameters #############################
motion_max_shift="$(calc_max_shift "$shift_log" $apix_x $apix_y)"
motion_max_shift=$( ROUND $motion_max_shift 2 )
######################## decide if image should be excluded from export ########
LIMIT_EXPORT motion_max_shift 0 $motion_input_maxshift
######################## export result parameters ##############################
RESULTS motion_max_shift


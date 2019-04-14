#!/bin/bash --noprofile -e
######################## get parameters from GUI ###############################

. data_connector.sh


######################## load modules ##########################################

module purge
module load eman2/2.2
module load unblur


######################## create destination folders ############################

mkdir -p $destination_path/movies_unblur
mkdir -p $destination_path/micrographs_unblur
mkdir -p $destination_path/micrographs_unblur_dw



######################## define output files ###################################

unblur_aligned_stack=$destination_path/movies_unblur/${short_name}_movie.mrc
unblur_log=$destination_path/movies_unblur/${short_name}_unblur.log
unblur_shift_txt=$destination_path/movies_unblur/${short_name}_shift.txt
unblur_shift_plot=$destination_path/movies_unblur/${short_name}_shift.png
unblur_frc_txt=$destination_path/movies_unblur/${short_name}_frc.txt

unblur_aligned_avg=$destination_path/micrographs_unblur/${short_name}.mrc
unblur_aligned_avg_png=$destination_path/micrographs_unblur/${short_name}.png
unblur_aligned_avg_fft_thumbnail=$destination_path/micrographs_unblur/${short_name}_fft.png

unblur_aligned_avg_dw=$destination_path/micrographs_unblur_dw/${short_name}.mrc
unblur_aligned_avg_png_dw=$destination_path/micrographs_unblur_dw/${short_name}.png
unblur_aligned_avg_fft_thumbnail_dw=$destination_path/micrographs_unblur_dw/${short_name}_fft.png

FILES unblur_aligned_stack unblur_log unblur_shift_txt unblur_shift_plot unblur_frc_txt unblur_aligned_avg unblur_aligned_avg_png unblur_aligned_avg_fft_thumbnail unblur_aligned_avg_dw unblur_aligned_avg_png_dw unblur_aligned_avg_fft_thumbnail_dw 


######################## define additional parameters ##########################

pixel_size=`CALCULATE "1e10*$apix_x"`
dose_per_frame=`CALCULATE "1e-20*$dose/$pixel_size/$pixel_size/$num_frames"`
export OMP_NUM_THREADS=4


######################## RUN processing if files are missing ###################

if FILES_MISSING; then
  rm -fr $unblur_aligned_stack > /dev/null
  clamped_stack=$scratch/${short_name}_clamped.mrcs
  clamped_stack_mrc=$scratch/${short_name}_clamped.mrc
  RUN e2proc2d.py $unpack_raw_stack $clamped_stack --process threshold.clampminmax.nsigma:nsigma=4:tomean=1 > $unblur_log
  ln -s $clamped_stack $clamped_stack_mrc >> $unblur_log

  gzip $unpack_raw_stack
  unpack_raw_stack=${unpack_raw_stack}.gz
  FILES unpack_raw_stack

  unblur_param=$scratch/${short_name}_unblur.param
  echo input_filename $clamped_stack_mrc  > $unblur_param
  echo number_of_frames_per_movie $num_frames >> $unblur_param
  echo output_filename $unblur_aligned_avg  >> $unblur_param
  echo shifts_filename $unblur_shift_txt >> $unblur_param
  echo pixel_size $pixel_size >> $unblur_param
  echo apply_dose_filter no >> $unblur_param
  echo save_aligned_frames yes >> $unblur_param
  echo aligned_frames_filename $unblur_aligned_stack  >> $unblur_param
  echo set_expert_options no >> $unblur_param

  RUN unblur  $unblur_param  >> $unblur_log
  rm -f ${unblur_aligned_stack}s
  mv $unblur_aligned_stack ${unblur_aligned_stack}s
  ln -s ${unblur_aligned_stack}s $unblur_aligned_stack

  RUN e2proc2d.py --process math.realtofft  --process mask.sharp:inner_radius=1 $unblur_aligned_avg $unblur_aligned_avg_fft_thumbnail
  RUN e2proc2d.py   --meanshrink 7 ${unblur_aligned_avg} ${unblur_aligned_avg_png}

  summovie_param=$scratch/${short_name}_summovie.param
  echo INPUT_FILENAME $unblur_aligned_stack > $summovie_param
  echo number_of_frames_per_movie $num_frames >> $summovie_param
  echo output_filename $unblur_aligned_avg_dw >> $summovie_param
  echo shifts_filename $unblur_shift_txt >> $summovie_param
  echo frc_filename $unblur_frc_txt >> $summovie_param
  echo first_frame 1 >> $summovie_param
  echo LAST_FRAME $num_frames >> $summovie_param
  echo PIXEL_SIZE $pixel_size >> $summovie_param
  echo apply_dose_filter yes >> $summovie_param
  echo dose_per_frame $dose_per_frame >> $summovie_param
  echo acceleration_voltage 300 >> $summovie_param
  echo pre_exposure_amount 0.0 >> $summovie_param
  echo restore_power yes >> $summovie_param

  RUN summovie  $summovie_param  >> $unblur_log
  RUN e2proc2d.py --process math.realtofft    --meanshrink 7  --process mask.sharp:inner_radius=1 $unblur_aligned_avg_dw $unblur_aligned_avg_fft_thumbnail_dw
  RUN e2proc2d.py   --meanshrink 7 ${unblur_aligned_avg_dw} ${unblur_aligned_avg_png_dw}
  python > $unblur_log <<EOT
import matplotlib.pyplot as plt
from matplotlib import cm
import numpy as np
with open("$unblur_shift_txt") as f:
    lines=f.readlines()[-2:]
    x=[float(v) for v in lines[0].split()]
    y=[float(v) for v in lines[1].split()]
x=np.array(x)
y=np.array(y)
plt.figure(figsize=(5,5))
plt.plot(x,y,'k-')
plt.scatter(x,y, marker='o',c=range(len(x)),cmap=cm.jet,s=30, zorder=9)
plt.scatter(x[:1],y[:1], marker='D',c=range(1),cmap=cm.jet,s=40, zorder=10)
plt.xlabel('shift x (A)')
plt.ylabel('shift y (A)')
plt.tight_layout()
plt.savefig("$unblur_shift_plot",dpi=100)
EOT

fi


######################## extract output parameters #############################

unblur_max_shift=`python <<EOT
from math import sqrt
with open("$unblur_shift_txt") as f:
    lines=f.readlines()[-2:]
    x=[float(v) for v in lines[0].split()]
    y=[float(v) for v in lines[1].split()]
dist2=0
for i in range(len(x)):
    for j in range(i+1,len(x)):
        dx=x[i]-x[j]
        dy=y[i]-y[j]
        dist2=max(dist2,dx*dx+dy*dy)
print sqrt(dist2)
EOT`


unblur_score=`fgrep "Final Score" $unblur_log|cut -f2 -d:|xargs`



######################## decide if image should be excluded from export ########

LIMIT_EXPORT unblur_max_shift 0 $unblur_input_max_shift



######################## create Relion jobs ####################################

rln_jobtype=Import
rln_inputstar=""


rln_nodetype=1
rln_starname=micrographs.star
rln_header=(MicrographName)

rln_jobid=1
rln_alias=micrographs_unblur
RELION_WRITE $destination_path $rln_jobtype $rln_jobid $rln_alias $rln_nodetype $rln_starname $rln_inputstar rln_header[@] micrographs_unblur/${short_name}.mrc

rln_jobid=2
rln_alias=micrographs_unblur_dw
RELION_WRITE $destination_path $rln_jobtype $rln_jobid $rln_alias $rln_nodetype $rln_starname $rln_inputstar rln_header[@] micrographs_unblur_dw/${short_name}.mrc

rln_nodetype=0
rln_starname=movies.star
rln_header=(MicrographMovieName)
rln_jobid=3
rln_alias=movies_unblur
RELION_WRITE $destination_path $rln_jobtype $rln_jobid $rln_alias $rln_nodetype $rln_starname $rln_inputstar rln_header[@] movies_unblur/${short_name}_movie.mrcs


######################## export result parameters ##############################

RESULTS unblur_score unblur_max_shift


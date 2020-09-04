#!/bin/bash --noprofile 
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
set -e
set -u
#CRYOFLARE_DEBUG=1
######################## get parameters from GUI ###############################
. data_connector.sh
######################## helper functions ######################################
transpose() {
    python <<EOT
import fileinput,sys
split_lines=[ l.split() for l in fileinput.input("$1")][-2:]
for i in range(len(split_lines[0])):
    values=[]
    for sl in split_lines:
        values.append(sl[i])
    print " ".join(values)
EOT
}
######################## load modules ##########################################
module purge
module load EMAN2/2.20
unblur_exe=/<path_to>/unblur
summovie_exe=/<path_to>/summovie 
######################## create Relion job #####################################
rln_jobtype=MotionCorr
rln_jobid=3
rln_alias=Movies
rln_starname=corrected_micrographs.star
rln_nodetype=1
rln_inputstar="Import/job001/movies.star"
RELION_CREATE_JOB "$rln_jobtype" "$rln_jobid" "$rln_alias" "$rln_starname" "$rln_nodetype" "$rln_inputstar"
mkdir -p $rln_jobpath/$( dirname ${raw_movie} )
######################## define output files ###################################
motion_log=$rln_jobpath/${short_name}_motion.log
aligned_micrograph_no_dw=$rln_jobpath/${raw_movie/.*/_noDW.mrc}
aligned_micrograph=$rln_jobpath/${raw_movie/.*/.mrc}
shift_log=$rln_jobpath/${raw_movie/.*/_shifts.txt}
FILES motion_log aligned_micrograph aligned_micrograph_no_dw shift_log
######################## define additional parameters ##########################
if [[ "$stack_suffix" == eer ]] ; then
  >&2 echo EER files are not supported in unblur
  exit -1
else
  movie_sampling=$apix_x
  dose_per_frame=`CALCULATE "$motion_input_dose_rate/$num_frames"`
fi
bft="300"
iter=4

######################## run processing if files are missing ###################
if FILES_MISSING; then

  clamped_stack=$scratch/${short_name}_clamped.mrcs
  clamped_stack_mrc=$scratch/${short_name}_clamped.mrc
  RUN e2proc2d.py $raw_movie $clamped_stack --process threshold.clampminmax.nsigma:nsigma=4:tomean=1 > $motion_log
  ln -s $clamped_stack $clamped_stack_mrc >> $motion_log

  mkdir -p $scratch/$rln_jobpath
  unblur_param=$scratch/${short_name}_unblur.param
  unblur_shifts=$scratch/shifts.txt
  unblur_aligned_stack=$scratch/${short_name}_aligned_stack.mrc
  echo input_filename $clamped_stack_mrc  > $unblur_param
  echo number_of_frames_per_movie $num_frames >> $unblur_param
  echo output_filename $aligned_micrograph_no_dw  >> $unblur_param
  echo shifts_filename $unblur_shifts >> $unblur_param
  echo pixel_size $movie_sampling >> $unblur_param
  echo apply_dose_filter no >> $unblur_param
  echo save_aligned_frames yes >> $unblur_param
  echo aligned_frames_filename $unblur_aligned_stack  >> $unblur_param
  echo set_expert_options no >> $unblur_param

  RUN $unblur_exe  $unblur_param  >> $motion_log

  summovie_param=$scratch/${short_name}_summovie.param
  unblur_frc_txt=$scratch/${short_name}_frc.txt
  echo INPUT_FILENAME $unblur_aligned_stack > $summovie_param
  echo number_of_frames_per_movie $num_frames >> $summovie_param
  echo output_filename $aligned_micrograph >> $summovie_param
  echo shifts_filename $unblur_shifts >> $summovie_param
  echo frc_filename $unblur_frc_txt >> $summovie_param
  echo first_frame 1 >> $summovie_param
  echo LAST_FRAME $num_frames >> $summovie_param
  echo PIXEL_SIZE $movie_sampling >> $summovie_param
  echo apply_dose_filter yes >> $summovie_param
  echo dose_per_frame $dose_per_frame >> $summovie_param
  echo acceleration_voltage $acceleration_voltage >> $summovie_param
  echo pre_exposure_amount 0.0 >> $summovie_param
  echo restore_power yes >> $summovie_param

  RUN $summovie_exe  $summovie_param  >> $motion_log
  transpose  $unblur_shifts > $shift_log 
fi
######################## write Relion star ####################################
rln_header=( MicrographNameNoDW MicrographName )
RELION_WRITE_STAR "$rln_alias" "$rln_starname" "rln_header[@]" "$aligned_micrograph_no_dw" "$aligned_micrograph"

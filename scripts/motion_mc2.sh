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
#CRYOFLARE_DEBUG=0
. data_connector.sh
######################## load modules ##########################################
module purge
module load sbgrid/relion/3.0.8_cu10.1
module load MotionCor2/1.3.1-cuda10.1
relion_run_mc_exe="/<path_to>/relion_run_motioncorr"
mc2_exe="/<path_to>/MotionCor2_v1.3.1-Cuda101"
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
aligned_micrograph_no_dw=$rln_jobpath/${raw_movie/.mrcs/_noDW.mrc}
aligned_micrograph=$rln_jobpath/${raw_movie/.mrcs/.mrc}
shift_log_ori=$rln_jobpath/${raw_movie/.mrcs/0-Patch-Full.log}
shift_log=$rln_jobpath/${raw_movie/.*/_shifts.txt}
motion_metadata=$rln_jobpath/${raw_movie/.mrcs/.star}
FILES motion_log aligned_micrograph aligned_micrograph_no_dw shift_log motion_metadata
######################## define additional parameters ##########################
dose_per_frame=`CALCULATE "$motion_input_dose/$num_frames"`
bft="300"
iter=4

if [ "$motion_input_ft_bin" -eq "0" ]; then
  mc2_ft_bin=1
fi
######################## create destination folders ############################
mkdir -p $rln_jobpath 
######################## run processing if files are missing ###################
if FILES_MISSING; then
  mkdir -p $scratch/$rln_jobpath
  relion_mc2_params="--i $raw_movie "
  relion_mc2_params+="--o $scratch/$rln_jobpath "
  relion_mc2_params+="--first_frame_sum 1 "
  relion_mc2_params+="--last_frame_sum -1 "
  relion_mc2_params+="--j 1 "
  relion_mc2_params+="--gain_rot 0 "
  relion_mc2_params+="--gain_flip 0 "
  relion_mc2_params+="--dose_weighting "
  relion_mc2_params+="--use_motioncor2 1 "
  relion_mc2_params+="--save_noDW 1 "
  relion_mc2_params+="--preexposure 0 "
  relion_mc2_params+="--bfactor $bft "
  relion_mc2_params+="--angpix $apix_x "
  relion_mc2_params+="--voltage 300 "
  relion_mc2_params+="--bin_factor $motion_input_ft_bin "
  relion_mc2_params+="--patch_x $motion_input_patch "
  relion_mc2_params+="--patch_y $motion_input_patch "
  relion_mc2_params+="--dose_per_frame $dose_per_frame "
  relion_mc2_params+="--gpu $gpu_id  "
  relion_mc2_other_params+="-Iter $iter"
  relion_mc2_gain_params=""
  if [ ! -z ${gain_reference+x} ] && [ -e $gain_reference ]; then
    relion_mc2_gain_params="--gainref $gain_reference "
  fi
  echo $relion_run_mc_exe $relion_mc2_params $relion_mc2_gain_params 
  RUN $relion_run_mc_exe $relion_mc2_params $relion_mc2_gain_params  --motioncor2_exe "$mc2_exe" --other_motioncor2_args "$relion_mc2_other_params" > $motion_log  2>&1
  mv $scratch/$aligned_micrograph_no_dw $aligned_micrograph_no_dw
  mv $scratch/$aligned_micrograph $aligned_micrograph
  cat $shift_log_ori |tr -s ' '| cut -f 2,3 -d ' ' > $shift_log
  mv $scratch/$motion_metadata $motion_metadata
fi
if [ "$motion_input_ft_bin" -ge "2" ]; then
  apix_x=`CALCULATE $motion_input_ft_bin*$apix_x`
  apix_y=`CALCULATE $motion_input_ft_bin*$apix_y`
  RESULTS apix_x apix_y
fi
accum_total=$(grep _noDW.mrc $scratch/$rln_jobpath/$rln_starname|cut -d" " -f4)
accum_early=$(grep _noDW.mrc $scratch/$rln_jobpath/$rln_starname|cut -d" " -f5)
accum_late=$(grep _noDW.mrc $scratch/$rln_jobpath/$rln_starname|cut -d" " -f6)
######################## write Relion star ####################################
rln_header=( MicrographNameNoDW MicrographName MicrographMetadata AccumMotionTotal AccumMotionEarly AccumMotionLate)
RELION_WRITE_STAR "$rln_alias" "$rln_starname" "rln_header[@]" "$aligned_micrograph_no_dw" "$aligned_micrograph" "$motion_metadata" $accum_total $accum_early $accum_late

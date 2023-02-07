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
#CRYOFLARE_DEBUG=1
. data_connector.sh
######################## load modules ##########################################
module purge
module load RELION
relion_run_mc_exe="/<path_to>/relion_run_motioncorr"
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
motion_metadata=$rln_jobpath/${raw_movie/.*/.star}
accumulation_data=$rln_jobpath/${raw_movie/.*/_accum_data.txt}
FILES motion_log aligned_micrograph aligned_micrograph_no_dw shift_log motion_metadata accumulation_data
######################## define additional parameters ##########################
relion_mc_params=""
if [[ "$stack_suffix" == eer ]] ; then
  eer_upsampling=2 # fixed in EPU data acquisition
  dose_per_frame=`CALCULATE "$motion_input_dose_rate/242*$motion_input_eer_grouping"`
  movie_sampling=$( CALCULATE $apix_x/$eer_upsampling )
  relion_mc_params+="--eer_upsampling $eer_upsampling "
  relion_mc_params+="--eer_grouping $motion_input_eer_grouping "
  relion_mc_params+="--bin_factor 2 "
else
  movie_sampling=$apix_x
  dose_per_frame=`CALCULATE "$motion_input_dose_rate/$num_frames"`
fi
bft="300"
iter=4

######################## run processing if files are missing ###################
if FILES_MISSING; then
  mkdir -p $scratch/$rln_jobpath
  relion_mc_params+="--i $raw_movie "
  relion_mc_params+="--o $scratch/$rln_jobpath "
  relion_mc_params+="--first_frame_sum 1 "
  relion_mc_params+="--last_frame_sum -1 "
  relion_mc_params+="--j 1 "
  relion_mc_params+="--gain_rot 0 "
  relion_mc_params+="--gain_flip 0 "
  relion_mc_params+="--dose_weighting "
  relion_mc_params+="--use_own 1 "
  relion_mc_params+="--save_noDW 1 "
  relion_mc_params+="--preexposure 0 "
  relion_mc_params+="--bfactor $bft "
  relion_mc_params+="--angpix $movie_sampling "
  relion_mc_params+="--voltage $acceleration_voltage "
  relion_mc_params+="--patch_x $motion_input_patch "
  relion_mc_params+="--patch_y $motion_input_patch "
  relion_mc_params+="--dose_per_frame $dose_per_frame "
  relion_mc_params+="--max_iter $iter "
  if [ -n "${gain_reference+x}" ]; then
    echo using gain reference: $gain_reference
    relion_mc_params+="--gainref $gain_reference "
  fi
  RUN $relion_run_mc_exe $relion_mc_params  > $motion_log  2>&1
  mv $scratch/$aligned_micrograph_no_dw $aligned_micrograph_no_dw
  mv $scratch/$aligned_micrograph $aligned_micrograph
  relion_star_printtable $scratch/$motion_metadata data_global_shift _rlnMicrographShiftX _rlnMicrographShiftY > $shift_log
  mv $scratch/$motion_metadata $motion_metadata  
  accum_total=$(grep _noDW.mrc $scratch/$rln_jobpath/$rln_starname| tr -s ' '|cut -d" " -f5)
  accum_early=$(grep _noDW.mrc $scratch/$rln_jobpath/$rln_starname| tr -s ' '|cut -d" " -f6)
  accum_late=$(grep _noDW.mrc $scratch/$rln_jobpath/$rln_starname | tr -s ' '|cut -d" " -f7)
  echo $accum_total $accum_early $accum_late > $accumulation_data
else
  accum_total=$(cat $accumulation_data| tr -s ' '|cut -d" " -f1)
  accum_early=$(cat $accumulation_data| tr -s ' '|cut -d" " -f2)
  accum_late=$(cat $accumulation_data| tr -s ' '|cut -d" " -f3)
fi
######################## write Relion star ####################################
rln_header=( MicrographNameNoDW MicrographName MicrographMetadata AccumMotionTotal AccumMotionEarly AccumMotionLate)
RELION_WRITE_STAR "$rln_alias" "$rln_starname" "rln_header[@]" "$aligned_micrograph_no_dw" "$aligned_micrograph" "$motion_metadata" $accum_total $accum_early $accum_late

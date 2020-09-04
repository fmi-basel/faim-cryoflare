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
######################## get parameters from GUI ###############################
set -u
set -e
######################## get parameters from GUI ###############################
. data_connector.sh
######################## load modules ##########################################
module purge
#module load Gautomatch/0.56-cuda10
#gautomatch_exe=Gautomatch_v0.56_sm62_cu10.0
module load cuda80/fft
gautomatch_exe=/<path_to>/gautomatch
######################## create Relion job #####################################
rln_jobtype=AutoPick
rln_jobid=5
rln_alias=Gautomatch
rln_starname=coords_suffix_autopick.star
rln_nodetype=2
rln_inputstar=MotionCorr/job003/corrected_micrographs.star
RELION_CREATE_JOB "$rln_jobtype" "$rln_jobid" "$rln_alias" "$rln_starname" "$rln_nodetype" "$rln_inputstar"
mkdir -p $rln_jobpath/$(dirname $raw_movie)
######################## define output files ###################################
picking_log=$rln_jobpath/${raw_movie%%.*}_gautomatch.log
picking_boxes=$rln_jobpath/${raw_movie%%.*}_autopick.box
picking_rejected_boxes=$rln_jobpath/${raw_movie%%.*}_rejected.box
picking_star_file=$rln_jobpath/${raw_movie%%.*}_autopick.star
aligned_micrograph_link=$rln_jobpath/${raw_movie%%.*}.mrc
FILES picking_log picking_boxes picking_rejected_boxes picking_star_file
######################## run processing if files are missing ###################
if FILES_MISSING; then
  ln -nfs ../../../$aligned_micrograph $aligned_micrograph_link
  picking_params=" --gid $gpu_id"
  picking_params+=" --apixM $apix_x"
  picking_params+=" --lsigma_cutoff ${picking_input_sigma_cutoff:=1.3}"
  picking_params+="  --diameter ${picking_input_diameter:=400}"
  picking_params+="  --cc_cutoff ${picking_input_cc_cutoff:=0.1}"
  #setting particle distance to particle diameter (suggested 0.9x-1.1x)
  picking_params+="  --min_dist ${picking_input_diameter:=100}"
  #using defaults for the rest
  #picking_params+=" --lsigma_D ${picking_input_sigma_d:=200}"
  #picking_params+=" --speed ${picking_input_speed:=2}"
  if [ -v picking_input_reference ] &&  [ -n "$picking_input_reference" ]; then
    picking_params+="  --T $picking_input_reference"
    picking_params+="  --apixT $picking_input_reference_pixel_size"
  fi
  echo $gautomatch_exe $picking_params $aligned_micrograph_link
  RUN $gautomatch_exe $picking_params $aligned_micrograph_link >> $picking_log 
  mv $rln_jobpath/${raw_movie%%.*}_automatch.star $picking_star_file
  mv $rln_jobpath/${raw_movie%%.*}_automatch.box $picking_boxes
fi
######################## create Relion job #####################################
picking_star=$rln_jobpath/coords_suffix_autopick.star
echo $rln_inputstar > $picking_star
SHARED_FILES picking_star





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
######################## get parameters from GUI ###############################
set -u
set -e
######################## get parameters from GUI ###############################
. data_connector.sh
######################## load modules ##########################################
cryolo_predict_exe=/scratch/miniconda3/envs/cryolo/bin/cryolo_predict.py
cryolo_gui_exe=/scratch/miniconda3/envs/cryolo/bin/cryolo_gui.py
######################## create Relion job #####################################
rln_jobtype=AutoPick
rln_jobid=5
rln_alias=Gautomatch
rln_starname=coords_suffix_autopick.star
rln_nodetype=1
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
  cryolo_config_params=" config $scratch/config_cryolo.json"
  cryolo_config_params+=" $picking_input_box"
  cryolo_config_params+=" --filter LOWPASS"
  cryolo_config_params+=" --low_pass_cutoff $picking_input_lowpass_cutoff"

  cryolo_params=" -g $gpu_id"
  cryolo_params+=" -c $scratch/config_cryolo.json"
  cryolo_params+=" -i $scratch/"
  cryolo_params+=" -o $scratch/"
  cryolo_params+=" -w $picking_input_model"
  
  ln -s ${destination_path}/${aligned_micrograph} $scratch/
  echo  $cryolo_gui_exe $cryolo_config_params  >> $picking_log 
  RUN $cryolo_gui_exe $cryolo_config_params  >> $picking_log 
  RUN $cryolo_predict_exe $cryolo_params  >> $picking_log
  echo  $cryolo_predict_exe $cryolo_params  >> $picking_log
  mv $scratch/STAR/*.star $picking_star_file
  mv $scratch/EMAN/*.box $picking_boxes
  touch $picking_rejected_boxes
fi
######################## create Relion job #####################################
picking_star=$rln_jobpath/coords_suffix_autopick.star
echo $rln_inputstar > $picking_star
SHARED_FILES picking_star



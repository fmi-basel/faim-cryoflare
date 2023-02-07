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
set -u
set -e
######################## get parameters from GUI ###############################
#CRYOFLARE_DEBUG=1
. data_connector.sh
######################## load modules ##########################################
module purge
module load EMAN2/2.20
janni_denoise_exe=/scratch/miniconda3/envs/cryolo/bin/janni_denoise.py
janni_model=/scratch/miniconda3/envs/cryolo/models/gmodel_janni_20190703.h5 
######################## define output files ###################################
denoised_big=${aligned_micrograph/.mrc/_denoised_big.mrc}
FILES  denoised_big
######################## run processing if files are missing ###################
if FILES_MISSING; then
  ori_dir=$scratch/ori
  denoised_dir=$scratch/denoised
  mkdir -p $ori_dir
  mkdir -p $denoised_dir
  ln -s ${destination_path}/${aligned_micrograph} $ori_dir/
  RUN $janni_denoise_exe denoise -g $gpu_id $ori_dir ${denoised_dir} $janni_model
  mv ${denoised_dir}/ori/${aligned_micrograph##*/} $denoised_big
fi



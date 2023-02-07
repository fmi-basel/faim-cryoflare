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
. data_connector.sh
######################## load modules ##########################################
module purge
module load EMAN2/2.20
######################## define output files ###################################
denoised_png=${aligned_micrograph/.mrc/_denoised.png}
denoised_boxes_png=${aligned_micrograph/.mrc/_denoise_boxes.png}
denoised_big_png=$scratch/denoise_big.png
FILES  denoised_boxes_png denoised_png
######################## run processing if files are missing ###################
if FILES_MISSING; then
  RUN e2proc2d.py --meanshrink 7 $denoised_big  $denoised_png
  RUN e2proc2d.py $denoised_big  $denoised_big_png
  draw_boxes.sh  $denoised_big_png $picking_boxes  $picking_rejected_boxes $denoised_boxes_png 0x0
fi

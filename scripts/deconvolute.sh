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
#module load iplt
module load EMAN2/2.20
iplt_exe=/<path_to>/iplt
script_path=/<path_to>/scripts
######################## define output files ###################################
deconvolute_png=${aligned_micrograph/.mrc/_deconv.png}
deconvolute_boxes_png=${aligned_micrograph/.mrc/_deconv_boxes.png}
FILES  deconvolute_png deconvolute_boxes_png
######################## run processing if files are missing ###################
if FILES_MISSING; then
  $iplt_exe $script_path/deconv.py ${aligned_micrograph}  $deconvolute_png $apix_x $defocus_u $defocus_v $defocus_angle
  draw_boxes.sh  $deconvolute_png $picking_boxes  $picking_rejected_boxes $deconvolute_boxes_png 0x0
fi

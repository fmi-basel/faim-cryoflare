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
module purge
module load EMAN2/2.20
######################## define output files ###################################
picking_boxes_png=${picking_boxes/_autopick.box/_boxes.png}

FILES  picking_boxes_png
######################## run processing if files are missing ###################
if FILES_MISSING; then
  full_png=$scratch/${short_name}.png
  RUN e2proc2d.py ${aligned_micrograph} $full_png
  draw_boxes.sh  $full_png $picking_boxes  $picking_rejected_boxes $picking_boxes_png 0x100
fi
######################## extract output parameters #############################
picking_num_particles=`cat $picking_boxes|wc -l`
######################## export result parameters ##############################
RESULTS picking_num_particles 

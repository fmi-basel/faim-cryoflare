#!/bin/bash --noprofile  
################################################################################
#
# Author: Andreas Schenk
# Friedrich Miescher Institute, Basel, Switzerland
#
# This file is part of CryoFLARE
#
# Copyright (C) 20172020 by the CryoFLARE Authors
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
######################## functions #############################################
calc_ice_thickness () {
local ref=$1
local im=$2
local mpf=$3
/usr/bin/env python <<EOT
from EMAN2 import EMData
import sys
from math import log
ref_name="$ref"
im_name="$im"
im=EMData(im_name)
ref=EMData(ref_name)
if im_name.endswith(".mrcs"):
  sum_im=im["nx"]*im["ny"]*im["MRC.mz"]*im["mean"]
else:
  sum_im=im["nx"]*im["ny"]*im["nz"]*im["mean"]
if ref_name.endswith(".mrcs"):
  sum_ref=ref["nx"]*ref["ny"]*ref["MRC.mz"]*ref["mean"]
else:
  sum_ref=ref["nx"]*ref["ny"]*ref["nz"]*ref["mean"]
print $mfp*log(sum_ref/sum_im)
EOT
}
calc_ice_ratio() {
local image_name=$1
local pixel_size=$2
local resolution_ref=5
local resolution_ice=3.89
local resolution_band=0.4
/usr/bin/env python <<EOT
from EMAN2 import EMData,Region
import numpy as np
import sys
e=EMData("$image_name")
min_size=min(e["nx"],e["ny"])
e_clip=e.get_clip(Region(0,0,min_size,min_size))
e_clip.do_fft_inplace()
e_clip.ri2inten()
rot_avg=e_clip.rotavg()
rpix=1/($pixel_size*min_size)
pos_ice_l=1.0/($resolution_ice+$resolution_band/2.0)/rpix
pos_ice_h=1.0/($resolution_ice-$resolution_band/2.0)/rpix
pos_ref_l=1.0/($resolution_ref+$resolution_band/2.0)/rpix
pos_ref_h=1.0/($resolution_ref-$resolution_band/2.0)/rpix
rot_avg_ice=rot_avg.get_clip(Region(int(pos_ice_l),int(pos_ice_h-pos_ice_l)))
rot_avg_ref=rot_avg.get_clip(Region(int(pos_ref_l),int(pos_ref_h-pos_ref_l)))
avg_ice=np.max(rot_avg_ice.get_data_as_vector())
avg_ref=np.max(rot_avg_ref.get_data_as_vector())
print avg_ice/avg_ref
EOT
}
######################## calculate output parameters ############################
ice_ratio=$( calc_ice_ratio $raw_micrograph $apix_x )
ice_ratio=$( ROUND $ice_ratio 2 )
if [ -v ice_input_blank ] && [ -n "$ice_input_blank" ]; then
  mfp=322.0 # Krios 300kV, 20eV slit, 100 um Obj aperture
  >&2 echo calculating ice thickness on: $raw_movie
  ice_thickness=$( calc_ice_thickness $ice_input_blank $raw_movie $mfp )
  ice_thickness=$( ROUND $ice_thickness 2 )
else
  ice_thickness=0
  echo "blank reference for ice thickness missing"
fi
######################## decide if image should be excluded from export ########
LIMIT_EXPORT ice_ratio $ice_input_ratio_low $ice_input_ratio_high
if [ -v ice_input_blank ] && [ -n "$ice_input_blank" ]; then
  LIMIT_EXPORT ice_thickness $ice_input_min_thickness $ice_input_max_thickness
fi
######################## export result parameters ##############################
RESULTS ice_ratio ice_thickness


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
shopt -s extglob
######################## get parameters from GUI ###############################
#CRYOFLARE_DEBUG=1
. data_connector.sh
######################## define ids for local microscopes ######################
#instruments are identified by d number
krios_id=3517
glacios_id=9951918
######################## calculate scalefactor for grid square image ###########
grid_square_scalefactor=$( CALCULATE 512.0/$camera_width )
RESULTS grid_square_scalefactor
############### determine suffix of stack file #################################
stack_suffix="${source_stack##*.}"
RESULTS stack_suffix
############### determine exact detector #######################################
if [ $camera == "BM-Falcon" ];then
    if [ $stack_suffix == "eer" ];then
        detector=Falcon-4
    else
        wait_complete 60 $source_stack_xml
        if fgrep -q Falcon-4 $source_stack_xml; then
            detector=Falcon-4
        else
            detector=Falcon-3
        fi
    fi
elif [ $camera == "EF-CCD" ]; then
    detector=K2
else
    >&2 echo "ERROR: Unknown camera $camera detected"
    exit -1
fi
RESULTS detector
################ set Cs ########################################################
declare -A cs_table
cs_table["$glacios_id"]="2.7"
cs_table["$krios_id"]="0.01"
cs=${cs_table[$instrument_id]}
RESULTS cs
################ set sampling for magnifications with external calibration #####
declare -A sampling_table
sampling_table["${krios_id}_KX_130000"]="0.86"
#override pixel size if better calibration available
if [ ${sampling_table[${instrument_id}_${detector}_${nominal_magnification}]+_} ]; then
    apix_x=${sampling[${instrument_id}_${detector}_${nominal_magnification}]}
    apix_y=$apix_y
    RESULTS apix_x apix_y
fi
######################## define mtf location for relion #########################
path_to_mtf="/<path_to>/mtfs"
declare -A mtf_table
mtf_table["K2_300.00"]="mtf_k2_300kV_FL2.star"
mtf_table["K2_200.00"]="mtf_k2_200kV_FL2.star"
#todo get mtf files for F4
mtf_table["Falcon-4_300.00"]="f4ec_300kv.star"
mtf_table["Falcon-3_300.00"]="f3ec_300kv.star"
mtf_table["Falcon-3_200.00"]="f3ec_200kv.star"
if [ ${mtf_table["${detector}_${acceleration_voltage}"]+_} ]; then
    mtf=${mtf_table["${detector}_${acceleration_voltage}"]}
    SHARED_FILES mtf
else
    mtf="not_found.star"
fi
if FILES_MISSING; then
    #todo find better workaround for race condition
    install  -m 644 ${path_to_mtf}/$mtf $mtf 2> /dev/null ||true
fi
################ set optics group for relion ###################################
amplitude_contrast=0.1
RESULTS amplitude_contrast
if [ $microscope_input_relion_31 = true ];then
    optics_group_values="$mtf $apix_x $acceleration_voltage $cs $amplitude_contrast $apix_x"
    RESULTS optics_group_values
    CRYOFLARE_WRITE_RELION_31=1
    RESULTS CRYOFLARE_WRITE_RELION_31
fi


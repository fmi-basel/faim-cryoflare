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
######################## write data ############################################
aggregate_folder=/data/FMI/AGGREGATE_DATA
date_string=`date +%y%m%d`
logfile=${aggregate_folder}/${date_string}.log
(
        flock -n 9 || exit 1
        echo $short_name,$x,$y,$z,$apix_x,$exposure_time,$dose,$motion_max_shift,$ice_ratio,$ice_thickness,$picking_num_particles,$ctf_max_resolution,$measured_defocus,$astigmatism,$defocus_angle,$phase_shift >> $logfile
) 9>> $logfile


#!/bin/bash --noprofile  
set -u
set -e
######################## get parameters from GUI ###############################

. data_connector.sh

aggregate_folder=/data/FMI/AGGREGATE_DATA
date_string=`date +%y%m%d`

logfile=${aggregate_folder}/${date_string}.log

(
        flock -n 9 || exit 1
        echo $short_name,$X,$Y,$Z,$apix_x,$exposure_time,$dose,$phase_plate_num,$phase_plate_pos,$motioncor2_max_shift,$analyze_ice_ice_ratio,$analyze_ice_ice_thickness,$gautomatch_num_particles,$gctf_epa_limit,$gctf_defocus,$gctf_astigmatism,$gctf_defocus_angle,$gctf_phase_shift >> $logfile
) 9>> $logfile


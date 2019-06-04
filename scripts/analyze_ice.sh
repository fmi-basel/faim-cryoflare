#!/bin/bash --noprofile  
set -u
set -e
######################## get parameters from GUI ###############################

. data_connector.sh


######################## load modules ##########################################

module purge
module load eman2/2.2

######################## define output files ###################################

analyze_ice_ice_ratio_log=movies_raw/${short_name}_ice_ratio.log
FILES analyze_ice_ice_ratio_log

######################## run processing if files are missing ###################

if FILES_MISSING; then
  ice_ratio.py $copyraw_raw_average $apix_x 5.0 3.89 0.4> $analyze_ice_ice_ratio_log
fi 
 
######################## extract output parameters #############################

 
if [ -e $analyze_ice_ice_ratio_log ]; then
  analyze_ice_ice_ratio=`tail -n 1 $analyze_ice_ice_ratio_log`
else
  analyze_ice_ice_ratio=0
  echo "missing ice ratio log:  $analyze_ice_ice_ratio_log"
fi


if [ -n "$analyze_ice_input_blank" ]; then
  analyze_ice_ice_thickness=`calc_ice_thickness.py $analyze_ice_input_blank $copyraw_raw_stack`
else
  analyze_ice_ice_thickness=0
  echo "blank reference for ice thickness missing"
fi

######################## decide if image should be excluded from export ########

LIMIT_EXPORT analyze_ice_ice_ratio $analyze_ice_input_ice_ratio_low $analyze_ice_input_ice_ratio_high


######################## export result parameters ##############################

RESULTS analyze_ice_ice_ratio analyze_ice_ice_thickness

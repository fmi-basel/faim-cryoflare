#!/bin/bash --noprofile
######################## get parameters from GUI ###############################

. data_connector.sh


######################## load modules ##########################################

module purge
module load gautomatch


######################## create destination folders ############################

mkdir -p $destination_path/


######################## define output files ###################################

if [ -n "motioncor2_aligned_avg" ]; then
    average_var=motioncor2_aligned_avg # use average from motioncor2
else
    average_var=unblur_aligned_avg # use average from unblur
fi


gautomatch_log=${!average_var/.mrc/_gautomatch.log}
gautomatch_box_file=${!average_var/.mrc/_automatch.box}
gautomatch_rejected_box_file=${!average_var/.mrc/_rejected.box}
gautomatch_star_file_no_DW=${!average_var/.mrc/_automatch.star}
gautomatch_star_file=${!average_var/.mrc/_DW_automatch.star}
FILES gautomatch_log gautomatch_box_file gautomatch_rejected_box_file gautomatch_star_file


######################## run processing if files are missing ###################

if FILES_MISSING; then
  gautomach_params=" --gid $gpu_id"
  gautomach_params+=" --apixM $apix_x"
  gautomach_params+=" --lsigma_cutoff ${gautomatch_input_sigma_cutoff:=1.3}"
  gautomach_params+=" --lsigma_D ${gautomatch_input_sigma_d:=200}"
  gautomach_params+=" --speed ${gautomatch_input_speed:=2}"
  gautomach_params+="  --diameter ${gautomatch_input_diameter:=400}"
  gautomach_params+="  --cc_cutoff ${gautomatch_input_cc_cutoff:=0.1}"
  gautomach_params+="  --min_dist ${gautomatch_input_min_dist:=100}"
  if [ -n "$gautomatch_input_reference" ]; then
    gautomach_params+="  --T $gautomatch_input_reference"
    gautomach_params+="  --apixT $gautomatch_input_reference_pixel_size"
  fi
  
  RUN gautomatch $gautomach_params ${!average_var} >> $gautomatch_log 
  mv $gautomatch_star_file_no_DW $gautomatch_star_file
fi



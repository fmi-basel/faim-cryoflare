#!/bin/bash --noprofile 
set -e
set -u
######################## get parameters from GUI ###############################

. data_connector.sh


######################## load modules ##########################################

module purge
module load motioncor2/1.0.5


######################## create destination folders ############################

mkdir -p micrographs_mc2 



######################## define output files ###################################

motioncor2_log=micrographs_mc2/${short_name}_motioncor2.log
motioncor2_aligned_avg=micrographs_mc2/${short_name}.mrc
motioncor2_aligned_avg_dw=micrographs_mc2/${short_name}_DW.mrc
FILES motioncor2_log motioncor2_aligned_avg motioncor2_aligned_avg_dw 


######################## define additional parameters ##########################
dose_per_frame=`CALCULATE "$mc2_input_dose/$num_frames"`
echo $dose_per_frame $mc2_input_dose $num_frames
bft="300"
iter=4

if [ "$mc2_input_ft_bin" -eq "0" ]; then
  mc2_ft_bin=1
fi

######################## run processing if files are missing ###################

if FILES_MISSING; then
  mc2_params="-InMrc $copyraw_raw_stack "
  mc2_params+="-Iter $iter "
  mc2_params+="-Patch $mc2_input_patch "
  mc2_params+="-Bft $bft "
  mc2_params+="-OutMrc $motioncor2_aligned_avg "
  mc2_params+="-LogFile $motioncor2_log "
  mc2_params+="-FmDose $dose_per_frame "
  mc2_params+="-PixSize $apix_x "
  mc2_params+="-kV 300 "
  mc2_params+="-Align 1 "
  mc2_params+="-FtBin $mc2_input_ft_bin "
  mc2_params+="-Gpu $gpu_id  "
  mc2_gain_params=""
  if [ ! -z ${copyraw_gain_ref+x} ] && [ -e $copyraw_gain_ref ]; then
    mc2_gain_params="-Gain $copyraw_gain_ref "
  fi
  
  echo will run: MotionCor2 $mc2_params $mc2_gain_params
  RUN MotionCor2 $mc2_params $mc2_gain_params  > $motioncor2_log  2>&1
fi
if [ "$mc2_input_ft_bin" -ge "2" ]; then
  apix_x=`CALCULATE $mc2_input_ft_bin*$apix_x`
  apix_y=`CALCULATE $mc2_input_ft_bin*$apix_y`
  RESULTS apix_x apix_y
fi


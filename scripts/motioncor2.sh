#!/bin/sh --noprofile
######################## get parameters from GUI ###############################

. data_connector.sh


######################## load modules ##########################################

module purge
module load motioncor2/1.0.5


######################## create destination folders ############################

mkdir -p $destination_path/micrographs_mc2 



######################## define output files ###################################

motioncor2_log=$destination_path/micrographs_mc2/${short_name}_motioncor2.log
motioncor2_aligned_avg=$destination_path/micrographs_mc2/${short_name}.mrc
motioncor2_aligned_avg_dw=$destination_path/micrographs_mc2/${short_name}_DW.mrc
FILES motioncor2_log motioncor2_aligned_avg motioncor2_aligned_avg_dw 


######################## define additional parameters ##########################
dose_per_frame=`CALCULATE "$dose/$num_frames"`
bft="300"
iter=4

if [ "$mc2_input_ft_bin" -eq "0" ]; then
  mc2_ft_bin=1
fi

######################## run processing if files are missing ###################

if FILES_MISSING; then
  RUN MotionCor2 -InMrc $copyraw_raw_stack  -Gain $copyraw_gain_ref -Iter $iter -Patch $mc2_input_patch -bft $bft -OutMrc $motioncor2_aligned_avg -LogFile $motioncor2_log -FmDose $dose_per_frame -PixSize $apix_x -kV 300 -Align 1 -FtBin $mc2_input_ft_bin -Gpu $gpu_id  > $motioncor2_log  2>&1
fi
if [ "$mc2_input_ft_bin" -ge "2" ]; then
  apix_x=`CALCULATE $mc2_input_ft_bin*$apix_x`
  apix_y=`CALCULATE $mc2_input_ft_bin*$apix_y`
  RESULTS apix_x apix_y
fi


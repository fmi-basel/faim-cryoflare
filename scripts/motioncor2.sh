#!/bin/sh --noprofile
######################## get parameters from GUI ###############################

. data_connector.sh


######################## load modules ##########################################

module purge
module load motioncor2/20161019


######################## create destination folders ############################

mkdir -p $destination_path/micrographs_mc2 



######################## define output files ###################################

motioncor2_log=$destination_path/micrographs_mc2/${short_name}_motioncor2.log
motioncor2_aligned_avg=$destination_path/micrographs_mc2/${short_name}.mrc
motioncor2_aligned_avg_dw=$destination_path/micrographs_mc2/${short_name}_DW.mrc
FILES motioncor2_log motioncor2_aligned_avg motioncor2_aligned_avg_dw 


######################## define additional parameters ##########################

pixel_size=`CALCULATE "1e10*$apix_x"`
dose_per_frame=`CALCULATE "1e-20*$dose/$pixel_size/$pixel_size/$num_frames"`
patch="2 2"
bft="300"


######################## run processing if files are missing ###################

if FILES_MISSING; then
  RUN MotionCor2 -InMrc $unpack_raw_stack -Patch $patch -bft $bft -OutMrc $motioncor2_aligned_avg -LogFile $motioncor2_log -FmDose $dose_per_frame -PixSize $pixel_size -kV 300 -Align 1 -Gpu $gpu_id  > $motioncor2_log  2>&1
fi



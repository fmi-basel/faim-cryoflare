#!/bin/bash
. ./data_connector.sh

clamped_stack=$destination_path/${name}_clamped.mrcs
clamped_stack_mrc=$destination_path/${name}_clamped.mrc
unblur_param=$destination_path/${name}_unblur.param
unblur_log=$destination_path/${name}_unblur.log
aligned_stack=$destination_path/${name}_aligned_movie.mrc
aligned_avg=$destination_path/${name}_aligned.mrc
aligned_avg_fft_thumbnail=$destination_path/${name}_aligned_fft.png
shift_txt=$destination_path/${name}_shift.txt
pixel_size=`calculate "1e10*$apix_x"`


if [ ! -e ${aligned_avg} ]; then
  e2proc2d.py $raw_stack $clamped_stack --process threshold.clampminmax.nsigma:nsigma=4:tomean=1 > $unblur_log
  ln -s $clamped_stack $clamped_stack_mrc >> $unblur_log

  export OMP_NUM_THREADS=4

  echo input_filename $clamped_stack_mrc  > $unblur_param
  echo number_of_frames_per_movie $num_frames >> $unblur_param
  echo output_filename $aligned_avg  >> $unblur_param
  echo shifts_filename $shift_txt >> $unblur_param
  echo pixel_size $pixel_size >> $unblur_param
  echo apply_dose_filter no >> $unblur_param
  echo save_aligned_frames yes >> $unblur_param
  echo aligned_frames_filename $aligned_stack  >> $unblur_param
  echo set_expert_options no >> $unblur_param

  unblur  $unblur_param  >> $unblur_log
  mv $aligned_stack ${aligned_stack}s

  rm  -fr $clamped_stack_mrc $clamped_stack
  
  e2proc2d.py --process math.realtofft --clip 512,512 --process mask.sharp:inner_radius=3 $aligned_avg $aligned_avg_fft_thumbnail
fi

RESULTS["aligned_stack"]=${aligned_stack}s
RESULTS["aligned_avg"]=${aligned_avg}
RESULTS["aligned_avg_fft_thumbnail"]=${aligned_avg_fft_thumbnail}
RESULTS["unblur_score"]=`fgrep "Final Score" $unblur_log|cut -f2 -d:|xargs`


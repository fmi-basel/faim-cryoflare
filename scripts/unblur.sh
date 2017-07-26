#!/bin/sh --noprofile
. $STACK_GUI_SCRIPTS/data_connector.sh

module purge
module load unblur
module load eman2

mkdir -p $destination_path/movies_unblur $destination_path/micrographs_unblur $destination_path/micrographs_unblur_dw

unblur_log=$destination_path/movies_unblur/${short_name}_unblur.log
aligned_stack=$destination_path/movies_unblur/${short_name}.mrc
shift_txt=$destination_path/movies_unblur/${short_name}_shift.txt
frc_txt=$destination_path/movies_unblur/${short_name}_frc.txt

aligned_avg=$destination_path/micrographs_unblur/${short_name}.mrc
aligned_avg_png=$destination_path/micrographs_unblur/${short_name}.png
aligned_avg_fft_thumbnail=$destination_path/micrographs_unblur/${short_name}_fft.png

aligned_avg_dw=$destination_path/micrographs_unblur_dw/${short_name}.mrc
aligned_avg_png_dw=$destination_path/micrographs_unblur_dw/${short_name}.png
aligned_avg_fft_thumbnail_dw=$destination_path/micrographs_unblur_dw/${short_name}_fft.png

pixel_size=`calculate "1e10*$apix_x"`
dose_per_frame=`calculate "1e-20*$dose/$pixel_size/$pixel_size/$num_frames"`


if [ ! -e ${aligned_avg} ]; then
  clamped_stack=$scratch/${short_name}_clamped.mrcs
  clamped_stack_mrc=$scratch/${short_name}_clamped.mrc
  e2proc2d.py $raw_stack $clamped_stack --process threshold.clampminmax.nsigma:nsigma=4:tomean=1 > $unblur_log
  ln -s $clamped_stack $clamped_stack_mrc >> $unblur_log


  unblur_param=$scratch/${short_name}_unblur.param
  echo input_filename $clamped_stack_mrc  > $unblur_param
  echo number_of_frames_per_movie $num_frames >> $unblur_param
  echo output_filename $aligned_avg  >> $unblur_param
  echo shifts_filename $shift_txt >> $unblur_param
  echo pixel_size $pixel_size >> $unblur_param
  echo apply_dose_filter no >> $unblur_param
  echo save_aligned_frames yes >> $unblur_param
  echo aligned_frames_filename $aligned_stack  >> $unblur_param
  echo set_expert_options no >> $unblur_param

  export OMP_NUM_THREADS=4
  unblur  $unblur_param  >> $unblur_log
  mv $aligned_stack ${aligned_stack}s
  
fi

[ -e ${aligned_avg_fft_thumbnail} ] ||  e2proc2d.py --process math.realtofft  --fouriershrink 7.49609375  --process mask.sharp:inner_radius=1 $aligned_avg $aligned_avg_fft_thumbnail
[ -e ${aligned_avg_png}           ] ||  e2proc2d.py --fouriershrink 7.49609375 ${aligned_avg} ${aligned_avg_png} 

if [ ! -e ${aligned_avg_dw} ]; then

  ln -s ${aligned_stack}s $aligned_stack
  summovie_param=$scratch/${short_name}_summovie.param
  echo INPUT_FILENAME $aligned_stack > $summovie_param
  echo number_of_frames_per_movie $num_frames >> $summovie_param
  echo output_filename $aligned_avg_dw >> $summovie_param
  echo shifts_filename $shift_txt >> $summovie_param
  echo frc_filename $frc_txt >> $summovie_param
  echo first_frame 1 >> $summovie_param
  echo LAST_FRAME $num_frames >> $summovie_param
  echo PIXEL_SIZE $pixel_size >> $summovie_param
  echo apply_dose_filter yes >> $summovie_param
  echo dose_per_frame $dose_per_frame >> $summovie_param
  echo acceleration_voltage 300 >> $summovie_param
  echo pre_exposure_amount 0.0 >> $summovie_param
  echo restore_power yes >> $summovie_param

  export OMP_NUM_THREADS=4
  summovie  $summovie_param  >> $unblur_log
fi
[ -e ${aligned_avg_fft_thumbnail_dw} ] ||  e2proc2d.py --process math.realtofft  --fouriershrink 7.49609375  --process mask.sharp:inner_radius=1 $aligned_avg_dw $aligned_avg_fft_thumbnail_dw
[ -e ${aligned_avg_png_dw}           ] ||  e2proc2d.py --fouriershrink 7.49609375 ${aligned_avg_dw} ${aligned_avg_png_dw} 

RESULT_FILE["aligned_stack"]=${aligned_stack}s
RESULT_FILE["unblur_log"]=${unblur_log}
RESULT_FILE["shift_txt"]=${shift_txt}
RESULT_FILE["frc_txt"]=${frc_txt}

RESULT_FILE["aligned_avg"]=${aligned_avg}
RESULT_FILE["aligned_avg_png"]=${aligned_avg_png}
RESULT_FILE["aligned_avg_fft_thumbnail"]=${aligned_avg_fft_thumbnail}

RESULT_FILE["aligned_avg_dw"]=${aligned_avg_dw}
RESULT_FILE["aligned_avg_png_dw"]=${aligned_avg_png_dw}
RESULT_FILE["aligned_avg_fft_thumbnail_dw"]=${aligned_avg_fft_thumbnail_dw}

RESULT["unblur_score"]=`fgrep "Final Score" $unblur_log|cut -f2 -d:|xargs`

relion_jobid=1
HEADER="\ndata_\n\nloop_\n_rlnMicrographName #1"
mkdir -p $destination_path/Import/job00$relion_jobid
[ -e $destination_path/Import/micrographs_unblur ] || ln -s ../Import/job00$relion_jobid $destination_path/Import/micrographs_unblur
write_to_star $destination_path/Import/job00$relion_jobid/micrographs.star "$HEADER" micrographs_unblur/${short_name}.mrc 
add_to_pipeline  $destination_path/default_pipeline.star Import $relion_jobid micrographs_unblur  "" "Import/job00$relion_jobid/micrographs.star:1"

relion_jobid=2
HEADER="\ndata_\n\nloop_\n_rlnMicrographName #1"
mkdir -p $destination_path/Import/job00$relion_jobid
[ -e $destination_path/Import/micrographs_unblur_dw ] || ln -s ../Import/job00$relion_jobid $destination_path/Import/micrographs_unblur_dw
write_to_star $destination_path/Import/job00$relion_jobid/micrographs.star "$HEADER" micrographs_unblur_dw/${short_name}.mrc 
add_to_pipeline  $destination_path/default_pipeline.star Import $relion_jobid micrographs_unblur_dw  "" "Import/job00$relion_jobid/micrographs.star:1"

relion_jobid=3
HEADER="\ndata_\nloop_\n_rlnMicrographMovieName"
mkdir -p $destination_path/Import/job00$relion_jobid
[ -e $destination_path/Import/movies_unblur ] || ln -s ../Import/job00$relion_jobid $destination_path/Import/movies_unblur
write_to_star $destination_path/Import/job00$relion_jobid/movies.star "$HEADER" movies_unblur/${short_name}.mrcs 
add_to_pipeline  $destination_path/default_pipeline.star Import $relion_jobid movies_unblur  "" "Import/job00$relion_jobid/movies.star:0"


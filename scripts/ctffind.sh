#!/bin/sh --noprofile
. $STACK_GUI_SCRIPTS/data_connector.sh


relion_jobid=5
relion_job_micrographs_dir=$destination_path/CtfFind/job00$relion_jobid/micrographs_unblur
mkdir -p $relion_job_micrographs_dir
relion_alias CtfFind $relion_jobid ctffind


pixel_size=`calculate "1e10*$apix_x"`
defocus_start=`calculate -0.5*$defocus`
defocus_end=`calculate -2.0*$defocus`
detector_pixel_size=`calculate $pixel_size*$nominal_magnification*1e-4`

ctffind_aligned_avg_link=$relion_job_micrographs_dir/${short_name}.mrc
ctffind_log=$relion_job_micrographs_dir/${short_name}_ctffind4.log
ctffind_out_txt=$relion_job_micrographs_dir/${short_name}.txt
ctffind_diag_file=${ctffind_aligned_avg_link/mrc/ctf}
ctffind_diag_file_png=${ctffind_aligned_avg_link/mrc/ctf.png}

ln -s ../../../micrographs_unblur/${aligned_avg##*/} $ctffind_aligned_avg_link
if [ ! -e $ctffind_diag_file ]; then
  module purge
  module load eman2/2.2 ctffind
  ctffind_param=$scratch/${short_name}_ctffind4.param
  echo "$ctffind_aligned_avg_link" > $ctffind_param
  echo "$ctffind_diag_file" >> $ctffind_param
  echo "$pixel_size"  >> $ctffind_param
  echo "300.0"  >> $ctffind_param
  echo "0.001"  >> $ctffind_param
  echo "0.07"  >> $ctffind_param
  echo "1024"  >> $ctffind_param
  echo "20" >> $ctffind_param 
  echo "3" >> $ctffind_param
  echo "$defocus_start" >> $ctffind_param
  echo "$defocus_end" >> $ctffind_param
  echo "500.0" >> $ctffind_param
  echo "no" >> $ctffind_param
  echo "yes" >> $ctffind_param
  echo "yes" >> $ctffind_param
  echo "500" >> $ctffind_param
  if [ $phase_plate == "true" ] ; then 
    echo "yes" >> $ctffind_param
    echo "0.175" >> $ctffind_param
    echo "3.14" >> $ctffind_param
    echo "0.175"  >> $ctffind_param
  else 
    echo "no" >> $ctffind_param
  fi
  echo "no" >> $ctffind_param
  cat $ctffind_param > $ctffind_log
  echo PhasePlate: $phase_plate >> $ctffind_log

  run ctffind   < $ctffind_param >> $ctffind_log
  run e2proc2d.py --parallel thread:n=8  --fouriershrink 2 $ctffind_diag_file $ctffind_diag_file_png
fi

defocus_u=`tail -n 1 $ctffind_out_txt|cut -f2 -d" "`
defocus_v=`tail -n 1 $ctffind_out_txt|cut -f3 -d" "`
phase_shift_rad=`tail -n 1 $ctffind_out_txt|cut -f5 -d" "`
phase_shift=`calculate $phase_shift_rad/3.141*180.0`
defocus_ctffind=`calculate \($defocus_u+$defocus_v\)/2.0`
astigmatism=`calculate \($defocus_u-$defocus_v\)/2.0`

defocus_angle=`tail -n 1 $ctffind_out_txt|cut -f4 -d" "`
ctffind_cc=`tail -n 1 $ctffind_out_txt|cut -f6 -d" "`
max_res=`tail -n 1 $ctffind_out_txt|cut -f7 -d" "`

if [ $phase_plate == "true" ] ; then
  HEADER="\ndata_\n\nloop_\n_rlnMicrographName #1\n_rlnCtfImage #2\n_rlnDefocusU #3\n_rlnDefocusV #4\n_rlnDefocusAngle #5\n_rlnVoltage #6\n_rlnSphericalAberration #7\n_rlnAmplitudeContrast #8\n_rlnMagnification #9\n_rlnDetectorPixelSize #10\n_rlnCtfFigureOfMerit #11\n_rlnCtfMaxResolution #12\n_rlnPhaseShift #13"
  write_to_star $destination_path/CtfFind/job00$relion_jobid/micrographs_ctf.star "$HEADER" micrographs_unblur/${short_name}.mrc CtfFind/job00$relion_jobid/micrographs_unblur/${short_name}.ctf:mrc $defocus_u $defocus_v $defocus_angle 300 0.001 0.07 $nominal_magnification $detector_pixel_size $ctffind_cc $max_res $phase_shift
else
  HEADER="\ndata_\n\nloop_\n_rlnMicrographName #1\n_rlnCtfImage #2\n_rlnDefocusU #3\n_rlnDefocusV #4\n_rlnDefocusAngle #5\n_rlnVoltage #6\n_rlnSphericalAberration #7\n_rlnAmplitudeContrast #8\n_rlnMagnification #9\n_rlnDetectorPixelSize #10\n_rlnCtfFigureOfMerit #11\n_rlnCtfMaxResolution #12"
  write_to_star $destination_path/CtfFind/job00$relion_jobid/micrographs_ctf.star "$HEADER" micrographs_unblur/${short_name}.mrc CtfFind/job00$relion_jobid/micrographs_unblur/${short_name}.ctf:mrc $defocus_u $defocus_v $defocus_angle 300 0.001 0.07 $nominal_magnification $detector_pixel_size $ctffind_cc $max_res
fi
add_to_pipeline  $destination_path/default_pipeline.star CtfFind $relion_jobid ctffind "Import/job001/micrographs.star" "CtfFind/job00$relion_jobid/micrographs_ctf.star:1"

RESULTS defocus_ctffind astigmatism defocus_angle phase_shift max_res max_res ctffind_cc ctffind_diag_file_png
FILES ctffind_aligned_avg_link ctffind_log ctffind_diag_file ctffind_out_txt

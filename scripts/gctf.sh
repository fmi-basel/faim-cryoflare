#!/bin/sh --noprofile
. $STACK_GUI_SCRIPTS/data_connector.sh


relion_jobid=6
relion_job_micrographs_dir=$destination_path/CtfFind/job00$relion_jobid/micrographs_unblur
mkdir -p $relion_job_micrographs_dir
relion_alias CtfFind $relion_jobid gctf

epa_cc_cutoff=0.75
pixel_size=`calculate "1e10*$apix_x"`
detector_pixel_size=`calculate $pixel_size*$nominal_magnification*1e-4`
gctf_diag_file=$relion_job_micrographs_dir/${short_name}.ctf
gctf_diag_file_mrc=$relion_job_micrographs_dir/${short_name}.ctf.mrc
gctf_diag_file_png=$relion_job_micrographs_dir/${short_name}.ctf.png

gctf_log=$relion_job_micrographs_dir/${short_name}_gctf.out
gctf_aligned_log=$relion_job_micrographs_dir/${short_name}_gctf.log
gctf_epa_log=$relion_job_micrographs_dir/${short_name}_EPA.log


if [ ! -e $gctf_aligned_log ]; then
  module purge
  module load gctf
  module switch gctf/1.06_cuda7
  module load eman2

  gctf_params=" --gid $gpu_id"
  gctf_params+=" --apix $pixel_size"
  gctf_params+=" --kV 300"
  gctf_params+=" --cs 0.001"
  gctf_params+=" --ac 0.0"
  gctf_params+=" --dstep $pixel_size"
  gctf_params+=" --astm 1000"
  gctf_params+=" --bfac 100"
  gctf_params+=" --resL 20.0"
  gctf_params+=" --resH 3.7"
  gctf_params+=" --boxsize 1024"
  gctf_params+=" --do_EPA 1"
  gctf_params+=" --refine_after_EPA 0"
  gctf_params+=" --convsize 30"
  gctf_params+=" --do_Hres_ref 1"
  gctf_params+=" --Href_resL 15.0"
  gctf_params+=" --Href_resH 3.0"
  gctf_params+=" --Href_bfac 50"
  gctf_params+=" --estimate_B 1"
  gctf_params+=" --B_resL 10.0"
  gctf_params+=" --B_resH 3.5"
  gctf_params+=" --do_validation 1"
  gctf_params+=" --do_local_refine 1"
  gctf_params+="  --boxsuffix _automatch.star"

  gctf_phase_plate_params=" --phase_shift_L 10"
  gctf_phase_plate_params+=" --phase_shift_H 150"
  gctf_phase_plate_params+=" --phase_shift_S 10"
  gctf_phase_plate_params+=" --phase_shift_T 1"

  gctf_defocus_params=" --defL `calculate -0.5*$defocus`"
  gctf_defocus_params+=" --defH `calculate -2.0*$defocus`"
  gctf_defocus_params+=" --defS 500"

  ln -s $gautomatch_star_file $relion_job_micrographs_dir
  ln -s ../../../micrographs_unblur/${aligned_avg##*/} $relion_job_micrographs_dir
  aligned_avg_link=$relion_job_micrographs_dir/${aligned_avg##*/}
  if [ $phase_plate == "true" ] ; then
    gctf $gctf_params $gctf_defocus_params $gctf_phase_plate_params $aligned_avg_link &>> $gctf_log
  else
    gctf $gctf_params $gctf_defocus_params  $aligned_avg_link  &>> $gctf_log
  fi
  measured_defocus=`fgrep -A 1 "Defocus_U" $gctf_aligned_log|head -n 2 |tail -n 1|xargs`
  defocus_u=`echo $measured_defocus|cut -f1 -d" "`
  defocus_v=`echo $measured_defocus|cut -f2 -d" "`
  average_defocus=`calculate \($defocus_u+$defocus_v\)/2.0`

  gctf_defocus_params=" --defL `calculate 0.8*$average_defocus`"
  gctf_defocus_params+=" --defH `calculate 1.2*$average_defocus`"
  gctf_defocus_params+=" --defS 100"
  if [ $phase_plate == "true" ] ; then
    gctf $gctf_params $gctf_defocus_params $gctf_phase_plate_params $aligned_avg_link &>> $gctf_log
  else
    gctf $gctf_params $gctf_defocus_params  $aligned_avg_link  &>> $gctf_log
  fi

  ln -s $gctf_diag_file $gctf_diag_file_mrc
  e2proc2d.py  --meanshrink 2  $gctf_diag_file_mrc  $gctf_diag_file_png
fi
measured_defocus=`fgrep -A 1 "Defocus_U" $gctf_aligned_log|head -n 2 |tail -n 1|xargs`
defocus_u=`echo $measured_defocus|cut -f1 -d" "`
defocus_v=`echo $measured_defocus|cut -f2 -d" "`

epa_limit_cc=`python - $gctf_epa_log << EOT
import sys
with open(sys.argv[1]) as f:
    for line in f.readlines()[1:]:
        sp=line.split()
        if float(sp[4])<0.75:
            break
        res=sp[0]
        cc=sp[4]
    print res,cc
EOT`
epa_limit=`echo $epa_limit_cc|cut -f1 -d' '`
epa_cc=`echo $epa_limit_cc|cut -f2 -d' '`

rm $destination_path/micrographs_all_gctf.star >& /dev/null


defocus_gctf=`calculate \($defocus_u+$defocus_v\)/2.0`
astigmatism_gctf=`calculate \($defocus_u-$defocus_v\)/2.0`
defocus_gctf_angle=`echo $measured_defocus|cut -f3 -d" "`

if [ $phase_plate == "true" ] ; then
  phase_shift_gctf`echo $measured_defocus|cut -f4 -d" "`
  HEADER="\ndata_\n\nloop_\n_rlnMicrographName #1\n_rlnCtfImage #2\n_rlnDefocusU #3\n_rlnDefocusV #4\n_rlnDefocusAngle #5\n_rlnVoltage #6\n_rlnSphericalAberration #7\n_rlnAmplitudeContrast #8\n_rlnMagnification #9\n_rlnDetectorPixelSize #10\n_rlnCtfFigureOfMerit #11\n_rlnCtfMaxResolution #12\n_rlnPhaseShift #13"
  write_to_star $destination_path/CtfFind/job00$relion_jobid/micrographs_ctf.star "$HEADER" micrographs_unblur/${short_name}.mrc CtfFind/job00$relion_jobid/micrographs_unblur/${short_name}.ctf:mrc $defocus_u $defocus_v $defocus_gctf_angle 300 0.001 0.07 $nominal_magnification $detector_pixel_size $epa_cc $epa_limit $phase_shift_gctf
else
  phase_shift_gctf=0
  HEADER="\ndata_\n\nloop_\n_rlnMicrographName #1\n_rlnCtfImage #2\n_rlnDefocusU #3\n_rlnDefocusV #4\n_rlnDefocusAngle #5\n_rlnVoltage #6\n_rlnSphericalAberration #7\n_rlnAmplitudeContrast #8\n_rlnMagnification #9\n_rlnDetectorPixelSize #10\n_rlnCtfFigureOfMerit #11\n_rlnCtfMaxResolution #12"
  write_to_star $destination_path/CtfFind/job00$relion_jobid/micrographs_ctf.star "$HEADER" micrographs_unblur/${short_name}.mrc CtfFind/job00$relion_jobid/micrographs_unblur/${short_name}.ctf:mrc $defocus_u $defocus_v $defocus_gctf_angle 300 0.001 0.07 $nominal_magnification $detector_pixel_size $epa_cc $epa_limit
fi
add_to_pipeline  $destination_path/default_pipeline.star CtfFind $relion_jobid gctf "Import/job001/micrographs.star" "CtfFind/job00$relion_jobid/micrographs_ctf.star:1"

RESULTS epa_limit epa_cc defocus_gctf astigmatism_gctf defocus_gctf_angle gctf_diag_file_png
FILES gctf_diag_file_png gctf_log gctf_aligned_log gctf_epa_log

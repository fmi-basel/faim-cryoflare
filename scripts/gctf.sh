#!/bin/bash --noprofile
######################## get parameters from GUI ###############################

. data_connector.sh


######################## load modules ##########################################

module purge
module load gctf
module load eman2/2.2


######################## create Relion job directory ###########################

if [ -n "motioncor2_aligned_avg" ]; then
    average_var=motioncor2_aligned_avg # use average from motioncor2
    micrographs_dir=micrographs_mc2
else
    average_var=unblur_aligned_avg # use average from unblur
    micrographs_dir=micrographs_unblur
fi

rln_jobtype=CtfFind
rln_jobid=6

printf -v jobfolder "job%03d" $rln_jobid
relion_job_micrographs_dir=$destination_path/$rln_jobtype/$jobfolder/$micrographs_dir
mkdir -p $relion_job_micrographs_dir



######################## define output files ###################################

gctf_diag_file=$relion_job_micrographs_dir/${short_name}.ctf
gctf_diag_file_mrc=$relion_job_micrographs_dir/${short_name}.ctf.mrc
gctf_diag_file_png=$relion_job_micrographs_dir/${short_name}.ctf.png

gctf_log=$relion_job_micrographs_dir/${short_name}_gctf.out
gctf_aligned_log=$relion_job_micrographs_dir/${short_name}_gctf.log
gctf_epa_log=$relion_job_micrographs_dir/${short_name}_EPA.log

gctf_avg_link=$relion_job_micrographs_dir/${!average_var##*/}


FILES gctf_diag_file gctf_diag_file_mrc gctf_diag_file_png gctf_log gctf_aligned_log gctf_epa_log gctf_avg_link


######################## define additional parameters ##########################

if [ "$camera" == "EF-CCD" ]; then
    dstep=5
else
    dstep=14
fi
magnification=`CALCULATE "1e4*$dstep/$apix_x"`

######################## run processing if files are missing ###################

if FILES_MISSING; then
  gctf_params=" --gid $gpu_id"
  gctf_params+=" --apix $apix_x"
  gctf_params+=" --kV 300"
  gctf_params+=" --cs 0.001"
  gctf_params+=" --ac 0.07"
  gctf_params+=" --dstep $dstep" 
  gctf_params+=" --astm 1000"
  gctf_params+=" --bfac 100"
  gctf_params+=" --resL 20.0"
  gctf_params+=" --resH 3.7"
  gctf_params+=" --boxsize 1024"
  gctf_params+=" --convsize 30"
  gctf_params+=" --ctfstar  NONE"
  gctf_params+=" --ctfout_bfac 50.00"
  gctf_phase_plate_params=" --phase_shift_L 10"
  gctf_phase_plate_params+=" --phase_shift_H 170"
  gctf_phase_plate_params+=" --phase_shift_S 10"
  gctf_phase_plate_params+=" --phase_shift_T 1"

  gctf_defocus_params=" --defL `CALCULATE -0.5*$defocus`"
  gctf_defocus_params+=" --defH `CALCULATE -2.5*$defocus`"
  gctf_defocus_params+=" --defS 500"

  ln -s ../../../$micrographs_dir/${gautomatch_star_file##*/} $relion_job_micrographs_dir
  ln -s ../../../$micrographs_dir/${!average_var##*/} $relion_job_micrographs_dir
  if [ $phase_plate == "true" ] ; then
    RUN gctf $gctf_params $gctf_defocus_params $gctf_phase_plate_params $gctf_avg_link &>> $gctf_log
  else
    RUN gctf $gctf_params $gctf_defocus_params  $gctf_avg_link  &>> $gctf_log
  fi
  measured_defocus=`cat $gctf_aligned_log|fgrep -v Local|tail -n 20|fgrep -A 1 "Defocus_U"|head -n 2 |tail -n 1|xargs`
  defocus_u=`echo $measured_defocus|cut -f1 -d" "`
  defocus_v=`echo $measured_defocus|cut -f2 -d" "`
  average_defocus=`CALCULATE \($defocus_u+$defocus_v\)/2.0`

  gctf_defocus_params=" --defL `CALCULATE 0.9*$average_defocus`"
  gctf_defocus_params+=" --defH `CALCULATE 1.1*$average_defocus`"
  gctf_defocus_params+=" --defS 100"
  gctf_params+=" --do_local_refine 1"
  gctf_params+=" --boxsuffix _DW_automatch.star"
  gctf_params+=" --do_EPA 1"
  gctf_params+=" --refine_after_EPA 0"
  gctf_params+=" --do_Hres_ref 1"
  gctf_params+=" --Href_resL 15.0"
  gctf_params+=" --Href_resH 3.0"
  gctf_params+=" --Href_bfac 50"
  gctf_params+=" --estimate_B 1"
  gctf_params+=" --B_resL 10.0"
  gctf_params+=" --B_resH 3.5"
  gctf_params+=" --do_validation 1"
  gctf_params+=" --local_radius 256"
  gctf_params+=" --local_boxsize 128"
  if [ $phase_plate == "true" ] ; then
    RUN gctf $gctf_params $gctf_defocus_params $gctf_phase_plate_params $gctf_avg_link &>> $gctf_log
  else
    RUN gctf $gctf_params $gctf_defocus_params  $gctf_avg_link  &>> $gctf_log
  fi

  ln -s ${gctf_diag_file##*/} $gctf_diag_file_mrc
  RUN e2proc2d.py  --meanshrink 2  $gctf_diag_file_mrc  $gctf_diag_file_png
fi


######################## extract output parameters #############################

measured_defocus=`cat $gctf_aligned_log|fgrep -v Local|tail -n 20|fgrep -A 1 "Defocus_U"|head -n 2 |tail -n 1|xargs`
defocus_u=`echo $measured_defocus|cut -f1 -d" "`
defocus_v=`echo $measured_defocus|cut -f2 -d" "`

gctf_epa_limit_cc=`python - $gctf_epa_log << EOT
import sys
with open(sys.argv[1]) as f:
    for line in f.readlines()[1:]:
        sp=line.split()
        if float(sp[4])<$gctf_input_epa_cutoff or sp[4]=="-nan":
            break
        res=sp[0]
        cc=sp[4]
    print res,cc
EOT`
gctf_epa_limit=`echo $gctf_epa_limit_cc|cut -f1 -d' '`
gctf_epa_cc=`echo $gctf_epa_limit_cc|cut -f2 -d' '`

rm $destination_path/micrographs_all_gctf.star >& /dev/null


gctf_defocus=`CALCULATE \($defocus_u+$defocus_v\)/2.0`
gctf_astigmatism=`CALCULATE abs\($defocus_u-$defocus_v\)/2.0`
gctf_defocus_angle=`echo $measured_defocus|cut -f3 -d" "`


######################## decide if image should be excluded from export ########

LIMIT_EXPORT gctf_epa_limit $gctf_input_epa_high $gctf_input_epa_low



######################## write Relion star #####################################

rln_alias=gctf
rln_nodetype=1
rln_starname=micrographs_ctf.star
rln_inputstar=Import/job001/micrographs.star
rln_header=(MicrographName CtfImage DefocusU DefocusV DefocusAngle Voltage SphericalAberration AmplitudeContrast Magnification DetectorPixelSize CtfFigureOfMerit CtfMaxResolution)


if [ -n "$defocus_u" ] ; then
  if [ $phase_plate == "true" ] ; then

    gctf_phase_shift=`echo $measured_defocus|cut -f4 -d" "`
    rln_header+=(PhaseShift)
    RELION_WRITE $destination_path $rln_jobtype $rln_jobid $rln_alias $rln_nodetype $rln_starname $rln_inputstar rln_header[@] $micrographs_dir/${short_name}_DW.mrc $rln_jobtype/$jobfolder/$micrographs_dir/${short_name}.ctf:mrc \
                                                                                                                             $defocus_u $defocus_v $gctf_defocus_angle 300 0.001 0.07 $magnification $dstep \
															     $gctf_epa_cc $gctf_epa_limit $gctf_phase_shift
  else
    gctf_phase_shift=0
    RELION_WRITE $destination_path $rln_jobtype $rln_jobid $rln_alias $rln_nodetype $rln_starname $rln_inputstar rln_header[@] $micrographs_dir/${short_name}_DW.mrc $rln_jobtype/$jobfolder/$micrographs_dir/${short_name}.ctf:mrc \
                                                                                                                             $defocus_u $defocus_v $gctf_defocus_angle 300 0.001 0.07 $magnification $dstep \
															     $gctf_epa_cc $gctf_epa_limit 

  fi
fi

######################## export result parameters ##############################

RESULTS gctf_epa_limit gctf_epa_cc gctf_defocus gctf_astigmatism gctf_defocus_angle gctf_phase_shift









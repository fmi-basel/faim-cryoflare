#!/bin/bash --noprofile  
################################################################################
#
# Author: Andreas Schenk
# Friedrich Miescher Institute, Basel, Switzerland
#
# This file is part of CryoFLARE
#
# Copyright (C) 2017-2020 by the CryoFLARE Authors
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 3.0 of the License.
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License
# along with CryoFLARE.  If not, see http://www.gnu.org/licenses/.
#
################################################################################
set -u
set -e
######################## functions ############# ###############################
calc_epa_limit_cc () {
  local epa_log=$1
  local cutoff=$2
  python - $epa_log << EOT
import sys
with open(sys.argv[1]) as f:
    for line in f.readlines()[1:]:
        sp=line.split()
        if float(sp[4])<$cutoff or sp[4]=="-nan":
            break
        res=sp[0]
        cc=sp[4]
    print res,cc
EOT
}
######################## get parameters from GUI ###############################
. data_connector.sh
######################## load modules ##########################################
module purge
module load Gctf/1.06-cuda10
gctf_exe=Gctf-v1.06_sm_30_cu10.0_x86_64
module load EMAN2/2.20
######################## create Relion job #####################################
rln_jobtype=CtfFind
rln_jobid=4
rln_alias=gctf
rln_nodetype=1
rln_starname=micrographs_ctf.star
rln_inputstar=MotionCorr/job003/corrected_micrographs.star
RELION_CREATE_JOB "$rln_jobtype" "$rln_jobid" "$rln_alias" "$rln_starname" "$rln_nodetype" "$rln_inputstar"
mkdir -p $rln_jobpath/$(dirname $raw_movie)
######################## define output files ###################################
ctf_image=$rln_jobpath/${raw_movie%%.*}.ctf
ctf_image_mrc=$rln_jobpath/${raw_movie%%.*}.ctf.mrc
ctf_image_png=$rln_jobpath/${raw_movie%%.*}.ctf.png
ctf_out=$rln_jobpath/${raw_movie%%.*}_gctf.out
ctf_r1_log=$rln_jobpath/${raw_movie%%.*}_gctf_r1.log
ctf_log=$rln_jobpath/${raw_movie%%.*}_gctf.log
epa_log=$rln_jobpath/${raw_movie%%.*}_EPA.log
aligned_micrograph_link=$rln_jobpath/${raw_movie%%.*}.mrc
FILES ctf_image ctf_image_png ctf_out ctf_log epa_log
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
  gctf_params+=" --kV $acceleration_voltage"
  gctf_params+=" --cs ${cs}"
  gctf_params+=" --ac 0.1"
  gctf_params+=" --dstep $dstep" 
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
  defocus_params=" --defL `CALCULATE -0.3*$defocus`"
  defocus_params+=" --defH `CALCULATE -3.0*$defocus`"
  defocus_params+=" --defS 500"
  defocus_params+=" --astm 5000"
  #ln -nfs ../../../$micrographs_dir/${gautomatch_star_file##*/} $relion_job_micrographs_dir
  ln -nfs ../../../$aligned_micrograph_no_dw $aligned_micrograph_link
  #ln -nfs ../../../$aligned_micrograph $aligned_micrograph_link
  if [ $phase_plate == "true" ] ; then
    RUN $gctf_exe $gctf_params $defocus_params $gctf_phase_plate_params $aligned_micrograph_link &>> $ctf_out
  else
    RUN $gctf_exe $gctf_params $defocus_params  $aligned_micrograph_link  &>> $ctf_out
  fi
  defocus_values=`cat $ctf_log|fgrep -v Local|tail -n 20|fgrep -A 1 "Defocus_U"|head -n 2 |tail -n 1|xargs`
  mv $ctf_log $ctf_r1_log
  mv $ctf_image ${ctf_image}_r1.mrc
  defocus_u=`echo $defocus_values|cut -f1 -d" "`
  defocus_v=`echo $defocus_values|cut -f2 -d" "`
  average_defocus=`CALCULATE \($defocus_u+$defocus_v\)/2.0`
  astigmatism=`CALCULATE abs\($defocus_u-$defocus_v\)`
  defocus_params=" --defL `CALCULATE 0.8*$average_defocus`"
  defocus_params+=" --defH `CALCULATE 1.2*$average_defocus`"
  defocus_params+=" --defS 100"
  defocus_params+=" --astm $astigmatism"
  #gctf_params+=" --do_local_refine 1"
  #gctf_params+=" --boxsuffix _DW_automatch.star"
  #gctf_params+=" --do_EPA 1"
  #gctf_params+=" --refine_after_EPA 0"
  gctf_params+=" --do_Hres_ref 1"
  gctf_params+=" --Href_resL 15.0"
  gctf_params+=" --Href_resH 3.0"
  gctf_params+=" --Href_bfac 50"
  gctf_params+=" --estimate_B 1"
  gctf_params+=" --B_resL 10.0"
  gctf_params+=" --B_resH 3.5"
  gctf_params+=" --do_validation 1"
  #gctf_params+=" --local_radius 256"
  #gctf_params+=" --local_boxsize 128"
  if [ $phase_plate == "true" ] ; then
    RUN $gctf_exe $gctf_params $defocus_params $gctf_phase_plate_params $aligned_micrograph_link &>> $ctf_out
  else
    RUN $gctf_exe $gctf_params $defocus_params  $aligned_micrograph_link  &>> $ctf_out
  fi
  ln -sf ${ctf_image##*/} $ctf_image_mrc
  RUN e2proc2d.py  --writejunk --meanshrink 2  $ctf_image_mrc  $ctf_image_png
fi
######################## extract output parameters #############################
defocus_values=`cat $ctf_log|fgrep -v Local|tail -n 20|fgrep -A 1 "Defocus_U"|head -n 2 |tail -n 1|xargs`
defocus_u=`echo $defocus_values|cut -f1 -d" "`
defocus_v=`echo $defocus_values|cut -f2 -d" "`
epa_limit_cc=$(calc_epa_limit_cc "$epa_log" $ctf_input_epa_cutoff)
ctf_max_resolution=`echo $epa_limit_cc|cut -f1 -d' '`
ctf_max_resolution=$( ROUND $ctf_max_resolution 2 )
ctf_epa_fom=`echo $epa_limit_cc|cut -f2 -d' '`
rm micrographs_all_gctf.star >& /dev/null || :
measured_defocus=`CALCULATE \($defocus_u+$defocus_v\)/2.0`
measured_defocus=$( ROUND $measured_defocus 2 )
# EPU uses opposite sign for defocus values. Therefore values have to be added instead of subtracted
defocus_difference=`CALCULATE $measured_defocus+$defocus`
astigmatism=`CALCULATE abs\($defocus_u-$defocus_v\)`
astigmatism=$( ROUND $astigmatism 2 )
defocus_angle=`echo $defocus_values|cut -f3 -d" "`
defocus_angle=$( ROUND $defocus_angle 2 )
######################## decide if image should be excluded from export ########
LIMIT_EXPORT ctf_max_resolution $ctf_input_epa_high $ctf_input_epa_low
######################## write Relion star #####################################
if [ -n "${CRYOFLARE_WRITE_RELION_31+x}" ]; then
  rln_header=(MicrographName CtfImage DefocusU DefocusV CtfAstigmatism DefocusAngle  CtfFigureOfMerit CtfMaxResolution)
else
  rln_header=(MicrographName CtfImage DefocusU DefocusV DefocusAngle Voltage SphericalAberration AmplitudeContrast Magnification DetectorPixelSize CtfFigureOfMerit CtfMaxResolution)
fi
if [ -n "$defocus_u" ] ; then
  if [ $phase_plate == "true" ] ; then

    phase_shift=`echo $defocus_values|cut -f4 -d" "`
    rln_header+=(PhaseShift)
    if [ -n "${CRYOFLARE_WRITE_RELION_31+x}" ]; then
      RELION_WRITE_STAR $rln_alias $rln_starname "rln_header[@]" $aligned_micrograph ${ctf_image}:mrc $defocus_u $defocus_v $astigmatism $defocus_angle $ctf_epa_fom $ctf_max_resolution $phase_shift
    else
      RELION_WRITE_STAR $rln_alias $rln_starname "rln_header[@]" $aligned_micrograph ${ctf_image}:mrc \
                        $defocus_u $defocus_v $defocus_angle ${acceleration_voltage} $cs $amplitude_contrast $magnification $dstep $ctf_epa_fom $ctf_max_resolution $phase_shift
    fi
  else
    phase_shift=0
    if [ -n "${CRYOFLARE_WRITE_RELION_31+x}" ]; then
      RELION_WRITE_STAR $rln_alias $rln_starname "rln_header[@]" $aligned_micrograph ${ctf_image}:mrc $defocus_u $defocus_v $astigmatism $defocus_angle $ctf_epa_fom $ctf_max_resolution 
    else
      RELION_WRITE_STAR $rln_alias $rln_starname "rln_header[@]" $aligned_micrograph ${ctf_image}:mrc \
                        $defocus_u $defocus_v $defocus_angle ${acceleration_voltage} $cs $amplitude_contrast $magnification $dstep $ctf_epa_fom $ctf_max_resolution 
    fi

  fi
fi
######################## export result parameters ##############################
RESULTS ctf_max_resolution ctf_epa_fom measured_defocus defocus_difference defocus_u defocus_v astigmatism defocus_angle phase_shift









#!/bin/bash
. /usr/prog/sb/em/sw/stack_gui/scripts/data_connector.sh

module load gctf
module switch gctf/1.06_cuda7

pixel_size=`calculate "1e10*$apix_x"`
gctf_diag_file=${aligned_avg/mrc/ctf}
gctf_diag_file_mrc=${aligned_avg/mrc/ctf_gctf.mrc}
gctf_diag_file_png=${aligned_avg/mrc/ctf_gctf.png}

gctf_log=$destination_path/${name}_gctf.log
gctf_aligned_log=$destination_path/${name}_aligned_gctf.log


if [ ! -e $gctf_aligned_log ]; then
  echo PhasePlare: $phase_plate > $gctf_log
  if [ $phase_plate == "true" ] ; then
    /usr/prog/sb/em/sw/gctf/1.06/bin/Gctf-v1.06_sm_20_cu7.0_x86_64 --phase_shift_L 0 --phase_shift_H 180 --phase_shift_T  1 --do_validation --do_EPA --gid $gpu_id --apix $pixel_size --kv 300 --cs 0.001 --AC 0.0 --do_local_refine 1  --boxsuffix _automatch.star  $aligned_avg &>> $gctf_log
  else
    /usr/prog/sb/em/sw/gctf/1.06/bin/Gctf-v1.06_sm_20_cu7.0_x86_64 --do_validation --do_EPA --gid $gpu_id --apix $pixel_size --kv 300 --cs 0.001 --AC 0.07  $aligned_avg  --do_local_refine 1  --boxsuffix _automatch.star &>> $gctf_log
  fi

fi
RESULTS[epa_limit]=`fgrep "EPA: RES_LIMIT" $gctf_aligned_log|cut -f7 -d" "|xargs`
measured_defocus=`fgrep -A 1 "Defocus_U" $gctf_aligned_log|head -n 2 |tail -n 1|xargs`
defocus_u=`echo $measured_defocus|cut -f1 -d" "`
defocus_v=`echo $measured_defocus|cut -f2 -d" "`
RESULTS[defocus_gctf]=`calculate \($defocus_u+$defocus_v\)/2.0`
RESULTS[astigmatism_gctf]=`calculate \($defocus_u-$defocus_v\)/2.0`

RESULTS[defocus_gctf_angle]=`echo $measured_defocus|cut -f3 -d" "`
if [ $phase_plate == "true" ] ; then
  RESULTS[phase_shift_gctf]=`echo $measured_defocus|cut -f4 -d" "`
else
  RESULTS[phase_shift_gctf]=0
fi

if [ ! -e $gctf_diag_file_png ]; then
  ln -s $gctf_diag_file $gctf_diag_file_mrc
  e2proc2d.py  --meanshrink 2  $gctf_diag_file_mrc  $gctf_diag_file_png
fi 
RESULTS[gctf_diag_file_png]=`echo $gctf_diag_file_png`


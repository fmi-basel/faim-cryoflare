#!/bin/bash
. ./data_connector.sh


pixel_size=`calculate "1e10*$apix_x"`
gctf_log=$destination_path/${name}_gctf.log
gctf_aligned_log=$destination_path/${name}_aligned_gctf.log


if [ ! -e $gctf_aligned_log ]; then
  /usr/prog/sb/em/sw/gctf/1.06/bin/Gctf-v1.06_sm_20_cu7.0_x86_64 --do_validation --do_EPA --gid $gpu_id --apix $pixel_size --kv 300 --cs 0.001 --AC 0.07  $aligned_avg >& $gctf_log
fi
RESULTS[epa_limit]=`fgrep "EPA: RES_LIMIT" $gctf_aligned_log|cut -f7 -d" "|xargs`
measured_defocus=`fgrep -A 1 "Defocus_U" $gctf_aligned_log|head -n 2 |tail -n 1|xargs`
RESULTS[defocus_u]=`echo $measured_defocus|cut -f1 -d" "`
RESULTS[defocus_v]=`echo $measured_defocus|cut -f2 -d" "`
RESULTS[defocus_angle]=`echo $measured_defocus|cut -f3 -d" "`


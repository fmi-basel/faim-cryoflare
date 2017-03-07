#!/bin/bash
. ./data_connector.sh


pixel_size=`calculate "1e10*$apix_x"`
ctffind_log=$destination_path/${name}_ctffind.log
ctffind_param=$destination_path/${name}_ctffind.param
ctffind_out_txt=$destination_path/${name}_aligned.ctf.txt
ctffind_diag_file=${aligned_avg/mrc/ctf.mrc}
ctffind_diag_file_png=${aligned_avg/mrc/ctf.png}
defocus_start=`calculate -0.5*1e10*$defocus`
defocus_end=`calculate -2.0*1e10*$defocus`
if [ ! -e $ctffind_diag_file ]; then
  echo "$aligned_avg" > $ctffind_param
  echo "no" >> $ctffind_param
  echo "$ctffind_diag_file" >> $ctffind_param
  echo "$pixel_size"  >> $ctffind_param
  echo "300.0"  >> $ctffind_param
  echo "0.1"  >> $ctffind_param
  echo "0.07"  >> $ctffind_param
  echo "512"  >> $ctffind_param
  echo "20" >> $ctffind_param 
  echo "3" >> $ctffind_param
  echo "$defocus_start" >> $ctffind_param
  echo "$defocus_end" >> $ctffind_param
  echo "500.0" >> $ctffind_param
  echo "no" >> $ctffind_param
  echo "no" >> $ctffind_param
  echo "yes" >> $ctffind_param
  echo "500" >> $ctffind_param
  echo "yes" >> $ctffind_param
  echo "0.0" >> $ctffind_param
  echo "3.14" >> $ctffind_param
  echo "0.5"  >> $ctffind_param
  echo "no" >> $ctffind_param
  cat $ctffind_param > $ctffind_log
  /usr/prog/sb/em/sw/ctffind4/4.1.5/ctffind   < $ctffind_param >> $ctffind_log
fi
if [ ! -e $ctffind_diag_file_png ]; then
  e2proc2d.py  $ctffind_diag_file $ctffind_diag_file_png
fi 
RESULTS[defocus_u]=`tail -n 1 $ctffind_out_txt|cut -f2 -d" "`
RESULTS[defocus_v]=`tail -n 1 $ctffind_out_txt|cut -f3 -d" "`
RESULTS[defocus_angle]=`tail -n 1 $ctffind_out_txt|cut -f4 -d" "`
RESULTS[max_res]=`tail -n 1 $ctffind_out_txt|cut -f7 -d" "`
phase_shift=`tail -n 1 $ctffind_out_txt|cut -f5 -d" "`
phase_shift_deg=`calculate $phase_shift/3.141*180.0`
RESULTS[phase_shift]=`echo $phase_shift_deg`
RESULTS[ctffind_diag_file_png]=`echo $ctffind_diag_file_png`


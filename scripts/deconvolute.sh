#!/bin/bash --noprofile
######################## get parameters from GUI ###############################

. data_connector.sh


######################## load modules ##########################################

module purge
module load eman2/2.2


######################## define output files ###################################


if [ -n "motioncor2_aligned_avg" ]; then
    average_var=motioncor2_aligned_avg # use average from motioncor2
else
    average_var=unblur_aligned_avg # use average from unblur
fi

deconvolute_png=${!average_var/.mrc/_deconv.png}
deconvolute_boxes_png=${!average_var/.mrc/_deconv_boxes.png}
FILES  deconvolute_png deconvolute_boxes_png


######################## run processing if files are missing ###################

if FILES_MISSING; then
  /programs/x86_64-linux/iplt/0.9.7/bin/iplt /usr/prog/sb/em/sw/cryoflare/1.6/scripts/deconv.py ${!average_var}  $deconvolute_png $apix_x $gctf_defocus_u $gctf_defocus_v $gctf_defocus_angle
  draw_boxes.sh  $deconvolute_png $gautomatch_box_file  $gautomatch_rejected_box_file $deconvolute_boxes_png 0x0
fi

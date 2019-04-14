#!/bin/bash --noprofile -e 
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

gautomatch_boxes_png=${!average_var/.mrc/_boxes.png}
FILES  gautomatch_boxes_png


######################## run processing if files are missing ###################

if FILES_MISSING; then
  full_png=$scratch/${short_name}_gautomatch_full.png
  RUN e2proc2d.py ${!average_var} $full_png
  draw_boxes.sh  $full_png $gautomatch_box_file  $gautomatch_rejected_box_file $gautomatch_boxes_png 0x100
fi


######################## extract output parameters #############################

gautomatch_num_particles=`cat $gautomatch_box_file|wc -l`



######################## export result parameters ##############################

RESULTS gautomatch_num_particles 

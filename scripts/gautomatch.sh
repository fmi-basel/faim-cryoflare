#!/bin/sh --noprofile
. $STACK_GUI_SCRIPTS/data_connector.sh


pixel_size=`calculate "1e10*$apix_x"`
gautomatch_log=${aligned_avg/.mrc/_gautomatch.log}
aligned_avg_boxes_png=${aligned_avg/.mrc/_boxes.png}
gautomatch_box_file=${aligned_avg/.mrc/_automatch.box}
gautomatch_star_file=${aligned_avg/.mrc/_automatch.star}

if [ ! -e $gautomatch_box_file ]; then
  gautomach_params=" --gid $gpu_id"
  gautomach_params+=" --apixM $pixel_size"
  gautomach_params+=" --lsigma_cutoff $sigma_cutoff"
  gautomach_params+=" --lsigma_D $sigma_d"
  gautomach_params+=" --speed $speed"
  gautomach_params+="  --diameter $box_size"

  module purge
  module load gautomatch
  module switch gautomatch/0.53_cuda7
  module load eman2
  full_png=$scratch/${short_name}_gautomatch_full.png
  gautomatch $gautomach_params $aligned_avg >> $gautomatch_log 
  e2proc2d.py $aligned_avg $full_png
  $STACK_GUI_SCRIPTS/draw_boxes.sh  $full_png $gautomatch_box_file  $aligned_avg_boxes_png
fi

num_particles=`cat $gautomatch_box_file|wc -l`

RESULTS num_particles  aligned_avg_boxes_png
FILES aligned_avg_boxes_png gautomatch_box_file gautomatch_star_file

#!/bin/bash
. /usr/prog/sb/em/sw/stack_gui/scripts/data_connector.sh

module load gautomatch
module switch gautomatch/0.53_cuda7

pixel_size=`calculate "1e10*$apix_x"`
gautomatch_log=$destination_path/${name}_gautomatch.log


if [ ! -e $destination_path/${name}_aligned_automatch.box  ]; then
  echo /usr/prog/sb/em/sw/gautomatch/0.53/bin/Gautomatch-v0.53_sm_20_cu7.0_x86_64 --gid $gpu_id  --apixM $pixel_size --lsigma_cutoff $sigma_cutoff --lsigma_D $sigma_d --speed $speed --diameter $box_size $aligned_avg > $gautomatch_log 
  /usr/prog/sb/em/sw/gautomatch/0.53/bin/Gautomatch-v0.53_sm_20_cu7.0_x86_64 --gid $gpu_id  --apixM $pixel_size --lsigma_cutoff $sigma_cutoff --lsigma_D $sigma_d --speed $speed --diameter $box_size $aligned_avg >> $gautomatch_log 
  e2proc2d.py $aligned_avg ${aligned_avg/.mrc/_full.png}
  /usr/prog/sb/em/sw/stack_gui/scripts/draw_boxes.sh  ${aligned_avg/.mrc/_full.png} $destination_path/${name}_aligned_automatch.box  ${aligned_avg/.mrc/_boxes.png}
  rm ${aligned_avg/.mrc/_full.png}
fi
RESULTS[num_particles]=`cat $destination_path/${name}_aligned_automatch.box|wc -l`
RESULTS[aligned_avg_boxes_png]=${aligned_avg/.mrc/_boxes.png}


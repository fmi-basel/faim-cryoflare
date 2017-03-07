#!/bin/bash
. ./data_connector.sh

pixel_size=`calculate "1e10*$apix_x"`
gautomatch_log=$destination_path/${name}_gautomatch.log


if [ ! -e $destination_path/${name}_aligned_automatch.box  ]; then
  /usr/prog/sb/em/sw/gautomatch/0.53/bin/Gautomatch-v0.53_sm_20_cu7.0_x86_64 --gid $gpu_id  --apixM $pixel_size --diameter 300 $aligned_avg > $gautomatch_log 
  e2proc2d.py $aligned_avg ${aligned_avg/.mrc/_full.png}
  ./draw_boxes.sh  ${aligned_avg/.mrc/_full.png} $destination_path/${name}_aligned_automatch.box  ${aligned_avg/.mrc/_boxes.png}
  rm ${aligned_avg/.mrc/_full.png}
fi
RESULTS[num_particles]=`cat $destination_path/${name}_aligned_automatch.box|wc -l`
RESULTS[aligned_avg_boxes_png]=${aligned_avg/.mrc/_boxes.png}


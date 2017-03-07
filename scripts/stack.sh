#!/bin/bash
. ./data_connector.sh
stack_log=$destination_path/${name}_stack.log
raw_stack=$destination_path/${name}.mrcs
raw_average_thumbnail=$destination_path/${name}.png
echo e2proc2d.py  $stack_frames  $destination_path/${name}.mrcs > $stack_log
if [ ! -e $destination_path/${name}.mrcs ]; then
  e2proc2d.py  $stack_frames  $raw_stack >> $stack_log
  e2proc2d.py  --average $raw_stack $raw_average>> $stack_log
fi
if [ ! -e $raw_average_thumbnail ]; then
  e2proc2d.py  clip 512,512  $raw_average $raw_average_thumbnail>> $stack_log
fi
RESULTS["raw_stack"]=$raw_stack
RESULTS["raw_average"]=$raw_average
RESULTS["raw_average_thumbnail"]=$raw_average_thumbnail

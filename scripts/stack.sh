#!/bin/bash
. /usr/prog/sb/em/sw/stack_gui/scripts/data_connector.sh
stack_log=$destination_path/${name}_stack.log
raw_stack=$destination_path/${name}.mrcs
raw_average_thumbnail=$destination_path/${name}.png
raw_average=$destination_path/${name}.mrc
echo e2proc2d.py  $stack_frames  $destination_path/${name}.mrcs > $stack_log
if [ ! -e $destination_path/${name}.mrcs ]; then
  cp $xml_file $destination_path
  e2proc2d.py  $stack_frames  $raw_stack >> $stack_log
  e2proc2d.py  --average $raw_stack $raw_average>> $stack_log
fi
if [ ! -e $raw_average_thumbnail ]; then
  echo e2proc2d.py  --fouriershrink 7.49609375  $raw_average $raw_average_thumbnail>> $stack_log
  e2proc2d.py  --fouriershrink 7.49609375  $raw_average $raw_average_thumbnail>> $stack_log
fi
RESULTS["raw_stack"]=$raw_stack
RESULTS["raw_average"]=$raw_average
RESULTS["raw_average_thumbnail"]=$raw_average_thumbnail

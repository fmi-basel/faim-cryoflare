#!/bin/sh --noprofile
. $STACK_GUI_SCRIPTS/data_connector.sh

module purge
module load eman2

mkdir -p $destination_path/movies_raw $destination_path/micrographs_raw $destination_path/xml

stack_log=$destination_path/movies_raw/${short_name}_stack.log
raw_stack=$destination_path/movies_raw/${short_name}.mrcs
raw_average_thumbnail=$destination_path/micrographs_raw/${short_name}.png
raw_average=$destination_path/micrographs_raw/${short_name}.mrc

if [ ! -e $raw_stack  ]; then
  cp $xml_file $destination_path/xml/${short_name}.xml
  e2proc2d.py  $stack_frames  $raw_stack >> $stack_log
  e2proc2d.py  --average $raw_stack $raw_average>> $stack_log
fi

if [ ! -e $raw_average_thumbnail ]; then
  e2proc2d.py  --fouriershrink 7.49609375  $raw_average $raw_average_thumbnail>> $stack_log
fi

RESULT_FILE["raw_stack"]=$raw_stack
RESULT_FILE["stack_log"]=$stack_log
RESULT_FILE["raw_average"]=$raw_average
RESULT_FILE["raw_average_thumbnail"]=$raw_average_thumbnail

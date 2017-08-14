#!/bin/sh --noprofile
. $STACK_GUI_SCRIPTS/data_connector.sh

mkdir -p $destination_path/movies_raw $destination_path/micrographs_raw $destination_path/xml

pixel_size=`calculate "1e10*$apix_x"`
unpack_log=$destination_path/movies_raw/${short_name}_unpack.log
ice_ratio_log=$destination_path/movies_raw/${short_name}_ice_ratio.log
raw_stack=$destination_path/movies_raw/${short_name}.mrcs
raw_average_thumbnail=$destination_path/micrographs_raw/${short_name}.png
raw_average=$destination_path/micrographs_raw/${short_name}.mrc
xml=$destination_path/xml/${short_name}.xml
gain_ref=$stack_source_path/${name}*gain-ref*
source_stack=$stack_source_path/${name::-4}*mrc

if [ ! -e $raw_stack ] || [ ! -e $raw_average_thumbnail ]; then
  module purge
  module load eman2/2.2
  module load imod

  cp $xml_file $xml
  rm  $raw_stack $raw_average >& /dev/null
  if [ -w $gain_ref ]; then
    # apply gain normalization
    run clip unpack $source_stack $gain_ref  $raw_stack >> $unpack_log
  else
    cp $source_stack $raw_stack >> $unpack_log
  fi
  run e2proc2d.py  --average $raw_stack $raw_average>> $unpack_log
  run e2proc2d.py  --fouriershrink 7.49609375  $raw_average $raw_average_thumbnail>> $unpack_log
fi
if [ ! -e $ice_ratio_log ] ; then
  run $STACK_GUI_SCRIPTS/ice_ratio.py $raw_average $pixel_size 5.0 3.89 0.4> $ice_ratio_log
fi

ice_ratio=`tail -n 1 $ice_ratio_log`

if (( $(echo "$ice_ratio > 1.2" |bc -l) )) || (( $(echo "$ice_ratio < 1.0" |bc -l) )); then
  export="false"
else
  export="true"
fi
RESULTS raw_average_thumbnail raw_stack ice_ratio export
FILES raw_stack unpack_log raw_average raw_average_thumbnail xml ice_ratio_log


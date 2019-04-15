#!/bin/bash --noprofile  
set -e
set -u


######################## get parameters from GUI ###############################

. data_connector.sh


######################## load modules ##########################################

module purge
module load eman2/2.2
module load imod


######################## create destination folders ############################

mkdir -p $destination_path/movies_raw
mkdir -p $destination_path/micrographs_raw
mkdir -p $destination_path/xml



######################## define output files ###################################

unpack_raw_stack=$destination_path/movies_raw/${short_name}_movie.mrcs
unpack_xml=$destination_path/xml/${short_name}.xml
RAW_FILES unpack_raw_stack unpack_xml

unpack_raw_average=$destination_path/micrographs_raw/${short_name}.mrc
unpack_raw_average_jpg=$destination_path/micrographs_raw/${short_name}.jpg
unpack_raw_average_fft_png=$destination_path/micrographs_raw/${short_name}_fft.png
unpack_log=$destination_path/movies_raw/${short_name}_unpack.log
unpack_ice_ratio_log=$destination_path/movies_raw/${short_name}_ice_ratio.log
FILES unpack_log unpack_raw_average unpack_raw_average_jpg unpack_ice_ratio_log unpack_raw_average_fft_png


######################## define additional parameters ##########################

pixel_size=`CALCULATE "1e10*$apix_x"`
gain_ref=$stack_source_path/${name}*gain-ref*
source_stack=$stack_source_path/${name::${#name}-4}*mrc


######################## RUN processing if files are missing ###################

if FILES_MISSING; then
  rm  $unpack_raw_stack $unpack_raw_average >& /dev/null
  cp $xml_file $unpack_xml
  cp ${xml_file/.xml/.mrc} $unpack_raw_average
  cp ${xml_file/.xml/.jpg} $unpack_raw_average_jpg
  if [ -w $gain_ref ]; then
    # apply gain normalization
    RUN clip unpack $source_stack $gain_ref  $unpack_raw_stack >> $unpack_log
  else
    cp $source_stack $unpack_raw_stack >> $unpack_log
  fi
  RUN e2proc2d.py --process math.realtofft  --meanshrink 7  --process mask.sharp:inner_radius=1 $unpack_raw_average $unpack_raw_average_fft_png  >> $unpack_log   2>&1
  ice_ratio.py $unpack_raw_average $pixel_size 5.0 3.89 0.4> $unpack_ice_ratio_log
fi


######################## extract output parameters #############################

if [ -e $unpack_ice_ratio_log ]; then
  unpack_ice_ratio=`tail -n 1 $unpack_ice_ratio_log`
else
  unpack_ice_ratio=0
  echo "missing ice ratio log:  $unpack_ice_ratio_log"
fi


######################## decide if image should be excluded from export ########

LIMIT_EXPORT unpack_ice_ratio $unpack_input_ice_ratio_low $unpack_input_ice_ratio_high



######################## create Relion job #####################################

rln_jobtype=Import
rln_jobid=7
rln_alias=movies_raw
rln_nodetype=0
rln_starname=movies.star
rln_inputstar=""
rln_header=(MicrographMovieName)
RELION_WRITE "$destination_path" "$rln_jobtype" "$rln_jobid" "$rln_alias"  "$rln_nodetype" "$rln_starname" "$rln_inputstar" "rln_header[@]" "movies_raw/${short_name}_movie.mrcs"


######################## export result parameters ##############################

RESULTS unpack_ice_ratio

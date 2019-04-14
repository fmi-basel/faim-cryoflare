#!/bin/bash --noprofile -e 
######################## get parameters from GUI ###############################

. data_connector.sh


######################## load modules ##########################################

module purge
module load eman2/2.2


######################## define additional parameters ##########################
gain_ref=$stack_source_path/${name}*gain-ref*
source_stack=$stack_source_path/${name::${#name}-4}*mrc

copyraw_movie_dir=$destination_path/movies_raw/
copyraw_micrographs_dir=$destination_path/micrographs_raw/
copyraw_xml_dir=$destination_path/xml/


######################## create destination folders ############################

mkdir -p $copyraw_movie_dir
mkdir -p $copyraw_micrographs_dir
mkdir -p $copyraw_xml_dir

#workaround for lag in OffloadPC
sleep 10

######################## define output files ###################################

copyraw_raw_stack=$copyraw_movie_dir/${short_name}.mrcs
copyraw_xml=$copyraw_xml_dir/${short_name}.xml
RAW_FILES copyraw_raw_stack copyraw_xml
if  compgen -G "$gain_ref"  ; then
  copyraw_gain_ref=$copyraw_movie_dir/${short_name}_gainref.mrc
  RAW_FILES copyraw_gain_ref 
fi

copyraw_log=$copyraw_micrographs_dir/${short_name}_copyraw.log
copyraw_raw_average=$copyraw_micrographs_dir/${short_name}.mrc
copyraw_raw_average_jpg=$copyraw_micrographs_dir/${short_name}.jpg
copyraw_raw_average_fft_png=$copyraw_micrographs_dir/${short_name}_fft.png
FILES copyraw_raw_average copyraw_raw_average_jpg copyraw_raw_average_fft_png  copyraw_log


######################## run processing if files are missing ###################

if FILES_MISSING; then
  if  compgen -G "$gain_ref"  ; then
    echo cp $gain_ref $copyraw_gain_ref
    cp $gain_ref $copyraw_gain_ref
  fi
  cp $source_stack $copyraw_raw_stack
  cp $xml_file $copyraw_xml
  cp ${xml_file/.xml/.mrc} $copyraw_raw_average
  cp ${xml_file/.xml/.jpg} $copyraw_raw_average_jpg
  RUN e2proc2d.py --process math.realtofft  --meanshrink 7  --process mask.sharp:inner_radius=1 $copyraw_raw_average $copyraw_raw_average_fft_png  >> $copyraw_log   2>&1
fi



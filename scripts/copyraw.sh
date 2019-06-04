#!/bin/bash --noprofile  
set -u
set -e
######################## get parameters from GUI ###############################

. data_connector.sh


######################## load modules ##########################################

module purge
module load eman2/2.2


######################## define additional parameters ##########################
gain_ref=$stack_source_path/FoilHole_${hole_id}_Data_${template_id}_${acquisition_id}_????????_??????-gain-ref.MRC
echo ${gain_ref}
source_stack=$stack_source_path/FoilHole_${hole_id}_Data_${template_id}_${acquisition_id}_????????_??????-??????.mrc

copyraw_movie_dir=movies_raw/
copyraw_micrographs_dir=micrographs_raw/
copyraw_xml_dir=xml/

apix_x=$copyraw_input_pixel_size
RESULTS apix_x



######################## create destination folders ############################

mkdir -p $copyraw_movie_dir
mkdir -p $copyraw_micrographs_dir
mkdir -p $copyraw_xml_dir

#workaround for lag in OffloadPC
sleep 5 

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
    echo install -m 644 $gain_ref $copyraw_gain_ref
    install -m 644 $gain_ref $copyraw_gain_ref
  fi
  install -m 644 $source_stack $copyraw_raw_stack
  install -m 644 $xml_file $copyraw_xml
  install -m 644 ${xml_file/.xml/.mrc} $copyraw_raw_average
  install -m 644 ${xml_file/.xml/.jpg} $copyraw_raw_average_jpg
  RUN e2proc2d.py --process math.realtofft  --meanshrink 7  --process mask.sharp:inner_radius=1 $copyraw_raw_average $copyraw_raw_average_fft_png  >> $copyraw_log   2>&1
fi



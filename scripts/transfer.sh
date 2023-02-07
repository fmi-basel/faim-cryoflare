#!/bin/bash --noprofile  
################################################################################
#
# Author: Andreas Schenk
# Friedrich Miescher Institute, Basel, Switzerland
#
# This file is part of CryoFLARE
#
# Copyright (C) 2017-2020 by the CryoFLARE Authors
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 3.0 of the License.
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License
# along with CryoFLARE.  If not, see http://www.gnu.org/licenses/.
#
################################################################################
set -u
#set -x
set -e
######################## get parameters from GUI ###############################
#CRYOFLARE_DEBUG=1
. data_connector.sh

######################## load modules ##########################################
module purge
module load EMAN2/2.20
######################## define additional parameters ##########################
movie_dir=Movies
raw_micrographs_dir=MicrographsRaw
xml_dir=xml/

######################## create destination folders ############################
mkdir -p $movie_dir
mkdir -p $raw_micrographs_dir
mkdir -p $xml_dir

######################## define output files ###################################
raw_movie=$movie_dir/${short_name}.${stack_suffix}
xml=$xml_dir/${short_name}.xml
RAW_FILES raw_movie xml
if  [ -n "${source_gain_ref+x}" ] || [ $stack_suffix == "eer" ] ; then
  gain_reference=$movie_dir/${short_name}_gainref.mrc
  RAW_FILES gain_reference 
fi
# gain reference data for eer. Todo get directly from microscope
eer_gain="/<path_to>/gain_post_ec_eer.raw"
#binary defects file is generated manually from defects xml file with convert_defects.py
eer_defects_npy="/<path_to>/SensorDefects.npy"

gain_dir=GainReferences
transfer_log=$raw_micrographs_dir/${short_name}_transfer.log
raw_micrograph=$raw_micrographs_dir/${short_name}.mrc
raw_micrograph_jpg=$raw_micrographs_dir/${short_name}.jpg
raw_micrograph_fft_png=$raw_micrographs_dir/${short_name}_fft.png
RAW_FILES raw_micrograph raw_micrograph_jpg
FILES raw_micrograph_fft_png  transfer_log
######################## run processing if files are missing ###################
if FILES_MISSING; then
  mkdir -p $gain_dir
  if  [ -n "${source_gain_ref+x}" ] ; then
    #expand glob pattern and take last file
    source_gain_ref= compgen -G "$source_gain_ref"|tail -n 1 
    wait_complete 60 $source_gain_ref
    # separate gain reference for each stack. 
    # use link for identical files. 
    hash=$( md5sum $source_gain_ref|cut -f 1 -d' ' )
    hash_mrc=$gain_dir/$hash.mrc
    ln -sf ../$hash_mrc $gain_reference
    if [ ! -f $hash_mrc ] ; then
      install -m 644 $source_gain_ref $hash_mrc 2> /dev/null|| true
      SHARED_FILES hash_mrc
    fi
  elif [ $stack_suffix == "eer" ];then
    # todo handle case where gain reference is updated mid acquisition
    # gain reference should always be older than acquired data
    hash=$( md5sum "$eer_gain"|cut -f 1 -d' ' )
    hash_mrc=$gain_dir/$hash.mrc
    ln -fs ../$hash_mrc $gain_reference
    if [ ! -f $hash_mrc ] ; then
      convert_eer_gain.py "$eer_gain" "$eer_defects_npy" $scratch/$hash.mrc
      mv $scratch/$hash.mrc $hash_mrc
      SHARED_FILES hash_mrc
    fi
  fi
  if [ ! -f $raw_movie ] ; then
    if [ -f ${raw_movie}.bz2 ] ; then
      bunzip2 ${raw_movie}.bz2
    else
      #expand glob pattern and take last file
      source_stack= compgen -G "$source_stack"|tail -n 1 
      wait_complete 60 $source_stack
      install -m 644 $source_stack $raw_movie
    fi
  fi
  if [ ! -f $xml ] ; then
    install -m 644 $xml_file $xml
  fi
  if [ ! -f $raw_micrograph ] ; then
    install -m 644 ${xml_file/.xml/.mrc} $raw_micrograph
  fi
  if [ ! -f $raw_micrograph_jpg ] ; then
    install -m 644 ${xml_file/.xml/.jpg} $raw_micrograph_jpg
  fi
  if [ "$transfer_input_move_data" = true ]; then
    rm -f ${xml_file/.xml/.mrc}
    rm -f ${xml_file/.xml/.jpg}
    rm -f $source_stack
    if  [ -n "${source_gain_ref+x}" ] ; then
      rm -f $source_gain_ref
    fi
  fi
  RUN e2proc2d.py --process math.realtofft  --meanshrink 7  --process mask.sharp:inner_radius=1 $raw_micrograph $raw_micrograph_fft_png  >> $transfer_log   2>&1
fi
if [ -z "$num_frames" ] ; then
	num_frames=$( e2iminfo.py $raw_movie|head -n 1|cut -f 2 -d" " )
	RESULTS num_frames
fi
######################## create Relion job #####################################
rln_jobtype=Import
rln_jobid=1
rln_alias=Movies
rln_nodetype=0
rln_starname=movies.star
rln_inputstar=""
rln_header=(MovieName)
RELION_CREATE_JOB "$rln_jobtype" "$rln_jobid" "$rln_alias" "$rln_starname" "$rln_nodetype" "$rln_inputstar"
RELION_WRITE_STAR "$rln_alias" "$rln_starname"  "rln_header[@]" "${raw_movie}"
rln_jobtype=Import
rln_jobid=2
rln_alias=RawMicrographs
rln_nodetype=1
rln_starname=micrographs.star
rln_inputstar=""
rln_header=(MicrographName)
RELION_CREATE_JOB "$rln_jobtype" "$rln_jobid" "$rln_alias" "$rln_starname" "$rln_nodetype" "$rln_inputstar"
RELION_WRITE_STAR "$rln_alias" "$rln_starname" "rln_header[@]" "${raw_micrograph}"

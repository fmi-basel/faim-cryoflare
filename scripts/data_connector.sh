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
set -e


######################## floating point calculator #############################

CALCULATE() {
python -c "from __future__ import division;from math import *;print $1"
}

ROUND() {
python -c "print str(round(float(\"$1\"),$2))"
}

######################## helper functions ######################################
wait_complete() {
  local filename+=$2
  local poll_time=0.2
  local timeout=$1
  end=$((SECONDS+$timeout))
  prev_size=0
  while [ $SECONDS -lt $end ]; do
    size=$(stat -c %s $filename 2>/dev/null|| echo 0)
    if [ "$size" -gt 0 ] && [ "$size" -eq "$prev_size" ]; then
      #echo wait complete : $(($SECONDS - $end + $timeout))
      return 0
    fi
    prev_size=$size
    sleep $poll_time
  done
   >&2 echo "ERROR: timemout of $timeout s when waiting for file $filename"
  exit 1
}
######################## Helper function to create files needed for Relion #####
RELION_CREATE_JOB() {
	local jobtype=$1
	local jobid=$2
	local jobalias=$3
	local starname=$4
	local nodetype=$5
	local inputstar=$6
	local jobfolder
	printf -v jobfolder "job%03d" $jobid
	rln_jobpath=$jobtype/$jobfolder
	local jobalias_link=$jobtype/$jobalias
	local default_pipeline="default_pipeline.star"
	local gui_projectdir=".gui_projectdir"
	### create folders and aliases ###
	mkdir -p $rln_jobpath
	(
    	flock  9 || exit 1
		[ -e $jobalias_link ] || ln -s ../$rln_jobpath $jobalias_link
  	) 9<$rln_jobpath

  	#[ -e $jobalias_link ] || ln -s ../$rln_jobpath $jobalias_link
  	SHARED_FILES jobalias_link
	### create node files ###
	mkdir -p .Nodes/$nodetype/$jobalias_link
	mkdir -p .Nodes/$nodetype/$rln_jobpath
	touch .Nodes/$nodetype/$jobalias_link/$starname
	touch .Nodes/$nodetype/$rln_jobpath/$starname
    add_to_pipeline.py "$default_pipeline"  "$jobtype" "$jobid" "$jobalias" "$inputstar" "$rln_jobpath/$starname:$nodetype"
    SHARED_FILES default_pipeline gui_projectdir
}

RELION_WRITE_STAR(){
	### write to star file ###
	local jobalias=$1
	local starname=$2
	declare -a header=("${!3}")
	local datastart=4
	starpath_varname=${jobalias}_${starname/./_}
	declare  "$starpath_varname=$rln_jobpath/$starname"
	(
		flock  9 || exit 1
     	local tmpfile=`mktemp --tmpdir=$scratch`
		if [ -n "${CRYOFLARE_WRITE_RELION_31+x}" ]; then
			data="1 ${@:$datastart}"
		else
			data="${@:$datastart}"
		fi
     	if [[ -s ${!starpath_varname} ]]; then
      		cp ${!starpath_varname} $tmpfile
     	else
			if [ -n "${CRYOFLARE_WRITE_RELION_31+x}" ]; then
				declare -a optics_group=( $optics_group_values )
				# Relion 3.1 add optics group	
				echo -e "data_optics\n\nloop_ \n_rlnOpticsGroupName #1 \n_rlnOpticsGroup #2 \n_rlnMtfFileName #3 \n_rlnMicrographOriginalPixelSize #4 \n_rlnVoltage #5 \n_rlnSphericalAberration #6 \n_rlnAmplitudeContrast #7 \n_rlnMicrographPixelSize #8 " > $tmpfile
				echo -en "opticsGroup1 1 " >> $tmpfile
				for opticsitem in ${optics_group[@]}; do
					echo -en "$opticsitem " >> $tmpfile
				done
				echo -e "\n" >> $tmpfile
				echo -e "\ndata_micrographs\n\nloop_\n_rlnOpticsGroup #1" >> $tmpfile
				i=2
			else
				# Relion 3.0 no optics group	
				echo -e "\ndata_\n\nloop_" >> $tmpfile
				i=1
			fi
			for headeritem in ${header[@]}; do
				echo -e "_rln$headeritem #  $i" >> $tmpfile
				let i++
			done
     	fi
		local imagename=${!datastart##*/}
     	local awk_string=' ! /'$imagename'/ {print} 
                	      END {print "'$data'"}'
     	[ $# -ge $datastart ] && awk "$awk_string" < $tmpfile > ${!starpath_varname}
     	SHARED_FILES ${starpath_varname} 
	) 9>> ${!starpath_varname}
}



######################## functions for exporting results and files #############

RESULTS (){
  for var in "$@" 
  do
    if [ -v $var ]; then 
      if [ -n "${!var}" ]; then 
        echo RESULT_EXPORT:$var=${!var}
      else 
        >&2 echo ERROR: result "$var" not defined
        exit 1
      fi
    else
      >&2 echo ERROR: variable "$var" not defined
      exit 1
    fi
  done
}


_export_files(){
  for var in "${@:2}"
  do
    if [ -v $var ]; then 
      if [ -n "${!var}" ]; then 
        echo $1:$var=${!var}
      else 
        echo ERROR: file "$var" not defined
        exit 1
      fi
    else
      >&2 echo ERROR: variable "$var" not defined
      exit 1
    fi
    _exported_files_+=(${!var})
  done
}


FILES() {
  RESULTS "$@"
  _export_files FILE_EXPORT "$@"
}

RAW_FILES() {
  RESULTS "$@"
  _export_files RAW_FILE_EXPORT "$@"
}

SHARED_FILES() {
  _export_files SHARED_FILE_EXPORT "$@"
}


FILES_MISSING() {
  for var in "${_exported_files_[@]}"
  do
    [ ! -e $var ] && return
  done
}




######################## functions for cleanup of scratch and running jobs #####

_cleanup(){
  END=`date +%s`
  echo execution time \(s\) : $(( $END-$START ))
  if [ -z "${CRYOFLARE_DEBUG+x}" ]; then
    rm -fr $scratch
  fi
}

_terminate(){
  kill -9 $PID 
  wait $PID
  exit
}

RUN(){
  echo running: "$@"
  "$@" &
  PID=$!
  wait $PID
} 



######################## function for export limitation ########################

LIMIT_EXPORT (){
  if python -c "if ${!1}>=$2 and ${!1}<=$3: exit(1) "; then
    export="false"
    RESULTS export
  fi
}


######################## scratch dir creation on RAM disk ######################
script_name=${0##*/}
scratch=$(mktemp -d /dev/shm/${script_name%%.sh}.XXXXXX)


######################## hooks for termination and cleanup functions ###########

trap _cleanup EXIT
trap _terminate INT TERM HUP

######################## definition of input parameters ########################
START=`date +%s`
while IFS='=' read k v; do
  declare $k="$v"
  if [ -n "${CRYOFLARE_DEBUG+x}" ]; then
    echo $k="$v"
  fi
done



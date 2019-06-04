#!/bin/bash --noprofile  
set -u
set -e


######################## floating point calculator #############################

CALCULATE() {
python -c "from math import *;print $1"
}


######################## Helper function to create files needed for Relion #####

RELION_WRITE(){
	local destination=$1
	local jobtype=$2
	local jobid=$3
	local jobalias=$4
	local nodetype=$5
	local starname=$6
	local inputstar=$7
        declare -a header=("${!8}")
	local datastart=9
	local jobfolder
	printf -v jobfolder "job%03d" $jobid
	local jobpath=$destination/$jobtype/$jobfolder
	starpath_varname=${jobalias}_${starname/./_}
	declare  "$starpath_varname=$jobpath/$starname"
	local default_pipeline="$destination/default_pipeline.star"
	local gui_projectdir=".gui_projectdir"
	local jobalias_link=$destination/$jobtype/$jobalias
	### create folders and aliases ###
	mkdir -p $destination/$jobtype/$jobfolder
        [ -e $jobalias_link ] || ln -s ../$jobtype/$jobfolder $jobalias_link
	
        SHARED_FILES jobalias_link
	### create node files ###
	mkdir -p $destination/.Nodes/$nodetype/$jobtype/$jobalias
	mkdir -p $destination/.Nodes/$nodetype/$jobtype/$jobfolder
	touch $destination/.Nodes/$nodetype/$jobtype/$jobalias/$starname
	touch $destination/.Nodes/$nodetype/$jobtype/$jobfolder/$starname
	
	### write to star file ###
	(
		flock -n 9 || exit 1
     		local tmpfile=`mktemp --tmpdir=$scratch`
     		if [[ -s ${!starpath_varname} ]]; then
       			cp ${!starpath_varname} $tmpfile
     		else
       			echo -e "\ndata_\n\nloop_" > $tmpfile
                        i=1
			for headeritem in ${header[@]}; do
				echo -e "_rln$headeritem #  $i" >> $tmpfile
                                let i++
			done
     		fi
		local imagename=${!datastart##*/}
     		local awk_string=' ! /'$imagename'/ {print} 
                  	     END {print "'${@:$datastart}'"}'
     		[ $# -ge $datastart ] && awk "$awk_string" < $tmpfile > ${!starpath_varname}
     		SHARED_FILES ${starpath_varname} 
	) 9>> ${!starpath_varname}
	
	### update pipeline file ### 
  	(
    		flock -n 9 || exit 1
    		touch $destination/.gui_projectdir
    		add_to_pipeline.py "$default_pipeline"  "$jobtype" "$jobid" "$jobalias" "$inputstar" "$jobtype/$jobfolder/$starname:$nodetype"
    		SHARED_FILES default_pipeline gui_projectdir
  	) 9>> $default_pipeline
}


######################## functions for exporting results and files #############

RESULTS (){
  for var in "$@" 
  do
    if [ -n "${!var}" ]; then 
      echo RESULT_EXPORT:$var=${!var}
    else 
      echo WARNING: result "$var" not defined
    fi
  done
}


_export_files(){
  for var in "${@:2}"
  do
    if [ -n "${!var}" ]; then 
      echo $1:$var=${!var}
    else 
      echo WARNING: file "$var" not defined
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
  rm -fr $scratch
}

_terminate(){
  kill -9 $PID 
  wait $PID
  exit
}

RUN(){
  echo $@
  $@ &
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

while IFS='=' read k v; do declare $k="$v"; done



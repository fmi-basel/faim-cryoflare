#!/bin/sh --noprofile

cleanup(){
  rm -fr $scratch
}

cleanup_and_terminate(){
  cleanup
  kill 0
}

calculate() {
echo $1 | sed 's/[Ee]+\?/*10^/g' | bc -l
}

relion_alias(){
  local local_job_id
  printf -v local_job_id "%03d" $2
  [ -e $destination_path/$1/$3 ] || ln -s ../$1/job00$local_job_id $destination_path/$1/$3
  SHARED_FILES $destination_path/$1/$3
}

write_to_star() {
  (
     flock -n 9 || exit 1
     tmpfile=`mktemp --tmpdir=$scratch`
     if [[ -s $1 ]]; then
       cp $1 $tmpfile
     else
       echo -e "$2" > $tmpfile
     fi
     awk_string=' ! /'${3##*/}'/ {print} 
                  END {print "'${@:3}'"}'
     [ $# -gt 2 ] && awk "$awk_string" < $tmpfile > $1
     SHARED_FILES  $1
  ) 9>> $1
}

add_to_pipeline() {
  (
    flock -n 9 || exit 1
    touch $destination_path/.gui_projectdir
    $STACK_GUI_SCRIPTS/add_to_pipeline.py "$@"
    SHARED_FILES "$destination_path/default_pipeline.star" "$destination_path/.gui_projectdir"
  ) 9>> $1
}

RESULTS (){
  for var in "$@"
  do
    echo RESULT_EXPORT:$var=${!var}
  done
}

FILES() {
  for var in "$@"
  do
    if [ -z ${!var+x} ];then
      echo FILE_EXPORT:$var
    else
    echo FILE_EXPORT:${!var}
    fi
  done
}

SHARED_FILES() {
  for var in "$@"
  do
    if [ -z ${!var+x} ];then
      echo SHARED_FILE_EXPORT:$var
    else
    echo SHARED_FILE_EXPORT:${!var}
    fi
  done
}

script_name=${0##*/}
#scratch=$(mktemp -d /scratch/$USER/${script_name%%.sh}.XXXXXX)
scratch=$(mktemp -d /data/Gatan_X/scratch/${script_name%%.sh}.XXXXXX)


trap cleanup EXIT
trap cleanup_and_terminate INT TERM HUP

while IFS='=' read k v; do declare $k="$v"; done


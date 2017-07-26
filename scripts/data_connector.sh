#!/bin/sh --noprofile

cleanup(){
  rm -fr $scratch
}

export_results() {
  for i in ${!RESULT[@]}; do echo RESULT_EXPORT:$i=${RESULT[$i]}; done
  for i in ${!RESULT_FILE[@]}; do echo RESULT_FILE_EXPORT:$i=${RESULT_FILE[$i]}; done
  cleanup 
}

calculate() {
echo $1 | sed 's/[Ee]+\?/*10^/g' | bc -l
}

#write_to_star() {
#  (
#     flock -n 9 || exit 1
#     [[  -s $1 ]] || echo -e "$2" >> $1
#     [ $# -gt 2 ] && echo  "${@:3}" >> $1
#  ) 9>> $1
#}
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
  ) 9>> $1
}

add_to_pipeline() {
  (
    flock -n 9 || exit 1
    touch $destination_path/.gui_projectdir
    $STACK_GUI_SCRIPTS/add_to_pipeline.py "$@"
  ) 9>> $1
}

script_name=${0##*/}
#scratch=$(mktemp -d /scratch/$USER/${script_name%%.sh}.XXXXXX)
scratch=$(mktemp -d /data/Gatan_X/scratch/${script_name%%.sh}.XXXXXX)


trap cleanup INT TERM HUP
trap export_results EXIT

declare -A RESULT
declare -A RESULT_FILE
while IFS='=' read k v; do declare $k="$v"; done


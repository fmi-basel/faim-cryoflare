#!/bin/bash

declare -A DATA
declare -A RESULTS

function export_results {
  for i in ${!RESULTS[@]}; do echo RESULT_EXPORT:$i=${RESULTS[$i]}; done
}

trap export_results EXIT

function calculate {
echo $1 | sed 's/[Ee]/*10^/g' | bc -l
}
#while IFS='=' read k v; do DATA[$k]=$v; done
while IFS='=' read k v; do declare $k="$v"; done
export DATA
export RESULTS

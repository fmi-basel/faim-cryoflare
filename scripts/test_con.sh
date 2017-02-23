#!/bin/bash
. ./data_connector.sh
#a=`calculate "1e10*${DATA[pixel_size]}"`
#echo $a
#for i in ${!DATA[@]}; do echo  $i=${DATA[$i]} >> ${DATA[name]}.log; done
#RESULTS["raw_stack"]="xxxyyyzzz"
#echo `calculate $pixel_size*1e10`
calculate "$defocus_v*1e-10"

#!/bin/bash --noprofile
################################################################################
#
# Author: Andreas Schenk
# Friedrich Miescher Institute, Basel, Switzerland
#
# This file is part of CryoFlare
#
# Copyright (C) 2017-2018 by the CryoFlare Authors
#
################################################################################
######################## get parameters from GUI ###############################

. data_connector.sh



hole_position_coordinates=$destination_path/micrographs_raw/${short_name}_hole_coordinates.txt
hole_position_image=$destination_path/micrographs_raw/${short_name}_hole_coordinates.png

FILES hole_position_coordinates hole_position_image

metadata=${avg_source_path}/../../../Metadata/${grid_name}/TargetLocation_${hole_id}.dm
hole_position_x=`xmllint --format $metadata|grep a:x |sed -e 's/<[^>]*>//g'| tr -d '[:space:]'`
hole_position_y=`xmllint --format $metadata|grep a:y |sed -e 's/<[^>]*>//g'| tr -d '[:space:]'`
echo $hole_position_x,$hole_position_y > $hole_position_coordinates
scalefactor=0.13340281396560708 # for K2 for now, todo determine dynamically
x_scaled=`CALCULATE round\($hole_position_x*$scalefactor\)`
y_scaled=`CALCULATE round\($hole_position_y*$scalefactor\)`
x_start=`CALCULATE $x_scaled-2`
x_end=`CALCULATE $x_scaled+2`
y_start=`CALCULATE $y_scaled-2`
y_end=`CALCULATE $y_scaled+2`

if FILES_MISSING; then
    grid_square_jpg=`ls -tr ${avg_source_path}/../GridSquare_*.jpg|tail -n 1`
    convert -limit thread 1  -draw "fill rgba( 100, 255, 100 , 0.4 ) stroke lightgreen circle $x_start,$y_start $x_end,$y_end" $grid_square_jpg $hole_position_image
fi
RESULTS hole_position_x hole_position_y

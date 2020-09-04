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
######################## get parameters from GUI ###############################

. data_connector.sh



hole_position_image=${raw_micrograph/.mrc/_hole_coordinates.png}
xmlfile=$xml
bs_file=${xmlfile/.xml/_bs.dat}
beamshift_plot=$bs_file.png

FILES  hole_position_image beamshift_plot

x_scaled=`CALCULATE round\($hole_x*$grid_square_scalefactor\)`
y_scaled=`CALCULATE round\($hole_y*$grid_square_scalefactor\)`
x_start=`CALCULATE $x_scaled-2`
x_end=`CALCULATE $x_scaled+2`
y_start=`CALCULATE $y_scaled-2`
y_end=`CALCULATE $y_scaled+2`

if FILES_MISSING; then
  grid_square_jpg=`ls -tr ${avg_source_path}/../GridSquare_*.jpg|tail -n 1`
  convert -limit thread 1  -draw "fill rgba( 100, 255, 100 , 0.4 ) stroke lightgreen circle $x_start,$y_start $x_end,$y_end" $grid_square_jpg $hole_position_image
  bs_x=`xmllint --format $xmlfile  |grep -A2 "<BeamShift" |grep a:_x |sed -e 's/<[^>]*>//g'| tr -d '[:space:]'`
  bs_y=`xmllint --format $xmlfile  |grep -A2 "<BeamShift" |grep a:_y |sed -e 's/<[^>]*>//g'| tr -d '[:space:]'`
  echo 0 0 $bs_x $bs_y > $bs_file
  gnuplot << EOT
set term png
set output "$beamshift_plot"
set xrange [-0.3:0.3]
set yrange [-0.3:0.3]
set size square
plot "$bs_file" using 1:2:3:4  with vectors filled head lw 3
EOT
  rm $bs_file
fi


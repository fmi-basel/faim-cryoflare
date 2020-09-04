#!/bin/bash --noprofile  
################################################################################
#
# Author: Andreas Schenk
# Friedrich Miescher Institute, Basel, Switzerland
#
# This file is part of CryoFLARE
#
# Copyright (C) 2020 by the CryoFLARE Authors
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

image=$1
boxfile=$2
boxfile_rejected=$3
out_image=$4
brightness_contrast=$5

boxes=$(while read line; do
  x=$(echo $line|sed "s/\t/ /g"|cut -d" " -f1)
  y=$(echo $line|sed "s/\t/ /g"|cut -d" " -f2)
  dx=$(echo $line|sed "s/\t/ /g"|cut -d" " -f3)
  dy=$(echo $line|sed "s/\t/ /g"|cut -d" " -f4|sed "s/\r//g")
  x2=`echo "$x+$dx"|bc -l`
  y2=`echo "$y+$dy"|bc -l`
  echo rectangle $x,$y $x2,$y2
done <$boxfile)
rejected_boxes=$(while read line; do
  x=$(echo $line|sed "s/\t/ /g"|cut -d" " -f1)
  y=$(echo $line|sed "s/\t/ /g"|cut -d" " -f2)
  dx=$(echo $line|sed "s/\t/ /g"|cut -d" " -f3)
  dy=$(echo $line|sed "s/\t/ /g"|cut -d" " -f4|sed "s/\r//g")
  x2=`echo "$x+$dx"|bc -l`
  y2=`echo "$y+$dy"|bc -l`
  echo rectangle $x,$y $x2,$y2
done <$boxfile_rejected)
convert -limit thread 4 -flip  -brightness-contrast $brightness_contrast -strokewidth 10 -draw "fill none stroke lightgreen $boxes"  -draw "fill none stroke red $rejected_boxes" -resize 12.5%  -flip $image $out_image

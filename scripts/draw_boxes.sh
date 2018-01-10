#!/bin/sh --noprofile

image=$1
boxfile=$2
boxfile_rejected=$3
out_image=$4

boxes=$(while read line; do
  x=$(echo $line|cut -d" " -f1)
  y=$(echo $line|cut -d" " -f2)
  dx=$(echo $line|cut -d" " -f3)
  dy=$(echo $line|cut -d" " -f4)
  x2=`echo "$x+$dx"|bc -l`
  y2=`echo "$y+$dy"|bc -l`
  echo rectangle $x,$y $x2,$y2
done <$boxfile)
rejected_boxes=$(while read line; do
  x=$(echo $line|cut -d" " -f1)
  y=$(echo $line|cut -d" " -f2)
  dx=$(echo $line|cut -d" " -f3)
  dy=$(echo $line|cut -d" " -f4)
  x2=`echo "$x+$dx"|bc -l`
  y2=`echo "$y+$dy"|bc -l`
  echo rectangle $x,$y $x2,$y2
done <$boxfile_rejected)
convert -limit thread 4 -flip  -brightness-contrast 0x100 -strokewidth 10 -draw "fill none stroke lightgreen $boxes"  -draw "fill none stroke red $rejected_boxes" -resize 12.5%  -flip $image $out_image

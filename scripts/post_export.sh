#!/bin/sh --noprofile

source_dir="$1"
destination_dir="$2"
for star_file in `find $destination_dir -name "micrographs*.star"`
do
	filter_star.py star_file  ${@:3}




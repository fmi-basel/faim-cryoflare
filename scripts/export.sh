#!/bin/sh --noprofile

source_dir="$1"
destination_dir="$2"
cd $source_dir
rsync -auR ${@:3} $2


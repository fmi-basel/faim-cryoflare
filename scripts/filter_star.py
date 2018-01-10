#!/usr/bin/env python
import sys
import re

pattern=re.compile("\d{8}_\d{6}")
star_file=sys.argv[1]
imagelist=sys.argv[2:]

lines_filtered=[]
with open(star_file,"r") as f:
	lines=f.readlines()
	for line in lines:
		match=pattern.search(line)
		if match:
			image=match.group(0)
			if image in imagelist:
				lines_filtered.append(line)
		else:
			lines_filtered.append(line)
with open(star_file,"w") as f:
	for line in lines_filtered:
		f.write(line)

#!/usr/bin/env python
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

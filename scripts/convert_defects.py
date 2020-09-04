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
#
# parsing of the defects file was adapted from an example script kindly provided
# by Thermo Fisher Scientific
#
################################################################################
import sys
import xml.etree.ElementTree as et
import numpy as np

xml_name=sys.argv[1]
npy_name=sys.argv[2]


points=[]
columns=[]
rows=[]
tree = et.parse(xml_name)
root = tree.getroot()
for elem in root.iter():
    if elem.tag=='point':
        points.append(map(int,elem.text.split(',')))
    elif elem.tag=='col':
        columns.append(map(int,elem.text.split('-')))
    elif elem.tag=='row':
        rows.append(map(int,elem.text.split('-')))

defects=np.ones((4096,4096))
for col in columns:
    for j in range(col[0]-1,col[1]+2):
        defects[:,j]=0
for row in rows:
    for i in range(row[0]-1,row[1]+2):
        defects[i,:]=0
for point in points:
    defects[point[1],point[0]]=0
defects[0,:]=0
defects[:,0]=0
defects[4095,:]=0
defects[:,4095]=0
np.save(npy_name,defects)

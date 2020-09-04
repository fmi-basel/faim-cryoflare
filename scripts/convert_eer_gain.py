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
# application of sensor defects on the gain reference data  was adapted
# from an example script kindly provided by Thermo Fisher Scientific
#
################################################################################
import sys
import numpy as np
from EMAN2 import EMNumPy
eer_gain_name=sys.argv[1]
defects_name=sys.argv[2]
mrc_gain_name=sys.argv[3]
with open(eer_gain_name,'rb') as f:            
        buffer=f.read()
lin_data=np.frombuffer(buffer,dtype=np.dtype(np.int32),count=4096**2,offset=49)
eer_gain=lin_data.reshape((4096,4096)).astype('float64')
defects=np.load(defects_name)
combined_gain=eer_gain*defects
valid_indices = np.where(combined_gain>0)
mean=combined_gain[valid_indices].mean()
std=combined_gain[valid_indices].std()
num_sigma=16
hot_indices=np.where(combined_gain>mean+std*num_sigma)
combined_gain[hot_indices]=0
e = EMNumPy.numpy2em(combined_gain.astype('float32'))
e.write_image(mrc_gain_name)	

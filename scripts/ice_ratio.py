#!/usr/bin/env python
from EMAN2 import EMData,Region
import numpy as np
import sys

image_name=sys.argv[1]
pixel_size=float(sys.argv[2])
resolution_ref=float(sys.argv[3])
resolution_ice=float(sys.argv[4])
resolution_band=float(sys.argv[5])

e=EMData(image_name)
min_size=min(e["nx"],e["ny"])
e_clip=e.get_clip(Region(0,0,min_size,min_size))
e_clip.do_fft_inplace()
e_clip.ri2inten()
rot_avg=e_clip.rotavg()
rpix=1/(pixel_size*min_size)
pos_ice_l=1.0/(resolution_ice+resolution_band/2.0)/rpix
pos_ice_h=1.0/(resolution_ice-resolution_band/2.0)/rpix
pos_ref_l=1.0/(resolution_ref+resolution_band/2.0)/rpix
pos_ref_h=1.0/(resolution_ref-resolution_band/2.0)/rpix


rot_avg_ice=rot_avg.get_clip(Region(int(pos_ice_l),int(pos_ice_h-pos_ice_l)))
rot_avg_ref=rot_avg.get_clip(Region(int(pos_ref_l),int(pos_ref_h-pos_ref_l)))
avg_ice=np.mean(rot_avg_ice.get_data_as_vector())
avg_ref=np.mean(rot_avg_ref.get_data_as_vector())
print avg_ice/avg_ref

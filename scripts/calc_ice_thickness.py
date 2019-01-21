#!/usr/prog/sb/em/sw/EMAN2/2.2/bin/python
from EMAN2 import EMData
import sys
from math import log

ref_name=sys.argv[1]
im_name=sys.argv[2]
im=EMData(im_name)
ref=EMData(ref_name)

if im_name.endswith(".mrcs"):
  sum_im=im["nx"]*im["ny"]*im["MRC.mz"]*im["mean"]
else:
  sum_im=im["nx"]*im["ny"]*im["nz"]*im["mean"]

if ref_name.endswith(".mrcs"):
  sum_ref=ref["nx"]*ref["ny"]*ref["MRC.mz"]*ref["mean"]
else:
  sum_ref=ref["nx"]*ref["ny"]*ref["nz"]*ref["mean"]

mfp=322.0 # Krios 300kV, 20eV slit, 100 um Obj aperture

print mfp*log(sum_ref/sum_im)


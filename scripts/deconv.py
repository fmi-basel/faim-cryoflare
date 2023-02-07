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
import iplt
import iplt.alg
import ost.io
import ost.img.alg
from math import *
from ost import Units
from ost.geom import *

falloff=1.0
strength=1.0
highpassnyquist=200

in_name=sys.argv[1]
out_name=sys.argv[2]
apix=float(sys.argv[3])*Units.A
defocus_u=float(sys.argv[4])
defocus_v=float(sys.argv[5])
defocus_angle=float(sys.argv[6])

mic_data=iplt.MicroscopeData()
mic_data.SetAccelerationVoltage(300*Units.kV)
mic_data.SetSphericalAberration(2.7*Units.mm)
mic_data.SetChromaticAberration(2.7*Units.mm)

defocus=iplt.Defocus(defocus_u*Units.A,defocus_v*Units.A,defocus_angle*Units.deg)


tcif_data=iplt.TCIFData(mic_data,defocus)
im=ost.io.LoadImage(in_name)
apix=im.SetSpatialSampling(Vec3(apix,apix,apix)) # assuming equal sampling in x,y, and z
im.ApplyIP(ost.img.alg.FFT())
ctf=im.Copy(False)
isnr=im.Copy(False)
ctf.ApplyIP(ost.img.alg.Fill(1))
ctf.ApplyIP(iplt.alg.CTF(tcif_data))
rpix=im.GetPixelSampling()[0]

for p in im:
    r=rpix*Length(p.ToVec3())
    highpass =min(1.0,r*2.0/highpassnyquist)*pi
    highpass =1-(cos(highpass)*0.5+0.5)
    snrval=exp(-r*2*falloff*100)*10.0**(3*strength)*highpass
    isnr.SetComplex(p,1.0/max(1e-15,snrval))

out_im=im*ctf/(ctf*ctf+isnr)
out_im.ApplyIP(ost.img.alg.FFT())
out_im.ApplyIP(ost.img.alg.Mirror(Axis.Y))
ost.io.SaveImage(out_im,out_name)

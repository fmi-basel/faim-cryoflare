#!/bin/sh --noprofile
. $STACK_GUI_SCRIPTS/data_connector.sh

mkdir -p $destination_path/micrographs_mc2_dw 

motioncorr2_log=$destination_path/micrographs_mc2_dw/${short_name}_motioncor2.log
aligned_avg_mc2_dw=$destination_path/micrographs_mc2_dw/${short_name}.mrc
aligned_avg_mc2_dw_png=$destination_path/micrographs_mc2_dw/${short_name}.png
aligned_avg_mc2_dw_fft_thumbnail=$destination_path/micrographs_mc2_dw/${short_name}_fft.png
mc2_shift_plot=$destination_path/mmicrographs_mc2_dw/${short_name}_shift.png

pixel_size=`calculate "1e10*$apix_x"`
dose_per_frame=`calculate "1e-20*$dose/$pixel_size/$pixel_size"`

if [ ! -e ${aligned_avg_mc2_dw} ] || [ ! -e $mc2_shift_plot ]; then
  module purge
  module load motioncor2/20161019
  module load eman2

  MotionCor2 -InMrc $raw_stack -Patch $patch -bft $bft -OutMrc $aligned_avg_mc2_dw -LogFile $motioncorr2_log -FmDose $dose_per_frame -PixSize $pixel_size -kV 300 -Align 1 -Gpu $gpu_id  >> $motioncorr2_log  2>&1
  e2proc2d.py --process math.realtofft  --fouriershrink 7.49609375  --process mask.sharp:inner_radius=1 $aligned_avg_mc2_dw $aligned_avg_mc2_dw_fft_thumbnail  >> $motioncorr2_log   2>&1

  python <<EOT
import matplotlib.pyplot as plt
x=[]
y=[]
with open("$motioncorr2_log") as f:
    for line in f.readlines():
        if line.startswith("...... Frame"):
            sp=line.split()
            x.append(float(sp[4]))
            y.append(float(sp[5]))
plt.figure(figsize=(5,5))
plt.plot(x,y, 'bo-')
plt.plot(x[:1],y[:1], 'ro')
plt.xlabel('shift x (A)')
plt.ylabel('shift y (A)')
plt.tight_layout()
plt.savefig("$mc2_shift_plot",dpi=100)
EOT

fi
[ -e ${aligned_avg_mc2_dw_png} ] || e2proc2d.py --fouriershrink 7.49609375 ${aligned_avg_mc2_dw} ${aligned_avg_mc2_dw_png}  >> $motioncorr2_log 2>&1




relion_jobid=4
HEADER="\ndata_\n\nloop_\n_rlnMicrographName #1"
mkdir -p $destination_path/Import/job00$relion_jobid
relion_alias Import $relion_jobid micrographs_mc2_d

write_to_star $destination_path/Import/job00$relion_jobid/micrographs.star "$HEADER" micrographs_mc2_dw/${short_name}.mrc 
add_to_pipeline  $destination_path/default_pipeline.star Import $relion_jobid micrographs_mc2_dw  "" "Import/job00$relion_jobid/micrographs.star:1"

RESULTS aligned_avg_mc2_dw_png aligned_avg_mc2_dw_fft_thumbnail mc2_shift_plot
FILES motioncorr2_log aligned_avg_mc2_dw aligned_avg_mc2_dw_png aligned_avg_mc2_dw_fft_thumbnail mc2_shift_plot

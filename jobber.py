import itertools as it
from pathlib import Path

accelerators = ["nvdla", "diannao", "conv1d"]
impl_files = ["systolic", "diannao", "systolic"]
signal_sizes = [2, 4, 8, 16, 32, 64]
filter_sizes = [1, 2, 4, 6, 8, 16, 32]

channels = [1, 2, 8, 24]

workingdir = Path("/u/jyoustra/scratch/hls_testbench")
condordir = workingdir / "condorfiles"
text = f"""universe = vanilla
getenv=true
Initialdir = /scratch/cluster/jyoustra/hls_testbench
Executable = {workingdir}/hls_and_synthesize.py
+Group = "UNDERGRAD"
+Project = "ARCHITECTURE"
+ProjectDescription = "conv accelerator comparison"
"""

def submit(accelerator, impl_file, signal, filt, channelIn, channelOut):
    global text
    designation = "{}-{}-{}-{}-{}".format(accelerator, signal, filt, channelIn, channelOut)
    text += f"""Error = {condordir / designation}.err
Output = {condordir / designation}.out
Arguments = -a {accelerator} -s {signal} -f {filt} -i {channelIn} -o {channelOut} -c {impl_file}
Queue
"""

# def retry():
#     result = sp.run("grep -r -l \"error\" condorfiles", shell=True, capture_output=True, check=True, text=True)

for signal, filt in it.product(signal_sizes, filter_sizes):
    if signal < filt:
        continue
    for accelerator, impl_file in zip(accelerators, impl_files):
        if accelerator == "nvdla":
            for channelIn, channelOut in it.product(channels, channels):
                submit(accelerator, impl_file, signal, filt, channelIn, channelOut)
        submit(accelerator, impl_file, signal, filt, 1, 1)

print(text)

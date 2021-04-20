import itertools as it
import subprocess as sp
import tempfile
import sys
import os
from pathlib import Path

accelerators = ["nvdla", "diannao", "conv1d"]
impl_files = ["systolic", "diannao", "systolic"]
signal_sizes = [2, 4, 8, 16, 32, 64]
filter_sizes = [1, 2, 4, 6, 8, 16, 32]

channels = [1, 2, 8, 24]

workingdir = Path("/u/jyoustra/scratch/hls_testbench")
tcl_dir = workingdir / "tcl"

os.chdir("accelerators")

def manualProcess(processString):
    result = sp.run("vivado_hls -f", shell=True, text=True, capture_output=True, input=processString)
    print(result.stdout)
    print(result.stderr, file=sys.stderr)

def condorProcess(accelerator, signal, filt, channelIn, channelOut, processString, override=False):
    designation = "{}-{}-{}-{}-{}".format(accelerator, signal, filt, channelIn, channelOut)
    tcl_path = tcl_dir / "{}.tcl".format(designation)
    if (not override) and tcl_path.exists():
        # already running
        return
    with open(tcl_path, 'w') as file:
        file.write(processString)
    command = "condorizer -j \"Synthesis_{}\" -o /u/jyoustra/scratch/hls_testbench/condorfiles/{} vivado_hls -f {}".format(designation, designation, tcl_path)
    print(command)
    result = sp.run(command, shell=True, text=True, capture_output=True)
    print(result.stdout)
    print(result.stderr, file=sys.stderr)

def submit(accelerator, impl_file, signal, filt, channelIn, channelOut, override=False):
    part = "{xc7z010iclg225-1L}"
    flags = "-std=c++0x -DSIGNAL_SIZE={} -DFILTER_SIZE={} -DCHANNEL_IN_COUNT={} -DCHANNEL_OUT_COUNT={}".format(signal, filt, channelIn, channelOut)
    solution = "solution_s{}_f{}_ci{}_co{}".format(signal, filt, channelIn, channelOut)
    processString = """open_project {}
set_top conv_2d
add_files {}/{}.cpp -cflags \"{}\"
add_files {}/systolic.h -cflags \"{}\"
add_files -tb {}/systolic_test.cpp -cflags \"{}\"
open_solution "{}"
set_part {}
create_clock -period 10 -name default
csynth_design
exit
""".format(accelerator, accelerator, impl_file, flags, accelerator, flags, accelerator, flags, solution, part)
    condorProcess(accelerator, signal, filt, channelIn, channelOut, processString, override=override)

# def retry():
#     result = sp.run("grep -r -l \"error\" condorfiles", shell=True, capture_output=True, check=True, text=True)


def batch():
    for signal, filt in it.product(signal_sizes, filter_sizes):
        if signal < filt:
            continue
        for accelerator, impl_file in zip(accelerators, impl_files):
            os.chdir(accelerator)
            if accelerator == "nvdla":
                for channelIn, channelOut in it.product(channels, channels):
                    submit(accelerator, impl_file, signal, filt, channelIn, channelOut)
            submit(accelerator, impl_file, signal, filt, 1, 1)
            # with tempfile.NamedTemporaryFile(mode='w') as fp:
            #     fp.write(processString)
            #     result = sp.run("vivado_hls -f {}".format(fp.name), shell=True, text=True, capture_output=True)
            #     print(result.stdout)
            #     print(result.stderr, file=sys.stderr)

            os.chdir("..")

batch()
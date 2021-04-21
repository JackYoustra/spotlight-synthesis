from pathlib import Path
import pandas as pd
import re
import xml.etree.ElementTree as et
from math import ceil

decoder = re.compile(r".*?solution_s([0-9]+)_f([0-9]+)_ci([0-9]+)_co([0-9]+).*?")
namer = re.compile(r".*?([a-zA-Z0-9]+)/solution.*?")

workingdir = Path("/u/jyoustra/scratch/hls_testbench")

accel_dir = workingdir / "accelerators"

dictlist = []

def ns_denominate(text):
    units, unit = text.split()
    if unit == "ns":
        return float(units)
    elif unit == "us":
        return float(units) * 1000.0
    elif unit == "ms":
        return float(units) * 1000.0 * 1000.0
    else:
        assert(False, "Need to define the normalization")

resnet_dims = [
    (256, 2, 64, 64),
    (28, 1, 128, 512),
    (7, 3, 512, 512),
]

for file in accel_dir.glob("*/*/solution*/syn/report/csynth.xml"):
    name = str(file.absolute())
    grouper = decoder.match(name)
    if grouper:
        signal = int(grouper.group(1))
        filt = int(grouper.group(2))
        chan_in = int(grouper.group(3))
        chan_out = int(grouper.group(4))

        name_match = namer.match(name)
        accel = name_match.group(1)

        tree = et.parse(file)

        lut = int(tree.find(".//LUT").text)
        # delay = ns_denominate(tree.find(".//Average-caseRealTimeLatency").text)
        delay = int(tree.find(".//Interval-min").text) * float(tree.find(".//EstimatedClockPeriod").text)

        if accel == "conv1d":
            accel = "eyeriss"

        sim_props = {
            "accelerator" : accel,
            "signal" : signal,
            "filter" : filt,
            "chan_in" : chan_in,
            "chan_out" : chan_out,
            "Latency" : delay,
            "DSP" : int(tree.find(".//DSP48E").text),
            "LUT" : lut,
            "flip-flop" : int(tree.find(".//FF").text),
            "bram" : int(tree.find(".//BRAM_18K").text),
            "Area-Throughput" : lut * delay,
        }

        # find number of times have to compute
        for idx, vals in enumerate(resnet_dims):
            s, f, ci, co = vals
            num_calls = int(ceil(ci / chan_in)) * int(ceil(co / chan_out)) * (int(ceil(f / filt)) ** 2) * (int(ceil(s / signal)) ** 2)
            sim_props.update({
                "Res {}: Itercount".format(idx) : num_calls,
                "Res {}: Area-Delay".format(idx) : num_calls * lut * delay
            })

        dictlist.append(sim_props)
    else:
        print("{} failed".format(name))
df = pd.DataFrame(dictlist)
df.to_csv("results.csv", index=False)
            

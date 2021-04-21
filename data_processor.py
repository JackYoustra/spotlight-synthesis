from pathlib import Path
import pandas as pd
import re
import xml.etree.ElementTree as et
from math import ceil
from scipy.stats import gmean

# decodes filepaths into specification strings
decoder = re.compile(r".*?(([a-zA-Z0-9]+)_s([0-9]+)_f([0-9]+)_ci([0-9]+)_co([0-9]+)).*?/solution1")
figure_regex = re.compile(r"Slack \(MET\) :\s*([0-9.]*.{2})\s.*?Data Path Delay:\s*([0-9.]*.+?)\s.*?logic \s*([0-9.]*.+?)\s\(\s*([0-9.]*.+?)\)\s*route\s*([0-9.]*.+?)\s\(\s*([0-9.]*.+?)\).*?Logic\sLevels.*?([0-9]+).*?", re.DOTALL)
workingdir = Path("/u/jyoustra/scratch/hls_testbench")

synth_projectdir = workingdir / "synthesized_projects"
hlsdir = synth_projectdir / "hls"
vivadodir = synth_projectdir / "vivado"

dictlist = []

def ns_denominate(text):
    units = text[:-2]
    unit = text[-2:]
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

for file in hlsdir.glob("*/solution1/syn/report/csynth.xml"):
    name = str(file.absolute())
    grouper = decoder.match(name)
    if grouper:
        designation = grouper.group(1)
        accel = grouper.group(2)
        signal = int(grouper.group(3))
        filt = int(grouper.group(4))
        chan_in = int(grouper.group(5))
        chan_out = int(grouper.group(6))

        tree = et.parse(file)

        lut = int(tree.find(".//LUT").text)
        synth_delay = int(tree.find(".//Interval-min").text) * float(tree.find(".//EstimatedClockPeriod").text)

        if accel == "conv1d":
            accel = "eyeriss"

        sim_props = {
            "accelerator" : accel,
            "signal" : signal,
            "filter" : filt,
            "chan_in" : chan_in,
            "chan_out" : chan_out,
            "Synth-Latency" : synth_delay,
            "DSP" : int(tree.find(".//DSP48E").text),
            "LUT" : lut,
            "flip-flop" : int(tree.find(".//FF").text),
            "bram" : int(tree.find(".//BRAM_18K").text),
            "Synth-Area-Throughput" : lut * synth_delay,
        }
        # add vivado synthesis, if it exists
        vivado_project = vivadodir / designation / f"{designation}.runs" / "impl_1"
        if vivado_project.exists():
            timingFile = vivado_project / "Zynq_Design_wrapper_timing_summary_routed.rpt"
            if timingFile.exists():
                timingsDicts = []
                with open(timingFile, "r") as timings:
                    # regex idea is sick, doesn't really work well... need to figure out how to stop backtracking
                    # Max Delay Paths\n-*\n(Slack \(MET\) :\s*([0-9.]*.{2})\s.*?Data Path Delay:\s*([0-9.]*.+?)\s.*?logic \s*([0-9.]*.+?)\s\(\s*([0-9.]*.+?)\)\s*route\s*([0-9.]*.+?)\s\(\s*([0-9.]*.+?)\).*?Logic\sLevels.*?([0-9]+).*?){5}Min Delay Paths
                    # slack_regex = re.compile(r"Slack \(MET\) :\s*([0-9.]+.*?)\s")
                    # min_parsing = False
                    # prefix = "max"
                    # for line in timings.readlines():
                    #     if min_parsing:
                    #         prefix = "min"
                    #         pass
                    #     else:
                    #         if "Min Delay Paths" in line:
                    #             min_parsing = True
                    #             continue
                    #     slack = slack_regex.match(line)
                    #     if slack:
                    #         slack = slack.group(1)
                    
                    # gdi, I can't believe it
                    # can't even
                    # just match more than once dummyyyyy
                    text = timingFile.read_text()
                    min_idx = text.find("Min Delay Paths")
                    
                    for match in figure_regex.finditer(text):
                        # figure out whether we're past min marker
                        label = "max" if match.start() < min_idx else "min"
                        timingsDicts.append({
                            "delay path case" : label,
                            "slack" : ns_denominate(match.group(1)),
                            "datapath delay" : ns_denominate(match.group(2)),
                            "logic datapath delay" : ns_denominate(match.group(3)),
                            "logic datapath delay pct" : float(match.group(4)[:-1]),
                            "route datapath delay" : ns_denominate(match.group(5)),
                            "route datapath delay pct" : float(match.group(6)[:-1]),
                            "logic level" : match.group(7),
                        })
                timingsdf = pd.DataFrame(timingsDicts)
                # print(timingsdf)
                grouping = timingsdf.groupby("delay path case").agg(gmean)
                # grouping.reset_index(inplace=True)
                # print(grouping)
                group_dict = pd.json_normalize(grouping.to_dict("index"), sep=" ").to_dict(orient="records")
                # print(group_dict)
                sim_props.update(group_dict[0])
                # print(sim_props)
                # exit()


        # find number of times have to compute
        for idx, vals in enumerate(resnet_dims):
            s, f, ci, co = vals
            num_calls = int(ceil(ci / chan_in)) * int(ceil(co / chan_out)) * (int(ceil(f / filt)) ** 2) * (int(ceil(s / signal)) ** 2)
            sim_props.update({
                "Res {}: Itercount".format(idx) : num_calls,
                "Res {}: Synth Area-Delay".format(idx) : num_calls * lut * synth_delay
            })

        dictlist.append(sim_props)
    else:
        print(f"{name} failed")
df = pd.DataFrame(dictlist)
df.to_csv("results.csv", index=False)
            

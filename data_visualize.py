import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from scipy.stats.stats import gmean
from pathlib import Path
import numpy as np
from sklearn.preprocessing import normalize
import matplotlib.ticker as mtick

df = pd.read_csv("results.csv")
# only select valid results
df.dropna(inplace=True)

def EADP_chart(key):
    path = Path(f"figs/Best PADP for {key} size.png")
    if not path.exists() or True:
        name = next(x for x in reversed(df.columns) if 'Total EADP' in x and 'Worst' in x)
        filters = df.pivot_table(values=name, index=key, columns='accelerator', aggfunc=min)
        filters.plot(legend=True, xticks=filters.index, logy=True, title=f"PADP vs {key} size (lower is better)", xlabel=f"{key} size".capitalize(), ylabel="Log PADP (W * s * % area utilization)")
        plt.savefig(path, bbox_inches='tight')
        plt.clf()
EADP_chart('filter')
EADP_chart('signal')
EADP_chart('Core power')

def stats():
    for desc in ['Worst', 'Best']:
        name = next(x for x in reversed(df.columns) if 'Total EADP' in x and desc in x)
        filters = df.pivot_table(values=name, columns='accelerator', aggfunc=min)
        f = filters.div(filters.min(axis=1), axis=0)
        print(f)
stats()

path = Path(f"figs/accel_counts.png")
if not path.exists():
    one_chan = df.loc[(df['chan_in'] == 1) & (df['chan_out'] == 1)]
    multi_chan = df.loc[(df['chan_in'] != 1) | (df['chan_out'] != 1)]
    single_counts = pd.value_counts(one_chan['accelerator'])
    total_counts = pd.value_counts(multi_chan['accelerator'])
    resulting = pd.concat([single_counts, total_counts], axis=1)
    resulting.fillna(0, inplace=True)
    resulting.columns = ["Monochannel", "Multichannel"]
    print(resulting)
    resulting.plot.bar(stacked=True, title="Evaluated configurations by accelerator", ylabel="Number of evaluated configurations", rot=0)
    plt.savefig(path, bbox_inches='tight')
    plt.clf()

def core_eng_chart():
    non_core_energy = (df["Total power"] - df["Core power"]).values
    df["non core energy"] = non_core_energy
    figpath = Path('figs/core_noncore_energy.png')
    if not figpath.exists():
        #pct_run = df.assign(non_core_energy=(df["Total power"] - df["Core power"]).values)
        x = [gmean(non_core_energy), gmean(df["Core power"].values)]
        labels = ["Non-Core Energy", "Convolution Core Energy"]
        # add max potential
        # maximum_observed = max(df["Core power"].values)
        # x[0] -= maximum_observed
        # x.append(maximum_observed)
        # labels.append("Maximum observed core energy")

        plt.pie(x, labels=labels, autopct='%1.1f%%')
        plt.legend()
        plt.title("Non-Core energy vs convolution core energy")
        # Caption: geometric mean over best synthesized configurations
        plt.savefig(figpath, bbox_inches='tight')
        plt.clf()
    figpath = Path('figs/top_core_noncore_energy.png')
    if not figpath.exists():
        accelerators = df["accelerator"].unique()
        core = []
        noncore = []
        for accel in accelerators:
            accel_df = df.loc[df["accelerator"] == accel]
            best_idx = np.argmin(accel_df["Resnet (average): Total EADP (Core power) (W*s*chip area used) (Worst-case)"].values)
            core.append(accel_df["Core power"].values[best_idx])
            noncore.append(accel_df["non core energy"].values[best_idx])
        x = [gmean(noncore), gmean(core)]
        labels = ["Non-Core Energy", "Convolution Core Energy"]
        plt.pie(x, labels=labels, autopct='%1.1f%%')
        plt.legend()
        plt.title("Non-Core energy vs convolution core energy (best configuration)")
        # Caption: geometric mean over best synthesized configurations
        plt.savefig(figpath, bbox_inches='tight')
        plt.clf()
    figpath = Path('figs/max_observed_core.png')
    if not figpath.exists():
        #pct_run = df.assign(non_core_energy=(df["Total power"] - df["Core power"]).values)
        idx = np.argmax(df["Core power"].values / non_core_energy)
        x = [non_core_energy[idx], df["Core power"].values[idx]]
        labels = ["Non-Core Energy", "Convolution Core Energy"]
        # add max potential
        # maximum_observed = max(df["Core power"].values)
        # x[0] -= maximum_observed
        # x.append(maximum_observed)
        # labels.append("Maximum observed core energy")

        plt.pie(x, labels=labels, autopct='%1.1f%%')
        plt.legend()
        plt.title("Non-Core energy vs convolution core energy (worst case synthesis)")
        # Caption: geometric mean over best synthesized configurations
        plt.savefig(figpath, bbox_inches='tight')
        plt.clf()

core_eng_chart()

def latency_chart():
    figpath = Path('figs/static_dynamic.png')
    if not figpath.exists():
        #pct_run = df.assign(non_core_energy=(df["Total power"] - df["Core power"]).values)
        x = [gmean(df["Static total power"].values), gmean(df["Dynamic total power"].values)]
        labels = ["Static total power", "Dynamic total power"]
        # add max potential
        # maximum_observed = max(df["Core power"].values)
        # x[0] -= maximum_observed
        # x.append(maximum_observed)
        # labels.append("Maximum observed core energy")

        plt.pie(x, labels=labels, autopct='%1.1f%%')
        plt.legend()
        plt.title("Share of power: Dynamic vs Static")
        # Caption: geometric mean over best synthesized configurations
        plt.savefig(figpath, bbox_inches='tight')
        plt.clf()
    figpath = Path('figs/top_static_dynamic.png')
    if not figpath.exists():
        accelerators = df["accelerator"].unique()
        core = []
        noncore = []
        for accel in accelerators:
            accel_df = df.loc[df["accelerator"] == accel]
            best_idx = np.argmin(accel_df["Resnet (average): Total EADP (Core power) (W*s*chip area used) (Worst-case)"].values)
            core.append(accel_df["Static total power"].values[best_idx])
            noncore.append(accel_df["Dynamic total power"].values[best_idx])
        x = [gmean(noncore), gmean(core)]
        labels = ["Static total power", "Dynamic total power"]
        plt.pie(x, labels=labels, autopct='%1.1f%%')
        plt.legend()
        plt.title("Share of power: Dynamic vs Static (best configuration)")
        # Caption: geometric mean over best synthesized configurations
        plt.savefig(figpath, bbox_inches='tight')
        plt.clf()

latency_chart()

def delay_chart():
    for label in ["max", "min"]:
        figpath = Path(f'figs/{label}_delay_sources.png')
        if not figpath.exists():
            #pct_run = df.assign(non_core_energy=(df["Total power"] - df["Core power"]).values)
            x = [gmean(df[f"{label} route datapath delay"].values), gmean(df[f"{label} logic datapath delay"].values)]
            labels = [f"{label} route".capitalize(), f"{label} logic".capitalize()]
            # add max potential
            # maximum_observed = max(df["Core power"].values)
            # x[0] -= maximum_observed
            # x.append(maximum_observed)
            # labels.append("Maximum observed core energy")

            plt.pie(x, labels=labels, autopct='%1.1f%%', normalize=True)
            plt.title(f"{label} route delay vs {label} logic delay".capitalize())
            # Caption: geometric mean over best synthesized configurations
            plt.savefig(figpath, bbox_inches='tight')
            plt.clf()
    figpath = Path(f'figs/delay_sources.png')
    if not figpath.exists():
        x = ["Worst case", "Best case"]
        y = []
        labels = []
        for label in ["max", "min"]:
            #pct_run = df.assign(non_core_energy=(df["Total power"] - df["Core power"]).values)
            y.append([gmean(df[f"{label} route datapath delay"].values), gmean(df[f"{label} logic datapath delay"].values)])
            labels.append([f"{label} route".capitalize(), f"{label} logic".capitalize()])
        y = np.array(y).transpose()
        y = normalize(y, axis=0, norm='l1')
        print(y)

        plt.bar(x, y[0], label="Route delay")
        plt.bar(x, y[1], label="Datapath delay", bottom=y[0])
        plt.gca().get_yaxis().set_major_formatter(mtick.PercentFormatter(1.0))
        #plt.xlabel(["Worst-case delay", "Best-case delay"])
        plt.ylabel("Percentage of total delay")
        plt.title(f"Route delay vs logic delay".capitalize())
        plt.legend()
        # Caption: geometric mean over best synthesized configurations
        plt.savefig(figpath, bbox_inches='tight')
        plt.clf()
    
delay_chart()
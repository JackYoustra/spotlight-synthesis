import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results.csv")

elements = df.loc[(df['chan_in'] == 1) & (df['chan_out'] == 1) & (df['signal'] == 16) & (df['accelerator'] != 'nvdla')]

sorts = elements.sort_values("filter")
#print(sorts)

for keyword in [x for x in sorts.columns if 'Area-Delay' in x]:
    # vary filters over the popular two
    for accelerator in sorts["accelerator"].unique():
        accel_cols = sorts.loc[sorts['accelerator'] == accelerator]
        xval = accel_cols['filter'].values[1:]
        y = accel_cols[keyword].values[1:]
        x = list(range(len(xval)))
        plt.plot(x, y, label=accelerator)
        plt.xticks(x, xval)
    plt.xlabel("Filter size")
    plt.ylabel(keyword + "(steady-state ns delay * LUTs * times called)")
    plt.title("EDP for entire ResNet layer (16x16 signal tile, lower is better)")
    plt.legend()
    plt.savefig('figs/{}.png'.format(keyword))
    plt.clf()


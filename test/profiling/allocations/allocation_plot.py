import matplotlib.pyplot as plt
from matplotlib import style
import numpy as np
import pandas as pd
import sys

if len(sys.argv) < 2:
    print("Bad usage.")
    print("    allocation_plot.py <csv_file> [0,1,2,3]")
    exit(-1)

if len(sys.argv) > 2:
    bp0 = sys.argv[2].find('0') != -1
    bp1 = sys.argv[2].find('1') != -1
    bp2 = sys.argv[2].find('2') != -1
    bp3 = sys.argv[2].find('3') != -1
else:
    bp0 = True
    bp1 = True
    bp2 = True
    bp3 = True

headers = ["Phase 0 Allocations", "Phase 0 Deallocations", "Phase 1 Allocations", "Phase 1 Deallocations",
        "Phase 2 Allocations", "Phase 2 Deallocations", "Phase 3 Allocations", "Phase 3 Deallocations"]

df = pd.read_csv(sys.argv[1], delimiter=",", names=headers)
p0a = [float(value) for value in df["Phase 0 Allocations"].values[1::]]
p1a = [float(value) for value in df["Phase 1 Allocations"].values[1::]]
p2a = [float(value) for value in df["Phase 2 Allocations"].values[1::]]
p3a = [float(value) for value in df["Phase 3 Allocations"].values[1::]]

axis = np.arange(1, len(p0a)+1)

style.use('ggplot')

if bp0:
    plt.plot(axis, p0a, label='Ph0')
if bp1:
    plt.plot(axis, p1a, label='Ph1')
if bp2:
    plt.plot(axis, p2a, label='Ph2')
if bp3:
    plt.plot(axis, p3a, label='Ph3')
plt.xlabel("Execution")
plt.ylabel("Allocations")
plt.title("Allocations in phases")
plt.xticks(axis, axis)
plt.legend()
plt.show()

#! /usr/bin/python

# pip install numpy
# pip install matplotlib

# compute ecdf of input file
# usage:  python ecdf2.py <filename> <col-to-cdf> <col1-id>
# where filename is single column of unsorted data
import sys
import numpy as np

# avoid _tkinter dependency
import matplotlib
matplotlib.use("agg")

import matplotlib.pyplot as plt

x = []
fd = open(sys.argv[1], 'r')
i=1
j=0
for line in fd:
    cols = line.split(',')
    if i > 1:
        if (int(cols[0]) == int(sys.argv[3])) and \
            (int(cols[1]) == int(sys.argv[4])) and \
            (int(cols[2]) >= int(sys.argv[5])) and \
            (int(cols[2]) <= int(sys.argv[6])) and \
            (int(cols[8].strip()) >= int(sys.argv[8])):
            # print(sys.argv[2])
            x.append(float(cols[int(sys.argv[2])].rstrip()))
            j=j+1
    i=i+1

print("%d matching data points" % j)
# print(np.sort(x))

# a = np.array(x)
# np.set_printoptions(suppress=True)
# np.set_printoptions(precision=3)
# asort = np.sort(a)
# i = 1 
# for xx in np.nditer(asort):
#     print("%.6f %.6f" % (xx, i / float(asort.size)))
#     i = i + 1

# plot 2"x2" plot area
plt.figure(figsize=(2.5,2.5))
ax = plt.gca()
ax.autoscale()
# endpoint=True means include last point
plt.plot(np.sort(x), np.linspace(0, 1, len(x), endpoint=True))
plt.savefig(sys.argv[7])
# plt.show()


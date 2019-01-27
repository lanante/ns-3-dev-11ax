#!/bin/bash
# Post-process results of individual simulation scenarios
# and prepare results files for further processing, e.g.,
# plotting of the collective results.

# Collect all measures from the individual output files
# (one for each scneario) into a single file of all results,
# per performance measure, that is then used for further
# processing.
# Hence, this script should be run after the scenarios and
# experiments scripts have been run, but before the results
# are plotted.

cd ../scripts

# Sxperiments

# Throughput
rm ./results/spatial-reuse-experiments-throughput-ap1.dat
rm ./results/spatial-reuse-experiments-throughput-ap2.dat

grep 'Throughput,  AP1 Uplink' ./results/spatial-reuse-SR-stats-experiments-*.dat > ./results/spatial-reuse-experiments-throughput-ap1.dat
grep 'Throughput,  AP2 Uplink' ./results/spatial-reuse-SR-stats-experiments-*.dat > ./results/spatial-reuse-experiments-throughput-ap2.dat

# Area Capacity
rm ./results/spatial-reuse-experiments-area-capacity-ap1.dat
rm ./results/spatial-reuse-experiments-area-capacity-ap2.dat

grep 'Area Capacity, AP1 Uplink' ./results/spatial-reuse-SR-stats-experiments-*.dat > ./results/spatial-reuse-experiments-area-capacity-ap1.dat
grep 'Area Capacity, AP2 Uplink' ./results/spatial-reuse-SR-stats-experiments-*.dat > ./results/spatial-reuse-experiments-area-capacity-ap2.dat

# Spectrum Efficiency
rm ./results/spatial-reuse-experiments-spectrum-efficiency-ap1.dat
rm ./results/spatial-reuse-experiments-spectrum-efficiency-ap2.dat

grep 'Spectrum Efficiency, AP1 Uplink' ./results/spatial-reuse-SR-stats-experiments-*.dat > ./results/spatial-reuse-experiments-spectrum-efficiency-ap1.dat
grep 'Spectrum Efficiency, AP2 Uplink' ./results/spatial-reuse-SR-stats-experiments-*.dat > ./results/spatial-reuse-experiments-spectrum-efficiency-ap2.dat

# Study 1

# results for study 1 are from the perspective of AP1

# Throughput
rm ./results/spatial-reuse-study1-throughput-ap1.dat

grep 'Throughput,  AP1 Uplink' ./results/spatial-reuse-SR-stats-study1-*.dat > ./results/spatial-reuse-study1-throughput-ap1.dat

# Area Capacity
rm ./results/spatial-reuse-study1-area-capacity-ap1.dat

grep 'Area Capacity, AP1 Uplink' ./results/spatial-reuse-SR-stats-study1-*.dat > ./results/spatial-reuse-study1-area-capacity-ap1.dat

# Spectrum Efficiency
rm ./results/spatial-reuse-study1-spectrum-efficiency-ap1.dat

grep 'Spectrum Efficiency, AP1 Uplink' ./results/spatial-reuse-SR-stats-study1-*.dat > ./results/spatial-reuse-study1-spectrum-efficiency-ap1.dat

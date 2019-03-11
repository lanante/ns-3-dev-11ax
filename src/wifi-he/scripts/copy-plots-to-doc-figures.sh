#!/bin/bash
# These are the scenarios for spatial-reuse experiments.

cd ../scripts

# copy experiments that are of interest, and that are documented as figures in the
# documentation
for experiment in 20-10-02-05 20-10-02-20 20-30-02-05 20-30-02-20 80-10-02-05 80-10-02-20 80-30-02-05 80-30-02-20 80-30-02-10 80-10-02-10  ; do
    cp "./results/spatial-reuse-positions-experiments-$experiment-0006-0000.png" ../doc/source/figures/.
    cp "./images/png/throughput-$experiment-ap1.png" ../doc/source/figures/.
    cp "./images/png/throughput-$experiment-ap2.png" ../doc/source/figures/.
    cp "./images/png/throughput-$experiment-both.png" ../doc/source/figures/.
    cp "./images/png/area-capacity-$experiment-ap1.png" ../doc/source/figures/.
    cp "./images/png/area-capacity-$experiment-ap2.png" ../doc/source/figures/.
    cp "./images/png/area-capacity-$experiment-both.png" ../doc/source/figures/.
    cp "./images/png/spectrum-efficiency-$experiment-ap1.png" ../doc/source/figures/.
    cp "./images/png/spectrum-efficiency-$experiment-ap2.png" ../doc/source/figures/.
    cp "./images/png/spectrum-efficiency-$experiment-both.png" ../doc/source/figures/.
    cp "./images/png/airtime-utilization-$experiment-ap1.png" ../doc/source/figures/.
    cp "./images/png/airtime-utilization-$experiment-ap2.png" ../doc/source/figures/.
    cp "./images/png/airtime-utilization-$experiment-both.png" ../doc/source/figures/.
done

# selected plots for sensitivity studies
cp "./images/png/area-capacity-combined1.png" ../doc/source/figures/.
cp "./images/png/area-capacity-distance.png" ../doc/source/figures/.
cp "./images/png/area-capacity-radius.png" ../doc/source/figures/.
cp "./images/png/area-capacity-nSTAs.png" ../doc/source/figures/.
cp "./images/png/area-capacity-radius-v2.png" ../doc/source/figures/.

# calibration results
# test1a
cp "./results/spatial-reuse-positions-calibration-test1a.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-calibration-test1a-ap1-noise.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-calibration-test1a-ap1-signal.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-calibration-test1a-ap2-noise.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-calibration-test1a-ap2-signal.png" ../doc/source/figures/.
# test1b
cp "./results/spatial-reuse-positions-calibration-test1b.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-calibration-test1b-ap1-noise.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-calibration-test1b-ap1-signal.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-calibration-test1b-ap2-noise.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-calibration-test1b-ap2-signal.png" ../doc/source/figures/.
# test2a
cp "./results/spatial-reuse-positions-calibration-test2a.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-calibration-test2a-ap1-noise.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-calibration-test2a-ap1-signal.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-calibration-test2a-ap2-noise.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-calibration-test2a-ap2-signal.png" ../doc/source/figures/.
# test2b
cp "./results/spatial-reuse-positions-calibration-test2b.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-calibration-test2b-ap1-noise.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-calibration-test2b-ap1-signal.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-calibration-test2b-ap2-noise.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-calibration-test2b-ap2-signal.png" ../doc/source/figures/.
# test3
cp "./results/spatial-reuse-positions-calibration-test3.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-calibration-test3-ap1-noise.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-calibration-test3-ap1-signal.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-calibration-test3-ap2-noise.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-calibration-test3-ap2-signal.png" ../doc/source/figures/.

# TGax Scenarios
# TGax-test1 Residential Scenario
cp "./results/spatial-reuse-positions-TGax-test1.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-TGax-test1-ap1-noise.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-TGax-test1-ap1-signal.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-TGax-test1-ap2-noise.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-TGax-test1-ap2-signal.png" ../doc/source/figures/.
# TGax-test2 Enterprise Scenario
cp "./results/spatial-reuse-positions-TGax-test2.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-TGax-test2-ap1-noise.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-TGax-test2-ap1-signal.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-TGax-test2-ap2-noise.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-TGax-test2-ap2-signal.png" ../doc/source/figures/.
# TGax-test3 Indoor Small BSSs Scenario
cp "./results/spatial-reuse-positions-TGax-test3.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-TGax-test3-ap1-noise.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-TGax-test3-ap1-signal.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-TGax-test3-ap2-noise.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-TGax-test3-ap2-signal.png" ../doc/source/figures/.
# TGax-test4 Outdoor Large BSS Scenario
cp "./results/spatial-reuse-positions-TGax-test4.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-TGax-test4-ap1-noise.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-TGax-test4-ap1-signal.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-TGax-test4-ap2-noise.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-TGax-test4-ap2-signal.png" ../doc/source/figures/.

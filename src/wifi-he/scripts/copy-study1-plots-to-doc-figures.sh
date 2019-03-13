#!/bin/bash
# These are the scenarios for study1.

cd ../scripts

if [ -f "./results/spatial-reuse-positions-test-study1.png" ]; then
    cp "./results/spatial-reuse-positions-test-study1.png" ../doc/source/figures/.
fi
cp "./images/png/throughput-study1.png" ../doc/source/figures/.
cp "./images/png/area-capacity-study1.png" ../doc/source/figures/.
cp "./images/png/spectrum-efficiency-study1.png" ../doc/source/figures/.
cp "./images/png/airtime-utilization-study1.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-study1-3464-10-05-2-180.0-20.0-ap1-signal.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-study1-3464-10-05-2-180.0-20.0-ap1-noise.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-study1-3464-10-20-2-180.0-20.0-ap1-signal.png" ../doc/source/figures/.
cp "./results/spatial-reuse-rx-sniff-study1-3464-10-20-2-180.0-20.0-ap1-noise.png" ../doc/source/figures/.

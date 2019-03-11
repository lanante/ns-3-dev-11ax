#!/bin/bash
# These are the scenarios for spatial-reuse experiments.
# Results will be documented in the Results Chapter

rm ./results/spatial-reuse-experiments*.*
rm ./results/spatial-reuse-SR-stats-experiments*.*

trap ctrl_c INT

function ctrl_c() {
         echo "Trapped CTRL-C, exiting..."
         exit 1
}

source spatial-reuse-functions.sh

# Run the test(s) for Spatial-Reuse experiments

# common parameters
export RngRun=1
export powSta=15
export powAp=20
# to be changed. set to "3" for now, for testing
export duration=3.0
export enableRts=0
# for now, just 802.11ac
export standard=11ac
export MCS=0
export payloadSizeUplink=1500
export payloadSizeDownlink=1500
export txStartOffset=5
export enableObssPd=1
export txGain=0
export rxGain=0
export antennas=1
export maxSupportedTxSpatialStreams=1
export maxSupportedRxSpatialStreams=1
export performTgaxTimingChecks=0
export maxAmpduSize=3142
export nodePositionsFile=NONE
export enablePcap=0
export enableAscii=0
export bw=20
export obssPdThreshold=-82
export useIdealWifiManager=0

cd ../examples

# for starters, base everything off of the Residential Scenerio
# TODO - Enterpise, Indoor Small BSS, Outdoor Large BSS Hotspot - see run-scenarios.sh for params

# Test: spatial-reuse-residential
# params that will not vary
export scenario=residential
export txRange=54

# for n in 5 10 20 ; do
for n in 5 10 20 ; do
    # params that will vary
    for d in 20 40 60 80 ; do
        for r in 10 20 30 ; do
            for nBss in 2 ; do
                for uplink in 1 2 3 4 5 6 ; do
                    downlink=0
                    test=$(printf "experiments-%02d-%02d-%02d-%02d-%04d-%04d" ${d} ${r} ${nBss} ${n} ${uplink} ${downlink})
                    sleep 1; run_one &
                done
            done
        done
	wait
    done
done
wait

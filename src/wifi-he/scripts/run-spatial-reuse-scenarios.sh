#!/bin/bash
# These are the scenarios for spatial-reuse experiements.
# Results will be documented in the Results Chapter

rm ./results/spatial-reuse-experiments*.*
rm ./results/spatial-reuse-SR-stats-experiments*.*

source spatial-reuse-functions.sh

# Run the test(s) for Spatial-Reuse experiments

# common parameters
RngRun=1
powSta=15
powAp=20
# to be changed. set to "3" for now, for testing
duration=3.0
enableRts=0
# for now, just 802.11ac
standard=11ac
MCS=0
payloadSizeUplink=1500
payloadSizeDownlink=1500
txStartOffset=5
enableObssPd=1
txGain=0
rxGain=0
antennas=1
maxSupportedTxSpatialStreams=1
maxSupportedRxSpatialStreams=1
performTgaxTimingChecks=0
maxAmpduSize=3140
nodePositionsFile=NONE
enablePcap=0
enableAscii=0
bw=20
obssPdThreshold=-82

cd ../examples

# for starters, base everything off of the Residential Scenerio
# TODO - Enterpise, Indoor Small BSS, Outdoor Large BSS Hotspot - see run-scenarios.sh for params

# Test: spatial-reuse-residential
# params that will not vary
scenario=residential
txRange=54

# params that will vary
for d in 20 40 60 80 ; do
    for r in 10 20 30 ; do
        for nBss in 2 ; do
            # for n in 5 10 20 ; do
            for n in 5 10 20 ; do
                for uplink in 1 2 3 4 5 6 ; do
                    downlink=0
                    test=$(printf "experiments-%02d-%02d-%02d-%02d-%04d-%04d" ${d} ${r} ${nBss} ${n} ${uplink} ${downlink})
                    run_one "$test" "$RngRun" "$powSta" "$powAp" "$duration" "$d" "$r" "$n" "$uplink" "$downlink" "$enableRts" "$standard" "$bw" "$txRange" "$MCS" "$payloadSizeUplink" "$payloadSizeDownlink" "$txStartOffset" "$enableObssPd" "$txGain" "$rxGain" "$antennas" "$maxSupportedTxSpatialStreams" "$maxSupportedRxSpatialStreams" "$performTgaxTimingChecks" "$scenario" "$nBss" "$maxAmpduSize" "$nodePositionsFile" "$enablePcap" "$enableAscii" "$obssPdThreshold"
                done
            done
        done
    done
done


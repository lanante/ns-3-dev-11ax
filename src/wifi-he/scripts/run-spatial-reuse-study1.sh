#!/bin/bash
# These are the scenarios for spatial-reuse study1 (802.11ac baseline).
# Results will be documented.

rm ./results/spatial-reuse-study1*.*
rm ./results/spatial-reuse-SR-stats-study1*.*

source spatial-reuse-functions.sh

# common parameters
RngRun=1
powSta=15
powAp=20
# to be changed to 100s. set to "1" for now, for testing
duration=3.0
enableRts=0
# for now, just 802.11ac
standard=11ac
MCS=9
# need to have payload1 and payload2, percentage of each
payloadSize=1500
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
# for starters, base everything off of the Residential Scenerio
scenario=residential
txRange=54

cd ../examples

# Run the test(s) for Spatial-Reuse study1

# params that will vary
# vary d from 10 to 80 in steps of 10
for d in 10 20 30 40 50 60 70 80 ; do
    # vary r from 10 to 50 in steps of 10
    for r in 10 20 30 40 50 ; do
        # 4 BSS
        for nBss in 4 ; do
            # vary n from 5 to 40  in steps of 5
            for n in 5 10 15 20 25 30 35 40 ; do
                for offeredLoad in 1 2 3 4 5 6 ; do
                    # uplink is 90% of total offered load
                    uplink="$((offeredLoad*0.9))"
                    # downlink is 10% of total offered load
                    downlink="$((offeredLoad*0.1))"
                    test=$(printf "study1-%02d-%02d-%02d-%02d-%04d-%04d" ${d} ${r} ${nBss} ${n} ${uplink} ${downlink})
                    run_one "$test" "$RngRun" "$powSta" "$powAp" "$duration" "$d" "$r" "$n" "$uplink" "$downlink" "$enableRts" "$standard" "$bw" "$txRange" "$MCS" "$payloadSize" "$txStartOffset" "$enableObssPd" "$txGain" "$rxGain" "$antennas" "$maxSupportedTxSpatialStreams" "$maxSupportedRxSpatialStreams" "$performTgaxTimingChecks" "$scenario" "$nBss" "$maxAmpduSize" "$nodePositionsFile" "$enablePcap" "$enableAscii"
                done
            done
        done
    done
done


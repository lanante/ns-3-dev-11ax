#!/bin/bash
# These are the scenarios for spatial-reuse study2 (802.11ax).
# Results will be documented.

rm ./results/spatial-reuse-study2*.*
rm ./results/spatial-reuse-SR-stats-study2*.*

source spatial-reuse-functions.sh

# common parameters
RngRun=1
powSta=15
powAp=20
# to be changed to TBD. set to low number for now, for testing so that script completes
duration=10.0
enableRts=0
# 802.11ax
standard=11ax_5GHZ
# TBD - use ideal channel model
MCS=0
# need to have payload1 and payload2, percentage of each
payloadSizeUplink=1500
payloadSizeDownlink=300
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
scenario=study2
txRange=15

cd ../examples

# Run the test(s) for Spatial-Reuse study2
d=17.32
r=10
nBss=7

rm ../scripts/study2.sh
echo "# import helper scripts" >> ../scripts/study2.sh
echo "source spatial-reuse-functions.sh" >> ../scripts/study2.sh
echo "" >> ../scripts/study2.sh
echo "cd ../examples" >> ../scripts/study2.sh
echo "" >> ../scripts/study2.sh

# script as generated and executed 11/27/2018 takes 12 hrs clock time on Scott's laptop.

# params that will vary
# vary n from 5 to 40  in steps of 5
# for n in 5 10 15 20 25 30 35 40 ; do
# for now, reducing this to n=10,20,30,40 to reduce number of simulations
for n in 10 20 30 40 ; do
    for offeredLoad in 1 2 3 4 5 6 ; do
        for obssPdThreshold in -82 -77 -72 -67 -62 ; do
            # uplink is 90% of total offered load
            uplink=$(awk "BEGIN {print $offeredLoad*0.9}")
            # downlink is 10% of total offered load
            downlink=$(awk "BEGIN {print $offeredLoad*0.1}")
            d1=$(awk "BEGIN {print $d*100}")
            ul1=$(awk "BEGIN {print $uplink*100}")
            dl1=$(awk "BEGIN {print $downlink*100}")
            test=$(printf "study2-%0.f-%02d-%02d-%04d-%0.f-%0.f%0.f\n" ${d1} ${r} ${n} ${offeredLoad} ${ul1} ${dl1} ${obssPdThreshold})
            echo "# run $test" >> ../scripts/study2.sh
            echo "run_one $test $RngRun $powSta $powAp $duration $d $r $n $uplink $downlink $enableRts $standard $bw $txRange $MCS $payloadSizeUplink $payloadSizeDownlink $txStartOffset $enableObssPd $txGain $rxGain $antennas $maxSupportedTxSpatialStreams $maxSupportedRxSpatialStreams $performTgaxTimingChecks $scenario $nBss $maxAmpduSize $nodePositionsFile $enablePcap $enableAscii $obssPdThreshold" >> ../scripts/study2.sh
            echo "" >> ../scripts/study2.sh
        done
    done
done

chmod +x ../scripts/study2.sh

echo "# the script '../scripts/study2.sh' has been created."
echo "# to run the study 2 simulations, you should run ./study2.sh"

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
# to be changed to TBD. set to low number for now, for testing so that script completes
duration=5.0
enableRts=0
# for now, just 802.11ac
standard=11ac
# TBD - use ideal channel model
MCS=0
# need to have payload1 and payload2, percentage of each
payloadSize=1500
txStartOffset=5
enableObssPd=0
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
scenario=study1
txRange=15

cd ../examples

# Run the test(s) for Spatial-Reuse study1
d=17.32
r=10
nBss=7

rm ../scripts/study1.sh
echo "# import helper scripts" >> ../scripts/study1.sh
echo "source spatial-reuse-functions.sh" >> ../scripts/study1.sh
echo "" >> ../scripts/study1.sh
echo "cd ../examples" >> ../scripts/study1.sh
echo "" >> ../scripts/study1.sh

# params that will vary
# vary n from 5 to 40  in steps of 5
for n in 5 10 15 20 25 30 35 40 ; do
    for offeredLoad in 1.0 2.0 3.0 4.0 5.0 6.0 ; do
        # uplink is 90% of total offered load
        uplink=$(awk "BEGIN {print $offeredLoad*0.9}")
        # downlink is 10% of total offered load
        downlink=$(awk "BEGIN {print $offeredLoad*0.1}")
        d1=$(awk "BEGIN {print $d*100}")
        ul1=$(awk "BEGIN {print $uplink*100}")
        dl1=$(awk "BEGIN {print $downlink*100}")
        test=$(printf "study1-%0.f-%02d-%02d-%0.f-%0.f\n" ${d1} ${r} ${n} ${ul1} ${dl1})
        echo "# run $test" >> ../scripts/study1.sh
        echo "run_one $test $RngRun $powSta $powAp $duration $d $r $n $uplink $downlink $enableRts $standard $bw $txRange $MCS $payloadSize $txStartOffset $enableObssPd $txGain $rxGain $antennas $maxSupportedTxSpatialStreams $maxSupportedRxSpatialStreams $performTgaxTimingChecks $scenario $nBss $maxAmpduSize $nodePositionsFile $enablePcap $enableAscii" >> ../scripts/study1.sh
        echo "" >> ../scripts/study1.sh
    done
done

chmod +x ../scripts/study1.sh

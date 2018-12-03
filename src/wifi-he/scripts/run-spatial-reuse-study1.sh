#!/bin/bash
# These are the scenarios for spatial-reuse study1 (802.11ac baseline).
# Results will be documented.

rm ./results/spatial-reuse-study1*.*
rm ./results/spatial-reuse-SR-stats-study1*.*

trap ctrl_c INT

function ctrl_c() {
         echo "Trapped CTRL-C, exiting..."
         exit 1
}

source spatial-reuse-functions.sh

rm ../scripts/study1.sh

echo "# import helper scripts" >> ../scripts/study1.sh
echo "source spatial-reuse-functions.sh" >> ../scripts/study1.sh
echo "" >> ../scripts/study1.sh
echo "cd ../examples" >> ../scripts/study1.sh
echo "" >> ../scripts/study1.sh

# common parameters
echo "export RngRun=1" >> ../scripts/study1.sh
echo "export powSta=15" >> ../scripts/study1.sh
echo "export powAp=20" >> ../scripts/study1.sh
# to be changed to TBD. set to low number for now, for testing so that script completes
echo "export duration=10.0" >> ../scripts/study1.sh
echo "export enableRts=0" >> ../scripts/study1.sh
# for now, just 802.11ac
echo "export standard=11ac" >> ../scripts/study1.sh
echo "export payloadSizeUplink=1500" >> ../scripts/study1.sh
echo "export payloadSizeDownlink=300" >> ../scripts/study1.sh
echo "export txStartOffset=5" >> ../scripts/study1.sh
echo "export enableObssPd=0" >> ../scripts/study1.sh
echo "export txGain=0" >> ../scripts/study1.sh
echo "export rxGain=0" >> ../scripts/study1.sh
echo "export antennas=1" >> ../scripts/study1.sh
echo "export maxSupportedTxSpatialStreams=1" >> ../scripts/study1.sh
echo "export maxSupportedRxSpatialStreams=1" >> ../scripts/study1.sh
echo "export performTgaxTimingChecks=0" >> ../scripts/study1.sh
echo "export nodePositionsFile=NONE" >> ../scripts/study1.sh
echo "export enablePcap=0" >> ../scripts/study1.sh
echo "export enableAscii=0" >> ../scripts/study1.sh
echo "export bw=20" >> ../scripts/study1.sh
# for starters, base everything off of the Residential Scenerio
echo "export scenario=study1" >> ../scripts/study1.sh
echo "export txRange=15" >> ../scripts/study1.sh
echo "export obssPdThreshold=-82" >> ../scripts/study1.sh
echo "export useIdealWifiManager=0" >> ../scripts/study1.sh

cd ../examples

# Run the test(s) for Spatial-Reuse study1
d=17.32
echo "export d=${d}" >> ../scripts/study1.sh
echo "export r=10" >> ../scripts/study1.sh
echo "export nBss=7" >> ../scripts/study1.sh

# TBD - use ideal channel model.  for now use MCS=7
echo "export MCS=7" >> ../scripts/study1.sh
# need to have payload1 and payload2, percentage of each
#maxAmpduSize=3140
# increasae max ampdu size to maximum to maximize throughput
echo "export maxAmpduSize=65535" >> ../scripts/study1.sh

# params that will vary
# vary n from 5 to 40  in steps of 5
for n in 5 10 15 20 25 30 35 40 ; do
#    for offeredLoad in 0.2 0.4 0.6 0.8 1.0 1.2 ; do
    # offered load for higher mcs
    echo "export n=${n}" >> ../scripts/study1.sh
    for offeredLoad in 1.0 2.0 3.0 4.0 5.0 ; do
        echo "export offeredLoad=${offeredLoad}" >> ../scripts/study1.sh
        ol1=$(awk "BEGIN {print $offeredLoad*1}")
        # uplink is 90% of total offered load
        uplink=$(awk "BEGIN {print $offeredLoad*0.9}")
        echo "export uplink=${uplink}" >> ../scripts/study1.sh
        # downlink is 10% of total offered load
        downlink=$(awk "BEGIN {print $offeredLoad*0.1}")
        echo "export downlink=${downlink}" >> ../scripts/study1.sh
        d1=$(awk "BEGIN {print $d*100}")
        ul1=$(awk "BEGIN {print $uplink*100}")
        dl1=$(awk "BEGIN {print $downlink*100}")
        test=$(printf "study1-%0.f-%02d-%02d-%0.1f-%0.f-%0.f\n" ${d1} ${r} ${n} ${ol1} ${ul1} ${dl1})
        echo "export test=${test}" >> ../scripts/study1.sh
        echo "# run $test" >> ../scripts/study1.sh
        echo "run_one" >> ../scripts/study1.sh
        echo "" >> ../scripts/study1.sh
    done
done

chmod +x ../scripts/study1.sh

echo "# the script '../scripts/study1.sh' has been created."
echo "# to run the study 1 simulations, you should run ./study1.sh"

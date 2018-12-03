#!/bin/bash
# These are the scenarios for spatial-reuse study2 (802.11ax).
# Results will be documented.

rm ./results/spatial-reuse-study2*.*
rm ./results/spatial-reuse-SR-stats-study2*.*

trap ctrl_c INT

function ctrl_c() {
         echo "Trapped CTRL-C, exiting..."
         exit 1
}

source spatial-reuse-functions.sh

rm ../scripts/study2.sh
echo "# import helper scripts" >> ../scripts/study2.sh
echo "source spatial-reuse-functions.sh" >> ../scripts/study2.sh
echo "" >> ../scripts/study2.sh
echo "cd ../examples" >> ../scripts/study2.sh
echo "" >> ../scripts/study2.sh

# common parameters
echo "export RngRun=1" >> ../scripts/study2.sh
echo "export powSta=15" >> ../scripts/study2.sh
echo "export powAp=20" >> ../scripts/study2.sh
# to be changed to TBD. set to low number for now, for testing so that script completes
echo "export duration=10.0" >> ../scripts/study2.sh
echo "export enableRts=0" >> ../scripts/study2.sh
# 802.11ax
echo "export standard=11ax_5GHZ" >> ../scripts/study2.sh
# TBD - use ideal channel model
echo "export MCS=0" >> ../scripts/study2.sh
# need to have payload1 and payload2, percentage of each
echo "export payloadSizeUplink=1500" >> ../scripts/study2.sh
echo "export payloadSizeDownlink=300" >> ../scripts/study2.sh
echo "export txStartOffset=5" >> ../scripts/study2.sh
echo "export enableObssPd=1" >> ../scripts/study2.sh
echo "export txGain=0" >> ../scripts/study2.sh
echo "export rxGain=0" >> ../scripts/study2.sh
echo "export antennas=1" >> ../scripts/study2.sh
echo "export maxSupportedTxSpatialStreams=1" >> ../scripts/study2.sh
echo "export maxSupportedRxSpatialStreams=1" >> ../scripts/study2.sh
echo "export performTgaxTimingChecks=0" >> ../scripts/study2.sh
echo "export maxAmpduSize=3140" >> ../scripts/study2.sh
echo "export nodePositionsFile=NONE" >> ../scripts/study2.sh
echo "export enablePcap=0" >> ../scripts/study2.sh
echo "export enableAscii=0" >> ../scripts/study2.sh
echo "export bw=20" >> ../scripts/study2.sh
# for starters, base everything off of the Residential Scenerio
echo "export scenario=study2" >> ../scripts/study2.sh
echo "export txRange=15" >> ../scripts/study2.sh
echo "export useIdealWifiManager=0" >> ../scripts/study2.sh

cd ../examples

# Run the test(s) for Spatial-Reuse study2
echo "export d=17.32" >> ../scripts/study2.sh
echo "export r=10" >> ../scripts/study2.sh
echo "export nBss=7" >> ../scripts/study2.sh

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
            echo "export uplink=${uplink}" >> ../scripts/study2.sh
            # downlink is 10% of total offered load
            downlink=$(awk "BEGIN {print $offeredLoad*0.1}")
            echo "export downlink=${downlink}" >> ../scripts/study2.sh
            d1=$(awk "BEGIN {print $d*100}")
            ul1=$(awk "BEGIN {print $uplink*100}")
            dl1=$(awk "BEGIN {print $downlink*100}")
            test=$(printf "study2-%0.f-%02d-%02d-%04d-%0.f-%0.f%0.f\n" ${d1} ${r} ${n} ${offeredLoad} ${ul1} ${dl1} ${obssPdThreshold})
            echo "export test=${test}" >> ../scripts/study2.sh
            echo "# run $test" >> ../scripts/study2.sh
            echo "run_one" >> ../scripts/study2.sh
            echo "" >> ../scripts/study2.sh
        done
    done
done

chmod +x ../scripts/study2.sh

echo "# the script '../scripts/study2.sh' has been created."
echo "# to run the study 2 simulations, you should run ./study2.sh"

#!/bin/bash

trap ctrl_c INT

function ctrl_c() {
         echo "Trapped CTRL-C, exiting..."
         exit 1
}

source spatial-reuse-functions.sh

# Run the test(s) for Box5 Scenarios for calibration of MAC simulator (page 56+)
# common parameters
export RngRun=1
export powSta=15
export powAp=20
export duration=3.0
export enableRts=0
export standard=11ax_5GHZ
export bw=80
# MCS is 5
export MCS=5
export payloadSizeUplink=1500
export payloadSizeDownlink=1500
export txStartOffset=5
export enableObssPd=1
export txGain=0
export rxGain=0
export antennas=1
export maxSupportedTxSpatialStreams=1
export maxSupportedRxSpatialStreams=1
export performTgaxTimingChecks=1
export scenario=indoor
export nBss=1
export maxAmpduSize=65535
export nodePositionsFile=src/wifi-he/scripts/box5.ns2
export enablePcap=0
export enableAscii=0
export obssPdThreshold=-82
export useIdealWifiManager=0

cd ../examples

# @TODO 
# MSDU length:[0:500:2000Bytes]
# MCS [0,8]
# Time traces
# specific location of APs, STAs ??? (Deferral Tests, NAV Test
# PER = 0%
# output metrics:
#  CDF or Histogram of per non-AP STA throughput
#  PER of all AP/STA (# success / # transmitted)

# Scenarios for calibration of Box5 simulator

# TODO - need to handle only 1 BSS or 2 BSS cases separately

# Box5 - 1BSS DL only
export test=calibration-box5-1bss-dl-only
export d=1000
export r=18
export nBss=1
export n=5
# CBR=10^8
export uplink=0.0
export downlink=500.0
export txRange=54
run_one &

# Box5 - 1BSS UL only 1 STA
export test=calibration-box5-1bss-ul-1sta
export d=1000
export r=18
export nBss=1
export n=5
# CBR=10^8
export uplink=500.0
export downlink=0.0
export txRange=54
run_one &

# Box5 - 1BSS UL only 2 STA
export test=calibration-box5-1bss-ul-2sta
export d=1000
export r=18
export nBss=1
export n=2
# CBR=10^8
# total load is allocated prorata to each STA, so 100 Mbit/s each x 2 = 200 Mbit/s total
export uplink=200.0
export downlink=0.0
export txRange=54
run_one &

# Box5 - 1BSS UL only 3 STA
export test=calibration-box5-1bss-ul-3sta
export d=1000
export r=18
export nBss=1
export n=3
# CBR=10^8
# total load is allocated prorata to each STA, so 100 Mbit/s each x 3 = 300 Mbit/s total
export uplink=300.0
export downlink=0.0
export txRange=54
run_one &

wait

# Box5 - 1BSS DL and UL
export test=calibration-box5-1bss-dl-and-ul
export d=1000
export r=50
export nBss=1
export n=5
# CBR=10^8
export uplink=100.0
export downlink=100.0
export txRange=54
run_one &

# Box5 - 2BSS Both DL only
export test=calibration-box5-2bss-dl-only
export d=1000
export r=18
export nBss=2
export n=5
# CBR=10^8
export uplink=0.0
export downlink=500.0
export txRange=54
run_one &

# Box5 - 2BSS Both UL only
export test=calibration-box5-2bss-ul-only
export d=1000
export r=18
export nBss=2
export n=5
# CBR=10^8
export uplink=500.0
export downlink=0.0
export txRange=54
run_one &

# TODO - need to handle the cases where BSS1 is UL while BSS2 is DL

# Box5 - 2BSS A DL and B UL
export test=calibration-box5-2bss-a-dl-b-ul
export d=1000
export r=18
export nBss=2
export n=5
# CBR=10^8
export uplink=500.0
export downlink=0.0
export txRange=54
run_one &

# Box5 - 2BSS A UL and B DL
export test=calibration-box5-2bss-a-dl-b-ul
export d=1000
export r=50
export nBss=2
export n=5
# CBR=10^8
export uplink=500.0
export downlink=0.0
export txRange=54
run_one &

wait

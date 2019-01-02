#!/bin/bash

trap ctrl_c INT

function ctrl_c() {
         echo "Trapped CTRL-C, exiting..."
         exit 1
}

source spatial-reuse-functions.sh

# Run the test(s) for Scenarios for calibration of MAC simulator
# common parameters
export RngRun=1
export powSta=15
export powAp=20
export duration=10.0
export enableRts=0
export standard=11ax_5GHZ
export bw=20
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
export performTgaxTimingChecks=1
export scenario=residential
export nBss=2
export maxAmpduSize=3142
export nodePositionsFile=NONE
export enablePcap=0
export enableAscii=0
export obssPdThreshold=-82
export useIdealWifiManager=0

cd ../examples

# @TODO 
# MSDU length:[0:500:2000Bytes]
# MCS [0,8]
# Time traces
# make 0 offset on Tx, so that all nodes send at same time.
# specific location of APs, STAs ??? (Deferral Tests, NAV Test
# PER = 0%

# Test 1a - MAC overhead w/o RTS/CTS
export test=calibration-test1a
export d=1000
export r=30
export nBss=1
export n=1
export uplink=10.0
export downlink=0.0
export txRange=54
run_one &

# Test 1b - MAC overhead w/o RTS/CTS
export test=calibration-test1b
export d=1000
export r=30
export nBss=1
export n=1
export uplink=10.0
export downlink=0.0
export enableRts=1
export txRange=54
run_one &

# Test 2a - Deferral Test 1
export test=calibration-test2a
export d=10
export r=10
export nBss=2
export n=1
export uplink=10.0
export downlink=0.0
# RTS=[OFF, ON]
export enableRts=0
export txRange=54
run_one &

# Test 2b - Deferral Test 2
export test=calibration-test2b
export d=20
export r=10
export nBss=2
export n=1
export uplink=10.0
export downlink=0.0
# RTS=OFF for this test
export enableRts=0
export txRange=54
run_one &

# Test 3 - NAV Deferral
# same as Test 2b, but with RTS/CTS ON
export test=calibration-test3
export d=20
export r=10
export nBss=2
export n=1
export uplink=10.0
export downlink=0.0
# RTS=ON for this test
export enableRts=1
export txRange=54
run_one &

wait

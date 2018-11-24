#!/bin/bash

source spatial-reuse-functions.sh

# Run the test(s) for Scenarios for calibration of MAC simulator
# common parameters
RngRun=1
powSta=15
powAp=20
duration=10.0
enableRts=0
standard=11ax_5GHZ
bw=20
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
performTgaxTimings=1
scenario=residential
nBss=2
maxAmpduSize=3140
nodePositionsFile=NONE
enablePcap=0
enableAscii=0

cd ../examples

# @TODO 
# MSDU length:[0:500:2000Bytes]
# MCS [0,8]
# Time traces
# make 0 offset on Tx, so that all nodes send at same time.
# specific location of APs, STAs ??? (Deferral Tests, NAV Test
# PER = 0%

# Test 1a - MAC overhead w/o RTS/CTS
test=calibration-test1a
d=1000
r=50
n=1
uplink=10.0
downlink=0.0
txRange=54
run_one "$test" "$RngRun" "$powSta" "$powAp" "$duration" "$d" "$r" "$n" "$uplink" "$downlink" "$enableRts" "$standard" "$bw" "$txRange" "$MCS" "$payloadSizeUplink" "$payloadSizeDownlink" "$txStartOffset" "$enableObssPd" "$txGain" "$rxGain" "$antennas" "$maxSupportedTxSpatialStreams" "$maxSupportedRxSpatialStreams" "$performTgaxTimings" "$scenario" "$nBss" "$maxAmpduSize" "$nodePositionsFile" "$enablePcap" "$enableAscii"

# Test 1b - MAC overhead w/o RTS/CTS
test=calibration-test1b
d=1000
r=50
n=1
uplink=10.0
downlink=0.0
enableRts=1
txRange=54
run_one "$test" "$RngRun" "$powSta" "$powAp" "$duration" "$d" "$r" "$n" "$uplink" "$downlink" "$enableRts" "$standard" "$bw" "$txRange" "$MCS" "$payloadSizeUplink" "$payloadSizeDownlink" "$txStartOffset" "$enableObssPd" "$txGain" "$rxGain" "$antennas" "$maxSupportedTxSpatialStreams" "$maxSupportedRxSpatialStreams" "$performTgaxTimings" "$scenario" "$nBss" "$maxAmpduSize" "$nodePositionsFile" "$enablePcap" "$enableAscii"

# Test 2a - Deferral Test 1
test=calibration-test2a
d=10
r=10
n=1
uplink=10.0
downlink=0.0
# RTS=[OFF, ON]
enableRts=0
txRange=54
run_one "$test" "$RngRun" "$powSta" "$powAp" "$duration" "$d" "$r" "$n" "$uplink" "$downlink" "$enableRts" "$standard" "$bw" "$txRange" "$MCS" "$payloadSizeUplink" "$payloadSizeDownlink" "$txStartOffset" "$enableObssPd" "$txGain" "$rxGain" "$antennas" "$maxSupportedTxSpatialStreams" "$maxSupportedRxSpatialStreams" "$performTgaxTimings" "$scenario" "$nBss" "$maxAmpduSize" "$nodePositionsFile" "$enablePcap" "$enableAscii"

# Test 2b - Deferral Test 2
test=calibration-test2b
d=20
r=10
n=1
uplink=10.0
downlink=0.0
# RTS=OFF for this test
enableRts=0
txRange=54
run_one "$test" "$RngRun" "$powSta" "$powAp" "$duration" "$d" "$r" "$n" "$uplink" "$downlink" "$enableRts" "$standard" "$bw" "$txRange" "$MCS" "$payloadSizeUplink" "$payloadSizeDownlink" "$txStartOffset" "$enableObssPd" "$txGain" "$rxGain" "$antennas" "$maxSupportedTxSpatialStreams" "$maxSupportedRxSpatialStreams" "$performTgaxTimings" "$scenario" "$nBss" "$maxAmpduSize" "$nodePositionsFile" "$enablePcap" "$enableAscii"

# Test 3 - NAV Deferral
# same as Test 2b, but with RTS/CTS ON
test=calibration-test3
d=20
r=10
n=1
uplink=10.0
downlink=0.0
# RTS=ON for this test
enableRts=1
txRange=54
run_one "$test" "$RngRun" "$powSta" "$powAp" "$duration" "$d" "$r" "$n" "$uplink" "$downlink" "$enableRts" "$standard" "$bw" "$txRange" "$MCS" "$payloadSizeUplink" "$payloadSizeDownlink" "$txStartOffset" "$enableObssPd" "$txGain" "$rxGain" "$antennas" "$maxSupportedTxSpatialStreams" "$maxSupportedRxSpatialStreams" "$performTgaxTimings" "$scenario" "$nBss" "$maxAmpduSize" "$nodePositionsFile" "$enablePcap" "$enableAscii"


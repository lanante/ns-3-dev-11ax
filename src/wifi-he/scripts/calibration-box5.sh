#!/bin/bash

source spatial-reuse-functions.sh

# Run the test(s) for Box5 Scenarios for calibration of MAC simulator (page 56+)
# common parameters
RngRun=1
powSta=15
powAp=20
duration=3.0
enableRts=0
standard=11ax_5GHZ
bw=80
# MCS is 5
MCS=5
payloadSizeUplink=1500
payloadSizeDownlink=1500
txStartOffset=5
enableObssPd=1
txGain=0
rxGain=0
antennas=1
maxSupportedTxSpatialStreams=1
maxSupportedRxSpatialStreams=1
performTgaxTimingChecks=1
scenario=indoor
nBss=1
maxAmpduSize=65535
nodePositionsFile=src/wifi-he/scripts/box5.ns2
enablePcap=0
enableAscii=0
obssPdThreshold=-82

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
test=calibration-box5-1bss-dl-only
d=1000
r=18
n=5
# CBR=10^8
uplink=0.0
downlink=500.0
txRange=54
run_one "$test" "$RngRun" "$powSta" "$powAp" "$duration" "$d" "$r" "$n" "$uplink" "$downlink" "$enableRts" "$standard" "$bw" "$txRange" "$MCS" "$payloadSizeUplink" "$payloadSizeDownlink" "$txStartOffset" "$enableObssPd" "$txGain" "$rxGain" "$antennas" "$maxSupportedTxSpatialStreams" "$maxSupportedRxSpatialStreams" "$performTgaxTimingChecks" "$scenario" "$nBss" "$maxAmpduSize" "$nodePositionsFile" "$enablePcap" "$enableAscii" "$obssPdThreshold"

# Box5 - 1BSS UL only 1 STA
test=calibration-box5-1bss-ul-1sta
d=1000
r=18
n=5
# CBR=10^8
uplink=500.0
downlink=0.0
txRange=54
run_one "$test" "$RngRun" "$powSta" "$powAp" "$duration" "$d" "$r" "$n" "$uplink" "$downlink" "$enableRts" "$standard" "$bw" "$txRange" "$MCS" "$payloadSizeUplink" "$payloadSizeDownlink" "$txStartOffset" "$enableObssPd" "$txGain" "$rxGain" "$antennas" "$maxSupportedTxSpatialStreams" "$maxSupportedRxSpatialStreams" "$performTgaxTimingChecks" "$scenario" "$nBss" "$maxAmpduSize" "$nodePositionsFile" "$enablePcap" "$enableAscii" "$obssPdThreshold"

# Box5 - 1BSS UL only 2 STA
test=calibration-box5-1bss-ul-2sta
d=1000
r=18
n=2
# CBR=10^8
# total load is allocated prorata to each STA, so 100Mbps each x 2 = 200 Mbps total
uplink=200.0
downlink=0.0
txRange=54
run_one "$test" "$RngRun" "$powSta" "$powAp" "$duration" "$d" "$r" "$n" "$uplink" "$downlink" "$enableRts" "$standard" "$bw" "$txRange" "$MCS" "$payloadSizeUplink" "$payloadSizeDownlink" "$txStartOffset" "$enableObssPd" "$txGain" "$rxGain" "$antennas" "$maxSupportedTxSpatialStreams" "$maxSupportedRxSpatialStreams" "$performTgaxTimingChecks" "$scenario" "$nBss" "$maxAmpduSize" "$nodePositionsFile" "$enablePcap" "$enableAscii" "$obssPdThreshold"

# Box5 - 1BSS UL only 3 STA
test=calibration-box5-1bss-ul-3sta
d=1000
r=18
n=3
# CBR=10^8
# total load is allocated prorata to each STA, so 100Mbps each x 3 = 300 Mbps total
uplink=300.0
downlink=0.0
txRange=54
run_one "$test" "$RngRun" "$powSta" "$powAp" "$duration" "$d" "$r" "$n" "$uplink" "$downlink" "$enableRts" "$standard" "$bw" "$txRange" "$MCS" "$payloadSizeUplink" "$payloadSizeDownlink" "$txStartOffset" "$enableObssPd" "$txGain" "$rxGain" "$antennas" "$maxSupportedTxSpatialStreams" "$maxSupportedRxSpatialStreams" "$performTgaxTimingChecks" "$scenario" "$nBss" "$maxAmpduSize" "$nodePositionsFile" "$enablePcap" "$enableAscii" "$obssPdThreshold"

# Box5 - 1BSS DL and UL
test=calibration-box5-1bss-dl-and-ul
d=1000
r=50
n=5
# CBR=10^8
uplink=100.0
downlink=100.0
txRange=54
run_one "$test" "$RngRun" "$powSta" "$powAp" "$duration" "$d" "$r" "$n" "$uplink" "$downlink" "$enableRts" "$standard" "$bw" "$txRange" "$MCS" "$payloadSizeUplink" "$payloadSizeDownlink" "$txStartOffset" "$enableObssPd" "$txGain" "$rxGain" "$antennas" "$maxSupportedTxSpatialStreams" "$maxSupportedRxSpatialStreams" "$performTgaxTimingChecks" "$scenario" "$nBss" "$maxAmpduSize" "$nodePositionsFile" "$enablePcap" "$enableAscii" "$obssPdThreshold"

# Box5 - 2BSS Both DL only
test=calibration-box5-2bss-dl-only
d=1000
r=18
n=5
# CBR=10^8
uplink=0.0
downlink=500.0
txRange=54
run_one "$test" "$RngRun" "$powSta" "$powAp" "$duration" "$d" "$r" "$n" "$uplink" "$downlink" "$enableRts" "$standard" "$bw" "$txRange" "$MCS" "$payloadSizeUplink" "$payloadSizeDownlink" "$txStartOffset" "$enableObssPd" "$txGain" "$rxGain" "$antennas" "$maxSupportedTxSpatialStreams" "$maxSupportedRxSpatialStreams" "$performTgaxTimingChecks" "$scenario" "$nBss" "$maxAmpduSize" "$nodePositionsFile" "$enablePcap" "$enableAscii" "$obssPdThreshold"

# Box5 - 2BSS Both UL only
test=calibration-box5-2bss-ul-only
d=1000
r=18
n=5
# CBR=10^8
uplink=500.0
downlink=0.0
txRange=54
run_one "$test" "$RngRun" "$powSta" "$powAp" "$duration" "$d" "$r" "$n" "$uplink" "$downlink" "$enableRts" "$standard" "$bw" "$txRange" "$MCS" "$payloadSizeUplink" "$payloadSizeDownlink" "$txStartOffset" "$enableObssPd" "$txGain" "$rxGain" "$antennas" "$maxSupportedTxSpatialStreams" "$maxSupportedRxSpatialStreams" "$performTgaxTimingChecks" "$scenario" "$nBss" "$maxAmpduSize" "$nodePositionsFile" "$enablePcap" "$enableAscii" "$obssPdThreshold"

# TODO - need to handle the cases where BSS1 is UL while BSS2 is DL

# Box5 - 2BSS A DL and B UL
test=calibration-box5-2bss-a-dl-b-ul
d=1000
r=18
n=5
# CBR=10^8
uplink=500.0
downlink=0.0
txRange=54
run_one "$test" "$RngRun" "$powSta" "$powAp" "$duration" "$d" "$r" "$n" "$uplink" "$downlink" "$enableRts" "$standard" "$bw" "$txRange" "$MCS" "$payloadSizeUplink" "$payloadSizeDownlink" "$txStartOffset" "$enableObssPd" "$txGain" "$rxGain" "$antennas" "$maxSupportedTxSpatialStreams" "$maxSupportedRxSpatialStreams" "$performTgaxTimingChecks" "$scenario" "$nBss" "$maxAmpduSize" "$nodePositionsFile" "$enablePcap" "$enableAscii" "$obssPdThreshold"

# Box5 - 2BSS A UL and B DL
test=calibration-box5-2bss-a-dl-b-ul
d=1000
r=50
n=5
# CBR=10^8
uplink=500.0
downlink=0.0
txRange=54
run_one "$test" "$RngRun" "$powSta" "$powAp" "$duration" "$d" "$r" "$n" "$uplink" "$downlink" "$enableRts" "$standard" "$bw" "$txRange" "$MCS" "$payloadSizeUplink" "$payloadSizeDownlink" "$txStartOffset" "$enableObssPd" "$txGain" "$rxGain" "$antennas" "$maxSupportedTxSpatialStreams" "$maxSupportedRxSpatialStreams" "$performTgaxTimingChecks" "$scenario" "$nBss" "$maxAmpduSize" "$nodePositionsFile" "$enablePcap" "$enableAscii" "$obssPdThreshold"


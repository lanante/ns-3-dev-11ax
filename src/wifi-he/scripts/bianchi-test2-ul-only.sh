# import helper scripts
source spatial-reuse-functions.sh

cd ../examples

# miscellaneous settings for spatial-reuse script / ns-3
export RngRun=1
export duration=5.0
export enableRts=0
export txStartOffset=50
export enableObssPd=0
export txGain=0
export rxGain=0
export antennas=1
export maxSupportedTxSpatialStreams=1
export maxSupportedRxSpatialStreams=1
export performTgaxTimingChecks=0
export nodePositionsFile=NONE
export enablePcap=0
export enableAscii=0
export rxSensitivity=-82
# only used by 11ax
export obssPdThreshold=-82

# see Saturaged Throughtput Analysis/Calibration for Multi BSS Throughput (pptx)
# Leonardo Lanante Jr.

# Reference Scenario
# Format= 11ac, 20MHz, 1 stream
export standard=11ac
export bw=20

# Carrier Frequency = 5190MHz (NS-3 default)
# handled by 11ac setting

# Path loss model based on [1]
# PL(d) = 40.05 + 20*log10(fc/2.4) + 20*log10(min(d,10)) + (d>10) * 35*log10(d/10) 
# –	d = max(3D-distance [m], 1)
# –	fc = frequency [GHz]
export scenario=study1
export nBss=7

# MSDUsize = 1500 octets
export payloadSizeUplink=1500
export payloadSizeDownlink=1500

# AMPDUsize = 3140 octets (nMPDU=2)
export maxAmpduSize=3142

# MCS=7 (rate = 65 Mbit/s, Ndbps = 260)
export useIdealWifiManager=0
export MCS=7

# EDCA parameters: Best Effort
# AIFSn=3
# CWmin=3
# unchanged from ns-3 defaults

# Dropping radius r=10m
export r=10

# Transmit Power = 20dBm for AP, 15dBm for STA
export powSta=15
export powAp=20

# CSR = 102m for AP
export txRange=102

# Uncoupled BSSes with all UL throughput
# UL traffic = 100 Mbit/s per STA
# DL traffic = 0 Mbit/s
# r= 10m
# d=120m
# nBSS=7
# nSTA=5:5:40 per BSS

export downlink=0.0
export d=120
# no shadowing loss
export sigma=0.0
export bianchi=1

echo "Expected Throughput for n=5: 38.4 Mbit/s per BSS"
export uplink=500.0
export n=5
export test="bianchi-test2-ul-only-n5"
run_one &

echo "Expected Throughput for n=10: 35.7 Mbit/s per BSS"
export uplink=1000.0
export n=10
export test="bianchi-test2-ul-only-n10"
run_one &

echo "Expected Throughput for n=15: 34.1 Mbit/s per BSS"
export uplink=1500.0
export n=15
export test="bianchi-test2-ul-only-n15"
run_one &

echo "Expected Throughput for n=20: 33.0 Mbit/s per BSS"
export uplink=2000.0
export n=20
export test="bianchi-test2-ul-only-n20"
run_one &

echo "Expected Throughput for n=25: 32.1 Mbit/s per BSS"
export uplink=2500.0
export n=25
export test="bianchi-test2-ul-only-n25"
run_one &

echo "Expected Throughput for n=30: 31.3 Mbit/s per BSS"
export uplink=3000.0
export n=30
export test="bianchi-test2-ul-only-n30"
run_one &

echo "Expected Throughput for n=35: 30.7 Mbit/s per BSS"
export uplink=3500.0
export n=35
export test="bianchi-test2-ul-only-n35"
run_one &

echo "Expected Throughput for n=40: 30.1 Mbit/s per BSS"
export uplink=4000.0
export n=40
export test="bianchi-test2-ul-only-n40"
run_one &

wait

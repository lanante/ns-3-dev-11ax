# Compare results for OBSS_PD by
# running 3 scenarios, for n=5 STA per each of 7 BSS:
# 1. 11ac (baseline, no OBSS_PD)
# 2. 11ax (with OBSS_PD disabled)
# 3. 11ax (with OBSS_PD enabled at obssPdThreshold=-82

# import helper scripts
source spatial-reuse-functions.sh

cd ../examples

# miscellaneous settings for spatial-reuse script / ns-3
export RngRun=1
# maybe change duration to 10.0, or more ???
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
# only used by 11ax
export obssPdThreshold=-82


# see Saturaged Throughtput Analysis/Calibration for Multi BSS Throughput (pptx)
# Leonardo Lanante Jr.

# Slide 2
# Refeence Scenario
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

# MCS=7 (Max rate= 6.5Mbps, Ndbps=260)
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

# CSR = 102m for AP - see slide 3
export txRange=102

# Slide 11
# 2. Uncoupled BSSes with all UL throughput
# Set
# UL traffic = 100Mbps per BSS
# DL traffic  = 0Mbps
# r= 10m
# d=120m
# nBSS=7
# nSTA=5:5:40 per BSS
# Expected Throughput = 38.6Mbps per BSS (n=1 in slide 9)

# for uplink, allocate traffic per STA x n STAs
export downlink=0.0
# based on Bianchi Test4 - fully coupled BSS with saturated uplink
export d=20
export sigma=0.0
export bianchi=1
export rxSensitivity=-82

# all 3 scenarios are for n=5 STA
export uplink=500.0
export n=5

# 1. 11ac (baseline, no OBSS_PD)
export standard=11ac
export test="obss-pd-test1-n5-11ac"
run_one &

# 2. 11ax (with OBSS_PD disabled)
export standard=11ax_5GHZ
export enableObssPd=0
export test="obss-pd-test1-n5-11ax-no-obsspd"
run_one &

# 3. 11ax (with OBSS_PD enabled at obssPdThreshold=-82
export standard=11ax_5GHZ
export enableObssPd=1
export obssPdThreshold=-82
export test="obss-pd-test1-n5-11ax-with-obsspd"
run_one &

wait

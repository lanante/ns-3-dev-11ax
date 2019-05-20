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
export obssPdAlgorithm=DynamicObssPdAlgorithm
# only used by 11ax
export obssPdThreshold=-72
export AUTO_DELETE_SPATIAL_REUSE_OUTPUT_FILES=1
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
export scenario=logdistance
export nBss=7

# MSDUsize = 1500 octets
export payloadSizeUplink=1500
export payloadSizeDownlink=1500

# AMPDUsize = 3142 octets (nMPDU=2)
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
export ccaTrSta=-62
export ccaTrAp=-62
export downlink=0.0
# based on Bianchi Test4 - fully coupled BSS with saturated uplink
export d=20
export sigma=0.0
export bianchi=1

# all 3 scenarios are for n=5 STA
export uplink=500.0
export n=5

# 1. 11ax (with OBSS_PD enabled at obssPdThreshold=-72
export standard=11ax_5GHZ
export enableObssPd=1
export obssPdThreshold=-72
export test="obss-pd-test1-n5-11ax-with-obsspd"
run_one 

# 2. 11ac (baseline, no OBSS_PD)
export standard=11ac
export test="obss-pd-test1-n5-11ac"
run_one 

# 3. 11ax (with OBSS_PD disabled)
export standard=11ax_5GHZ
export enableObssPd=0
export test="obss-pd-test1-n5-11ax-no-obsspd"
run_one 



wait

# This script is based on the
# Bianchi Test2 fully-decoupled saturated uplink
# test scenario, and is modified to test the
# sensitivity of the parmameters:
# --duration
# --appliationTxStart
# --RngRun
#
# These parameters control the spatial-reuse script:
#
# --duration is the simulation time that nodes are transmitting
# and is the time used in calculating the throughput
#
# --applicationTxStart is the simulation time that the
# network is allowed to readch a steady state before
# the nodes begin to transmit packets
#
# the total ns-3 simulation time is thus applicationTxStart + duration
#
# --RngRun is the number used to seed the ns-3 RNG
#
# Approach:
#
# The Bianchi Test2 is used with n=10 STAs.
#
# The following simulation parameters will be varied individually as follows:
# Parameter              Sensitivity Values
# --duration:            1, 5, 10, 20
# --applicationTxStart:  1, 5, 10
# --RngRun:              1, 88, 101
#
# Results:
#
#-----------------------------------------------------------
# Sensitivity to --duration
# --applicationStartTx   --duration   --RngRun    Throughput
# 1                      1            1           27.288
# 1                      5            1           28.75
# 1                      10           1           28.53
# 1                      20           1           28.80
#
# Analysis and Discussion
# The average throughput of these experiments is 28.34 Mb/s.
# The result for --duration=1 is 3.72% lower than this average,
# and the other experiments vary by up to 1.62% from that average.
# If we exclude the data point for --duration=1, then the average
# of the remaining 3 experiments (i.e., duration 5-20s) is
# 28.69 Mb/s, and all 3 experiments vary by no more than 0.57%
# from that average.
#
# Conclusion:
# A short duration of 1s should be avoided.  Other durations
# of 5-20s do not appear to effect the resulting throughput.
#
#-----------------------------------------------------------
# Sensitivity to --applicationTxStart
# --applicationStartTx   --duration   --RngRun    Throughput
# 1                      20           1           28.80
# 5                      20           1           28.60
# 10                     20           1           28.54
#
# Analysis and Discussion
# The average of these experiments is 28.65 Mb/s, and all 3 experiments
# vary by no more than 0.54% from that average.
#
# Conclusion:
# Throughput results are not affected by --applicationTxStart values
# greater than 1s.  Thus, 1s of start-up-time seems sufficient
# for the network to read a steady state before node begin
# transmitting packets
#
#-----------------------------------------------------------
# Sensitivity to --RngRun
# --applicationStartTx   --duration   --RngRun    Throughput
# 1                      5            1           28.78
# 1                      5            88          28.52
# 1                      5            101         28.48
#
# Analysis and Discussion
# The average of these experiments is 28.58 Mb/s, and all 3 experiments
# vary by no more than 0.58% from that average.
#
# Conclusion:
# Throughput results are not affected by --RngRun values.
# Thus, it is unlikely that varying the value of RngRun and
# averaging results will significantly influence results.
#
#-----------------------------------------------------------
# Summary:
#
# A well-validated spatial-reuse scenario was repeated with
# parametric variation of a) the simulation duration time
# used to generate packet transmissions and measure throughput,
# b) the time to allow the network to ready a steady-state
# before such packet transmissions are started, and c) the
# value of RngRun that seeds the ns-3 RNG.
#
# The results of this sensitivity study implies that values
# of --applicationStartTx=1 and --duration=<any value >= 5>
# and for any value of --RngRun will yield results that vary
# insignificantly from averages (i.e., will likely be within
# 0.58% of the averages of different runs that vary such
# parameters).
#
#-----------------------------------------------------------
#
# Limitations and Threats to Validity:
#
# The study is conducted for n=10 STAs.  The study may be
# repeated for other values, i.e., n=20 or n=40 STAs to evaluate
# if the conclusions hold true for higher numbers of n.
#
# Note that this test scenario sets --sigma=0.0 and thus
# eliminates the (randomized) effects of shadowing loss.
# As such, this study does not consider, for example, the
# the impact of varying RngRun when shadowing loss is enabled.
#

# import helper scripts
source spatial-reuse-functions.sh

cd ../examples

# miscellaneous settings for spatial-reuse script / ns-3
export enableRts=0
export txStartOffset=5000
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
# Reference Scenario
# Format= 11ac, 20MHz, 1 stream
export standard=11ac
export bw=20

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

# CSR = 102m for AP - see slide 3
export txRange=102
export rxSensitivity=-91

export downlink=0.0
export sigma=0.0
export bianchi=1

export d=120
# using the indoor prop loss model, and limit this experiement to 1 BSS
export scenario=indoor
export nBss=1

# n=10
export n=10
export uplink=1000.0

# defaults for the values that will be varied
export duration=5
export applicationTxStart=1
export RngRun=1

# Sensitivity Test 1 - vary --duration

# duration = 1s
export duration=1
export test="bianchi-test2-duration1"
run_one &

# duration = 5s
export duration=5
export test="bianchi-test2-duration5"
run_one &

# duration = 10s
export duration=10
export test="bianchi-test2-duration10"
run_one &

# duration = 20s
export duration=20
export test="bianchi-test2-duration20"
run_one &

wait

# Sensitivity Test 2 - vary --applicationTxStart

# start offset = 1s
export applicationTxStart=1
export test="bianchi-test2-start1"
run_one &

# start offset = 5s
export applicationTxStart=5
export test="bianchi-test2-start5"
run_one &

# start offset = 10s
export applicationTxStart=10
export test="bianchi-test2-start10"
run_one &

wait

# defaults for the values that will be varied
export duration=5
export applicationTxStart=1
export RngRun=1

# Sensitivity Test 3 - vary --RngRun

# RngRun=1
export RngRun=1
export test="bianchi-test2-rngrun1"
run_one &

# RngRun=88
export RngRun=88
export test="bianchi-test2-rngrun88"
run_one &

# RngRun=100
export RngRun=101
export test="bianchi-test2-rngrun101"
run_one &

wait

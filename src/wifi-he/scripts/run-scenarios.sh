#!/bin/bash

trap ctrl_c INT

function ctrl_c() {
         echo "Trapped CTRL-C, exiting..."
         exit 1
}

source spatial-reuse-functions.sh

# Run the test(s) for TGax Simulation Scenarios
# common parameters
export RngRun=1
export powSta=15
export powAp=20
export duration=10.0
export enableRts=0
export standard=11ax_5GHZ
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
export performTgaxTimingChecks=0
export nBss=2
export maxAmpduSize=3142
export nodePositionsFile=NONE
export enablePcap=0
export enableAscii=0
export obssPdThreshold=-82
export useIdealWifiManager=0

cd ../examples

# Test: 1 - Residential Scenario
export test=TGax-test1
export d=10
export r=5
export n=10
export uplink=10.0
export downlink=0.0
export bw=80
export txRange=54
export scenario=residential
run_one &

# Test: 2 - Enterprise Scenario
export test=TGax-test2
export d=10
export r=5
export n=100
export uplink=10.0
export downlink=0.0
export bw=80
export txRange=68
# TODO - because this runs so slow.
export duration=2
export scenario=enterprise
run_one &
# TODO restore duration for next test
export duration=10

# Test: 3 - Indoor Small BSS Scenario
export test=TGax-test3
export d=34.64
export r=10
export n=30
export uplink=10.0
export downlink=0.0
export bw=80
export txRange=107
export scenario=indoor
run_one &

# Test: 4 - Outdoor Large BSS Hotspot Scenario
export test=TGax-test4
export d=130
export r=70
export n=50
export uplink=10.0
export downlink=0.0
export bw=80
export txRange=50
export scenario=outdoor
run_one &

wait

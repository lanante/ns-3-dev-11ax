#!/bin/bash

source spatial-reuse-functions.sh

# Run the test(s) for TGax Simulation Scenarios
# common parameters
RngRun=1
powSta=15
powAp=20
duration=10.0
enableRts=0
standard=11ax_5GHZ

cd ../examples

# Test: 1 - Resideential Scenario
test=TGax-test1
d=10
r=5
n=10
uplink=10.0
downlink=0.0
bw=80
txRange=54
run_one "$test" "$RngRun" "$powSta" "$powAP" "$duration" "$d" "$r" "$n" "$uplink" "$downlink" "$enableRts" "$standard" "$bw" "$txRange"

# Test: 2 - Enterprise Scenario
test=TGax-test2
d=10
r=5
n=100
uplink=10.0
downlink=0.0
bw=80
txRange=68
run_one "$test" "$RngRun" "$powSta" "$powAP" "$duration" "$d" "$r" "$n" "$uplink" "$downlink" "$enableRts" "$standard" "$bw" "$txRange"

# Test: 3 - Indoor Small BSS Scenario
test=TGax-test3
d=17.32
r=10
n=30
uplink=10.0
downlink=0.0
bw=80
txRange=107
run_one "$test" "$RngRun" "$powSta" "$powAP" "$duration" "$d" "$r" "$n" "$uplink" "$downlink" "$enableRts" "$standard" "$bw" "$txRange"

# Test: 4 - Outdoor Large BSS Hotspot Scenario
test=TGax-test4
d=130
r=70
n=50
uplink=10.0
downlink=0.0
bw=80
txRange=50
run_one "$test" "$RngRun" "$powSta" "$powAP" "$duration" "$d" "$r" "$n" "$uplink" "$downlink" "$enableRts" "$standard" "$bw" "$txRange"

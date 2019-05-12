#! /usr/bin/env python
## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# A list of C++ examples to run in order to ensure that they remain
# buildable and runnable over time.  Each tuple in the list contains
#
#     (example_name, do_run, do_valgrind_run).
#
# See test.py for more information.
cpp_examples = [
    ("spatial-reuse --RngRun=1 --powSta=15 --powAp=20 --duration=1 --d=34.64 --r=10 --n=5 --uplink=2.7 --downlink=0.3 --standard=11ac --bw=20 --txRange=15 --scenario=indoor --nBss=7 --nodePositionsFile=NONE --useIdealWifiManager=1 --filterOutNonAddbaEstablished=1 --verifyResults=1 --minExpectedThroughputBss1Up=2.6 --maxExpectedThroughputBss1Up=2.7  --minExpectedThroughputBss1Down=0.3 --maxExpectedThroughputBss1Down=0.4", "True", "True"),
    ("spatial-reuse --RngRun=1 --powSta=15 --powAp=20 --duration=1 --d=34.64 --r=10 --n=40 --uplink=2.7 --downlink=0.3 --standard=11ac --bw=20 --txRange=15 --scenario=indoor --nBss=7 --nodePositionsFile=NONE --useIdealWifiManager=1 --filterOutNonAddbaEstablished=1 --verifyResults=1 --minExpectedThroughputBss1Up=2.2 --maxExpectedThroughputBss1Up=2.3 --minExpectedThroughputBss1Down=0.2 --maxExpectedThroughputBss1Down=0.3", "True", "True"),
    ("spatial-reuse --RngRun=1 --powSta=15 --powAp=20 --duration=1 --d=34.64 --r=10 --n=20 --uplink=2.7 --downlink=0.3 --standard=11ax_5GHZ --bw=20 --txRange=15 --scenario=indoor --nBss=7 --nodePositionsFile=NONE --useIdealWifiManager=1 --filterOutNonAddbaEstablished=1 --verifyResults=1 --minExpectedThroughputBss1Up=2.1 --maxExpectedThroughputBss1Up=2.2  --minExpectedThroughputBss1Down=0.33 --maxExpectedThroughputBss1Down=0.34", "True", "True"),
    ("spatial-reuse --RngRun=1 --powSta=15 --powAp=20 --duration=1 --d=34.64 --r=10 --n=35 --uplink=2.7 --downlink=0.3 --standard=11ax_5GHZ --bw=20 --txRange=15 --scenario=indoor --nBss=7 --enableObssPd=1 --obssPdThresholdBss1=-62 --obssPdThresholdBss2=-62 --obssPdThresholdBss2=-62 --obssPdThresholdBss3=-62 --obssPdThresholdBss4=-62 --obssPdThresholdBss5=-62 --obssPdThresholdBss6=-62 --obssPdThresholdBss7=-62 --nodePositionsFile=NONE --useIdealWifiManager=1 --filterOutNonAddbaEstablished=1 --verifyResults=1 --minExpectedThroughputBss1Up=0.8 --maxExpectedThroughputBss1Up=0.9  --minExpectedThroughputBss1Down=0.3 --maxExpectedThroughputBss1Down=0.4", "True", "True"),
    ("spatial-reuse --RngRun=1 --powSta=15 --powAp=20 --duration=1 --d=34.64 --r=10 --n=30 --uplink=2.7 --downlink=0.3 --standard=11ax_5GHZ --bw=20 --txRange=15 --scenario=indoor --nBss=7 --enableObssPd=1 --obssPdThresholdBss1=-72 --obssPdThresholdBss2=-72 --obssPdThresholdBss2=-72 --obssPdThresholdBss3=-72 --obssPdThresholdBss4=-72 --obssPdThresholdBss5=-72 --obssPdThresholdBss6=-72 --obssPdThresholdBss7=-72 --nodePositionsFile=NONE --useIdealWifiManager=1 --filterOutNonAddbaEstablished=1 --verifyResults=1 --minExpectedThroughputBss1Up=1.6 --maxExpectedThroughputBss1Up=1.7  --minExpectedThroughputBss1Down=0.2 --maxExpectedThroughputBss1Down=0.3", "True", "True"),
]

# A list of Python examples to run in order to ensure that they remain
# runnable over time.  Each tuple in the list contains
#
#     (example_name, do_run).
#
# See test.py for more information.
python_examples = []

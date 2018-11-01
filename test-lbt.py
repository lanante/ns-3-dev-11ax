#! /usr/bin/env python
#
# Wrapper around test.py and waf to only run examples/tests of interest
#
import sys
import os
import subprocess

return_code_catcher = 0
stop_on_error = 1

FNULL = open(os.devnull, 'w')

shell_tests = [
    ("test-simple-phy-duration.sh"),
]

examples = [
    ("lte-wifi-simple"),
    ("lte-wifi-simple --cellConfigA=Wifi --cellConfigB=Wifi"),
    ("lte-wifi-simple --cellConfigA=Wifi --cellConfigB=Lte"),
    ("lte-wifi-simple --lteDutyCycle=0.5"),
    ("lte-wifi-simple --lteDutyCycle=0.5 --transport=Tcp"),
    ("wifi-co-channel-networks"),
    ("lte-wifi-indoor --cellConfigA=Lte --cellConfigB=Wifi"),
    ("lte-wifi-indoor --cellConfigA=Wifi --cellConfigB=Wifi"),
    ("lte-wifi-indoor --cellConfigA=Wifi --cellConfigB=Lte"),
    ("lte-wifi-itu-umi-pathloss"),
    ("lte-wifi-outdoor"),
    ("lte-wifi-itu-inh-pathloss"),
    ("lte-wifi-80211ax-pathloss"),
    ("lena-dual-stripe --epc=1 --fadingTrace=src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad --simTime=0.01"),
]

lbt_tests = [
    ("lbt-txop-test"),
    ("lbt-access-manager-ed-threshold"),
    ("lbt-access-manager"),
    ("lte-duty-cycle"),
    ("lte-interference-abs"),
    ("lte-unlicensed-interference"),
    ("spectrum-wifi-phy"),
]

lte_tests = [
    ("lte-antenna"),
    ("lte-cell-selection"),
    ("lte-cqa-ff-mac-scheduler"),
    ("lte-cqi-generation"),
    ("lte-downlink-power-control"),
    ("lte-downlink-sinr"),
    ("lte-epc-e2e-data"),
    ("lte-fdbet-ff-mac-scheduler"),
    ("lte-fdmt-ff-mac-scheduler"),
    ("lte-fdtbfq-ff-mac-scheduler"),
    ("lte-frequency-reuse"),
    ("lte-handover-delay"),
    ("lte-handover-target"),
    ("lte-harq"),
    ("lte-interference"),
    ("lte-interference-fr"),
    ("lte-link-adaptation"),
    ("lte-mimo"),
    ("lte-pathloss-model"),
    ("lte-pf-ff-mac-scheduler"),
    ("lte-phy-error-model"),
    ("lte-pss-ff-mac-scheduler"),
    ("lte-rlc-am-e2e"),
    ("lte-rlc-am-transmitter"),
    ("lte-rlc-um-e2e"),
    ("lte-rlc-um-transmitter"),
    ("lte-rr-ff-mac-scheduler"),
    ("lte-rrc"),
    ("lte-tdbet-ff-mac-scheduler"),
    ("lte-tdmt-ff-mac-scheduler"),
    ("lte-tdtbfq-ff-mac-scheduler"),
    ("lte-test-deactivate-bearer"),
    ("lte-tta-ff-mac-scheduler"),
    ("lte-ue-measurements"),
    ("lte-ue-measurements-handover"),
    ("lte-ue-measurements-piecewise-1"),
    ("lte-ue-measurements-piecewise-2"),
    ("lte-unlicensed-interference"),
    ("lte-uplink-power-control"),
    ("lte-uplink-sinr"),
    ("lte-x2-handover"),
    ("lte-x2-handover-measures"),
    ("lte-earfcn"),
    ("lte-rlc-header"),
    ("lte-spectrum-value-helper")
]

os.chdir ("src/lte-wifi-coexistence/test")
return_code = subprocess.call(["bash", "test-simple-phy-duration.sh"], stdout=FNULL, stderr=subprocess.STDOUT)
if (return_code):
    print("FAIL: test-simple-phy-duration.sh")
    if (stop_on_error):
        exit(return_code)
else:
    print("PASS: test-simple-phy-duration.sh")
return_code_catcher |= return_code
os.chdir ("../../../")

for test in lbt_tests:
    return_code = subprocess.call(["python", "test.py", "-s", test] + sys.argv[1:], stdout=FNULL, stderr=subprocess.STDOUT)
    if (return_code):
        print("FAIL: %s; rerun directly with test.py or test-runner" % test)
        if (stop_on_error):
            exit(return_code)
    else:
        print("PASS: %s" % test)
    return_code_catcher |= return_code

for example in examples:
    program = "python waf --run " + "\"" + example + "\""
    return_code = subprocess.call(program, shell=True, stdout=FNULL, stderr=subprocess.STDOUT)
    if (return_code):
        print("FAIL: example %s; rerun directly with test.py or waf" % example)
        if (stop_on_error):
            exit(return_code)
    else:
        print("PASS: example %s" % example)
    return_code_catcher |= return_code

for test in lte_tests:
    return_code = subprocess.call(["python", "test.py", "-s", test] + sys.argv[1:], stdout=FNULL, stderr=subprocess.STDOUT)
    if (return_code):
        print("FAIL: %s; rerun directly with test.py or test-runner" % test)
        if (stop_on_error):
            exit(return_code)
    else:
        print("PASS: %s" % test)
    return_code_catcher |= return_code

if (return_code_catcher):
    print("Some tests FAILED")
else:
    print("All tests PASSED")
exit(return_code_catcher)


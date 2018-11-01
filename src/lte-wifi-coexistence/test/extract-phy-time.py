#!/usr/bin/python
import sys

# Process lines of the form:
#time(s) nodeId type sender endTime(s) duration(ms)     powerDbm
#2.011000000 4  lte 0 2.011214285 0.214285000 -38.969438230
#2.011000000 4  lte 1 2.011214285 0.214285000 -49.530422026
#2.011000000 4  lte 2 2.011214285 0.214285000 -60.168115762
#2.011000000 4  lte 3 2.011214285 0.214285000 -65.144738373

#
# and extract the total duration of time that LTE and WiFi has a signal 
# At the end of the program, output the LTE duration (seconds), WIFI
# duration (seconds) and total transfer time observed (seconds)

if len(sys.argv) < 2:
    print("Error: filename not provided")
    exit(1);

if len(sys.argv) < 3:
    print("Error: observing node number not provided")
    exit(1);

receiver = sys.argv[2]

first = True

lte_duration = 0
current_lte_start = 0
current_lte_end = 0

wifi_duration = 0
current_wifi_start = 0
current_wifi_end = 0
state = 'IDLE'
last_time = 0
first_time = 0

with open(sys.argv[1], "r") as f:
    for line in f:
        fields = line.split()
        if line.strip().startswith("#"):
            continue
        fields = line.split()
        if len(fields) != 7:
            continue
        if fields[1] != receiver: 
            continue
        if first is True:
            first_time = float(fields[0])
            first = False
        last_time = float(fields[0])
        if state == 'IDLE':
            # This is a new transmission block of either LTE or WiFi
            if (fields[2] == 'lte' or fields[2] == '1'):
                current_lte_start = float(fields[0])
                current_lte_end = float(fields[4])
                state = 'LTE'
            elif (fields[2] == 'wifi' or fields[2] == '0'):
                current_wifi_start = float(fields[0])
                current_wifi_end = float(fields[4])
                state = 'WIFI'
            else:
                print("Error on field 2: %s" % fields[2])
                exit(1)
        elif state == 'LTE':
            if (fields[2] == 'lte' or fields[2] == '1'):
                if float(fields[0]) > current_lte_end:
                    # LTE completed, then a new LTE transmission started
                    lte_duration += (current_lte_end - current_lte_start)
                    current_lte_start = float(fields[0])
                    current_lte_end = float(fields[4])
                else:
                    # extend current_lte_end
                    current_lte_end = float(fields[4])
            elif (fields[2] == 'wifi' or fields[2] == '0'):
                if float(fields[0]) > current_lte_end:
                    # LTE completed, then a new Wi-Fi transmission started
                    lte_duration += (current_lte_end - current_lte_start)
                    current_wifi_start = float(fields[0])
                    current_wifi_end = float(fields[4])
                    state = 'WIFI'
                else:
                    # this wifi signal overlaps; ignore it and stay in LTE
                    state = 'LTE'
        elif state == 'WIFI':
            if (fields[2] == 'wifi' or fields[2] == '0'):
                if float(fields[0]) > current_wifi_end:
                    # WIFI completed, then a new WIFI transmission started
                    wifi_duration += (current_wifi_end - current_wifi_start)
                    current_wifi_start = float(fields[0])
                    current_wifi_end = float(fields[4])
                else:
                    # extend current_wifi_end
                    current_wifi_end = float(fields[4])
            elif (fields[2] == 'lte' or fields[2] == '1'):
                if float(fields[0]) > current_wifi_end:
                    # Wi-Fi completed, then a new LTE transmission started
                    wifi_duration += (current_wifi_end - current_wifi_start)
                    current_lte_start = float(fields[0])
                    current_lte_end = float(fields[4])
                    state = 'LTE'
                else:
                    # this LTE signal overlaps; end the Wifi occupancy 
                    # prematurely and move to LTE
                    wifi_duration += (float(fields[0]) - current_wifi_start)
                    current_lte_start = float(fields[0])
                    current_lte_end = float(fields[4])
                    state = 'LTE'

# Out of lines to process.  If we were in state LTE or WIFI, 
# add the duration of the pending signal before quitting
if state == 'WIFI':
    wifi_duration += (current_wifi_end - current_wifi_start)
elif state == 'LTE':
    lte_duration += (current_lte_end - current_lte_start)

if last_time == 0:
    print("Error:  no signals processed")
    exit(1)

duration = last_time - first_time
print("%s %s %s" % (lte_duration, wifi_duration, duration))


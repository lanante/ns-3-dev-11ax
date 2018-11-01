#!/bin/bash

#
# Copyright (c) 2015 University of Washington
# Copyright (c) 2015 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Authors: Tom Henderson <tomh@tomh.org> and Nicola Baldo <nbaldo@cttc.es>
#

control_c()
{
  echo "exiting"
  exit $?
}

trap control_c SIGINT

if test ! -f ../../../waf ; then
    echo "please run this program from within the directory `dirname $0`, like this:"
    echo "cd `dirname $0`"
    echo "./`basename $0`"
    exit 1
fi

# return 0 if $1 <= $2 <= $3, 1 otherwise
within_tolerance()
{
  if [ 1 -eq "$(echo "$1 <= $2" | bc)" ]
  then  
    if [ 1 -eq "$(echo "$2 <= $3" | bc)" ]
    then
      echo "0"
      return
    fi
  fi 
  echo "1"
}

#set -x
set -o errexit

outputDir=`pwd`/results-phy-duration
rm -rf "${outputDir}"
mkdir -p "${outputDir}"


passed_tests=()
failed_tests=()

#################################################################
#  Test description
#
# 10 seconds of data transfer at 10 Kbps, so 12 1028 byte packets
# for each network.  Also, ~10 beacons per AP per second.  From our
# vantage point on node 3, we do not see node 3's transmissions in the
# phy_log.  So, we should observe 24 data frames, 12 acks, and 200 beacons.
# In addition, each node does an ICMP echo request/response during the
# test duration, with associated acks, so we have an additional 150 us.
# 
# We observe from node 3 which means we are blind to node 3's transmissions
# (which amounts to 13 acks that we miss in this test)
#
# - Beacon is 113 data bytes, and 24 bytes of MAC header, and 4 bytes FCS
#   = 141 bytes at OFDM 6 Mb/s (MCS 0) where 24 uncoded bits/symbol
#     according to Sec. 18.3.2.3, Table 18-4 of IEEE Std 802.11-2012
#   = 141 bytes at roughly 3 bytes per symbol = 48 symbols @ 6 Mb/s 
#     beacon rate (see equation 20-32 of IEEE Std 802.11-2012 for why
#     this is 48 and not 47 symbols)
#   = 48* 4 us/symbol + 20 us PLCP preamble/header duration (16 + 4)  
#   = 212 us each
# - UDP frame 1028 data bytes IP, 1066 bytes at Wifi layer, which at
#   HtMcs15 is 17 symbols * 4 us/symbols = 68 us
#   = 68 + PLCP Preamble (16 us) + PLCP header (4 us)
#   = 88 us + PLCP Ht sig duration (8 us) + PLCP Ht training seq (12 us)
#   = 108 us each
# - Ack is 28 us (2 symbols * 4 us/symbols) + 20 us PLCP preamble/header
#   = 28 us each
# - Echo request (2 such frames observed, 2 acks observed)
#   = 48 us each
# - Echo response (2 such frames observed, 1 ack observed)
#   = 48 us each
#
# Total of 24 UDP data frames, 12 + 1 + 2 = 15 Acks, and ~200 beacons
# 
# All together, the above is about 42.4ms (Beacons), plus 2.8ms for data 
# (UDP plus ICMP), plus 0.4ms for acks, so we should see value of wifi 
# channel occupancy of around 45.6ms.  We will test around +/- 5% of the nominal
# value of wifi channel occupancy, so about 43.3ms < occupancy < 46.2ms
# 
#################################################################

label="10Kbps-Wifi"
lower_bound=0.0433
upper_bound=0.0479

d1=10
d2=10
simTag="10Kbps-Wifi"
transport=Udp
udpRate=10Kbps
duration=10

# need this as otherwise waf won't find the executables
cd ../../../

./waf --run lte-wifi-simple --command="%s --cellConfigA=Wifi --cellConfigB=Wifi --d1=${d1} --d2=${d2} --simTag=${simTag} --transport=${transport} --udpRate=${udpRate} --logPhyArrivals=1 --logPhyNodeId=3 --duration=10 --outputDir=${outputDir}"

cd $outputDir
if ! [ -e "lte_wifi_simple_${simTag}_phy_log" ]; then
    echo "file not found"
    failed_tests+=("$label;")
else
    # Report output
    DURATIONS=`python ../extract-phy-time.py lte_wifi_simple_${simTag}_phy_log 3`
    durations=($DURATIONS)
    # durations[1] is wifi duration
    result=`within_tolerance $lower_bound ${durations[1]} $upper_bound`
    if [ 0 -eq $result ]
    then
        passed_tests+=("$label;")
        echo ""
        echo "Test $label passed"
    else
        echo ""
        echo "Failed tolerance due to observed duration ${durations[1]}"
        echo "Test $label failed"
        failed_tests+=("$label;")
    fi
fi

cd ..

#################################################################
#  Test description
#
# 10 seconds of data transfer at 100 Kbps, so ~120 1000 byte packets
# for each network (because of delayed start after 2 sec).  
# Also, ~10 beacons per AP per second (around 200), and we miss the
# acks sent from observing node 3.
# should see about 240 data frames, 120 acks, and 200 beacons total
# From above:
# - Beacon is 212 us each
# - UDP frames are 108 us each
# - Ack is 28 us each
# 
# All together, the above comes out to ~72ms.  We observe slightly
# less than this in practice (241 data frames), due to random start 
# times for clients, so we'll test around +/- 5% of the nominal
# value of wifi channel occupancy of about 68.4 < occupancy < 75.6 ms
# 
#################################################################

label="100Kbps-Wifi"
lower_bound=0.0684
upper_bound=0.0756

d1=10
d2=10
simTag="100Kbps-Wifi"
transport=Udp
udpRate=100Kbps
duration=10

# need this as otherwise waf won't find the executables
cd ../../../

./waf --run lte-wifi-simple --command="%s --cellConfigA=Wifi --cellConfigB=Wifi --d1=${d1} --d2=${d2} --simTag=${simTag} --transport=${transport} --udpRate=${udpRate} --logPhyArrivals=1 --logPhyNodeId=3 --duration=10 --outputDir=${outputDir}"

cd $outputDir
if ! [ -e "lte_wifi_simple_${simTag}_phy_log" ]; then
    echo "file not found"
    failed_tests+=("$label;")
else
    # Report output
    DURATIONS=`python ../extract-phy-time.py lte_wifi_simple_${simTag}_phy_log 3`
    durations=($DURATIONS)
    # durations[1] is wifi duration
    result=`within_tolerance $lower_bound ${durations[1]} $upper_bound`
    if [ 0 -eq $result ]
    then
        passed_tests+=("$label;")
        echo ""
        echo "Test $label passed"
    else
        echo ""
        echo "Failed tolerance due to observed duration ${durations[1]}"
        echo "Test $label failed"
        failed_tests+=("$label;")
    fi
fi

cd ..

##############
# End of tests
##############

rm -rf "${outputDir}"

if [ ${#failed_tests[@]} == 0 ]; then
    echo "All tests passed"
    exit 0
else
    echo "Some test failed: ${failed_tests[@]}"
    exit 1
fi
 


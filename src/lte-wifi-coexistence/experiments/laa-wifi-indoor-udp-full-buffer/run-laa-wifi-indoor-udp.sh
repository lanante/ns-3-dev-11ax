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

if test ! -f ../../../../waf ; then
    echo "please run this program from within the directory `dirname $0`, like this:"
    echo "cd `dirname $0`"
    echo "./`basename $0`"
    exit 1
fi

outputDir=`pwd`/results
if [ -d $outputDir ] && [ "$1" != "-a" ]; then
    echo "$outputDir directory exists; exiting!"
    echo ""
    echo "Pass the '-a' option if you want to append; else move results/ out of the way"
    echo ""
    exit 1
else
    mkdir -p "${outputDir}"
fi

if [ -d $outputDir ] && [ "$1" == "-a" ]; then
    echo "Appending results to existing results/ directory"
fi

set -x
set -o errexit

# need this as otherwise waf won't find the executables
cd ../../../../

# Random number generator seed
RngRun=1
# Which node number in the scenario to enable logging on
logNodeId=6
# Transport protocol (Ftp, Tcp, or Udp).  Udp corresponds to full buffer.
transport=Udp
# TXOP duration (ms) for LAA
lbtTxop=8
# Base simulation duration (seconds); scaled below
base_duration=120
# Enable voice instead of FTP on two UEs
voiceEnabled=1
# Set to value '0' for LAA SISO, '2' for LAA MIMO
laaTxMode=2

for udpRate in 75Kbps 200Kbps 400Kbps ; do
    for energyDetection in -72.0 ; do
        for cell in Wifi Laa ; do
            # Make the simulation duration inversly proportional to ftpLambda
            duration=$base_duration
            simTag="eD_${energyDetection}_udpRate_${udpRate}_cellA_${cell}"
            /usr/bin/time -f '%e %U %S %K %M %x %C' -o "${outputDir}"/time_stats -a \
            ./waf --run lte-wifi-indoor --command="%s --cellConfigA=${cell} --cellConfigB=Wifi --lbtTxop=${lbtTxop} --logWifiRetries=1 --logWifiFailRetries=1 --logPhyArrivals=1 --logPhyNodeId=${logNodeId} --transport=${transport} --udpRate=${udpRate} --duration=${duration} --cwUpdateRule=nacks80 --logHarqFeedback=1 --logTxops=1 --logCwChanges=1 --logBackoffChanges=1 --laaEdThreshold=${energyDetection} --simTag=${simTag} --outputDir=${outputDir} --voiceEnabled=${voiceEnabled} --ns3::LteEnbRrc::DefaultTransmissionMode=${laaTxMode} --RngRun=${RngRun}"
        done
    done
done

for udpRate in 75Kbps 200Kbps 400Kbps ; do
    for energyDetection in -62.0 -82.0 ; do
        for cell in Laa ; do
            # Make the simulation duration inversly proportional to ftpLambda
            duration=$base_duration
            simTag="eD_${energyDetection}_udpRate_${udpRate}_cellA_${cell}"
            /usr/bin/time -f '%e %U %S %K %M %x %C' -o "${outputDir}"/time_stats -a \
            ./waf --run lte-wifi-indoor --command="%s --cellConfigA=${cell} --cellConfigB=Wifi --lbtTxop=${lbtTxop} --logWifiRetries=1 --logWifiFailRetries=1 --logPhyArrivals=1 --logPhyNodeId=${logNodeId} --transport=${transport} --udpRate=${udpRate} --duration=${duration} --cwUpdateRule=nacks80 --logHarqFeedback=1 --logTxops=1 --logCwChanges=1 --logBackoffChanges=1 --laaEdThreshold=${energyDetection} --simTag=${simTag} --outputDir=${outputDir} --voiceEnabled=${voiceEnabled} --ns3::LteEnbRrc::DefaultTransmissionMode=${laaTxMode} --RngRun=${RngRun}"
        done
    done
done


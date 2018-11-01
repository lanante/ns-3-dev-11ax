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
# Authors: Tom Henderson <tomh@tomh.org> and Nicola Baldo <nbaldo@cttc.es> and Biljana Bojovic <bbojovic@cttc.es>
#

source config

outputDir=`pwd`/results

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

set -x
set -o errexit

# If the script is run directly, then we need to clean results directory
# Otherwise if it is run from a run-in-parallel script this is already done
if [[ $# -eq 0 ]] ; then
  outputDir=`pwd`/results
  mkdir -p "${outputDir}"
  rm -f "${outputDir}"/lte_wifi_indoor_*_operator?
  rm -f "${outputDir}"/time_stats
  rm -f "${outputDir}"/lte_wifi_indoor_*_log
else
  lbtTxopList="$1"
fi

# need this as otherwise waf won't find the executables
cd ../../../../


# for wifiEd in ${wifiEnergyDetectionList} ; do
for drsPeriod in ${drsPeriodList} ; do
 for rlcAmRbsTimer in ${rlcAmRbsTimerList}; do
  for lbtTxop in ${lbtTxopList} ; do
   for ftpLambda in ${ftpLambdas} ; do
    for transport in ${transports} ; do
     for energyDetection in ${energyDetectionList} ; do
      for cwUpdateRule in ${cwUpdateRules} ; do
       duration=$(echo "$base_duration/$ftpLambda" | bc)
       simTag="tX_${lbtTxop}_${transport}_${ftpLambda}_cellA_${cell}_${cwUpdateRule}_eD_${energyDetection}_${rlcAmRbsTimer}_${drsPeriod}_${dropPackets}"
       /usr/bin/time -f '%e %U %S %K %M %x %C' -o "${outputDir}"/time_stats -a \
       ./waf --run lte-wifi-indoor --command="%s --cellConfigA=${cell} --cellConfigB=Wifi --lteDutyCycle=${lteDutyCycle} --lbtTxop=${lbtTxop} --logPhyArrivals=${logPhyArrivals} --logCwChanges=${logCwChanges} --logBackoffChanges=${logBackoffChanges} --logBeaconArrivals=${logBeaconArrivals} --logBeaconNodeId=${logBeaconNodeId} --logWifiRetries=${logWifiRetries} --logWifiFailRetries=${logWifiFailRetries} --logHarqFeedback=${logHarqFeedback} --logTxops=${logTxops} --transport=${transport} --duration=${duration} --simTag=${simTag} --outputDir=${outputDir} --RngRun=${RngRun}  --laaEdThreshold=${energyDetection} --ftpLambda=${ftpLambda} --cwUpdateRule=${cwUpdateRule} --drsPeriod=${drsPeriod} --dropPackets=${dropPackets} --tcpRlcMode=${tcpRlcMode} --rlcAmRbsTimer=${rlcAmRbsTimer} --voiceEnabled=${voiceEnabled} --ns3::TcpSocket::SegmentSize=${tcpSegSize} --ns3::TcpSocket::InitialCwnd=${tcpInitialCw} --wifiQueueMaxSize=2000p --ns3::LteEnbRrc::DefaultTransmissionMode=2 --bsCornerPlacement=${bsCornerPlacement}"
      done
     done
    done
   done
  done
 done
done
# done

echo "$0 Simulations run-laa-wifi-indoor finished: lbtTxopList=${lbtTxopList},transports=${transports},ftpLambdas=${ftpLambdas},energyDetectionList=${energyDetectionList}, cwUpdateRules=${cwUpdateRules}"






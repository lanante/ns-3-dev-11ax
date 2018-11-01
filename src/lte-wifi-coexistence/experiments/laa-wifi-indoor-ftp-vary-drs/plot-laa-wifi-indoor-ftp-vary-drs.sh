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

source ../utils/shot
source ../utils/common
source config

IMGDIR=images
THUMBNAILS=true
NOX11=true

BASE_OPTIONS=""
BASE_OPTIONS="${BASE_OPTIONS} ; set style line 1 pt 4 lt 1"
BASE_OPTIONS="${BASE_OPTIONS} ; set style line 2 pt 7 lt 2"
BASE_OPTIONS="${BASE_OPTIONS} ; set style increment;"
BASE_OPTIONS="${BASE_OPTIONS} ; set pointsize 1 ; set grid;"


set -o errexit

##############################################################
#  Latency CDF
##############################################################

for rlcAmRbsTimer in ${rlcAmRbsTimerList}; do
for transport in ${transports} ; do
for cwUpdateRule in ${cwUpdateRules} ; do
for lbtTxop in ${lbtTxopList} ; do
for ftpLambda in ${ftpLambdas} ; do
for energyDetection in ${energyDetectionList} ; do
    index=0
    for drsPeriod in ${drsPeriodList} ; do
    for OPERATOR in A B ; do
        simTag="tX_${lbtTxop}_${transport}_${ftpLambda}_cellA_${cell}_${cwUpdateRule}_eD_${energyDetection}_${rlcAmRbsTimer}_${drsPeriod}_${dropPackets}"
        imgTag="indoor_"`echo "${lbtTxop}_${transport}_lambda_${ftpLambda}" | tr '.' '_'`
        TRAFFIC=`print_traffic_model ${transport}`
        #TITLE="txOp=${lbtTxop},${transport},${ftpLambda},${cwUpdateRule},${rlcAmRbsTimer},${drsPeriod},${dropPackets}"
        LATENCY_COLUMN=9
        CURRENT=results/lte_wifi_indoor_${simTag}_operator${OPERATOR}
        `../utils/cdf.sh $LATENCY_COLUMN $CURRENT > results/cdf_latency_${simTag}_${OPERATOR}${drsPeriod}`

        FILES[$index]=results/cdf_latency_${simTag}_${OPERATOR}${drsPeriod}
        YCOLS[$index]='($2)'
        XCOLS[$index]='($1)'
        LABELS[$index]=`print_operator_laa_wifi $OPERATOR`"-DRS period-${drsPeriod}ms"
        index=`expr $index + 1`
    done
    done

    OPTIONS="$BASE_OPTIONS ; set key bottom Left right reverse"
    PLOTTYPE="with linespoints"
    XLABEL="Latency [ms]"
    YLABEL="CDF"
    RANGE=$LATENCY_CDF_RANGE
    #IMGFILENAME="${imgTag}_latency_${transport}_${ftpLambda}_${cwUpdateRule}_${energyDetection}_${transport}_${rlcAmRbsTimer}_${drsPeriod}_${dropPackets}"
    IMGFILENAME="latency"
    plot

    unset FILES
    unset LABELS
    unset YCOLS
    unset XCOLS
done
done
done
done
done
done


##############################################################
#  Throughput CDF
##############################################################

for rlcAmRbsTimer in ${rlcAmRbsTimerList}; do
for transport in ${transports} ; do
for cwUpdateRule in ${cwUpdateRules} ; do
for lbtTxop in ${lbtTxopList} ; do
for ftpLambda in ${ftpLambdas} ; do
for energyDetection in ${energyDetectionList} ; do
    index=0
    for drsPeriod in ${drsPeriodList} ; do
    for OPERATOR in A B ; do
        simTag="tX_${lbtTxop}_${transport}_${ftpLambda}_cellA_${cell}_${cwUpdateRule}_eD_${energyDetection}_${rlcAmRbsTimer}_${drsPeriod}_${dropPackets}"
        imgTag="indoor_"`echo "${lbtTxop}_${transport}_lambda_${ftpLambda}" | tr '.' '_'`
        TRAFFIC=`print_traffic_model ${transport}`
        #TITLE="txOp=${lbtTxop},${transport},${ftpLambda},${cwUpdateRule},${rlcAmRbsTimer},${drsPeriod},${dropPackets}"
        THROUGHPUT_COLUMN=8
        CURRENT=results/lte_wifi_indoor_${simTag}_operator${OPERATOR}
        `../utils/cdf.sh $THROUGHPUT_COLUMN $CURRENT > results/cdf_throughput_${simTag}_${OPERATOR}${drsPeriod}`
        FILES[$index]=results/cdf_throughput_${simTag}_${OPERATOR}${drsPeriod}
        YCOLS[$index]='($2)'
        XCOLS[$index]='($1)'
        LABELS[$index]=`print_operator_laa_wifi $OPERATOR`"-DRS-${drsPeriod}ms"
        index=`expr $index + 1`
    done
    done

    PLOTTYPE="with linespoints"
    XLABEL="Throughput [Mbps]"
    YLABEL="CDF"
    RANGE=$THROUGHPUT_CDF_RANGE
    OPTIONS="$BASE_OPTIONS ; set key top left Left reverse"
    #IMGFILENAME="${imgTag}_throughput_${transport}_${ftpLambda}_${cwUpdateRule}_${energyDetection}_${transport}_${rlcAmRbsTimer}_${drsPeriod}_${dropPackets}"
    IMGFILENAME="throughput"
    plot

    unset FILES
    unset LABELS
    unset YCOLS
    unset XCOLS

done
done
done
done
done
done


../utils/shot_thumbnails.sh $IMGDIR/thumbnails "laa-wifi-indoor: transport=${transport}, ${ftpLambdas}, cw update rule=${cwUpdateRules}"


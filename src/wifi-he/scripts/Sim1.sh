#!/bin/bash
source spatial-reuse-functions.sh
cd ../examples
export AUTO_DELETE_SPATIAL_REUSE_OUTPUT_FILES=1

export enablePcap=0
export enableAscii=0
export rxSensitivity=-82
export standard=11ax_5GHZ

obssPdLevel=-82
#export NS_LOG=DynamicObssPdAlgorithm=logic

for i in 1 2 3 4 5; do
for j in 0 5 11;do
for k in 2 3 4 5 6 7 ; do
RngRunpar=${i}
nBsspar=${k}
MCSpar=${j}
npar=10
durpar=5
if [ ${MCSpar} -eq 0 ]
then
ampdusizepar=4915
dlpar=6
ulpar=1
fi
if [ ${MCSpar} -eq 1 ]
then
ampdusizepar=9830
fi
if [ ${MCSpar} -eq 2 ]
then
ampdusizepar=14745
fi
if [ ${MCSpar} -eq 3 ]
then
ampdusizepar=19661
fi
if [ ${MCSpar} -eq 4 ]
then
ampdusizepar=29491
fi
if [ ${MCSpar} -eq 5 ]
then
ampdusizepar=39321
dlpar=52
ulpar=6
fi
if [ ${MCSpar} -eq 6 ]
then
ampdusizepar=49151
fi
if [ ${MCSpar} -eq 7 ]
then
ampdusizepar=58982
fi
if [ ${MCSpar} -eq 8 ]
then
ampdusizepar=65535
fi
if [ ${MCSpar} -eq 9 ]
then
ampdusizepar=65535
fi
if [ ${MCSpar} -eq 10 ]
then
ampdusizepar=65535
fi
if [ ${MCSpar} -eq 11 ]
then
ampdusizepar=65535
dlpar=109
ulpar=12
fi

Res=Sim1
export RngRun=${RngRunpar}
export powerBackoff=0
export obssPdAlgorithm=DynamicObssPdAlgorithm
export obssPdThreshold=-82
export maxAmpduSize=${ampdusizepar}
export MCS=${MCSpar}
export downlink=${dlpar}
export uplink=${ulpar}
export duration=${durpar}
export enableRts=0
export txStartOffset=50
export enableObssPd=1
export txGain=0
export rxGain=0
export antennas=1
export maxSupportedTxSpatialStreams=1
export maxSupportedRxSpatialStreams=1
export performTgaxTimingChecks=0
export nodePositionsFile=NONE
export bw=20
export scenario=logdistance
export nBss=${nBsspar}
export payloadSizeUplink=1500
export payloadSizeDownlink=1500
export useIdealWifiManager=0
export r=5
export powSta=20
export powAp=20
export txRange=102
export ccaTrSta=-62
export ccaTrAp=-62
export d=20
export sigma=3.5
export bianchi=1
export n=${npar}
export enableObssPd=1
export test=Res_${obssPdAlgorithm}_${obssPdThreshold}
export test=${Res}_${obssPdAlgorithm}_${obssPdThreshold}_${nBss}_${enableObssPd}_${MCS}_${powerBackoff}_${d}_${RngRun}
run_one &
sleep 1


export powerBackoff=1
export RngRun=${RngRunpar}
export obssPdAlgorithm=ConstantObssPdAlgorithm
export obssPdThreshold=-82
export maxAmpduSize=${ampdusizepar}
export MCS=${MCSpar}
export downlink=${dlpar}
export uplink=${ulpar}
export duration=${durpar}
export enableRts=0
export txStartOffset=50
export enableObssPd=1
export txGain=0
export rxGain=0
export antennas=1
export maxSupportedTxSpatialStreams=1
export maxSupportedRxSpatialStreams=1
export performTgaxTimingChecks=0
export nodePositionsFile=NONE
export bw=20
export scenario=logdistance
export nBss=${nBsspar}
export payloadSizeUplink=1500
export payloadSizeDownlink=1500
export useIdealWifiManager=0
export r=5
export powSta=20
export powAp=20
export txRange=102
export ccaTrSta=-62
export ccaTrAp=-62
export d=20
export sigma=3.5
export bianchi=1
export n=${npar}
export enableObssPd=1
export test=Res_${obssPdAlgorithm}_${obssPdThreshold}
export test=${Res}_${obssPdAlgorithm}_${obssPdThreshold}_${nBss}_${enableObssPd}_${MCS}_${powerBackoff}_${d}_${RngRun}
run_one &
sleep 1

export powerBackoff=1
export RngRun=${RngRunpar}
export obssPdAlgorithm=ConstantObssPdAlgorithm
export obssPdThreshold=-72
export maxAmpduSize=${ampdusizepar}
export MCS=${MCSpar}
export downlink=${dlpar}
export uplink=${ulpar}
export duration=${durpar}
export enableRts=0
export txStartOffset=50
export enableObssPd=1
export txGain=0
export rxGain=0
export antennas=1
export maxSupportedTxSpatialStreams=1
export maxSupportedRxSpatialStreams=1
export performTgaxTimingChecks=0
export nodePositionsFile=NONE
export bw=20
export scenario=logdistance
export nBss=${nBsspar}
export payloadSizeUplink=1500
export payloadSizeDownlink=1500
export useIdealWifiManager=0
export r=5
export powSta=20
export powAp=20
export txRange=102
export ccaTrSta=-62
export ccaTrAp=-62
export d=20
export sigma=3.5
export bianchi=1
export n=${npar}
export enableObssPd=1
export test=Res_${obssPdAlgorithm}_${obssPdThreshold}
export test=${Res}_${obssPdAlgorithm}_${obssPdThreshold}_${nBss}_${enableObssPd}_${MCS}_${powerBackoff}_${d}_${RngRun}
run_one &
sleep 1

export powerBackoff=1
export RngRun=${RngRunpar}
export obssPdAlgorithm=ConstantObssPdAlgorithm
export obssPdThreshold=-62
export maxAmpduSize=${ampdusizepar}
export MCS=${MCSpar}
export downlink=${dlpar}
export uplink=${ulpar}
export duration=${durpar}
export enableRts=0
export txStartOffset=50
export enableObssPd=1
export txGain=0
export rxGain=0
export antennas=1
export maxSupportedTxSpatialStreams=1
export maxSupportedRxSpatialStreams=1
export performTgaxTimingChecks=0
export nodePositionsFile=NONE
export bw=20
export scenario=logdistance
export nBss=${nBsspar}
export payloadSizeUplink=1500
export payloadSizeDownlink=1500
export useIdealWifiManager=0
export r=5
export powSta=20
export powAp=20
export txRange=102
export ccaTrSta=-62
export ccaTrAp=-62
export d=20
export sigma=3.5
export bianchi=1
export n=${npar}
export enableObssPd=1
export test=Res_${obssPdAlgorithm}_${obssPdThreshold}
export test=${Res}_${obssPdAlgorithm}_${obssPdThreshold}_${nBss}_${enableObssPd}_${MCS}_${powerBackoff}_${d}_${RngRun}
run_one &

wait
done
done
done

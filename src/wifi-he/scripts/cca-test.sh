#!/bin/bash
source spatial-reuse-functions.sh
cd ../examples
export AUTO_DELETE_SPATIAL_REUSE_OUTPUT_FILES=1

export enablePcap=0
export enableAscii=0
export rxSensitivity=-82
export standard=11ac

obssPdLevel=-82
#export NS_LOG=WifiPhy=logic

for k in 1 2 3 4 5 6 7 8 9 10; do
for i in 1 2 3 4 5 6 7 8 9 10; do
for j in 2 3 4; do
RngRunpar=$k
nBsspar=$j
MCSpar=0
npar=$i
durpar=5
obsspdpar=-82
ampdusizepar=4000
dlpar=0
ulpar=8


Res=cca-test

export powerBackoff=0
export RngRun=${RngRunpar}
export obssPdAlgorithm=ConstantObssPdAlgorithm
export obssPdThreshold=${obsspdpar}
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
export r=1
export powSta=20
export powAp=20
export txRange=102
export ccaTrSta=-62
export ccaTrAp=-62
export d=10
export sigma=3.5
export bianchi=1
export n=${npar}
export enableObssPd=0
export test=${Res}_${nBss}_${r}_${d}_${n}_${RngRun}
run_one &
wait

done
done 
done

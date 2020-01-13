#!/bin/bash
source spatial-reuse-functions.sh
cd ../examples
export AUTO_DELETE_SPATIAL_REUSE_OUTPUT_FILES=1

export enablePcap=0
export enableAscii=0
export rxSensitivity=-82
export standard=11ac

obssPdLevel=-82
#export NS_LOG=DynamicObssPdAlgorithm=logic

for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do
for j in 1 2 4 8; do
RngRunpar=${i}
npar=${j}
Res=Sim0


export powerBackoff=0
export RngRun=${RngRunpar}
export obssPdAlgorithm=ConstantObssPdAlgorithm
export obssPdThreshold=-82
export maxAmpduSize=4900
export MCS=0
export downlink=0
export uplink=10
export duration=5
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
export nBss=2
export payloadSizeUplink=1500
export payloadSizeDownlink=1500
export useIdealWifiManager=0
export r=4
export powSta=20
export powAp=20
export txRange=102
export ccaTrSta=-62
export ccaTrAp=-62
export d=30
export sigma=3.5
export bianchi=1
export n=${npar}
export enableObssPd=1
export test=${Res}_${obssPdAlgorithm}_${obssPdThreshold}_${nBss}_${enableObssPd}_${MCS}_${powerBackoff}_${d}_${RngRun}
run_one 

wait
done
done

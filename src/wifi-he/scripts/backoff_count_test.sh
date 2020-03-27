#!/bin/bash
source spatial-reuse-functions.sh
cd ../examples
export AUTO_DELETE_SPATIAL_REUSE_OUTPUT_FILES=1

export enablePcap=1
export enableAscii=0
export rxSensitivity=-82
export standard=11ac

obssPdLevel=-82
#export NS_LOG=DynamicObssPdAlgorithm=logic

for i in 1; do
for j in 1; do
RngRunpar=${i}
npar=${j}
Res=Sim0

export enableFrameCapture=true
export powerBackoff=0
export RngRun=${RngRunpar}
export obssPdAlgorithm=ConstantObssPdAlgorithm
export obssPdThreshold=-82
export maxAmpduSize=2000
export MCS=0
export downlink=0
export uplink=10
export duration=10
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
export r=0.01
export powSta=20
export powAp=20
export txRange=102
export ccaTrSta=-62
export ccaTrAp=-62
export d=5
export sigma=3.5
export bianchi=1
export n=${npar}
export enableObssPd=0
export test=${Res}_${RngRun}
run_one 

wait
done
done

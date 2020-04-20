#!/bin/bash


standard=11ac
enableFrameCapture=true
RngRun=1

maxAmpduSize=65535
MCS=8
downlink=0
uplink=400
duration=10
bw=20
nBss=2
payloadSizeUplink=1500
payloadSizeDownlink=1500

r=8
powSta=20
powAp=20

ccaTrSta=62
ccaTrAp=62
d=15
bianchi=1
n=1
fixedPosition=1 


../waf --run "spatial-reuse --bianchi=${bianchi} --enableFrameCapture=${enableFrameCapture} --RngRun=${RngRun} --payloadSizeUplink=${payloadSizeUplink} --payloadSizeDownlink=${payloadSizeDownlink} --r=${r} --powSta=${powSta} --powAp=${powAp} --ccaTrSta=${ccaTrSta} --ccaTrAp=${ccaTrAp} --d=${d} --n=${n} --fixedPosition=${fixedPosition} --uplink=${uplink} --downlink=${downlink} --duration=${duration} --MCS=${MCS} --nBss=${nBss} --maxAmpduSizeBss1=${maxAmpduSize} --maxAmpduSizeBss2=${maxAmpduSize}" 





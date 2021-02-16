#!/bin/bash


standard=11ac
enableFrameCapture=true
RngRun=1
enableThresholdPreambleDetection=false
maxAmpduSize=65535
MCS=8
downlink=0
uplink=100
duration=10
bw=20
nBss=2
payloadSizeUplink=1500
payloadSizeDownlink=1500

r=8
powSta=20
powAp=20


d=15
bianchi=1
n=10
fixedPosition=0
obssMCS=0
ccaTr=-62
ccaTrObss=-82
echo Known
../waf --run "spatial-reuse --bianchi=${bianchi} --enableThresholdPreambleDetection=${enableThresholdPreambleDetection} --ccaTrStaObss=${ccaTrObss} --ccaTrApObss=${ccaTrObss} --ccaTrSta=${ccaTr} --ccaTrAp=${ccaTr}  --enableFrameCapture=${enableFrameCapture} --RngRun=${RngRun} --payloadSizeUplink=${payloadSizeUplink} --payloadSizeDownlink=${payloadSizeDownlink} --r=${r} --powSta=${powSta} --powAp=${powAp} --d=${d} --n=${n} --fixedPosition=${fixedPosition} --uplink=${uplink} --downlink=${downlink} --duration=${duration} --obssMCS=${obssMCS} --MCS=${MCS} --nBss=${nBss} --maxAmpduSizeBss1=${maxAmpduSize} --maxAmpduSizeBss2=${maxAmpduSize}" 

ccaTrObss=-62
echo Unknown
../waf --run "spatial-reuse --bianchi=${bianchi} --enableThresholdPreambleDetection=${enableThresholdPreambleDetection} --ccaTrStaObss=${ccaTrObss} --ccaTrApObss=${ccaTrObss} --ccaTrSta=${ccaTr} --ccaTrAp=${ccaTr}  --enableFrameCapture=${enableFrameCapture} --RngRun=${RngRun} --payloadSizeUplink=${payloadSizeUplink} --payloadSizeDownlink=${payloadSizeDownlink} --r=${r} --powSta=${powSta} --powAp=${powAp} --d=${d} --n=${n} --fixedPosition=${fixedPosition} --uplink=${uplink} --downlink=${downlink} --duration=${duration} --obssMCS=${obssMCS} --MCS=${MCS} --nBss=${nBss} --maxAmpduSizeBss1=${maxAmpduSize} --maxAmpduSizeBss2=${maxAmpduSize}" 

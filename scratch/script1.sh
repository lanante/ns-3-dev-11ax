#!/bin/bash

cd ../examples/wireless

channelBssA=36
channelBssB=36
channelBssC=40
loadBssA=54
loadBssB=54
loadBssC=54
useDynamicChannelBonding=false
interBssDistance=7
distance=10;
export NS_LOG=SpectrumWifiPhy=logic

../../waf --run "wifi-dcb --channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC --useDynamicChannelBonding=$useDynamicChannelBonding --loadBssA=$loadBssA --loadBssB=$loadBssB --loadBssC=$loadBssC --interBssDistance=$interBssDistance --distance=$distance"

cd ../../scratch

#!/bin/bash



channelBssA=36
channelBssB=36
channelBssC=40
loadBssA=20
loadBssB=20
loadBssC=50

useDynamicChannelBonding=false
interBssDistance=5
distance=10;
export NS_LOG=SpectrumWifiPhy=logic

../waf --run "wifi-dcb --loadBssC=$loadBssC  --loadBssA=$loadBssA --loadBssB=$loadBssB --channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC --useDynamicChannelBonding=$useDynamicChannelBonding --interBssDistance=$interBssDistance --distance=$distance"


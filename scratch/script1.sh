channelBssA=36
channelBssB=40
channelBssC=38
loadBssA=100
loadBssB=100
loadBssC=100
payloadSize=1472

intervalA=$(bc -l <<< $payloadSize*8/$loadBssA/1000000)
intervalB=$(bc -l <<< $payloadSize*8/$loadBssB/1000000)
intervalC=$(bc -l <<< $payloadSize*8/$loadBssC/1000000)
mcs=HtMcs7

#intervalA=0.01
#intervalB=0.01
#intervalC=0.01




echo 

useDynamicChannelBonding=true
interBssDistance=5
distance=10;
n=10
#export NS_LOG=SpectrumWifiPhy=logic

../waf --run "Case1 --loadBssA=$intervalA --loadBssB=$intervalB --loadBssC=$intervalC --useDynamicChannelBonding=$useDynamicChannelBonding --mcs=$mcs --n=$n"


#../waf --run "wifi-dcb-2 --loadBssC=$loadBssC  --loadBssA=$loadBssA --loadBssB=$loadBssB --channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC --useDynamicChannelBonding=$useDynamicChannelBonding --interBssDistance=$interBssDistance --distance=$distance"


channelBssA=36
channelBssB=40
channelBssC=38
loadBssA=7
loadBssB=7
loadBssC=7
payloadSize=1472

intervalA=$(bc -l <<< $payloadSize*8/$loadBssA/1000000)
intervalB=$(bc -l <<< $payloadSize*8/$loadBssB/1000000)
intervalC=$(bc -l <<< $payloadSize*8/$loadBssC/1000000)

#intervalA=0.01
#intervalB=0.01
#intervalC=0.01




echo 

useDynamicChannelBonding=true
interBssDistance=5
distance=10;
#export NS_LOG=SpectrumWifiPhy=logic

../waf --run "Case1 --loadBssA=$intervalA --loadBssB=$intervalB --loadBssC=$intervalC --useDynamicChannelBonding=$useDynamicChannelBonding"


#../waf --run "wifi-dcb-2 --loadBssC=$loadBssC  --loadBssA=$loadBssA --loadBssB=$loadBssB --channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC --useDynamicChannelBonding=$useDynamicChannelBonding --interBssDistance=$interBssDistance --distance=$distance"


channelBssA=36
channelBssB=36
channelBssC=36
loadBssA=70
loadBssB=70
loadBssC=70
payloadSize=1472

intervalA=$(bc -l <<< $payloadSize*8/$loadBssA/1000000)
intervalB=$(bc -l <<< $payloadSize*8/$loadBssB/1000000)
intervalC=$(bc -l <<< $payloadSize*8/$loadBssC/1000000)
mcs=HtMcs0
ccaEdThresholdPrimary=-62
ccaEdThresholdSecondary=-62
useDynamicChannelBonding=false
interBssDistance=5000
distance=10;
n=1
#export NS_LOG=SpectrumWifiPhy=logic

../waf --run "Case1 --loadBssA=$intervalA --loadBssB=$intervalB --loadBssC=$intervalC --useDynamicChannelBonding=$useDynamicChannelBonding --mcs=$mcs --n=$n --interBssDistance=$interBssDistance --distance=$distance --ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssC=$ccaEdThresholdPrimary --ccaEdThresholdSecondaryBssA=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssB=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssC=$ccaEdThresholdSecondary --channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC "



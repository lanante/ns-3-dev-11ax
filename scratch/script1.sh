channelBssA=36
channelBssB=36
channelBssC=36

uplinkA=70
downlinkA=0
uplinkB=70
downlinkB=0
uplinkC=70
downlinkC=0
payloadSize=1472


mcs=HtMcs0
ccaEdThresholdPrimary=-62
ccaEdThresholdSecondary=-62
useDynamicChannelBonding=false
interBssDistance=5000
distance=10;
n=1
#export NS_LOG=SpectrumWifiPhy=logic

../waf --run "Case1 --uplinkA=$uplinkA --uplinkB=$uplinkB --uplinkC=$uplinkC --downlinkA=$downlinkA --downlinkB=$downlinkB --downlinkC=$downlinkC --useDynamicChannelBonding=$useDynamicChannelBonding --mcs=$mcs --n=$n --interBssDistance=$interBssDistance --distance=$distance --ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssC=$ccaEdThresholdPrimary --ccaEdThresholdSecondaryBssA=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssB=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssC=$ccaEdThresholdSecondary --channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC "



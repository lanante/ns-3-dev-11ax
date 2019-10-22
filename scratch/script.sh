
channelBssA=36
channelBssB=40
channelBssC=38

uplinkA=100
uplinkC=100
uplinkB=100
downlinkA=0
downlinkB=0
downlinkC=0
payloadSize=1472
simulationTime=1


ccaEdThresholdPrimary=-62
ccaEdThresholdSecondary=-62
useDynamicChannelBonding=true
interBssDistance=30
distance=10;
n=10
RngRun=1
interBssDistance=30

../waf --run "channel-bonding --simulationTime=$simulationTime --RngRun=$RngRun --uplinkA=$uplinkA --uplinkB=$uplinkB --uplinkC=$uplinkC --downlinkA=$downlinkA --downlinkB=$downlinkB --downlinkC=$downlinkC --useDynamicChannelBonding=$useDynamicChannelBonding --n=$n --interBssDistance=$interBssDistance --distance=$distance --ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssC=$ccaEdThresholdPrimary --ccaEdThresholdSecondaryBssA=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssB=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssC=$ccaEdThresholdSecondary --channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC "



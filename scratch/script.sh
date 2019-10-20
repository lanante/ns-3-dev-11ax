rm ../*.csv

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
simulationTime=5


ccaEdThresholdPrimary=-62
secondaryChannelBssC="UPPER"
useDynamicChannelBonding=true
interBssDistance=10
distance=10;
n=2
mcs=HtMcs0
RngRun=1

ccaEdThresholdSecondary=-52
#../waf --run "Case1 --secondaryChannelBssC=$secondaryChannelBssC --simulationTime=$simulationTime --RngRun=$RngRun --uplinkA=$uplinkA --uplinkB=$uplinkB --uplinkC=$uplinkC --downlinkA=$downlinkA --downlinkB=$downlinkB --downlinkC=$downlinkC --useDynamicChannelBonding=$useDynamicChannelBonding --mcs=$mcs --n=$n --interBssDistance=$interBssDistance --distance=$distance --ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssC=$ccaEdThresholdPrimary --ccaEdThresholdSecondaryBssA=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssB=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssC=$ccaEdThresholdSecondary --channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC "




ccaEdThresholdSecondary=-92
#../waf --run "Case1 --secondaryChannelBssC=$secondaryChannelBssC  --simulationTime=$simulationTime --RngRun=$RngRun --uplinkA=$uplinkA --uplinkB=$uplinkB --uplinkC=$uplinkC --downlinkA=$downlinkA --downlinkB=$downlinkB --downlinkC=$downlinkC --useDynamicChannelBonding=$useDynamicChannelBonding --mcs=$mcs --n=$n --interBssDistance=$interBssDistance --distance=$distance --ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssC=$ccaEdThresholdPrimary --ccaEdThresholdSecondaryBssA=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssB=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssC=$ccaEdThresholdSecondary --channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC "
cd ../examples/wireless

../../waf --run "channel-bonding  --channelBssA=36 --channelBssB=38 --secondaryChannelBssB=LOWER --useDynamicChannelBonding=true --distance=1 --interBssDistance=20 --ccaEdThresholdSecondaryBssB=-100"

cd ../../scratch


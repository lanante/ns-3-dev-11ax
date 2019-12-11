
channelBssA=36
channelBssB=36
channelBssC=36
channelBssD=36
channelBssE=36
channelBssF=36
channelBssG=36

primaryChannelBssA=36
primaryChannelBssB=36
primaryChannelBssC=36
primaryChannelBssD=36
primaryChannelBssE=36
primaryChannelBssF=36
primaryChannelBssG=36

uplinkA=0
uplinkB=0
uplinkC=0

uplinkD=0
uplinkE=0
uplinkF=0
uplinkG=0
downlinkA=10
downlinkB=10
downlinkC=10
downlinkD=10
downlinkE=10
downlinkF=10
downlinkG=10

payloadSize=1472
simulationTime=5


ccaEdThresholdPrimary=-62
constantCcaEdThresholdSecondaryBss=-1000
channelBondingType=DynamicThreshold
interBssDistance=100
distance=10;
n=1
nBss=7
RngRun=1
mcs=MaxMcs



../waf

for RngRun in 1 ;
do

	for constantCcaEdThresholdSecondaryBss in  -100; do
for interBssDistance in 100;
do
				Test=${nBss}_${n}_${interBssDistance}_${constantCcaEdThresholdSecondaryBss}_${mcs}_${RngRun}
				echo "Starting $Test"
../waf --run "channel-bonding --Test=$Test --mcs=$mcs --nBss=$nBss --simulationTime=$simulationTime --RngRun=$RngRun --uplinkA=$uplinkA --uplinkB=$uplinkB --uplinkC=$uplinkC --uplinkD=$uplinkD --uplinkE=$uplinkE --uplinkF=$uplinkF --uplinkG=$uplinkG --downlinkA=$downlinkA --downlinkB=$downlinkB --downlinkC=$downlinkC --downlinkD=$downlinkD --downlinkE=$downlinkE --downlinkF=$downlinkF --downlinkG=$downlinkG --channelBondingType=$channelBondingType  --n=$n --interBssDistance=$interBssDistance --distance=$distance --ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssC=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssD=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssE=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssF=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssG=$ccaEdThresholdPrimary --ccaEdThresholdSecondaryBssA=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssB=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssC=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssD=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssE=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssF=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssG=$constantCcaEdThresholdSecondaryBss  --channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC --channelBssD=$channelBssD --channelBssE=$channelBssE --channelBssF=$channelBssF --channelBssG=$channelBssG --primaryChannelBssA=$primaryChannelBssA --primaryChannelBssB=$primaryChannelBssB --primaryChannelBssC=$primaryChannelBssC --primaryChannelBssD=$primaryChannelBssD --primaryChannelBssE=$primaryChannelBssE --primaryChannelBssF=$primaryChannelBssF --primaryChannelBssG=$primaryChannelBssG" 

		done
	done
done




channelBssA=42
channelBssB=40
channelBssC=44
channelBssD=42
channelBssE=42
channelBssF=42
channelBssG=42

primaryChannelBssA=36
primaryChannelBssB=40
primaryChannelBssC=44
primaryChannelBssD=48
primaryChannelBssE=36
primaryChannelBssF=40
primaryChannelBssG=44


uplinkA=100
uplinkB=50
uplinkC=50

uplinkD=100
uplinkE=100
uplinkF=100
uplinkG=100
downlinkA=0
downlinkB=0
downlinkC=0
downlinkD=0
downlinkE=0
downlinkF=0
downlinkG=0

payloadSize=1472
simulationTime=5


ccaEdThresholdPrimary=-62
constantCcaEdThresholdSecondaryBss=-1000
channelBondingType=ConstantThreshold
interBssDistance=10
distance=10;
n=10
nBss=3
RngRun=1
mcs=MaxMcs



../waf

for RngRun in 1 ;
do

	for constantCcaEdThresholdSecondaryBss in  -100; do
for interBssDistance in 10;
do
				Test=${nBss}_${n}_${interBssDistance}_${constantCcaEdThresholdSecondaryBss}_${mcs}_${RngRun}
				echo "Starting $Test"
../waf --run "channel-bonding --Test=$Test --mcs=$mcs --nBss=$nBss --simulationTime=$simulationTime --RngRun=$RngRun --uplinkA=$uplinkA --uplinkB=$uplinkB --uplinkC=$uplinkC --uplinkD=$uplinkD --uplinkE=$uplinkE --uplinkF=$uplinkF --uplinkG=$uplinkG --downlinkA=$downlinkA --downlinkB=$downlinkB --downlinkC=$downlinkC --downlinkD=$downlinkD --downlinkE=$downlinkE --downlinkF=$downlinkF --downlinkG=$downlinkG --channelBondingType=$channelBondingType  --n=$n --interBssDistance=$interBssDistance --distance=$distance --ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssC=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssD=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssE=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssF=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssG=$ccaEdThresholdPrimary --ccaEdThresholdSecondaryBssA=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssB=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssC=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssD=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssE=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssF=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssG=$constantCcaEdThresholdSecondaryBss  --channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC --channelBssD=$channelBssD --channelBssE=$channelBssE --channelBssF=$channelBssF --channelBssG=$channelBssG --primaryChannelBssA=$primaryChannelBssA --primaryChannelBssB=$primaryChannelBssB --primaryChannelBssC=$primaryChannelBssC --primaryChannelBssD=$primaryChannelBssD --primaryChannelBssE=$primaryChannelBssE --primaryChannelBssF=$primaryChannelBssF --primaryChannelBssG=$primaryChannelBssG" 

		done
	done
done



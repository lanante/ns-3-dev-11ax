
channelBssA=38
channelBssB=40

primaryChannelBssA=36
primaryChannelBssB=40


uplinkA=10
uplinkB=10

downlinkA=100
downlinkB=100

payloadSize=1472
simulationTime=5


ccaEdThresholdPrimary=-62
constantCcaEdThresholdSecondaryBss=-1000
channelBondingType=DynamicThreshold
interBssDistance=10
distance=10;
n=10
nBss=2
RngRun=1
mcs1=VhtMcs8
mcs2=VhtMcs0

../waf

for RngRun in 1;
do
for constantCcaEdThresholdSecondaryBss in -72;
do
Test=${nBss}_${n}_${interBssDistance}_${channelBondingType}_${constantCcaEdThresholdSecondaryBss}_${mcs1}_${RngRun}
				echo "Starting $Test"
../waf --run "channel-bonding --Test=$Test --mcs1=$mcs1 --mcs2=$mcs2 --nBss=$nBss --simulationTime=$simulationTime --RngRun=$RngRun --uplinkA=$uplinkA --uplinkB=$uplinkB  --downlinkA=$downlinkA --downlinkB=$downlinkB --channelBondingType=$channelBondingType  --n=$n --interBssDistance=$interBssDistance --distance=$distance --ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary  --ccaEdThresholdSecondaryBssA=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssB=$constantCcaEdThresholdSecondaryBss   --channelBssA=$channelBssA --channelBssB=$channelBssB  --primaryChannelBssA=$primaryChannelBssA --primaryChannelBssB=$primaryChannelBssB" 
sleep 0.1
	done
done

wait



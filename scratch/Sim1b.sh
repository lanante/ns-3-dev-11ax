
channelBssA=38
channelBssB=40

primaryChannelBssA=36
primaryChannelBssB=40


uplinkA=0
uplinkB=0

downlinkA=100
downlinkB=10

payloadSize=1472
simulationTime=5


ccaEdThresholdPrimary=-62
constantCcaEdThresholdSecondaryBss=-1000
channelBondingType=ConstantThreshold
interBssDistance=5
distance=10;
n=20
nBss=2
RngRun=1
mcs1=VhtMcs8
mcs2=VhtMcs0

../waf


for constantCcaEdThresholdSecondaryBss in -87 -85 -83 -81 -79 -77 -75 -73 -71 -69 -67 -65 -63 -61 -59 -57;
do
for RngRun in 1 2 3 4 5 6 7 8 9 10;
do
Test=${nBss}_${n}_${interBssDistance}_${channelBondingType}_${constantCcaEdThresholdSecondaryBss}_${mcs1}_${RngRun}
				echo "Starting $Test"
../waf --run "channel-bonding --Test=$Test --mcs1=$mcs1 --mcs2=$mcs2 --nBss=$nBss --simulationTime=$simulationTime --RngRun=$RngRun --uplinkA=$uplinkA --uplinkB=$uplinkB  --downlinkA=$downlinkA --downlinkB=$downlinkB --channelBondingType=$channelBondingType  --n=$n --interBssDistance=$interBssDistance --distance=$distance --ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary  --ccaEdThresholdSecondaryBssA=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssB=$constantCcaEdThresholdSecondaryBss   --channelBssA=$channelBssA --channelBssB=$channelBssB  --primaryChannelBssA=$primaryChannelBssA --primaryChannelBssB=$primaryChannelBssB" &
sleep 0.1
	done
wait
done


constantCcaEdThresholdSecondaryBss=-72
for mcs1 in VhtMcs0 VhtMcs1 VhtMcs2 VhtMcs3 VhtMcs4 VhtMcs5 VhtMcs6 VhtMcs7 VhtMcs8;
do
for RngRun in 1 2 3 4 5 6 7 8 9 10;
do
Test=${nBss}_${n}_${interBssDistance}_${channelBondingType}_${constantCcaEdThresholdSecondaryBss}_${mcs1}_${RngRun}
				echo "Starting $Test"
../waf --run "channel-bonding --Test=$Test --mcs1=$mcs1 --mcs2=$mcs2 --nBss=$nBss --simulationTime=$simulationTime --RngRun=$RngRun --uplinkA=$uplinkA --uplinkB=$uplinkB  --downlinkA=$downlinkA --downlinkB=$downlinkB --channelBondingType=$channelBondingType  --n=$n --interBssDistance=$interBssDistance --distance=$distance --ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary  --ccaEdThresholdSecondaryBssA=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssB=$constantCcaEdThresholdSecondaryBss   --channelBssA=$channelBssA --channelBssB=$channelBssB  --primaryChannelBssA=$primaryChannelBssA --primaryChannelBssB=$primaryChannelBssB" &
sleep 0.1
	done
wait
done




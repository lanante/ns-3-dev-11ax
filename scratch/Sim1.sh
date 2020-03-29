
channelBssA=38
channelBssB=40

primaryChannelBssA=36
primaryChannelBssB=40


uplinkA=0
uplinkB=0

downlinkA=140
downlinkB=10

payloadSize=1472
simulationTime=5


ccaEdThresholdPrimary=-62
constantCcaEdThresholdSecondaryBss=-1000
channelBondingType=ConstantThreshold
interBssDistance=20
distance=10
n=10
nBss=2
RngRun=1
mcs1=VhtMcs8
mcs2=VhtMcs0

../waf

for mcs1 in VhtMcs0 VhtMcs4 VhtMcs8; do
for constantCcaEdThresholdSecondaryBss in -87 -85 -83 -81 -79 -77 -75 -73 -71 -69 -67 -65 -63 -61 -59 -57;
do
for RngRun_j in 0 10 20 30 40 50 60 70 80 90; do
for RngRun_i in 1 2 3 4 5 6 7 8 9 10; do
RngRun=$((RngRun_j+RngRun_i))
Test=${nBss}_${n}_${interBssDistance}_${channelBondingType}_${constantCcaEdThresholdSecondaryBss}_${mcs1}_${RngRun}
				echo "Starting $Test"
../waf --run "channel-bonding --Test=$Test --mcs1=$mcs1 --mcs2=$mcs2 --nBss=$nBss --simulationTime=$simulationTime --RngRun=$RngRun --uplinkA=$uplinkA --uplinkB=$uplinkB  --downlinkA=$downlinkA --downlinkB=$downlinkB --channelBondingType=$channelBondingType  --n=$n --interBssDistance=$interBssDistance --distance=$distance --ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary  --ccaEdThresholdSecondaryBssA=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssB=$constantCcaEdThresholdSecondaryBss   --channelBssA=$channelBssA --channelBssB=$channelBssB  --primaryChannelBssA=$primaryChannelBssA --primaryChannelBssB=$primaryChannelBssB" &
sleep 0.1
	done
wait
done
done
done

for constantCcaEdThresholdSecondaryBss in -82 -72 -62; do
for mcs1 in VhtMcs0 VhtMcs1 VhtMcs2 VhtMcs3 VhtMcs4 VhtMcs5 VhtMcs6 VhtMcs7 VhtMcs8;
do
for RngRun_j in 0 10 20 30 40 50 60 70 80 90; do
for RngRun_i in 1 2 3 4 5 6 7 8 9 10; do
RngRun=$((RngRun_j+RngRun_i))
Test=${nBss}_${n}_${interBssDistance}_${channelBondingType}_${constantCcaEdThresholdSecondaryBss}_${mcs1}_${RngRun}
				echo "Starting $Test"
../waf --run "channel-bonding --Test=$Test --mcs1=$mcs1 --mcs2=$mcs2 --nBss=$nBss --simulationTime=$simulationTime --RngRun=$RngRun --uplinkA=$uplinkA --uplinkB=$uplinkB  --downlinkA=$downlinkA --downlinkB=$downlinkB --channelBondingType=$channelBondingType  --n=$n --interBssDistance=$interBssDistance --distance=$distance --ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary  --ccaEdThresholdSecondaryBssA=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssB=$constantCcaEdThresholdSecondaryBss   --channelBssA=$channelBssA --channelBssB=$channelBssB  --primaryChannelBssA=$primaryChannelBssA --primaryChannelBssB=$primaryChannelBssB" &
sleep 0.1
	done
wait
done
done
done


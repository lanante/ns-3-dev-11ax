
channelBssA=42
channelBssB=42
channelBssC=42
channelBssD=42
channelBssE=42
channelBssF=42

primaryChannelBssA=4
primaryChannelBssB=4
primaryChannelBssC=4
primaryChannelBssD=4
primaryChannelBssE=4
primaryChannelBssF=4

uplinkA=200
uplinkB=200
uplinkC=200
uplinkD=200
uplinkE=200
uplinkF=200


downlinkA=0
downlinkB=0
downlinkC=0
downlinkD=0
downlinkE=0
downlinkF=0

payloadSize=1472
simulationTime=5


ccaEdThresholdPrimary=-62
constantCcaEdThresholdSecondaryBss=-72
channelBondingType=ConstantThreshold
interBssDistance=20
distance=10;
n=10
nBss=6
RngRun=3
mcs=proposedMcs



../waf

for mcs in MaxMcs proposedMcs ;do 
for RngRun_k in {0..900..100}; do
for RngRun_j in {0..90..10}; do
for RngRun_i in 1 2 3 4 5 6 7 8 9 10; do
RngRun=$((RngRun_j+RngRun_i+RngRun_k))

Test=${nBss}_${n}_${interBssDistance}_${channelBondingType}_${constantCcaEdThresholdSecondaryBss}_${mcs}_${RngRun}_all
echo $Test
../waf --run "channel-bonding --Test=$Test 
--channelBondingType=$channelBondingType  --n=$n --interBssDistance=$interBssDistance --distance=$distance 
--nBss=$nBss --simulationTime=$simulationTime --RngRun=$RngRun 
--mcs1=$mcs --mcs2=$mcs  --mcs3=$mcs --mcs4=$mcs  --mcs5=$mcs  --mcs6=$mcs 
--uplinkA=$uplinkA --uplinkB=$uplinkB --uplinkC=$uplinkC --uplinkD=$uplinkD --uplinkE=$uplinkE --uplinkF=$uplinkF 
--downlinkA=$downlinkA --downlinkB=$downlinkB --downlinkC=$downlinkC --downlinkD=$downlinkD --downlinkE=$downlinkE --downlinkF=$downlinkF 
--ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssC=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssD=$ccaEdThresholdPrimary  --ccaEdThresholdPrimaryBssE=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssF=$ccaEdThresholdPrimary 
--ccaEdThresholdSecondaryBssA=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssB=$constantCcaEdThresholdSecondaryBss -ccaEdThresholdSecondaryBssC=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssD=$constantCcaEdThresholdSecondaryBss   -ccaEdThresholdSecondaryBssE=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssF=$constantCcaEdThresholdSecondaryBss 
--channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC --channelBssD=$channelBssD   --channelBssE=$channelBssE --channelBssF=$channelBssF  
--primaryChannelBssA=$primaryChannelBssA --primaryChannelBssB=$primaryChannelBssB --primaryChannelBssC=$primaryChannelBssC --primaryChannelBssD=$primaryChannelBssD  --primaryChannelBssE=$primaryChannelBssE --primaryChannelBssF=$primaryChannelBssF" &
sleep 0.1
done
wait
done
done
done






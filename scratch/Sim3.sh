
channelBssA=42
channelBssB=1
channelBssC=1
channelBssD=1
channelBssE=1
channelBssF=1

primaryChannelBssA=36
primaryChannelBssB=4
primaryChannelBssC=4
primaryChannelBssD=4
primaryChannelBssE=4
primaryChannelBssF=4

uplinkA=250
uplinkB=10
uplinkC=10
uplinkD=10
uplinkE=10
uplinkF=10


downlinkA=0
downlinkB=0
downlinkC=0
downlinkD=0
downlinkE=0
downlinkF=0

payloadSize=1472
simulationTime=50


ccaEdThresholdPrimary=-62
constantCcaEdThresholdSecondaryBss=-72
channelBondingType=ConstantThreshold
interBssDistance=20
distance=8
n=10
nBss=6
RngRun=3
mcs1=MaxMcs
mcs2=VhtMcs0
mcs3=VhtMcs0
mcs4=VhtMcs0
mcs5=VhtMcs0
mcs6=VhtMcs0


../waf

constantCcaEdThresholdSecondaryBssA=-72

for RngRun_k in {0..100..100}; do
for RngRun_j in {0..80..20}; do
for RngRun_i in {1..20}; do
RngRun=$((RngRun_j+RngRun_i+RngRun_k))

Test=${nBss}_${n}_${interBssDistance}_${channelBondingType}_${constantCcaEdThresholdSecondaryBssA}_${mcs1}_${RngRun}
echo $Test
../waf --run "channel-bonding --Test=$Test 
--channelBondingType=$channelBondingType  --n=$n --interBssDistance=$interBssDistance --distance=$distance 
--nBss=$nBss --simulationTime=$simulationTime --RngRun=$RngRun 
--mcs1=$mcs1 --mcs2=$mcs2  --mcs3=$mcs3 --mcs4=$mcs4  --mcs5=$mcs5  --mcs6=$mcs6 
--uplinkA=$uplinkA --uplinkB=$uplinkB --uplinkC=$uplinkC --uplinkD=$uplinkD --uplinkE=$uplinkE --uplinkF=$uplinkF 
--downlinkA=$downlinkA --downlinkB=$downlinkB --downlinkC=$downlinkC --downlinkD=$downlinkD --downlinkE=$downlinkE --downlinkF=$downlinkF 
--ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssC=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssD=$ccaEdThresholdPrimary  --ccaEdThresholdPrimaryBssE=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssF=$ccaEdThresholdPrimary 
--ccaEdThresholdSecondaryBssA=$constantCcaEdThresholdSecondaryBssA --ccaEdThresholdSecondaryBssB=$constantCcaEdThresholdSecondaryBss -ccaEdThresholdSecondaryBssC=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssD=$constantCcaEdThresholdSecondaryBss   -ccaEdThresholdSecondaryBssE=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssF=$constantCcaEdThresholdSecondaryBss 
--channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC --channelBssD=$channelBssD   --channelBssE=$channelBssE --channelBssF=$channelBssF  
--primaryChannelBssA=$primaryChannelBssA --primaryChannelBssB=$primaryChannelBssB --primaryChannelBssC=$primaryChannelBssC --primaryChannelBssD=$primaryChannelBssD  --primaryChannelBssE=$primaryChannelBssE --primaryChannelBssF=$primaryChannelBssF" &
sleep 0.1
done
wait
done
done


mcs1=proposedMcs
constantCcaEdThresholdSecondaryBssA=0
for RngRun_k in {0..100..100}; do
for RngRun_j in {0..80..20}; do
for RngRun_i in {1..20}; do
RngRun=$((RngRun_j+RngRun_i+RngRun_k))

Test=${nBss}_${n}_${interBssDistance}_${channelBondingType}_${constantCcaEdThresholdSecondaryBssA}_${mcs1}_${RngRun}
echo $Test
../waf --run "channel-bonding --Test=$Test 
--channelBondingType=$channelBondingType  --n=$n --interBssDistance=$interBssDistance --distance=$distance 
--nBss=$nBss --simulationTime=$simulationTime --RngRun=$RngRun 
--mcs1=$mcs1 --mcs2=$mcs2  --mcs3=$mcs3 --mcs4=$mcs4  --mcs5=$mcs5  --mcs6=$mcs6 
--uplinkA=$uplinkA --uplinkB=$uplinkB --uplinkC=$uplinkC --uplinkD=$uplinkD --uplinkE=$uplinkE --uplinkF=$uplinkF 
--downlinkA=$downlinkA --downlinkB=$downlinkB --downlinkC=$downlinkC --downlinkD=$downlinkD --downlinkE=$downlinkE --downlinkF=$downlinkF 
--ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssC=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssD=$ccaEdThresholdPrimary  --ccaEdThresholdPrimaryBssE=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssF=$ccaEdThresholdPrimary 
--ccaEdThresholdSecondaryBssA=$constantCcaEdThresholdSecondaryBssA --ccaEdThresholdSecondaryBssB=$constantCcaEdThresholdSecondaryBss -ccaEdThresholdSecondaryBssC=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssD=$constantCcaEdThresholdSecondaryBss   -ccaEdThresholdSecondaryBssE=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssF=$constantCcaEdThresholdSecondaryBss 
--channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC --channelBssD=$channelBssD   --channelBssE=$channelBssE --channelBssF=$channelBssF  
--primaryChannelBssA=$primaryChannelBssA --primaryChannelBssB=$primaryChannelBssB --primaryChannelBssC=$primaryChannelBssC --primaryChannelBssD=$primaryChannelBssD  --primaryChannelBssE=$primaryChannelBssE --primaryChannelBssF=$primaryChannelBssF" &
sleep 0.1
done
wait
done
done



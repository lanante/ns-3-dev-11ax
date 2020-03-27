
channelBssA=42
channelBssB=42
channelBssC=42
channelBssD=42
channelBssE=42
channelBssF=42

primaryChannelBssA=36
primaryChannelBssB=4
primaryChannelBssC=4
primaryChannelBssD=4
primaryChannelBssE=4
primaryChannelBssF=4

uplinkA=0
uplinkB=0
uplinkC=0
uplinkD=0
uplinkE=0
uplinkF=0


downlinkA=400
downlinkB=400
downlinkC=400
downlinkD=400
downlinkE=400
downlinkF=400

payloadSize=1472
simulationTime=5


ccaEdThresholdPrimary=-62
constantCcaEdThresholdSecondaryBss=-1000
channelBondingType=ConstantThreshold
interBssDistance=10
distance=10;
n=10
nBss=6
RngRun=1
mcs1=VhtMcs0
mcs2=VhtMcs0
mcs3=VhtMcs0
mcs4=VhtMcs0
mcs5=VhtMcs0
mcs6=VhtMcs0


../waf




for mcs1 in VhtMcs0 VhtMcs4 VhtMcs8; do
for constantCcaEdThresholdSecondaryBss in -82 -72 -62; do
for RngRun in 1 2 3 4 5 6 7 8 9 10; do
Test=${nBss}_${n}_${interBssDistance}_${channelBondingType}_${constantCcaEdThresholdSecondaryBss}_${mcs1}_${RngRun}
../waf --run "channel-bonding --Test=$Test 
--channelBondingType=$channelBondingType  --n=$n --interBssDistance=$interBssDistance --distance=$distance 
--nBss=$nBss --simulationTime=$simulationTime --RngRun=$RngRun 
--mcs1=$mcs1 --mcs2=$mcs2  --mcs3=$mcs3 --mcs4=$mcs4  --mcs5=$mcs5  --mcs6=$mcs6 
--uplinkA=$uplinkA --uplinkB=$uplinkB --uplinkC=$uplinkC --uplinkD=$uplinkD --uplinkE=$uplinkE --uplinkF=$uplinkF 
--downlinkA=$downlinkA --downlinkB=$downlinkB --downlinkC=$downlinkC --downlinkD=$downlinkD --downlinkE=$downlinkE --downlinkF=$downlinkF 
--ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssC=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssD=$ccaEdThresholdPrimary  --ccaEdThresholdPrimaryBssE=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssF=$ccaEdThresholdPrimary 
--ccaEdThresholdSecondaryBssA=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssB=$constantCcaEdThresholdSecondaryBss -ccaEdThresholdSecondaryBssC=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssD=$constantCcaEdThresholdSecondaryBss   -ccaEdThresholdSecondaryBssE=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssF=$constantCcaEdThresholdSecondaryBss 
--channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC --channelBssD=$channelBssD   --channelBssE=$channelBssE --channelBssF=$channelBssF  
--primaryChannelBssA=$primaryChannelBssA --primaryChannelBssB=$primaryChannelBssB --primaryChannelBssC=$primaryChannelBssC --primaryChannelBssD=$primaryChannelBssD  --primaryChannelBssE=$primaryChannelBssE --primaryChannelBssF=$primaryChannelBssF" &
done
wait
done
done




uplinkA=400
uplinkB=400
uplinkC=400
uplinkD=400
uplinkE=400
uplinkF=400


downlinkA=0
downlinkB=0
downlinkC=0
downlinkD=0
downlinkE=0
downlinkF=0




mcs1=MaxMcs
constantCcaEdThresholdSecondaryBss=-72
for RngRun in 1 2 3 4 5 6 7 8 9 10; do
Test=${nBss}_${n}_${interBssDistance}_${channelBondingType}_${constantCcaEdThresholdSecondaryBss}_${mcs1}_${RngRun}
../waf --run "channel-bonding --Test=$Test 
--channelBondingType=$channelBondingType  --n=$n --interBssDistance=$interBssDistance --distance=$distance 
--nBss=$nBss --simulationTime=$simulationTime --RngRun=$RngRun 
--mcs1=$mcs1 --mcs2=$mcs2  --mcs3=$mcs3 --mcs4=$mcs4    --mcs5=$mcs5  --mcs6=$mcs6 
--uplinkA=$uplinkA --uplinkB=$uplinkB --uplinkC=$uplinkC --uplinkD=$uplinkD --uplinkE=$uplinkE --uplinkF=$uplinkF 
--downlinkA=$downlinkA --downlinkB=$downlinkB --downlinkC=$downlinkC --downlinkD=$downlinkD --downlinkE=$downlinkE --downlinkF=$downlinkF 
--ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssC=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssD=$ccaEdThresholdPrimary  --ccaEdThresholdPrimaryBssE=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssF=$ccaEdThresholdPrimary 
--ccaEdThresholdSecondaryBssA=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssB=$constantCcaEdThresholdSecondaryBss -ccaEdThresholdSecondaryBssC=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssD=$constantCcaEdThresholdSecondaryBss   -ccaEdThresholdSecondaryBssE=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssF=$constantCcaEdThresholdSecondaryBss 
--channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC --channelBssD=$channelBssD   --channelBssE=$channelBssE --channelBssF=$channelBssF  
--primaryChannelBssA=$primaryChannelBssA --primaryChannelBssB=$primaryChannelBssB --primaryChannelBssC=$primaryChannelBssC --primaryChannelBssD=$primaryChannelBssD  --primaryChannelBssE=$primaryChannelBssE --primaryChannelBssF=$primaryChannelBssF" &
done 
wait


channelBssA=42
channelBssB=4
channelBssC=4
channelBssD=4
channelBssE=4
channelBssF=4

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


downlinkA=250
downlinkB=70
downlinkC=70
downlinkD=70
downlinkE=70
downlinkF=70

payloadSize=1472
simulationTime=5


ccaEdThresholdPrimary=-62
constantCcaEdThresholdSecondaryBss=-1000
channelBondingType=ConstantThreshold
interBssDistance=20
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
sleep 0.1
done
wait
done
done




uplinkA=250
uplinkB=70
uplinkC=70
uplinkD=70
uplinkE=70
uplinkF=70


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
sleep 0.1
done 
wait

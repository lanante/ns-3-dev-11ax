rm ../Tput_10.000000.csv
rm ../Tput_20.000000.csv
rm ../Tput_30.000000.csv



channelBssA=38
channelBssB=38
channelBssC=42
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

uplinkA=400
uplinkB=400
uplinkC=0
uplinkD=0
uplinkE=0
uplinkF=0
uplinkG=0
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
ccaEdThresholdSecondary=-62
useDynamicChannelBonding=true
interBssDistance=10
distance=10;
n=10
nBss=1

mcs=VhtMcs0
../waf




for ccaEdThresholdSecondary in -82;
do

for RngRun in 1;
do

for interBssDistance in 10;
do
../waf --run "channel-bonding --mcs=$mcs --nBss=$nBss --simulationTime=$simulationTime --RngRun=$RngRun --uplinkA=$uplinkA --uplinkB=$uplinkB --uplinkC=$uplinkC --uplinkD=$uplinkD --uplinkE=$uplinkE --uplinkF=$uplinkF --uplinkG=$uplinkG --downlinkA=$downlinkA --downlinkB=$downlinkB --downlinkC=$downlinkC --downlinkD=$downlinkD --downlinkE=$downlinkE --downlinkF=$downlinkF --downlinkG=$downlinkG --useDynamicChannelBonding=$useDynamicChannelBonding  --n=$n --interBssDistance=$interBssDistance --distance=$distance --ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssC=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssD=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssE=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssF=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssG=$ccaEdThresholdPrimary --ccaEdThresholdSecondaryBssA=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssB=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssC=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssD=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssE=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssF=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssG=$ccaEdThresholdSecondary  --channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC --channelBssD=$channelBssD --channelBssE=$channelBssE --channelBssF=$channelBssF --channelBssG=$channelBssG --primaryChannelBssA=$primaryChannelBssA --primaryChannelBssB=$primaryChannelBssB --primaryChannelBssC=$primaryChannelBssC --primaryChannelBssD=$primaryChannelBssD --primaryChannelBssE=$primaryChannelBssE --primaryChannelBssF=$primaryChannelBssF --primaryChannelBssG=$primaryChannelBssG" &
done
wait

done
mv ../Tput_10.000000.csv ../Tput_10_${ccaEdThresholdSecondary}.csv 
mv ../Tput_20.000000.csv ../Tput_20_${ccaEdThresholdSecondary}.csv 
mv ../Tput_30.000000.csv ../Tput_30_${ccaEdThresholdSecondary}.csv 
done



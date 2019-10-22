rm ../Tput_VhtMcs0.csv
rm ../Tput_VhtMcs4.csv
rm ../Tput_VhtMcs8.csv

channelBssA=38
channelBssB=38
channelBssC=38


primaryChannelBssA=36
primaryChannelBssB=40
primaryChannelBssC=36

uplinkA=100
uplinkB=100
uplinkC=100
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
distance=10
n=10
nBss=2


../waf



for ccaEdThresholdSecondary in -70;
do
for RngRun in 1;
do

for mcs in VhtMcs0 ;
do
../waf --run "channel-bonding --nBss=$nBss --simulationTime=$simulationTime --RngRun=$RngRun --uplinkA=$uplinkA --uplinkB=$uplinkB --uplinkC=$uplinkC -downlinkA=$downlinkA --downlinkB=$downlinkB --downlinkC=$downlinkC --useDynamicChannelBonding=$useDynamicChannelBonding --mcs=$mcs --n=$n --interBssDistance=$interBssDistance --distance=$distance --ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssC=$ccaEdThresholdPrimary  --ccaEdThresholdSecondaryBssA=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssB=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssC=$ccaEdThresholdSecondary   --channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC  --primaryChannelBssA=$primaryChannelBssA --primaryChannelBssB=$primaryChannelBssB --primaryChannelBssC=$primaryChannelBssC " &
done
wait
done
mv ../Tput_VhtMcs0.csv ../Tput_VhtMcs0_${ccaEdThresholdSecondary}.csv 
mv ../Tput_VhtMcs4.csv ../Tput_VhtMcs4_${ccaEdThresholdSecondary}.csv 
mv ../Tput_VhtMcs8.csv ../Tput_VhtMcs8_${ccaEdThresholdSecondary}.csv 
done



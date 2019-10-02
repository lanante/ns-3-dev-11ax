rm ../Tput_VhtMcs0.csv
rm ../Tput_VhtMcs4.csv
rm ../Tput_VhtMcs8.csv

channelBssA=36
channelBssB=40
channelBssC=38

uplinkA=100
uplinkC=100
uplinkB=100
downlinkA=0
downlinkB=0
downlinkC=0
payloadSize=1472
simulationTime=5


ccaEdThresholdPrimary=-62
ccaEdThresholdSecondary=-62
useDynamicChannelBonding=true
interBssDistance=10
distance=10;
n=10



../waf


for RngRun in {1..50};
do

for ccaEdThresholdSecondary in {-90..-50};
do

for mcs in VhtMcs0 VhtMcs4 VhtMcs8;
do
../waf --run "Case1 --simulationTime=$simulationTime --RngRun=$RngRun --uplinkA=$uplinkA --uplinkB=$uplinkB --uplinkC=$uplinkC --downlinkA=$downlinkA --downlinkB=$downlinkB --downlinkC=$downlinkC --useDynamicChannelBonding=$useDynamicChannelBonding --mcs=$mcs --n=$n --interBssDistance=$interBssDistance --distance=$distance --ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssC=$ccaEdThresholdPrimary --ccaEdThresholdSecondaryBssA=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssB=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssC=$ccaEdThresholdSecondary --channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC " &
done
wait
done

done



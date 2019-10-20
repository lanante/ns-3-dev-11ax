rm ../Tput_10-82.csv
rm ../Tput_10-72.csv
rm ../Tput_10-62.csv
rm ../Tput_20-82.csv
rm ../Tput_20-72.csv
rm ../Tput_20-62.csv
rm ../Tput_30-82.csv
rm ../Tput_30-72.csv
rm ../Tput_30-62.csv



channelBssA=36
channelBssB=40
channelBssC=38
channelBssD=36
channelBssE=40
channelBssF=38
channelBssG=38

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

for ccaEdThresholdSecondary in {-82 -72 -62};
do

for interBssDistance in 10 20 30;
do
../waf --run "Case1 --simulationTime=$simulationTime --RngRun=$RngRun --uplinkA=$uplinkA --uplinkB=$uplinkB --uplinkC=$uplinkC --downlinkA=$downlinkA --downlinkB=$downlinkB --downlinkC=$downlinkC --useDynamicChannelBonding=$useDynamicChannelBonding --n=$n --interBssDistance=$interBssDistance --distance=$distance --ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssC=$ccaEdThresholdPrimary --ccaEdThresholdSecondaryBssA=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssB=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssC=$ccaEdThresholdSecondary --channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC " &
done
wait
done

done



rm ../Tput_10.000000.csv
rm ../Tput_20.000000.csv
rm ../Tput_30.000000.csv




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
simulationTime=1


ccaEdThresholdPrimary=-62
ccaEdThresholdSecondary=-62
useDynamicChannelBonding=true
interBssDistance=10
distance=10;
n=10



../waf




for ccaEdThresholdSecondary in -82;
do

for RngRun in 1;
do

for interBssDistance in 10 20 30;
do
../waf --run "channel-bonding --simulationTime=$simulationTime --RngRun=$RngRun --uplinkA=$uplinkA --uplinkB=$uplinkB --uplinkC=$uplinkC --downlinkA=$downlinkA --downlinkB=$downlinkB --downlinkC=$downlinkC --useDynamicChannelBonding=$useDynamicChannelBonding --n=$n --interBssDistance=$interBssDistance --distance=$distance --ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssC=$ccaEdThresholdPrimary --ccaEdThresholdSecondaryBssA=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssB=$ccaEdThresholdSecondary --ccaEdThresholdSecondaryBssC=$ccaEdThresholdSecondary --channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC " &
done
wait
done
mv ../Tput_10.000000.csv ../Tput_10_${ccaEdThresholdSecondary}.csv 
mv ../Tput_20.000000.csv ../Tput_20_${ccaEdThresholdSecondary}.csv 
mv ../Tput_30.000000.csv ../Tput_30_${ccaEdThresholdSecondary}.csv 
done



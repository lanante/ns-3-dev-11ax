
channelBssA=42
channelBssB=42
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
uplinkC=400
uplinkD=400
uplinkE=400
uplinkF=400
uplinkG=400
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
constantCcaEdThresholdSecondaryBss=-1000
channelBondingType=ConstantThreshold
interBssDistance=10
distance=10;
n=10
nBss=7
RngRun=2


mcs=IdealWifi
../waf

  if [ -z "$1" ]; then
    startRng=1
else
startRng=$1
  fi
  if [ -z "$2" ]; then
    endRng=10
else
endRng=$2
  fi


for (( c=$startRng; c<=$endRng; c++ ))
do
RngRun=$c
for constantCcaEdThresholdSecondaryBss in -82 -72 -62;
do
for interBssDistance in 10 20 30;
do
Test=${nBss}_${n}_${interBssDistance}_${constantCcaEdThresholdSecondaryBss}_${mcs}_${RngRun}
echo "Starting $Test"
nohup ../waf --run "channel-bonding --mcs=$mcs --nBss=$nBss --simulationTime=$simulationTime --RngRun=$RngRun --uplinkA=$uplinkA --uplinkB=$uplinkB --uplinkC=$uplinkC --uplinkD=$uplinkD --uplinkE=$uplinkE --uplinkF=$uplinkF --uplinkG=$uplinkG --downlinkA=$downlinkA --downlinkB=$downlinkB --downlinkC=$downlinkC --downlinkD=$downlinkD --downlinkE=$downlinkE --downlinkF=$downlinkF --downlinkG=$downlinkG --channelBondingType=$channelBondingType  --n=$n --interBssDistance=$interBssDistance --distance=$distance --ccaEdThresholdPrimaryBssA=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssB=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssC=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssD=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssE=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssF=$ccaEdThresholdPrimary --ccaEdThresholdPrimaryBssG=$ccaEdThresholdPrimary --ccaEdThresholdSecondaryBssA=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssB=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssC=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssD=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssE=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssF=$constantCcaEdThresholdSecondaryBss --ccaEdThresholdSecondaryBssG=$constantCcaEdThresholdSecondaryBss  --channelBssA=$channelBssA --channelBssB=$channelBssB --channelBssC=$channelBssC --channelBssD=$channelBssD --channelBssE=$channelBssE --channelBssF=$channelBssF --channelBssG=$channelBssG --primaryChannelBssA=$primaryChannelBssA --primaryChannelBssB=$primaryChannelBssB --primaryChannelBssC=$primaryChannelBssC --primaryChannelBssD=$primaryChannelBssD --primaryChannelBssE=$primaryChannelBssE --primaryChannelBssF=$primaryChannelBssF --primaryChannelBssG=$primaryChannelBssG"  > /dev/null 2>&1& 
done
done
wait
done




#!/bin/bash

# function to run one test
function run_one () {
  echo Running $1
  # run the test
  ../../../waf --run "spatial-reuse --RngRun=$2 --powSta=$3 --powAp=$4 --duration=$5 --d=$6 --r=$7 --n=$8 --uplink=$9 --downlink=${10} --enableRts=${11} --standard=${12} --bw=${13} --txRange=${14} --MCS=${15} --payloadSize=${16} --txStartOffset=${17} --enableObssPd=${18} --txGain=${19} --rxGain=${20} --antennas=${21} --maxSupportedTxSpatialStreams=${22} --maxSupportedRxSpatialStreams=${23} --checkTimings=${24} --scenario=${25} --nBss=${26} --maxAmpduSize=${27}"

  # copy results files
  cd ../scripts
  cp ../../../spatial-reuse-positions.csv "results/spatial-reuse-positions-$1.csv"
  cp ../../../spatial-reuse-rx-sniff.dat  "results/spatial-reuse-rx-sniff-$1.dat"
  cp ../../../spatial-reuse-SR-stats.dat  "results/spatial-reuse-SR-stats-$1.dat"
  if (("${24}" == "1")); then
    cp ../../../spatial-reuse-tgax-calibration-timings.dat  "results/spatial-reuse-tgax-calibration-timings-$1.dat"
  fi

  # positions file
  cd ../scripts
  cp ../../../spatial-reuse-positions.csv spatial-reuse-positions.csv
  gnuplot spatial-reuse-positions.plt
  cp spatial-reuse-positions.png "results/spatial-reuse-positions-$1.png"

  # rx-sniff file
  cd ../scripts
  cp ../../../spatial-reuse-rx-sniff.dat spatial-reuse-rx-sniff.dat
  # in the *rx-sniff.dat file, column 7 is the signal, column 6 is the noise
  signal=7
  noise=6
  # node id of AP1 is 0
  ap1=0
  # noide is of first STA for AP1 is 1
  sta1_1=1
  # node id of last STA for AP1 is n
  sta1_n="$8"
  # node id of AP2 is n+1
  ap2="$((sta1_n+1))"
  # node id of first STA for AP2 is ap2+1
  sta2_1="$((ap2+1))"
  # node id of last STA for AP2
  sta2_n="$((sta2_1+($8)-1))"
  # note:  only getting received packets > 1500b (last parameter below...)
  # AP1 signal
  python ecdf2.py spatial-reuse-rx-sniff.dat "$signal" 0 "$ap1" "$sta1_1" "$sta1_n" "spatial-reuse-rx-sniff-$1-ap1-signal" 1500
  # AP2 noise
  python ecdf2.py spatial-reuse-rx-sniff.dat "$noise"  0 "$ap1" "$sta1_1" "$sta1_n" "spatial-reuse-rx-sniff-$1-ap1-noise" 1500
  # AP2 signal
  python ecdf2.py spatial-reuse-rx-sniff.dat "$signal" 1 "$ap2" "$sta2_1" "$sta2_n" "spatial-reuse-rx-sniff-$1-ap2-signal" 1500
  # AP2 noise
  python ecdf2.py spatial-reuse-rx-sniff.dat "$noise"  1 "$ap2" "$sta2_1" "$sta2_n" "spatial-reuse-rx-sniff-$1-ap2-noise" 1500
  cp *.png ./results/.
  rm *.png
  cd ../examples
}

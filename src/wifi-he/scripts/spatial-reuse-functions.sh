#!/bin/bash

# function to run one test
function run_one () {
  echo Running ${test}
  # run the test
  ../../../waf --run "spatial-reuse \
	--RngRun=${RngRun} \
	--powSta=${powSta} \
	--powAp=${powAp} \
	--duration=${duration} \
	--d=${d} \
	--r=${r} \
	--n=${n} \
	--uplink=${uplink} \
	--downlink=${downlink} \
	--enableRts=${enableRts} \
	--standard=${standard} \
	--bw=${bw} \
	--txRange=${txRange} \
	--MCS=${MCS} \
	--payloadSizeUplink=${payloadSizeUplink} \
	--payloadSizeDownlink=${payloadSizeDownlink} \
	--txStartOffset=${txStartOffset} \
	--enableObssPd=${enableObssPd} \
	--txGain=${txGain} \
	--rxGain=${rxGain} \
	--antennas=${antennas} \
	--maxSupportedTxSpatialStreams=${maxSupportedTxSpatialStreams} \
	--maxSupportedRxSpatialStreams=${maxSupportedRxSpatialStreams} \
	--checkTimings=${performTgaxTimingChecks} \
	--scenario=${scenario} \
	--nBss=${nBss} \
	--maxAmpduSize=${maxAmpduSize} \
	--nodePositionsFile=${nodePositionsFile} \
	--enablePcap=${enablePcap} \
	--enableAscii=${enableAscii} \
	--obssPdThreshold=${obssPdThreshold} \
        --useIdealWifiManager=${useIdealWifiManager}"

  # copy results files
  cd ../scripts
  mkdir -p results
  cp ../../../spatial-reuse-positions.csv "results/spatial-reuse-positions-$test.csv"
  cp ../../../spatial-reuse-rx-sniff.dat  "results/spatial-reuse-rx-sniff-$test.dat"
  cp ../../../spatial-reuse-SR-stats.dat  "results/spatial-reuse-SR-stats-$test.dat"
  if (("${performTgaxTimingChecks}" == "1")); then
    cp ../../../spatial-reuse-tgax-calibration-timings.dat  "results/spatial-reuse-tgax-calibration-timings-${test}.dat"
  fi

  # positions file
  cd ../scripts
  cp ../../../spatial-reuse-positions.csv spatial-reuse-positions.csv
  gnuplot spatial-reuse-positions.plt
  cp spatial-reuse-positions.png "results/spatial-reuse-positions-${test}.png"

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
  sta1_n="$n"
  # node id of AP2 is n+1
  ap2="$((sta1_n+1))"
  # node id of first STA for AP2 is ap2+1
  sta2_1="$((ap2+1))"
  # node id of last STA for AP2
  sta2_n="$((sta2_1+($n)-1))"
  # note:  only getting received packets > 1500b (last parameter below...)
  # AP1 signal
  python ecdf2.py spatial-reuse-rx-sniff.dat "$signal" 0 "$ap1" "$sta1_1" "$sta1_n" "spatial-reuse-rx-sniff-$test-ap1-signal.png" 1500
  # AP2 noise
  python ecdf2.py spatial-reuse-rx-sniff.dat "$noise"  0 "$ap1" "$sta1_1" "$sta1_n" "spatial-reuse-rx-sniff-$test-ap1-noise.png" 1500
  # AP2 signal
  python ecdf2.py spatial-reuse-rx-sniff.dat "$signal" 1 "$ap2" "$sta2_1" "$sta2_n" "spatial-reuse-rx-sniff-$test-ap2-signal.png" 1500
  # AP2 noise
  python ecdf2.py spatial-reuse-rx-sniff.dat "$noise"  1 "$ap2" "$sta2_1" "$sta2_n" "spatial-reuse-rx-sniff-$test-ap2-noise.png" 1500
  cp *.png ./results/.
  rm *.png
  cd ../examples
}

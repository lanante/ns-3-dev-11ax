#!/bin/bash

# function to check if a file already exists, and if so,
# instruct the user to move it somewhere and then run the script
# again.
function check_file () {
  cd ../scripts

  if [[ -e $1 ]]; then

    delete_it=0
    if [ -z "$AUTO_DELETE_SPATIAL_REUSE_OUTPUT_FILES" ]; then
      delete_it=0
    else
      delete_it="$AUTO_DELETE_SPATIAL_REUSE_OUTPUT_FILES"
    fi

    if [ "0" == "$delete_it" ]; then
      echo "****** !!!WARNING!!! ******"
      echo "The following file already exists and cannot be overwritten by the calling script."
      echo "   --> $1"
      echo "Please move the contents to a safe location, or delete it manually, and run the script again."
      echo " "
      echo "If you would prefer to automatically remove pre-existing output files as your script starts,"
      echo "then try > export=AUTO_DELETE_SPATIAL_REUSE_OUTPUT_FILES=1"
      echo "To disable this, try > unset AUTO_DELETE_SPATIAL_REUSE_OUTPUT_FILES"
      echo "EXITING..."
      cd ../examples
      exit 1
    else
      # the var AUTO_DELETE_SPATIAL_REUSE_OUTPUT_FILES is defined (to anything)
      # so auto delete the folder
      rm -f $1
    fi
  fi

  cd ../examples
}

# function to run one test
function run_one () {

  # before doing anything, ensure that the expected output files
  # do not exist.  if any exist, do not continue - make the user
  # move them, or explicitly delete them.
  check_file "./results/spatial-reuse-positions-$test.csv"
  check_file "./results/spatial-reuse-SR-stats-$test.dat"
  check_file "./results/spatial-reuse-A-$test.flowmon"
  check_file "./results/spatial-reuse-operatorA-$test"
  check_file "./results/spatial-reuse-tgax-calibration-timings-${test}.dat"
  check_file "./results/spatial-reuse-positions-${test}.png"
  check_file "./results/spatial-reuse-rx-sniff-$test-ap1-signal.png"
  check_file "./results/spatial-reuse-rx-sniff-$test-ap1-noise.png"
  check_file "./results/spatial-reuse-rx-sniff-$test-ap2-signal.png"
  check_file "./results/spatial-reuse-rx-sniff-$test-ap2-noise.png"
  check_file "./results/spatial-reuse-rx-sniff-$test-ap1-snr.png"
  check_file "./results/spatial-reuse-rx-sniff-$test-ap2-snr.png"

  if [ -z "$MCS" ]; then
    export MCS=0
  fi

  if [ -z "$maxSlrc" ]; then
    export maxSlrc=7
  fi

  if [ -z "$bianchi" ]; then
    export bianchi=0
  fi

  if [ -z "$sigma" ]; then
    export sigma=5.0
  fi

  if [ -z "$applicationTxStart" ]; then
    export applicationTxStart=1.0
  fi

  if [ -z "$rxSensitivity" ]; then
    export rxSensitivity=-91.0
  fi

  if [ -z "$filterOutNonAddbaEstablished" ]; then
    export filterOutNonAddbaEstablished=0
  fi

  if [ -z "$useExplicitBarAfterMissedBlockAck" ]; then
    export useExplicitBarAfterMissedBlockAck=1
  fi

  if [ -z "$enableFrameCapture" ]; then
    export enableFrameCapture=0
  fi

  if [ -z "$enableThresholdPreambleDetection" ]; then
    export enableThresholdPreambleDetection=1
  fi

  if [ -z "$disableArp" ]; then
    export disableArp=1
  fi

  if [ -z "$maxAmpduSizeBss1" ]; then
    export maxAmpduSizeBss1=${maxAmpduSize}
  fi

  if [ -z "$maxAmpduSizeBss2" ]; then
    export maxAmpduSizeBss2=${maxAmpduSize}
  fi

  if [ -z "$maxAmpduSizeBss3" ]; then
    export maxAmpduSizeBss3=${maxAmpduSize}
  fi

  if [ -z "$maxAmpduSizeBss4" ]; then
    export maxAmpduSizeBss4=${maxAmpduSize}
  fi

  if [ -z "$maxAmpduSizeBss5" ]; then
    export maxAmpduSizeBss5=${maxAmpduSize}
  fi

  if [ -z "$maxAmpduSizeBss6" ]; then
    export maxAmpduSizeBss6=${maxAmpduSize}
  fi

  if [ -z "$maxAmpduSizeBss7" ]; then
    export maxAmpduSizeBss7=${maxAmpduSize}
  fi

  if [ -z "$colorBss1" ]; then
    export colorBss1=1
  fi

  if [ -z "$colorBss2" ]; then
    export colorBss2=2
  fi

  if [ -z "$colorBss3" ]; then
    export colorBss3=3
  fi

  if [ -z "$colorBss4" ]; then
    export colorBss4=4
  fi

  if [ -z "$colorBss5" ]; then
    export colorBss5=5
  fi

  if [ -z "$colorBss6" ]; then
    export colorBss6=6
  fi

  if [ -z "$colorBss7" ]; then
    export colorBss7=7
  fi

  if [ -z "$obssPdThreshold" ]; then
    export obssPdThreshold=-99.0
  fi

  if [ -z "$obssPdThresholdBss1" ]; then
    export obssPdThresholdBss1=${obssPdThreshold}
  fi

  if [ -z "$obssPdThresholdBss2" ]; then
    export obssPdThresholdBss2=${obssPdThreshold}
  fi

  if [ -z "$obssPdThresholdBss3" ]; then
    export obssPdThresholdBss3=${obssPdThreshold}
  fi

  if [ -z "$obssPdThresholdBss4" ]; then
    export obssPdThresholdBss4=${obssPdThreshold}
  fi

  if [ -z "$obssPdThresholdBss5" ]; then
    export obssPdThresholdBss5=${obssPdThreshold}
  fi

  if [ -z "$obssPdThresholdBss6" ]; then
    export obssPdThresholdBss6=${obssPdThreshold}
  fi

  if [ -z "$obssPdThresholdBss7" ]; then
    export obssPdThresholdBss7=${obssPdThreshold}
  fi

  if [ -z "$powerBackoff" ]; then
    export powerBackoff=1
  fi

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
        --maxAmpduSizeBss1=${maxAmpduSizeBss1} \
        --maxAmpduSizeBss2=${maxAmpduSizeBss2} \
        --maxAmpduSizeBss3=${maxAmpduSizeBss3} \
        --maxAmpduSizeBss4=${maxAmpduSizeBss4} \
        --maxAmpduSizeBss5=${maxAmpduSizeBss5} \
        --maxAmpduSizeBss6=${maxAmpduSizeBss6} \
        --maxAmpduSizeBss7=${maxAmpduSizeBss7} \
        --nodePositionsFile=${nodePositionsFile} \
        --enablePcap=${enablePcap} \
        --enableAscii=${enableAscii} \
        --obssPdThresholdBss1=${obssPdThresholdBss1} \
        --obssPdThresholdBss2=${obssPdThresholdBss2} \
        --obssPdThresholdBss3=${obssPdThresholdBss3} \
        --obssPdThresholdBss4=${obssPdThresholdBss4} \
        --obssPdThresholdBss5=${obssPdThresholdBss5} \
        --obssPdThresholdBss6=${obssPdThresholdBss6} \
        --obssPdThresholdBss7=${obssPdThresholdBss7} \
        --useIdealWifiManager=${useIdealWifiManager} \
        --test=${test} \
        --maxSlrc=${maxSlrc} \
        --bianchi=${bianchi} \
        --sigma=${sigma} \
        --applicationTxStart=${applicationTxStart} \
        --rxSensitivity=${rxSensitivity} \
        --filterOutNonAddbaEstablished=${filterOutNonAddbaEstablished} \
        --useExplicitBarAfterMissedBlockAck=${useExplicitBarAfterMissedBlockAck} \
        --enableFrameCapture=${enableFrameCapture} \
        --enableThresholdPreambleDetection=${enableThresholdPreambleDetection} \
        --disableArp=${disableArp} \
        --colorBss1=${colorBss1} \
        --colorBss2=${colorBss2} \
        --colorBss3=${colorBss3} \
        --colorBss4=${colorBss4} \
        --colorBss5=${colorBss5} \
        --colorBss6=${colorBss6} \
        --colorBss7=${colorBss7} \
        --powerBackoff=${powerBackoff}\
	--ccaTrSta=${ccaTrSta}\
	--ccaTrAp=${ccaTrAp}\
	--obssPdAlgorithm=${obssPdAlgorithm}" 

  # copy results files
  cd ../scripts
  mkdir -p results
  cp "../../../spatial-reuse-positions-$test.csv" "results/spatial-reuse-positions-$test.csv"
  # to save on disk space, the rx-sniff and tx-power files is not copied to the results/ folder, as this
  # file can grow to be very large and is only used to create the noise, signal and snr cdf plots.
  # cp "../../../spatial-reuse-rx-sniff-$test.dat"  "results/spatial-reuse-rx-sniff-$test.dat"
  # cp "../../../spatial-reuse-tx-power-$test.dat"  "results/spatial-reuse-tx-power-$test.dat"
  cp "../../../spatial-reuse-SR-stats-$test.dat"  "results/spatial-reuse-SR-stats-$test.dat"
  cp "../../../spatial-reuse-A-$test.flowmon"  "results/spatial-reuse-A-$test.flowmon"
  cp "../../../spatial-reuse-operatorA-$test"  "results/spatial-reuse-operatorA-$test"
  if [ -f "../../../spatial-reuse-$test.tr" ]; then
      cp "../../../spatial-reuse-$test.tr"  "results/spatial-reuse-$test.tr"
  fi
  if (("${performTgaxTimingChecks}" == "1")); then
    cp "../../../spatial-reuse-tgax-calibration-timings-$test.dat"  "results/spatial-reuse-tgax-calibration-timings-${test}.dat"
  fi

  # positions file
  cd ../scripts
  cp "../../../spatial-reuse-positions-$test.csv" "spatial-reuse-positions.csv"
  gnuplot -c spatial-reuse-positions.plt "${nBss}"
  cp spatial-reuse-positions.png "results/spatial-reuse-positions-${test}.png"

  # rx-sniff file
  cd ../scripts
  cp "../../../spatial-reuse-rx-sniff-$test.dat" "spatial-reuse-rx-sniff-$test.dat"
  # in the *rx-sniff.dat file, column 7 is the signal, column 6 is the noise, column 9 is snr
  signal=7
  noise=6
  snr=9
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
  python ecdf2.py "spatial-reuse-rx-sniff-$test.dat" "$signal" 0 "$ap1" "$sta1_1" "$sta1_n" "spatial-reuse-rx-sniff-$test-ap1-signal.png" 1500 &
  # AP1 noise
  python ecdf2.py "spatial-reuse-rx-sniff-$test.dat" "$noise"  0 "$ap1" "$sta1_1" "$sta1_n" "spatial-reuse-rx-sniff-$test-ap1-noise.png" 1500 &
  # AP1 snr
  python ecdf2.py "spatial-reuse-rx-sniff-$test.dat" "$snr"  0 "$ap1" "$sta1_1" "$sta1_n" "spatial-reuse-rx-sniff-$test-ap1-snr.png" 1500 &
  # AP2 signal
  python ecdf2.py "spatial-reuse-rx-sniff-$test.dat" "$signal" 1 "$ap2" "$sta2_1" "$sta2_n" "spatial-reuse-rx-sniff-$test-ap2-signal.png" 1500 &
  # AP2 noise
  python ecdf2.py "spatial-reuse-rx-sniff-$test.dat" "$noise"  1 "$ap2" "$sta2_1" "$sta2_n" "spatial-reuse-rx-sniff-$test-ap2-noise.png" 1500 &
  # AP2 snr
  python ecdf2.py "spatial-reuse-rx-sniff-$test.dat" "$snr"  1 "$ap2" "$sta2_1" "$sta2_n" "spatial-reuse-rx-sniff-$test-ap2-snr.png" 1500 &
  wait

  # to reduce disk space usage, the 'rx-sniff' and 'tx-power' files are deleted here, after
  # the simulation has completed, and after the noise and signal plots have been generated
  rm -f "../../../spatial-reuse-rx-sniff-$test.dat"
  rm -f "spatial-reuse-rx-sniff-$test.dat"
  rm -f "../../../spatial-reuse-tx-power-$test.dat"
  rm -f "spatial-reuse-tx-power-$test.dat"

  # copy the signal and noise png files
  cp "spatial-reuse-rx-sniff-$test-ap1-signal.png" ./results/.
  cp "spatial-reuse-rx-sniff-$test-ap1-noise.png" ./results/.
  cp "spatial-reuse-rx-sniff-$test-ap2-signal.png" ./results/.
  cp "spatial-reuse-rx-sniff-$test-ap2-noise.png" ./results/.
  cp "spatial-reuse-rx-sniff-$test-ap1-snr.png" ./results/.
  cp "spatial-reuse-rx-sniff-$test-ap2-snr.png" ./results/.
  rm "spatial-reuse-rx-sniff-$test-ap1-signal.png"
  rm "spatial-reuse-rx-sniff-$test-ap1-noise.png"
  rm "spatial-reuse-rx-sniff-$test-ap2-signal.png"
  rm "spatial-reuse-rx-sniff-$test-ap2-noise.png"
  rm "spatial-reuse-rx-sniff-$test-ap1-snr.png"
  rm "spatial-reuse-rx-sniff-$test-ap2-snr.png"

  # clean up all the output files that are in the root folder
  echo "Complete.  Removing the following files:"
  ls ../../../*$test*
  rm ../../../*$test*

  cd ../examples

  unset MCS
  unset maxSlrc
  unset bianchi
  unset sigma
  unset applicationTxStart
  unset rxSensitivity
  unset filterOutNonAddbaEstablished
  unset useExplicitBarAfterMissedBlockAck
  unset enableFrameCapture
  unset enableThresholdPreambleDetection
  unset disableArp
  unset maxAmpduSizeBss1
  unset maxAmpduSizeBss2
  unset maxAmpduSizeBss3
  unset maxAmpduSizeBss4
  unset maxAmpduSizeBss5
  unset maxAmpduSizeBss6
  unset maxAmpduSizeBss7
  unset colorBss1
  unset colorBss2
  unset colorBss3
  unset colorBss4
  unset colorBss5
  unset colorBss6
  unset colorBss7
  unset obssPdThreshold
  unset obssPdThresholdBss1
  unset obssPdThresholdBss2
  unset obssPdThresholdBss3
  unset obssPdThresholdBss4
  unset obssPdThresholdBss5
  unset obssPdThresholdBss6
  unset obssPdThresholdBss7
  unset powerBackoff
}

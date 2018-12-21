#!/bin/bash

# function to check if a file already existgs, and if so,
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
      echo "then try > export=AUTO_DELETE_SPATIAL_RESUSE_OUTPUT_FILES=1"
      echo "To disable this, try > unset AUTO_DELETE_SPATIAL_REUSE_OUTPUT_FILES"
      echo "EXITING..."
      cd ../examples
      exit 1
    else
      # the var AUTO_DELETE_SPATIAL_REUSE_OUTPUT_FILES is defined (to anything)
      # so auto delete the fult
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

  if [ -z "$maxSlrc" ]; then
    # echo "maxSlrc is not set, defaulting to 7.";
    export maxSlrc=7
  # else
  #  # echo "maxSlrc is set to $maxSlrc";
  fi
  if [ -z "$bianchi" ]; then
    # echo "bianchi is not set, defaulting to 'false'.";
    export bianchi=0
  # else
  #  # echo "bianchi is set to $bianchi";
  fi
  if [ -z "$sigma" ]; then
    # echo "sigma is not set, defaulting to 5.0.";
    export sigma=5.0
  # else
  #   echo "sigma is set to $sigma";
  fi
  if [ -z "$applicationTxStart" ]; then
    # echo "applicationTxStart is not set, defaulting to 1.0.";
    export applicationTxStart=1.0
  # else
  #   echo "applicationTxStart is set to $applicationTxStart";
  fi
  if [ -z "$rxSensitivity" ]; then
    # echo "rxSensitivity is not set, defaulting to -91.0.";
    export rxSensitivity=-91.0
  # else
  #   echo "rxSensitivity is set to $rxSensitivity";
  fi
  if [ -z "$filterOutNonAddbaEstablished" ]; then
    # echo "filterOutNonAddbaEstablished is not set, defaulting to 0.";
    export filterOutNonAddbaEstablished=0
  # else
  #   echo "filterOutNonAddbaEstablished is set to $filterOutNonAddbaEstablished";
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
	--maxAmpduSize=${maxAmpduSize} \
	--nodePositionsFile=${nodePositionsFile} \
	--enablePcap=${enablePcap} \
	--enableAscii=${enableAscii} \
	--obssPdThreshold=${obssPdThreshold} \
        --useIdealWifiManager=${useIdealWifiManager} \
        --test=${test} \
        --maxSlrc=${maxSlrc} \
        --bianchi=${bianchi} \
        --sigma=${sigma} \
        --applicationTxStart=${applicationTxStart} \
        --rxSensitivity=${rxSensitivity} \
        --filterOutNonAddbaEstablished=${filterOutNonAddbaEstablished}"

  # copy results files
  cd ../scripts
  mkdir -p results
  cp "../../../spatial-reuse-positions-$test.csv" "results/spatial-reuse-positions-$test.csv"
  # to save on disk space, the rx-sniff file is not copied to the results/ folder, as this
  # file can grow to be very large and is only used to create the noise and signal cdf plots.
  # cp "../../../spatial-reuse-rx-sniff-$test.dat"  "results/spatial-reuse-rx-sniff-$test.dat"
  cp "../../../spatial-reuse-SR-stats-$test.dat"  "results/spatial-reuse-SR-stats-$test.dat"
  cp "../../../spatial-reuse-A-$test.flowmon"  "results/spatial-reuse-A-$test.flowmon"
  cp "../../../spatial-reuse-operatorA-$test"  "results/spatial-reuse-operatorA-$test"
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
  python ecdf2.py "spatial-reuse-rx-sniff-$test.dat" "$signal" 0 "$ap1" "$sta1_1" "$sta1_n" "spatial-reuse-rx-sniff-$test-ap1-signal.png" 1500 &
  # AP2 noise
  python ecdf2.py "spatial-reuse-rx-sniff-$test.dat" "$noise"  0 "$ap1" "$sta1_1" "$sta1_n" "spatial-reuse-rx-sniff-$test-ap1-noise.png" 1500 &
  # AP2 signal
  python ecdf2.py "spatial-reuse-rx-sniff-$test.dat" "$signal" 1 "$ap2" "$sta2_1" "$sta2_n" "spatial-reuse-rx-sniff-$test-ap2-signal.png" 1500 &
  # AP2 noise
  python ecdf2.py "spatial-reuse-rx-sniff-$test.dat" "$noise"  1 "$ap2" "$sta2_1" "$sta2_n" "spatial-reuse-rx-sniff-$test-ap2-noise.png" 1500 &
  wait

  # to reduce disk space usage, the 'rx-sniff' files are deleted here, after
  # the simulation has completed, and after the noise and signal plots have been generated
  rm "../../../spatial-reuse-rx-sniff-$test.dat"
  rm "spatial-reuse-rx-sniff-$test.dat"

  # copy the signal and noise png files
  cp "spatial-reuse-rx-sniff-$test-ap1-signal.png" ./results/.
  cp "spatial-reuse-rx-sniff-$test-ap1-noise.png" ./results/.
  cp "spatial-reuse-rx-sniff-$test-ap2-signal.png" ./results/.
  cp "spatial-reuse-rx-sniff-$test-ap2-noise.png" ./results/.
  rm "spatial-reuse-rx-sniff-$test-ap1-signal.png"
  rm "spatial-reuse-rx-sniff-$test-ap1-noise.png"
  rm "spatial-reuse-rx-sniff-$test-ap2-signal.png"
  rm "spatial-reuse-rx-sniff-$test-ap2-noise.png"

  # clean up all the output files that are in the root folder
  echo "Complete.  Removing the following files:"
  ls ../../../*$test*
  rm ../../../*$test*

  cd ../examples

}

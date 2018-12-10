Usage
***************************

.. include:: replace.txt
.. highlight:: cpp

.. heading hierarchy:
   ------------- Chapter
   ************* Section (#.#)
   ============= Subsection (#.#.#)
   ############# Paragraph (no number)

Overview
========

This section provides an overview of an example script that simulates 
an 802.11ax spatial reuse scenario, spatial-reuse.cc, and describes
how parameters can be controlled and changed, and what output is produced.

Additionally, utilities and supporting scripts are also described. These
scripts are used to generate experiments that simulate various scenarios
and to plot results from those experiments.

spatial-reuse.cc
================

The program examples/spatial-reuse.cc allows the user to select various
command line parameters that control behaviors of a simulation of a 
wireless network.  Such simulations are especially suited for spatial-reuse
evaluations.

Output
######

By default, the program will simulate 20s of traffic flows for 2 BSSs, each
with 1 AP and 1 STA.  The APs are separate by 100m, and the the STAs are each
placed at a random position within a circle of radius 50m centered at the AP.
Traffic is allocated at 1 Mbps uplink of 1500 byte packets, and 1 Mbps downlink
of 300 byte byptes.  WiFi standard 802.11ac is used by default with 20 MHz channels
using the constant rate wifi manager for datarates with MCS=0.  The residential 
scenario is used, which uses the Ieee80211IndoorPropagationLossModel.

./waf --run spatial-reuse

The above output shows the following results:

Uplink interval:+12000000.0ns Downlink interval:+2400000.0ns

Scenario: residential

Uplink Efficiency   0 [%]

Downlink Efficiency 0 [%]

Throughput,  AP1 Uplink   [Mbps] : 0

Throughput,  AP1 Downlink [Mbps] : 0

Throughput,  AP2 Uplink   [Mbps] : 1.0002

Throughput,  AP2 Downlink [Mbps] : 0.99996

Spatial Reuse Stats written to: spatial-reuse-SR-stats-test.dat

writing flowmon results to spatial-reuse-A-test.flowmon

These results show that the network with AP2 was successful in delivering
the intended uplink and downlink aggregated offered loads of 1 Mbps, while
the first network with AP1 did not achieve any successful data deliveries.
This is because of the randomized node placements of the STAs. For the first
network, the STA is positioned beyond the transmission range of AP1, whereas
the STA for second network is within AP2's transmission range. This
shows that the indoor propagation loss model has limited transmission
range.

Command Line Options
####################

The complete list of program command lines options and there meanings are 
listed below and can be obtained via:

../waf --run "spatial-reuse --PrintHelp"

Program Options:

    --duration:                      Duration of simulation (s) [20]

    --powSta:                        Power of STA (dBm) [10]

    --powAp:                         Power of AP (dBm) [21]

    --txGain:                        Transmission gain (dB) [0]

    --rxGain:                        Reception gain (dB) [0]

    --antennas:                      The number of antennas on each device. [1]

    --maxSupportedTxSpatialStreams:  The maximum number of supported Tx spatial streams. [1]

    --maxSupportedRxSpatialStreams:  The maximum number of supported Rx spatial streams. [1]

    --ccaTrSta:                      CCA Threshold of STA (dBm) [-102]

    --ccaTrAp:                       CCA Threshold of AP (dBm) [-82]

    --d:                             Distance between AP1 and AP2 (m) [100]

    --n:                             Number of STAs to scatter around each AP [1]

    --r:                             Radius of circle around each AP in which to scatter STAs (m) [50]

    --uplink:                        Aggregate uplink load, STAs-AP (Mbps) [1]

    --downlink:                      Aggregate downlink load, AP-STAs (Mbps) [1]

    --standard:                      Set standard (802.11a, 802.11b, 802.11g, 802.11n-5GHz, 802.11n-2.4GHz, 802.11ac, 802.11-holland, 802.11-10MHz, 802.11-5MHz, 802.11ax-5GHz, 802.11ax-2.4GHz) [11ax_5GHZ]

    --bw:                            Bandwidth (consistent with standard, in MHz) [20]

    --enableObssPd:                  Enable OBSS_PD [false]

    --csr:                           Carrier Sense Range (CSR) [m] [1000]

    --enableRts:                     Enable or disable RTS/CTS [false]

    --maxSlrc:                       MaxSlrc [7]

    --txRange:                       Max TX range [m] [54]

    --payloadSizeUplink:             Payload size of 1 uplink packet [bytes] [1500]

    --payloadSizeDownlink:           Payload size of 1 downlink packet [bytes] [300]

    --MCS:                           Modulation and Coding Scheme (MCS) index (default=0) [0]

    --txStartOffset:                 N(0, mu) offset for each node's start of packet transmission.  Default mu=5 [ns] [5]

    --obssPdThreshold:               Energy threshold (dBm) of received signal below which the PHY layer can avoid declaring CCA BUSY for inter-BSS frames. [-99]

    --obssPdThresholdMin:            Minimum value (dBm) of OBSS_PD threshold. [-82]

    --obssPdThresholdMax:            Maximum value (dBm) of OBSS_PD threshold. [-62]

    --checkTimings:                  Perform TGax timings checks (for MAC simulation calibrations). [0]

    --scenario:                      The spatial-reuse scenario (residential, enterprise, indoor, outdoor, study1, study2). [residential]

    --nBss:                          The number of BSSs.  Can be either 1 or 2 (default). [2]

    --maxAmpduSize:                  The maximum A-MPDU size (bytes). [65535]

    --nodePositionsFile:             Node positions file, ns-2 format for Ns2MobilityHelper. []

    --enablePcap:                    Enable PCAP trace file generation. [false]

    --enableAscii:                   Enable ASCII trace file generation. [false]

    --useIdealWifiManager:           Use IdealWifiManager instead of ConstantRateWifiManager [false]

    --test:                          The testname. [test]



Utilities and Supporting Scripts
================================

spatial-reuse-functions.sh
##########################

This is a collection of bash script helper functions that are used by
some additional scripts to run experiments.  In particular, a bash
script function named 'run_one()' is defined that runs
an ns-3 simluation, and then generates various plots
and charts of the results.


The script is invoked by first
setting variable values as environment variables.  For example, a caller
may set a variable as follows:

export RngRun=7

The caller may then invoke the 'run_one' function, which uses 
such parameters to invoke the spatial-reuse.cc script.  For example,
the following shows the first few lines of the run_one() function and
show the invocation of 3 parameters.

#!/bin/bash

# function to run one test
function run_one () {
  echo Running ${test}
  # run the test
  ../../../waf --run "spatial-reuse \
	--RngRun=${RngRun} \
	--powSta=${powSta} \
	--powAp=${powAp} \
        ...

spatial-reuse-positions.plt
##########################

This is a gnuplot script that produces a node positions plot.  
This script has been tested with gnuplot Version 5.0 patchlevel 3. 
Note that other versions of gnuplot may give different results, or fail
to generate plots, due to changing gnuplot parameters and/or features.  
For example, it is noted that gnuplot linestyles are particularly sensitive
to the version of gnuplot used.

ecdf2.py
########

This is a Python program that post-processes simulation results and produces
an Empirical Cumulative Distribution Function (ECDF) plot from a data file.  
The ECDF is plotted using gnuplot.

Bash scripts
============

The following bash shell scripts used to execute 
simulations and validate results.

These scripts are found in the module's `/scripts/` folder.

run-scenarios.sh
################

This script runs several scenarios defined in [TGax15], including the
scenarios for residential, enterprise, small indoor BSSs, and large outdoor BSS.

This script is used to simulate those scenario 
with results documented in the Results chapter.

run-spatial-reuse-scenarios.sh
##############################

This script runs several scenarios that conduct experiments with
2 BSSs and vary parameters, such as the separation distance between
the APs and the number of STAs associated with each AP. 

This script is used to conduct sensitivity studies of key parameters
with results documented in the Results chapter.

run-spatial-reuse-study1.sh
###########################

This script runs several scenarios that conduct experiments with
7 BSSs and vary parameters, as defined by parametric study 1 defined
in this documentation.

This script assume the use of standard 802.11ac, and serves as a baseline
for comparison to 802.11ax and coexistence results.

calibration-scenarios.sh
########################

This script runs several scenarios defined in [TGax15], specifically a 
subset of the MAC simulator calibration scenarios.

This script is used to simulate those scenario 
with results documented in the Results chapter.

calibration-box5.sh
###################

This script runs several scenarios defined in [TGax15], specifically 
several additiona calibration scenarios referred to as the Box5 
scenarios in [TGax15].

This script is used to simulate those scenario 
with results documented in the Results chapter.

plot-individual.sh
##################

This script generates plots from the results that are generated by the
run-spatial-reuse-scenarios.sh script.  A set of plots is generated for each
scenario that was executed.  These plots are used within this documentation.

plot-combined.sh
################

This script generates additional plots from the results that are generated by the
run-spatial-reuse-scenarios.sh script.  These plots are specific plots that
combine two or more scenarios into a single plot.  These plots are 
used within this documentation.

make-data-files.sh
##################

This script is run after the script run-spatial-reuse-scenarios, and prepares
several data files from those results, in preparation for running the plot-individual.sh
and plot-combined.sh scripts.

copy-plots-to-doc-figures.sh
############################

This script copies the plots that are referenced in this documentation and 
that are generated by the plot.sh and plot-combined.sh scripts.  The plots
are copied from the location into which there are generated, and into the
location where the documentation needs the figures, in order for the documentation
generation process to complete.


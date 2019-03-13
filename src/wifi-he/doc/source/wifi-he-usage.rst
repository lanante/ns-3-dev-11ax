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

Finally, a "HOWTO" guide describes several example bash scripts that can
orchestrate various experiments.

spatial-reuse.cc
================

The program ``examples/spatial-reuse.cc`` allows the user to select various
command line parameters that control behaviors of a simulation of a 
wireless network.  Such simulations are especially suited for spatial-reuse
evaluations.

Output
######

By default, the program will simulate 20s of traffic flows for 2 BSSs, each
with 1 AP and 1 STA.  The APs are separated by 100m, and the STAs are each
placed at a random position within a circle of radius 50m centered at the AP.
Traffic is allocated at 1 Mbps uplink consisting of 1500 byte packets, and 
1 Mbps downlink with 300 byte packets.  WiFi standard 802.11ac is used 
by default with 20 MHz channels and a SISO antenna system,
using the constant rate wifi manager for a constant VHT MCS 0.  The residential 
scenario is used, which uses the Ieee80211IndoorPropagationLossModel.

.. sourcecode:: bash

    $ ./waf --run spatial-reuse

The above output shows the following results:

::

    Uplink interval:+12000000.0ns Downlink interval:+2400000.0ns
    ApplicationTxStart: 1 Duration: 20
    nBss: 2 nStas/Bss: 1 => nStas: 2
    Spatial Reuse Statistics
    Scenario: residential
    APs: 2
    Nodes per AP: 1
    Distance between APs [m]: 100
    Radius [m]: 50
    Uplink [Mbps]: 1
    Downlink [Mbps]: 1
    Uplink Efficiency   95.64 [%]
    Downlink Efficiency 95.604 [%]
    Throughput,  AP1 Uplink   [Mbps] : 0.9564
    Throughput,  AP1 Downlink [Mbps] : 0.95604
    Throughput,  AP2 Uplink   [Mbps] : 0.9552
    Throughput,  AP2 Downlink [Mbps] : 0.95604
    Total Throughput Uplink   [Mbps] : 1.9116
    Total Throughput Downlink [Mbps] : 1.91208

These results show that the two networks delivered comparable throughput
performance in this scenario.

Additionally, the spatial-reuse script produces a number of output 
files that are placed in the ``scripts/results/`` folder.  These output files 
will include the name of the test that is specified with the 
--test=<test name> command line option.  The default test name is 
"test".  Thus, for example, using the default configuration, there will
be an output file found as '/scripts/results/spatial-reuse-positions-test.csv'.

 * ``spatial-reuse-positions-<test name>.csv`` - The positions of all nodes in
   the network, used for plotting the topology of the experiment.  This 
   file is divided into several sections, with each section separated by
   two blank lines.  The sections of the positions file are:  1) the 
   bounding box of the topology plot, 2) the next nBss sections are the
   locations of the N APs of the network, and 3) the remaining N sections
   are the locations of the n STAs for each of the nBss BSSs.

 * ``spatial-reuse-<test name>.tr'`` - Ascii trace files if --ascii=1 is specified

 * ``spatial-reuse-state-<test name>.dat`` - PHY states transitions.

 * ``spatial-reuse-rx-sniff-<test name>.dat`` - CSV trace of all received 
   packets, including the receiving node id, the destination node id, the 
   source node id, the receiving node address, the MAC header addr1 and 
   addr2 fields, the noise level of the received packet, the RSSI of the 
   received packet, and the packet length of the received packet.

 * ``spatial-reuse-phy-log-<test name>.dat`` - The statistics of all packet 
   arrivals.

 * ``spatial-reuse-SR-stats-<test name>.dat`` - Main spatial reuse statistics 
   output file.  This includes 1) a summary of the scenario, 2) the totals of 
   aggregated throughput (uplink and downlink), area capacity, and spectrum 
   efficiency for each AP in the network, and 3) the per-node throughput 
   received by each node in the network.

 * ``spatial-reuse-A-<test name>.flowmon`` - Flowmonitor output file for the 
   UDP flows to and from the primary AP (i.e., for operator "A") in the
   network).

 * ``spatial-reuse-operatorA-<test name>`` - Text formatted output of the 
   "per-flow" UDP flows for Operator "A" in the network, including:
   source address, destination address, total packets sent, and total
   bytes sent.  For received packets, the total received bytes, throughput,
   total delay, and total jitter values are also provided.


Command Line Options
####################

The complete list of program command lines options and their meanings can be obtained
by running the following command:

.. sourcecode:: bash

  $ ../waf --run "spatial-reuse --PrintHelp"

Utilities and Supporting Scripts
================================

spatial-reuse-functions.sh
##########################

This is a collection of bash script helper functions that are used by
some additional scripts to run experiments.  In particular, a bash
script function named ``run_one()`` is defined that runs
an ns-3 simulation, and then generates various plots
and charts of the results.

The script is invoked by first
setting variable values as environment variables.  For example, a caller
may set a variable as follows:

.. sourcecode:: bash

  $ export RngRun=7

The caller may then invoke the ``run_one`` function, which uses 
such parameters to invoke the spatial-reuse.cc script.  For example,
the following shows the first few lines of the run_one() function and
show the invocation of 3 parameters.

.. sourcecode:: bash

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
###########################

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

These scripts are found in the module's ``/scripts/`` folder.

run-scenarios.sh
################

This script runs several scenarios defined in [TGax15]_, including the
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
for comparison to 802.11ax results.

run-spatial-reuse-study2.sh
###########################

This script runs several scenarios that conduct experiments with
7 BSSs and vary parameters, as defined by parametric study 2 defined
in this documentation.

This script assume the use of standard 802.11ax and OBSS_PD enabled.

calibration-scenarios.sh
########################

This script runs several scenarios defined in [TGax15]_, specifically a 
subset of the MAC simulator calibration scenarios.

This script is used to simulate those scenarios
with results documented in the Results chapter.

calibration-box5.sh
###################

This script runs several scenarios defined in [TGax15]_, specifically 
several additional calibration scenarios referred to as the Box5 
scenarios in [TGax15]_.

This script is used to simulate those scenarios 
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
are copied from the location into which they are generated, and into the
location where the documentation needs the figures, in order for the documentation
generation process to complete.


"How To" Guide to Running Experiments
=====================================

The following sections describes several example bash scripts that are used
to run various experiments.  For each experiment set, the bash script name
is given along with some remarks that describe what the script does, how it 
is structured, and what results can be expected.

These scripts can be used as a reference for researchers wishing to 
create additional experiments.

All scripts can be found in the wifi-he/scripts/ folder.

These experiments are decomposed into the following subsets:

* Calibration Tests

* TGax Experiments

* Simple Examples

* Bianchi Validation

* Simulation Parameter Sensitivity Study

* Parametric Studies

Calibration Tests
=================

The purpose of the Calibration Tests is to implement the 
calibration tests outlined in [TGax15]_ for validation of 
the Wi-Fi models.

The following bash scripts are available:

 * ``./calibration-scenarios.sh``

 * ``./calibration-box5.sh``

The 'calibration-scenarios.sh' script executes the following calibration tests:
1) Test 1a - MAC overhead w/o RTS/CTS, 2) Test 1b - MAC overhead with RTS/CTS,
3) Test 2a - Deferral Test 1, 4) Test 2b - Deferral Test 2, and
5) Test 3 - NAV Deferral.  The tests are launched as separate ns-3 simulations
that are run in parallel.  Details on these tests can be found in [TGax15]_.
Results of these tests are provided in the Results section of this document.
The scripts are provided to allow researchers to recreate the calibration
results.

The 'calibration-box5.sh' script executes the following calibration tests:
1) Box5 - 1BSS DL only,
2) Box5 - 1BSS UL only 1 STA,
3) Box5 - 1BSS UL only 2 STA,
4) Box5 - 1BSS UL only 3 STA,
5) Box5 - 1BSS DL and UL,
6) Box5 - 2BSS Both DL only,
7) Box5 - 2BSS Both UL only,
8) Box5 - 2BSS A DL and B UL,
9) Box5 - 2BSS A UL and B DL.

The tests are launched as separate ns-3 simulations
that are run in parallel.  Details on these tests can be found in [TGax15]_.
Results of these tests are provided in the Results section of this document.
The scripts are provided to allow researchers to recreate the calibration
results.

TGax Experiments
================

The purpose of the TGax Experiments is to implement the 
TGax scenarios outlined in [TGax15]_ for validation of 
the ns-3 simulator.

These scenarios include:

1 - Residential Scenario

2 - Enterprise Scenario

3 - Indoor Small BSSs Scenario

4 - Outdoor Large BSS Scenario

These scenarios differ the number and distances of the STAs
and APs, and use different propagation loss model parameters.

The following bash script is available:

.. sourcecode:: bash

  $ ./run-scenarios.sh

The tests are launched as separate ns-3 simulations
that are run in parallel.  Details on these tests can be found in [TGax15]_.
Results of these tests are provided in the Results section of this document.
The scripts are provided to allow researchers to recreate the results for
the TGax Experiments.

Simple Examples
===============

The purpose of this simple examples section is to demonstrate some 
simple usage
of the spatial-reuse script and to provide researchers with insights
into the network performance of a dense spatial-reuse scenarios sensitive to
the variation of key parameters.  These tests are based on the 
Residential Scenario of the TGax Experiments script.  A subset of the 
results of this script is included in the Results section of this document.

Transmitted packets use a payload size of 1500 bytes, with an A-MDPU
set to aggregate at most 2 packets (i.e., MaxAmpduSize=3142).  Packets
are sent at a data rate using MCS=0.

The following bash script is available:

.. sourcecode:: bash

   $ ./run-spatial-reuse-scenarios.sh

The following parameters are varied:

n - for values 5, 10, and 20 STAs

d - for values 20, 40, 60, and 80 m

r - for values 10, 20, and 30 m

uplink - for values 1, 2, 3, 4, 5, 6 Mbps

The scripts are provided to allow researchers to study the effects on
network performance when varying key script parameters.

Bianchi Validation
==================

The purpose of the Bianchi Validation tests is to validate
the spatial-reuse script according to the expected performance in 
light of "Bianchi conditions", which are generally in the presence of 
a fully saturated network with each transmitting node having infinite
transmission retries, and no hidden nodes.

The scenario places one BSS of interest at the center of the topology,
surrounded by 6 other BSSs arranged in a hexagonal pattern.  There is 
1 AP at the center of each of the 7 total BSSs.  The APs are separated
by a distance so as to either a) fully decouple the BSSs (i.e., d=120m),
or b) fully couple the BSSs (i.e., d=20m).  Around each AP there are n
STAs that are randomly placed within a circle of radius r=10m centered
at the AP.  The number of STAs varies depending on the validation test,
but generally varies from 5 to 40 STAs per BSS.

Transmitted packets use a payload size of 1500 bytes, with an A-MDPU
set to aggregate at most 2 packets (i.e., MaxAmpduSize=3142).  Packets
are sent at a data rate using MCS=7.  Shadowing loss effects of the
propagation loss model are disabled.

A theoretical calculation of the expected throughput is compared to the
resulting throughput realized by the ns-3 simulation.

The following bash scripts are available:

.. sourcecode:: bash

    $ ./bianchi-test1-dl-only.sh

    $ ./bianchi-test2-up-only.sh

    $ ./bianchi-test3-dl-only.sh

    $ ./bianchi-test4-ul-only.sh

Test 1 considers the scenario where the BSSs are fully decoupled (with
separation distance of 120m between APs) with 40 STAs per BSS receiving
downlink traffic only in a fully saturated network.

Test 2 considers the scenario where the BSSs are fully decoupled (with
separation distance of 120m between APs) with varying numbers of STAs 
(e.g., from 5 to 40 in steps of 5) per BSS transmitting uplink traffic only 
in a fully saturated network.

Test 3 considers the scenario where the BSSs are fully coupled (with
separation distance of 20m between APs) with 40 STAs per BSS receiving
downlink traffic only in a fully saturated network.

Test 4 considers the scenario where the BSSs are fully coupled (with
separation distance of 20m between APs) with varying numbers of STAs 
(e.g., from 5 to 40 in steps of 5) per BSS transmitting uplink traffic only 
in a fully saturated network.

The scripts are provided to allow researchers to recreate the validations
of  the ns-3 simulation performance of dense spatial-reuse scenarios in terms
of Bianchi conditions.

Simulation Parameter Sensitivity Study
======================================

The purpose of the Simulation Parameter Sensitivity Study is to
examine the sensitivity of the spatial-reuse.cc script to the following
command line option parameters:

::

 --applicationStartTx

 --duration

 --RngRun

The script is based on the Bianchi Validation test (Test2) for fully
saturated uplink traffic for n=5 nodes per BSS, with 1 AP in each of 
7 BSSs, where the APs are separated by d=120m..

The following bash script is available:

.. sourcecode:: bash

  $ ./bianchi-test2-run-sensitivity.sh

The tests are launched as separate ns-3 simulations
that are run in parallel.  The scripts are provided to allow 
researchers to study the parametric effects of varying 
parameters that may influence simulation results.

Parametric Studies
==================

The purpose of the Parametric Study 1 is to evaluate spatial reuse
effects for dense node scenarios, while varying several parameters,
such as:

n - vary the number of STAs associated with each BSS from 5 to 40 (in steps of 5)

offered load - vary the total system offered load (aggregated uplink and downlink) 
in Mb/s

The scenario places one BSS of interest at the center of the topology,
surrounded by 6 other BSSs arranged in a hexagonal pattern.  There is 
1 AP at the center of each of the 7 total BSSs. The APs are separated
by a distance of d=34.64m, as modeled in the Residential Scenario described
in [TGax15]_. 
Around each AP there are n STAs that are randomly placed within a circle
of radius r=10m centered at the AP.  The number of STAs varies from 5 to 40 STAs per BSS.

Transmitted packets use a payload size of 1500 bytes, with a MaxAmpduSize=65535.
Packets are sent using the ns-3 Wifi IdealRateManager, which adapts the 
transmission rate per node based on the received SNR.

The following bash script is available:

.. sourcecode:: bash

  $ ./run-spatial-reuse-study1.sh

Running this script produces another script, 'study1.sh' that must then
be run.

The tests are launched as separate ns-3 simulations
that are run in parallel.  The scripts are provided to allow 
researchers to study the effects of varying key parameters 
in a baseline 802.11ac network.  A subset of these results
are provided in the Results section of this document.

It is expected that throughput increases as offered load increases, but
plateaus.  Throughput also decreases as the number of nodes per BSS
is increased.

This study serves as a baseline evaluation of dense spatial-reuse
scenario where all nodes are configured to operate according to the
802.11ac standard.  The script serves as a starting point for researchers
that wish to evaluate the comparable performance found by varying
other key parameters.

For 802.11ax and OBSS_PD enabled tests, the following bash script is available:

.. sourcecode:: bash
 
 $ ./run-spatial-reuse-study2.sh

Running this script produces another script, 'study2.sh' that must then
be run.

Validation
******************************

.. include:: replace.txt
.. highlight:: cpp

.. heading hierarchy:
   ------------- Chapter
   ************* Section (#.#)
   ============= Subsection (#.#.#)
   ############# Paragraph (no number)
   ~~~~~~~~~~~~~ Test case

Overview
========

In this section we first describe the calibration process followed to achieve
comparable results from the WiFi module.  Then we describes the test suites
that are provided in order to validate the proper functionality and correct
simulation output of the modeule.

Calibration Process
===================

To validate the 11ax model implementations, several experiments were conducted
using ns-3 scripts to repeat the calibration tests and scenarios outlines in
[TGax15]_. The parameters as noted in [TGax15]_ are used to set up ns-3
simulations, and any deviations from these parameters are noted below.

The document [TGax15]_ defines simulation scenarios to be used for

* Evaluation of performance of proposed 11ax features

* Generation of results for simulators calibration purposes

These two concepts are generally divided into two sets of simulation scenarios
that we refer to as: 1) Calibration Tests and 2) TGax Scenarios.

The Calibration Tests are used to confirm the calibration of the ns-3
simulator to expected results.  The TGax scenarios evaluate the 
expected performance of the 11ax features, via ns-3 simulations.

Calibration Tests
=================

The calibration tests are described in [TGax15]_ in the section titled
"Scenarios for calibration of MAC simulator".

The following tests are the subset of the TGax calibration tests that have been
simulated in ns-3:

* Test 1a - MAC overhead w/o RTS/CTS

* Test 1b - MAC overhead w/ RTS/CTS

* Test 2a - Deferral Test 1

* Test 2b - Deferral Test 2

* Test 3 -  NAV Deferral

Generally, these tests are quite simple and limit the traffic flow.  For
example, the topology of Test 1a and Test 1b is a single AP with a single STA
with uplink traffic only from  the STA to the AP.  The goal of this test is
verify that the simulator can correctly handle the basic frame exchange
procedure.

Test 2a and Test 2b extend the topology to 2 APs each with 1 STA and test
that the simulator correctly handles the deferral procedure after collisions
happen without hidden nodes.  

Test 3 is the same as Test 2b, but with RTS/CTS on.

All tests assume that all packets offered to the MAC at the sender are
successfully received at the receiver (i.e., PER is 0).

All nodes are configured to use the 11ax features in the 5GHz band
with a 20 MHz bandwidth per channel.
The power at the AP (20 dBm) is slightly higher than the power at
the STAs (15 dBm).

Senders transmit packets of 1500 bytes using MPDU aggregation with a
2 MPDUs per A-MPDU limit.  Data rates using MCS 0 are assumed.

Path loss uses the 'residential' scenario described later.

Common Parameters
#################

The following parameters are set for all all calibration scenarios.

* ``RngRun`` = 1

* ``powSta`` = 15

* ``powAp`` = 20

* ``duration`` = 10.0

* ``enableRts`` = 0

* ``standard`` = 11ax_5GHZ

* ``bw`` = 20

* ``MCS`` = 0

* ``payloadSize`` = 1500

* ``txStartOffset`` = 5

* ``enableObssPd`` = 1

* ``txGain`` = 0

* ``rxGain`` = 0

* ``antennas`` = 1

* ``maxSupportedTxSpatialStreams`` = 1

* ``maxSupportedRxSpatialStreams`` = 1

* ``performTgaxTimings`` = 1

* ``scenario`` = residential

* ``nBss`` = 2

* ``maxAmpduSize`` = 3140

* ``nodePositionsFile`` = NONE

* ``enablePcap`` = 0

* ``enableAscii`` = 0

* ``obssPdThreshold`` = -82

* ``useIdealWifiManager`` = 0



Test 1a - MAC overhead w/o RTS/CTS
##################################

The goal of this test is to verify whether the simulator can correctly
handle the basic frame exchange procedure, including AIFS+backoff
procedure and A-MPDU + SIFS + BA sequence, and also to make sure the
overheads are computed correctly.

The simulation parameters vary from the common parameters by reducing 
the number of BSSs to nBSS=1, and the number of STAs also to n=1, where the 
single STA is dropped into a random position with a radius of r=30m 
centered at the AP.  An total offered load of 10 Mbps is allocated to
uplink traffic only.

The following parameters further refine the Test 1a calibration scenario
from the common parameters.


* ``test`` = calibration-test1a

* ``d`` = 1000

* ``r`` = 30

* ``nBss`` = 1

* ``n`` = 1

* ``uplink`` = 10.0

* ``downlink`` = 0.0

* ``txRange`` = 54

Results
#######

The figures in the section were generated using the ns-3-dev-11ax
repository as of commit changeset c85f4dc.

The node positions for the calibration test1a scenario are given below.

.. _positions-calibration-test1a:

.. figure:: figures/spatial-reuse-positions-calibration-test1a.*
   :align: center

   Node positions for the calibration test 1a.

Figure :ref:`positions-calibration-test1a` illustrates the node positions
of the calibration test 1a scenario.

We can see that there is one BSS with one AP at its center, and 1 STA at
a random position within the r=30m radius about the AP.

The simulation results in throughput at the AP of 6.627 Mbps, or 66.27% of the
10 Mbps offered load, indicating that this simulation scenario saturates the
network.

Since there is only 1 transitting STA, all signal levels
and noise levels from the perspective of the receiving AP should be identical,
as the STA is not moving, and there are no other signals in the network that 
interfere with its transmissions.  The Empirical Cumulative Distribution 
Function (ECDF) for the signal level and noise levels are provided below,
and show that there is no variation amoung the levels.

The ECDF of the signals received by AP1 is given below.

.. _calibration-test1a-ap1-signal:

.. figure:: figures/spatial-reuse-rx-sniff-calibration-test1a-ap1-signal.*
   :align: center

   ECDF of the signals received by AP1 for the calibration test 1a.

Figure :ref:`calibration-test1a-ap1-signal` illustrates the ECDF of the signals
received by AP1 for the calibration test 1a scenario.

The ECDF of the noise levels received by AP1 is given below.

.. _calibration-test1a-ap1-noise:

.. figure:: figures/spatial-reuse-rx-sniff-calibration-test1a-ap1-noise.*
   :align: center

   ECDF of the noise level received by AP1 for the calibration test 1a.

Figure :ref:`calibration-test1a-ap1-noise` illustrates the ECDF of the noise levels
received by AP1 for the calibration test 1a scenario.


Test 1b - MAC overhead w/ RTS/CTS
#################################

Test 1b is nearly identical to Test 1a, except that RTS/CTS is enabled
in Test 1b, whereas it is disabled in Test 1a.  Thus, this test case is
designed to further verify whether the simulator correctly handles the 
frame exchange procedure with RTS/CTS protection based on test1a. 
It also tests whether the correct overhead computation with RTS /CTS.

The following parameters further refine the Test 1b calibration scenario from
the common parameters.

* ``test`` = calibration-test1b

* ``d`` = 1000

* ``r`` = 30

* ``nBss`` = 1

* ``n`` = 1

* ``uplink`` = 10.0

* ``downlink`` = 0.0

* ``enableRts`` = 1

* ``txRange`` = 54

Results
#######

The node positions for the calibration test1b scenario are identical to test1a.

The simulation results in throughput at the AP of 6.4452 Mbps, or 64.45% of the
10 Mbps offered load, indicating that this simulation scenario saturates the
network.  With CTS/RTS enabled, the effective throughput is lower for Test 1b
than it is with Test 1a, where CTS/RTS is disabled.

Since there is only 1 transitting STA, all signal levels
and noise levels from the perspective of the receiving AP should be identical,
as the STA is not moving, and there are no other signals in the network that 
interfere with its transmissions.  The Empirical Cumulative Distribution 
Function (ECDF) for the signal level and noise levels are provided below,
and show that there is no variation amoung the levels.

Test 2a - Deferral Test 1
#########################

The goal of this test is to verify whether the simulator can correctly handle 
the deferral procedure after collision happens without hidden nodes. It also 
checks whether deferral because of energy levels is happening correctly.

The topology of this test assumes two APs each with one STA, such that each  
STA is essentially co-located with the other AP.  For example, AP1 and STA2
are essentially co-located.  Each STA generated uplink traffic to its AP.
Because of the co-location of nodes, we can expect transmission collisions that
require the deferral procedure.

The following parameters further refine the Test 2a calibration scenario from
the common parameters.

* ``test`` = calibration-test2a

* ``d`` = 10

* ``r`` = 10

* ``nBss`` = 2

* ``n`` = 1

* ``uplink`` = 10.0

* ``downlink`` = 0.0

* ``enableRts`` = 0

* ``txRange`` = 54

Results
#######

The node positions for the calibration test2a scenario are given below.

.. _positions-calibration-test2a:

.. figure:: figures/spatial-reuse-positions-calibration-test2a.*
   :align: center

   Node positions for the calibration test 2a.

Figure :ref:`positions-calibration-test2a` illustrates the node positions
of the calibration test 2a scenario.

We can see that there are two BSS each with one AP at its center that are 
separated by d=10m.  There is 1 STA for each BSS placed in 
a random position within the r=10m radius about the correponding AP.

Because all of the nodes are within the Carrier Sense Range (CSR) of the each
AP, we can expect transmission collisions to require the deferral procedure.

The simulation results in throughput of 3.042 Mbps and 3.0528 Mbps at AP1 and AP2,
respectively.  This is 30.04% and 30.05%, respectively, of the 10 Mbps offered load.  
The results indicate that this simulation scenario saturates the
network and results in significantly lower throughput as compared to the Test 1a, 
where only 1 STA was transmitting without any potential transmission interference from
other nodes.  The reduced throughput is explained by the deferral procedure.

The signal levels at each receiving AP for the transmissions of its 
associated STA should be identical, as the STA is not moving.  
The Empirical Cumulative Distribution Function (ECDF) for the signal 
level and noise levels are provided below, and show that there is no variation
amoung the levels within each respective BSS.

The ECDF of the signals received by AP1 and AP2 are given below.

.. _calibration-test2a-ap1-signal:

.. figure:: figures/spatial-reuse-rx-sniff-calibration-test2a-ap1-signal.*
   :align: center

   ECDF of the signals received by AP1 for the calibration test 2a.

Figure :ref:`calibration-test2a-ap1-signal` illustrates the ECDF of the signals
received by AP1 for the calibration test 2a scenario.

.. _calibration-test2a-ap2-signal:

.. figure:: figures/spatial-reuse-rx-sniff-calibration-test2a-ap2-signal.*
   :align: center

   ECDF of the signals received by AP2 for the calibration test 2a.

Figure :ref:`calibration-test2a-ap2-signal` illustrates the ECDF of the signals
received by AP2 for the calibration test 2a scenario.

The ECDF of the noise levels received by AP1 and AP2 are given below.

.. _calibration-test2a-ap1-noise:

.. figure:: figures/spatial-reuse-rx-sniff-calibration-test2a-ap1-noise.*
   :align: center

   ECDF of the noise level received by AP1 for the calibration test 2a.

Figure :ref:`calibration-test2a-ap1-noise` illustrates the ECDF of the noise levels
received by AP1 for the calibration test 2a scenario.

.. _calibration-test2a-ap2-noise:

.. figure:: figures/spatial-reuse-rx-sniff-calibration-test2a-ap2-noise.*
   :align: center

   ECDF of the noise level received by AP2 for the calibration test 2a.

Figure :ref:`calibration-test2a-ap2-noise` illustrates the ECDF of the noise levels
received by AP2 for the calibration test 2a scenario.

Test 2b - Deferral Test 2
#########################

The goal of this test case is to verify that the simulator correctly handles 
the deferral procedure after collisions happens with the existence of 
hidden nodes.

The topology of this test is similar to that of Test 2a, except that the APs
are further separated so that they cannot hear each other, while the STAs are 
essentially co-located.  

The following parameters further refine the Test 2a calibration scenario from
the common parameters.

* ``test`` = calibration-test2b

* ``d`` = 20

* ``r`` = 10

* ``nBss`` = 2

* ``n`` = 1

* ``uplink`` = 10.0

* ``downlink`` = 0.0

* ``enableRts`` = 0

* ``txRange`` = 54

Results
#######

The node positions for the calibration test2b scenario are given below.

.. _positions-calibration-test2b:

.. figure:: figures/spatial-reuse-positions-calibration-test2b.*
   :align: center

   Node positions for the calibration test 2b.

Figure :ref:`positions-calibration-test2b` illustrates the node positions
of the calibration test 2b scenario.

We can see that there are two BSS each with one AP at its center that are 
separated by d=20m.  There is 1 STA for each BSS placed in 
a random position within the r=10m radius about the correponding AP.  The two
STAs are near one another.

The simulation results in throughput of 3.4416 Mbps and 3.3492 Mbps at AP1 and AP2,
respectively.  This is 34.16% and 33.49%, respectively, of the 10 Mbps offered load.  
The results indicate that this simulation scenario saturates the
network and results in significantly lower throughput as compared to the Test 1a, 
where only 1 STA was transmitting without any potential transmission interference from
other nodes.  The reduced throughput is explained by the deferral procedure.

The signal levels at each receiving AP for the transmissions of its 
associated STA should be identical, as the STA is not moving.  
The Empirical Cumulative Distribution Function (ECDF) for the signal 
level and noise levels are provided below, and show that there is no variation
amoung the levels within each respective BSS.

The ECDF of the signals received by AP1 and AP2 are given below.

.. _calibration-test2b-ap1-signal:

.. figure:: figures/spatial-reuse-rx-sniff-calibration-test2b-ap1-signal.*
   :align: center

   ECDF of the signals received by AP1 for the calibration test 2b.

Figure :ref:`calibration-test2b-ap1-signal` illustrates the ECDF of the signals
received by AP1 for the calibration test 2b scenario.

.. _calibration-test2b-ap2-signal:

.. figure:: figures/spatial-reuse-rx-sniff-calibration-test2b-ap2-signal.*
   :align: center

   ECDF of the signals received by AP2 for the calibration test 2b.

Figure :ref:`calibration-test2b-ap2-signal` illustrates the ECDF of the signals
received by AP2 for the calibration test 2b scenario.

The ECDF of the noise levels received by AP1 and AP2 are given below.

.. _calibration-test2b-ap1-noise:

.. figure:: figures/spatial-reuse-rx-sniff-calibration-test2b-ap1-noise.*
   :align: center

   ECDF of the noise level received by AP1 for the calibration test 2b.

Figure :ref:`calibration-test2b-ap1-noise` illustrates the ECDF of the noise levels
received by AP1 for the calibration test 2b scenario.

.. _calibration-test2b-ap2-noise:

.. figure:: figures/spatial-reuse-rx-sniff-calibration-test2b-ap2-noise.*
   :align: center

   ECDF of the noise level received by AP2 for the calibration test 2b.

Figure :ref:`calibration-test2b-ap2-noise` illustrates the ECDF of the noise levels
received by AP2 for the calibration test 2b scenario.

Test 3 - NAV Deferral
#####################

NOTE:  Currently Test 3 fails to complete, and documentation results are therefore incomplete.

This test is the same as test 2b, but with RTS/CTS on.

The topology of this test is identical to that of Test 2b.
	
The following parameters further refine the Test 3 calibration scenario from
the common parameters.

* ``test`` = calibration-test3

* ``d`` = 20

* ``r`` = 10

* ``nBss`` = 2

* ``n`` = 1

* ``uplink`` = 10.0

* ``downlink`` = 0.0

* ``enableRts`` = 1

* ``txRange`` = 54

Results
#######

The simulation results in throughput of 3.2664 Mbps and 3.2076 Mbps at AP1 and AP2,
respectively.  This is 32.66% and 32.08%, respectively, of the 10 Mbps offered load.  
The results indicate that this simulation scenario saturates the
network and results in significantly lower throughput as compared to the Test 1a, 
where only 1 STA was transmitting without any potential transmission interference from
other nodes.  The reduced throughput is explained by the deferral procedure.

Box5 Calibration Tests
======================

In addition to the calibration tests mentioned above, additional tests are
defined in [TGax15]_ for "Box5 calibration".  These tests assume specific node
locations for 3 BSSs and the STAs associated with them.  

The box5 calibration tests are described in [TGax15]_ in the section titled
"Scenarios for calibration of Box5 simulator".

The following TGax Box5 calibration tests that have been simulated in ns-3:

* Test 1 - 1 BSS, Downlink only

* Test 2 - 1 BSS, Uplink only, 1 STA

* Test 3 - 1 BSS, Uplink only, 2 STAs

* Test 4 - 1 BSS, Uplink only, 3 STAs

* Test 5 - 1 BSS, Downlink and Uplink

* Test 6 - 2 BSS, both Downlink only

* Test 7 - 2 BSS, both Uplink only

* Test 8 - 2 BSS, A Downlink and B Uplink

* Test 9 - 2 BSS, A Uplink and B Downlink

Generally, these tests are similar to the calibration scenarios described
above, except that these box5 scenarios have precise locations for all nodes.
This feature is modelled in ns-3 using the MobilityHelper and a positions 
trace file was created to represent the node locations.

Path loss uses the 'residential' scenario described later.

Common Parameters
#################

The following parameters are set for all all calibration scenarios.

* ``RngRun`` = 1

* ``powSta`` = 15

* ``powAp`` = 20

* ``duration`` = 3.0

* ``enableRts`` = 0

* ``standard`` = 11ax_5GHZ

* ``bw`` = 80

* ``MCS`` = 5

* ``payloadSize`` = 1500

* ``txStartOffset`` = 5

* ``enableObssPd`` = 1

* ``txGain`` = 0

* ``rxGain`` = 0

* ``antennas`` = 1

* ``maxSupportedTxSpatialStreams`` = 1

* ``maxSupportedRxSpatialStreams`` = 1

* ``performTgaxTimings`` = 1

* ``scenario`` = indoor

* ``nBss`` = 1

* ``maxAmpduSize`` = 65535

* ``nodePositionsFile`` = src/wifi-he/scripts/box5.ns2

* ``enablePcap`` = 0

* ``enableAscii`` = 0

* ``obssPdThreshold`` = -82

* ``useIdealWifiManager`` = 0


Validation
==========

The figures in the section were generated using the ns-3-dev-11ax
repository as of commit changeset c85f4dc.

The validation tests describe here follow the tests described 
in [TGax15]_ in the section titled "Scenarios summary".

These scenarios are more extensive than the calibration scenarios
documented above.

The following tests are the subset of the TGax scnearios that have been
simulated in ns-3:

* Test 1 - Residential Scenario

* Test 2 - Enterprise Scenario

* Test 3 - Indoor Small BSSs Scenario

* Test 4 - Outdoor Large BSS Scenario


Generally, these tests introduce more densely arranged scenarios with
larger numbers of STAs and APs that are near one another.  Such scenarios
are useful in evaluating the spatial reuse features of the 802.11ax
standard.  The scenarios differ in the distances between APs, 
the number of STAs per BSS, and the path loss models used.

Common Parameters
#################

All tests assume that all packets offered to the MAC at the sender are
successfully received at the receiver (i.e., PER is 0).

All nodes are configured to use the 11ax features in the 5GHz band
with a 80 MHz bandwidth per channel.
The power at the AP (20 dBm) is slightly higher than the power at
the STAs (15 dBm).

Senders transmit packets of 1500 bytes using MPDU aggregation with a
2 MPDUs per A-MPDU limit.  Data rates using MCS 0 are assumed.

Path loss is distance-based and are evaluated with the operating carrier
frequency (i.e., 5 GHz).  

Nodes are placed assuming 2-D positions, and antenna heights are thus
assumed to be 0m.

The following parameters are set for all all validation scenarios.

* ``RngRun`` = 1

* ``powSta`` = 15

* ``powAp`` = 20

* ``duration`` = 10.0

* ``enableRts`` = 0

* ``standard`` = 11ax_5GHZ

* ``bw`` = 80

* ``MCS`` = 0

* ``payloadSize`` = 1500

* ``txStartOffset`` = 5

* ``enableObssPd`` = 1

* ``txGain`` = 0

* ``rxGain`` = 0

* ``antennas`` = 1

* ``maxSupportedTxSpatialStreams`` = 1

* ``maxSupportedRxSpatialStreams`` = 1

* ``performTgaxTimings`` = 1

* ``scenario`` = residential

* ``nBss`` = 2

* ``maxAmpduSize`` = 3140

* ``nodePositionsFile`` = NONE

* ``enablePcap`` = 0

* ``enableAscii`` = 0

* ``obssPdThreshold`` = -82

* ``useIdealWifiManager`` = 0

It is noted that some parameters vary from those mentioned
in [TGax15]_.  In particular, the nx-3 simulations assume
1 antenna per node (AP or STA) with no antenna gains.  Contrastingly.
the parameters given in [TGax15]_ assume 4 antennas per AP with 0 dBi gain,
and 2 antennas per STA with -2 dBi gain.

Test 1 - Residential Scenario
#############################

The residential scenario is derived from an environment described by 
a multi-floor building, where there are 2 - 10m x 10m apartments on each floor.
One AP and some number of STAs are assumed in each apartment.

The scenario is simulated in ns-3 as follows:  2 APs are placed d=10m 
apart, each with n=10 STAs that are dropped within a circle of r=5m around
the AP.  

Traffic flow is modelled as a total aggregated offered load of 10 Mbps of 
uplink traffic only.  With a total uplink load of 10 Mbps and n=10 STAs per
network, the uplink load per STA is 10 Mbps / 10 nodes = 1 Mbps per STA.
Note that this load is a deviation from the scenario as documented in 
[TGax15]_, in which different traffic models are assigned 
separately to each "apartment".

The residential path loss model is given by:

Pathloss model

PL(d) = 40.05 + 20*log10(fc/2.4) + 20*log10(min(d,5)) + (d>5) * 35*log10(d/5) + 18.3*F^((F+2)/(F+1)-0.46) + 5*W

where:

–	d = max(3D distance [m], 1)

–	fc = frequency [GHz]

–	F = number of floors traversed

–	W = number of walls traversed in x-direction plus number of walls traversed in y-direction

This pathloss model is modelled in ns-3 using the 
Ieee80211axIndoorPropagationLoss model with attribute values
"DistanceBreakPoint" = 5, "Walls" = 1, and "WallsFactor" = 5.

The following parameters further refine the Residential Scenario
from the common parameters.


* ``test`` = TGax-test1

* ``d`` = 10

* ``r`` = 5

* ``n`` = 10

* ``uplink`` = 10.0

* ``downlink`` = 0.0

* ``txRange`` = 54

* ``scenario`` = residential

Results
#######

The node positions for the Residential Scenario are given below.

.. _positions-TGax-test1:

.. figure:: figures/spatial-reuse-positions-TGax-test1.*
   :align: center

   Node positions for the Residential Scenario.

Figure :ref:`positions-TGax-test1` illustrates the node positions
of the Residential Scenario.

We can see that there are two BSS each with one AP at its center, 
representing two apartments, each with 10 STAs at random position 
within the r=5m radius about the AP.  

The simulation results in throughput of 8.3652 Mbps and 8.3736 Mbps at AP1 and AP2,
respectively.  This is 83.65% and 83.74%, respectively, of the 10 Mbps offered load.  
The results indicate that this simulation scenario saturates the
network.

Since each network has a number of sending STAs, an examination
of the distribution of signal level and noise levels, from the
perstpective of the receiving AP, helps provide insight into
the behavior of the network on a node-by-node level.
The Empirical Cumulative Distribution 
Function (ECDF) for the signal level and noise levels are provided below,
and show that there is variation amoung the levels.

Steps in the signal and noise level plots indicate transitions in 
the data that generally correspond to different nodes.  For example,
the plots below are generated from the scenario with n=10 STAs 
associated per AP, and there are roughly 10 steps.  Vertical
lines indicate that the signal and noise levels per STA remain
constant, which is explained by the deterministic evaluation of
the signal at the receiver, which is distance-based, and is not
varying because the nodes are not moving.

The ECDF of the signals received by AP1 and AP2 are given below.

.. _TGax-test1-ap1-signal:

.. figure:: figures/spatial-reuse-rx-sniff-TGax-test1-ap1-signal.*
   :align: center

   ECDF of the signals received by AP1 for the Residential Scenario.

Figure :ref:`TGax-test1-ap1-signal` illustrates the ECDF of the signals
received by AP1 for the Residential Scenario.

.. _TGax-test1-ap2-signal:

.. figure:: figures/spatial-reuse-rx-sniff-TGax-test1-ap2-signal.*
   :align: center

   ECDF of the signals received by AP2 for the Residential Scenario.

Figure :ref:`TGax-test1-ap2-signal` illustrates the ECDF of the signals
received by AP2 for the Residential Scenario.

It is observed that the distribution of the signal levels from the
10 randomly positioned STAs is almost linear, although there is a 
larger concentration of signal levels in the range of 
approximately -58 to -53 dB.

The ECDF of the noise levels received by AP1 and AP2 are given below.

.. _TGax-test1-ap1-noise:

.. figure:: figures/spatial-reuse-rx-sniff-TGax-test1-ap1-noise.*
   :align: center

   ECDF of the noise level received by AP1 for the Residential Scenario.

Figure :ref:`TGax-test1-ap1-noise` illustrates the ECDF of the noise levels
received by AP1 for the Residential Scenario.

.. _TGax-test1-ap2-noise:

.. figure:: figures/spatial-reuse-rx-sniff-TGax-test1-ap2-noise.*
   :align: center

   ECDF of the noise level received by AP2 for the Residential Scenario.

Figure :ref:`TGax-test1-ap2-noise` illustrates the ECDF of the noise levels
received by AP2 for the Residential Scenario.

It is observed that the distribution of the noise levels shows that
noise levels are predominantly low, with greater than 80% of the time that 
a packet is received, the noise level being approximately -86 dB (BW = 80 MHz).


Test 2 - Enterprise Scenario
#############################

The enterprise scenario is derived from a "wireless office" environment described by 
STA clusters (cubicles) with APs positioned to service them.

The scenario is simulated in ns-3 as follows:  2 APs are placed d=10m 
apart, each with n=100 STAs that are dropped within a circle of r=5m around
the AP.  

Traffic flow is modelled as a total aggregated offered load of 10 Mbps of 
uplink traffic only.  With a total uplink load of 10 Mbps and n=100 STAs per
network, the uplink load per STA is 10 Mbps / 100 nodes = 0.1 Mbps per STA.
Note that this load is a deviation from the scenario as documented in 
[TGax15]_, in which different traffic models are assigned 
separately to each AP.

The enterprise path loss model is given by:

Pathloss model

PL(d) = 40.05 + 20*log10(fc/2.4) + 20*log10(min(d,10)) + (d>10) * 35*log10(d/10) + 7*W

where:

–	d = max(3D-distance [m], 1)

–	fc = frequency [GHz]

–	W = number of office walls traversed in x-direction plus number of office walls traversed in y-direction


This pathloss model is modelled in ns-3 using the 
Ieee80211axIndoorPropagationLoss model with attribute values
"DistanceBreakPoint" = 10, "Walls" = 1, and "WallsFactor" = 7.

The following parameters further refine the Enterprise Scenario
from the common parameters.


* ``test`` = TGax-test2

* ``d`` = 10

* ``r`` = 5

* ``n`` = 100

* ``uplink`` = 10.0

* ``downlink`` = 0.0

* ``txRange`` = 68

* ``scenario`` = enterprise

* ``duration`` = 2

Note that the duration is reduced here in order to reduce the 
clock time to complete the simulation.

Results
#######

The node positions for the Enterprise Scenario are given below.

.. _positions-TGax-test2:

.. figure:: figures/spatial-reuse-positions-TGax-test2.*
   :align: center

   Node positions for the Enterprise Scenario.

Figure :ref:`positions-TGax-test2` illustrates the node positions
of the Enterprise Scenario.

We can see a very dense scenario where there are two BSS each with one AP at its center, 
each with 100 STAs at random position within the r=5m radius about the AP.  

The simulation results in throughput of 6.138 Mbps and 5.766 Mbps at AP1 and AP2,
respectively.  This is 61.38% and 57.66%, respectively, of the 10 Mbps offered load.  
The results indicate that this simulation scenario saturates the
network.

Since each network has a number of sending STAs, an examination
of the distribution of signal level and noise levels, from the
perstpective of the receiving AP, helps provide insight into
the behavior of the network on a node-by-node level.
The Empirical Cumulative Distribution 
Function (ECDF) for the signal level and noise levels are provided below,
and show that there is variation amoung the levels.

Steps in the signal and noise level plots indicate transitions in 
the data that generally correspond to different nodes.  For example,
the plots below are generated from the scenario with n=100 STAs 
associated per AP, and there are roughly 100 steps.  Vertical
lines indicate that the signal and noise levels per STA remain
constant, which is explained by the deterministic evaluation of
the signal at the receiver, which is distance-based, and is not
varying because the nodes are not moving.

The ECDF of the signals received by AP1 and AP2 are given below.

.. _TGax-test2-ap1-signal:

.. figure:: figures/spatial-reuse-rx-sniff-TGax-test2-ap1-signal.*
   :align: center

   ECDF of the signals received by AP1 for the Enterprise Scenario.

Figure :ref:`TGax-test2-ap1-signal` illustrates the ECDF of the signals
received by AP1 for the Enterprise Scenario.

.. _TGax-test2-ap2-signal:

.. figure:: figures/spatial-reuse-rx-sniff-TGax-test2-ap2-signal.*
   :align: center

   ECDF of the signals received by AP2 for the Enterprise Scenario.

Figure :ref:`TGax-test2-ap2-signal` illustrates the ECDF of the signals
received by AP2 for the Enterprise Scenario.

It is observed that the distribution of the signal levels from the
100 randomly positioned STAs is almost linear in the range of 
-58 to -38 dB

The ECDF of the noise levels received by AP1 and AP2 are given below.

.. _TGax-test2-ap1-noise:

.. figure:: figures/spatial-reuse-rx-sniff-TGax-test2-ap1-noise.*
   :align: center

   ECDF of the noise level received by AP1 for the Enterprise Scenario.

Figure :ref:`TGax-test2-ap1-noise` illustrates the ECDF of the noise levels
received by AP1 for the Enterprise Scenario.

.. _TGax-test2-ap2-noise:

.. figure:: figures/spatial-reuse-rx-sniff-TGax-test2-ap2-noise.*
   :align: center

   ECDF of the noise level received by AP2 for the Enterprise Scenario.

Figure :ref:`TGax-test2-ap2-noise` illustrates the ECDF of the noise levels
received by AP2 for the Enterprise Scenario.

It is observed that the distribution of the noise levels shows that
noise levels are predominantly low, with 60-80% of the time that 
a packet is received, the noise level being approximately -86 dB (BW = 80 MHz).
The remainder of noise levels concentrate in levels above -60 dB.


Test 3 - Indoor Small BSSs Scenario
###################################

The indoor small BSSs scenario has the objective to be representative
of real-world deployments with high density of APs and STAs [TGax15]_.

The scenario is simulated in ns-3 as follows: 2 APs are placed d=34.64m 
apart, each with n=30 STAs that are dropped within a circle of r=10m around
the AP.

This configuration as modelled in this ns-3 scenario is a simplification
of the scenario as presented in [TGax15].  Whereas a 19 cell area is outlined
in [TGax15], this model only deploys 2 adjacent cells.

Traffic flow is modelled as a total aggregated offered load of 10 Mbps of 
uplink traffic only.  With a total uplink load of 10 Mbps and n=30 STAs per
network, the uplink load per STA is 10 Mbps / 30 nodes = 0.33 Mbps per STA.
Note that this load is a deviation from the scenario as documented in 
[TGax15]_, in which different traffic models are assigned 
separately to each BSS.

The Indoor Small BSSs path loss model is given by:

Pathloss model

PL(d) = 40.05 + 20*log10(fc/2.4) + 20*log10(min(d,10)) + (d>10) * 35*log10(d/10) 

where:

–	d = max(3D-distance [m], 1)

–	fc = frequency [GHz

This pathloss model is modelled in ns-3 using the 
Ieee80211axIndoorPropagationLoss model with attribute values
"DistanceBreakPoint" = 10, "Walls" = 0, and "WallsFactor" = 0.

The following parameters further refine the Indoor Small BSSs Scenario
from the common parameters.


* ``test`` = TGax-test3

* ``d`` = 34.64

* ``r`` = 10

* ``n`` = 30

* ``uplink`` = 10.0

* ``downlink`` = 0.0

* ``txRange`` = 107

* ``scenario`` = indoor

Results
#######

The node positions for the Indoor Small BSSs Scenario are given below.

.. _positions-TGax-test3:

.. figure:: figures/spatial-reuse-positions-TGax-test3.*
   :align: center

   Node positions for the Indoor Small BSSs Scenario.

Figure :ref:`positions-TGax-test3` illustrates the node positions
of the Indoor Small BSSs Scenario.

We can see a dense scdnario where there are two BSS each with one AP at its center, 
each with 30 STAs at random position 
within the r=10m radius about the AP.  

The simulation results in throughput of 7.434 Mbps and 6.9564 Mbps at AP1 and AP2,
respectively.  This is 74.34% and 69.56%, respectively, of the 10 Mbps offered load.  
The results indicate that this simulation scenario saturates the
network.

Since each network has a number of sending STAs, an examination
of the distribution of signal level and noise levels, from the
perstpective of the receiving AP, helps provide insight into
the behavior of the network on a node-by-node level.
The Empirical Cumulative Distribution 
Function (ECDF) for the signal level and noise levels are provided below,
and show that there is variation amoung the levels.

Steps in the signal and noise level plots indicate transitions in 
the data that generally correspond to different nodes.  For example,
the plots below are generated from the scenario with n=30 STAs 
associated per AP, and there are roughly 30 steps.  Vertical
lines indicate that the signal and noise levels per STA remain
constant, which is explained by the deterministic evaluation of
the signal at the receiver, which is distance-based, and is not
varying because the nodes are not moving.

The ECDF of the signals received by AP1 and AP2 are given below.

.. _TGax-test3-ap1-signal:

.. figure:: figures/spatial-reuse-rx-sniff-TGax-test3-ap1-signal.*
   :align: center

   ECDF of the signals received by AP1 for the Indoor Small BSSs Scenario.

Figure :ref:`TGax-test3-ap1-signal` illustrates the ECDF of the signals
received by AP1 for the Indoor Small BSSs Scenario.

.. _TGax-test3-ap2-signal:

.. figure:: figures/spatial-reuse-rx-sniff-TGax-test3-ap2-signal.*
   :align: center

   ECDF of the signals received by AP2 for the Indoor Small BSSs Scenario.

Figure :ref:`TGax-test3-ap2-signal` illustrates the ECDF of the signals
received by AP2 for the Indoor Small BSSs Scenario.

It is observed that the distribution of the signal levels from the
30 randomly positioned STAs is almost linear, although there is a 
larger concentration of signal levels in the range of 
approximately -58 to -32 dB.

The ECDF of the noise levels received by AP1 and AP2 are given below.

.. _TGax-test3-ap1-noise:

.. figure:: figures/spatial-reuse-rx-sniff-TGax-test3-ap1-noise.*
   :align: center

   ECDF of the noise level received by AP1 for the Indoor Small BSSs Scenario.

Figure :ref:`TGax-test3-ap1-noise` illustrates the ECDF of the noise levels
received by AP1 for the Indoor Small BSSs Scenario.

.. _TGax-test3-ap2-noise:

.. figure:: figures/spatial-reuse-rx-sniff-TGax-test3-ap2-noise.*
   :align: center

   ECDF of the noise level received by AP2 for the Indoor Small BSSs Scenario.

Figure :ref:`TGax-test3-ap2-noise` illustrates the ECDF of the noise levels
received by AP2 for the Indoor BSSs Scenario.

It is observed that the distribution of the noise levels shows that
noise levels are predominantly low, with 60-80% of the time that 
a packet is received, the noise level being approximately -86 dB (BW = 80 MHz).
The remainder of noise levels concentrate in levels above -60 dB.


Test 4 - Outdoor Small BSS Scenario
###################################

The outdoor small BSS scenario has the objective to be representative
of real-world outdoor deployments with a high separation between APs
(BSS edge with low SNR) with high density of STAs [TGax15]_.

The scenario is simulated in ns-3 as follows:  2 APs are placed d=130m 
apart, each with n=50 STAs that are dropped within a circle of r=70m around
the AP.

This configuration as modelled in this ns-3 scenario is a simplification
of the scenario as presented in [TGax15].  Whereas a 19 cell area is outlined
in [TGax15], this model only deploys 2 adjacent cells.

Traffic flow is modelled as a total aggregated offered load of 10 Mbps of 
uplink traffic only.  With a total uplink load of 10 Mbps and n=50 STAs per
network, the uplink load per STA is 10 Mbps / 50 nodes = 0.20 Mbps per STA.
Note that this load is a deviation from the scenario as documented in 
[TGax15]_, in which different traffic models are assigned 
separately to each BSS.

The Outdoor Small BSS path loss model is given in [TGax15]_ for LOS and NLOS 
conditions.

This pathloss model is modelled in ns-3 using the 
ItuUmiPropagationLoss model.

The following parameters further refine the Outdoor Large BSS Scenario
from the common parameters.

* ``test`` = TGax-test4

* ``d`` = 130

* ``r`` = 70

* ``n`` = 50

* ``uplink`` = 10.0

* ``downlink`` = 0.0

* ``txRange`` = 50

* ``scenario`` = indoor

Results
#######

The node positions for the Outdoor Large BSS Scenario are given below.

.. _positions-TGax-test4:

.. figure:: figures/spatial-reuse-positions-TGax-test4.*
   :align: center

   Node positions for the Outdoor Large BSS Scenario.

Figure :ref:`positions-TGax-test4` illustrates the node positions
of the Outdoor Large BSS Scenario.

We can see a dense scenario where there are two BSS each with one AP at its center, 
each with 50 STAs at random position 
within the r=70m radius about the AP.  

The simulation results in throughput of 5.8104 Mbps and 5.6664 Mbps at AP1 and AP2,
respectively.  This is 58.10% and 56.66%, respectively, of the 10 Mbps offered load.  
The results indicate that this simulation scenario saturates the
network.

Since each network has a number of sending STAs, an examination
of the distribution of signal level and noise levels, from the
perstpective of the receiving AP, helps provide insight into
the behavior of the network on a node-by-node level.
The Empirical Cumulative Distribution 
Function (ECDF) for the signal level and noise levels are provided below,
and show that there is variation amoung the levels.

Steps in the signal and noise level plots indicate transitions in 
the data that generally correspond to different nodes.  For example,
the plots below are generated from the scenario with n=50 STAs 
associated per AP, and there are roughly 10 steps.  Vertical
lines indicate that the signal and noise levels per STA remain
constant, which is explained by the deterministic evaluation of
the signal at the receiver, which is distance-based, and is not
varying because the nodes are not moving.

The ECDF of the signals received by AP1 and AP2 are given below.

.. _TGax-test4-ap1-signal:

.. figure:: figures/spatial-reuse-rx-sniff-TGax-test4-ap1-signal.*
   :align: center

   ECDF of the signals received by AP1 for the Outdoor Large BSS Scenario.

Figure :ref:`TGax-test4-ap1-signal` illustrates the ECDF of the signals
received by AP1 for the Outdoor Large BSS Scenario.

.. _TGax-test4-ap2-signal:

.. figure:: figures/spatial-reuse-rx-sniff-TGax-test4-ap2-signal.*
   :align: center

   ECDF of the signals received by AP2 for the Outdoor Large BSS Scenario.

Figure :ref:`TGax-test4-ap2-signal` illustrates the ECDF of the signals
received by AP2 for the Outdoor Large BSS Scenario.

It is observed that the distribution of the signal levels from the
50 randomly positioned STAs is almost linear for signal levels 
above approximately -72 dB.

The ECDF of the noise levels received by AP1 and AP2 are given below.

.. _TGax-test4-ap1-noise:

.. figure:: figures/spatial-reuse-rx-sniff-TGax-test4-ap1-noise.*
   :align: center

   ECDF of the noise level received by AP1 for the Outdoor Large BSS Scenario.

Figure :ref:`TGax-test4-ap1-noise` illustrates the ECDF of the noise levels
received by AP1 for the Outdoor Large BSS Scenario.

.. _TGax-test4-ap2-noise:

.. figure:: figures/spatial-reuse-rx-sniff-TGax-test4-ap2-noise.*
   :align: center

   ECDF of the noise level received by AP2 for the Outdoor Large BSS Scenario.

Figure :ref:`TGax-test4-ap2-noise` illustrates the ECDF of the noise levels
received by AP2 for the Outdoor BSS Scenario.

It is observed that the distribution of the noise levels shows that
noise levels are high, as compared to other scenarios modelled here, 
with only 40-60% of the time that a packet is received, the noise level 
being approximately -86 dB (BW = 80 MHz).



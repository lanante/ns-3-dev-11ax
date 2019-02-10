Results
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

In this chapter we provide simulation results obtained to evaluate 
the WiFi HE features, especially spatial reuse and coexistence of 802.11ac, 
802.11ax, and/or LTE networks. We first describe the framework used to launch
simulation scenarios and process the results.  Then we present the results for
some selected scenarios and parametric studies.

Scenario
========

For this module, we introduce the notion of a scenario.  This subsection
describes what a scenario is and how to use it.

A scenario refers to a simulation experiment for which a simulation runs
is conducted and the data post-processed to yield results such as plots.
By varying experiment parameters, several similar scenarios can be run
and results among them can be compared as sensitivity studies of those
parameters.

A complete scenario is defined by one or more networks.  Each network has
a single AP, and one or more STAs.  The STAs are randomly dropped 
within a circle centered at the networks AP, and are uniformly distributed
within the circle.  

Nodes communicate by sending traffic with an offered
uplink load (total summation of all traffic from all STAs toward the AP) 
or downlink load (total summation of load from the AP that is allocated
prorata to the STAs).

When more than one network is simulated, the APs are separated from one
another by some separation distance.

Key Parameters
==============

The key parameters in defining a network, then, are:

* d - separating distance between each AP

* n - the number of STAs per network

* r - the radius within which the STAs are distributed

* nBss - the number of networks

* uplink - the total offered load from the STAs towards the AP

* downlink - the total load from the AP towards the STAs

Nomenclature: `d-r-nBss-n-uplink-downlink`
==========================================

The notation we use to define a scenario takes the form of:

d-r-nBss-n-uplink-downlink

For example, scenario 80-30-02-05-0003-0000 means that the
networks are separated by d=80m, with STAs dropped into each network
within a radius of r=30m, and there are nBss=2 networks each with
n=5 STAs with a total uplink traffic load of 3 Mbps from the STAs towards
the AP and a downlink load of 0 Mbps from the AP towards the STAs.

The behaviors of the network can be evaluated by varying these 
parameters and measuring the resulting network performance.

As an example evaluation, we run simulations for several scenarios
by varying the parameters as follows:

* d - 80, 60, 40, and 20m

* r - 30, 20, and 10m

* nBss - 2 (only two networks, A and B, are enabled)

* n - 5, 10, or 20 STAs

* uplink - 1, 2, 3, 4, 5, or 6 Mbps

* downlink - 0 Mbps

Other Simulation Parameters
===========================

Several other simulation parameters are held steady, including:

* standard - 802.11ac

* more to be documented

The results of these experiments provide insights into the behaviors
of the networks as the distances between them, and the number and
positioning of the STAs within them are varied.  

For example, the effects of spatial reuse can be evaluated by varying
the separating distances between networks.  As networks are more
closely colocated, overlapping nodes from one networks would tend
to interfere with the transmissions of other networks, reducing
network throughput.

Performance Measures
====================

Performance evaluation of the simulations can be evaluated in terms
of metrics, such as:

throughput - the amount of packets successfully received at the 
target destinations, measured in Mbps.  Throughput is calculated over
the period of time specified by the spatial-reuse script parameter
--duration.  The counting of successfully received packets starts
after the network reaches a steady state, and after the ADDBA 
exchanges across all links have been successfully estalished.

Throughput is a common term used in network performance evaluations. 
However, throughput alone does not completely capture the effects of
spatial reuse features.  For example, if networks A and B both 
give a throughput of 2 Mbps, this alone does not reflect the number
and location of nodes within each network.  Thus, a metric of density
complementary to throughput helps to describe the network performance.

We consider further two additional metrics:

1. Area capacity - throughput divided by the area within which the
nodes of the network have been placed, measured in Mbps / m^2.

2. Spectrum efficiency - throughput per Hertz divide by the area within
which the nodes of the network have been placed, measured in 
Mbps / Hz / m^2.

Examples of these metrics with further explanation are given below.

Example Scenarios
=================

The figures in the section were generated using the ns-3-dev-11ax 
repository as of commit changeset 7d56f75.

As an example comparison, we consider two of these scenarios:

1. Scenario 80-30-02-20 and
2. Scenario 20-10-02-05

The first scenario separates the two networks by a distance of 80m, and 
places 20 nodes within a radius of 30m around each AP, while the second
scenario reducies the AP separation to 20m and also reduces the number
of APs and the radius into which they are positioned.


Scenario 80-30-02-20
====================

In this scenario, two networks (A and B) each have one AP that are placed
80m apart.  Each network has 20 STAs dropped within a radius of 30m from
the AP.

The node positions for the 80-30-02-20 scenario is given below.

.. _positions-80-30-02-20:

.. figure:: figures/spatial-reuse-positions-experiments-80-30-02-20-0006-0000.*
   :align: center 

   Node positions for the 80-30-02-20 experiment.

Figure :ref:`positions-80-30-02-20` illustrates the node positions 
of the 80-30-02-20 scenario.

We can see that each network is separated by distance such that interference 
between networks should be minimized.  We would expect each network to behave
similarly, and that communications effect between them would not generally
impact their performance, as, for example, the offered load is increased and
the networks reach saturation levels.

Throughput (AP1 only)
#####################

The throughput as offered load is increased is shown below for AP1 (network A).

.. _throughput-80-30-02-20-ap1:

.. figure:: figures/throughput-80-30-02-20-ap1.*
   :align: center 

   Throughput for AP1 for the 80-30-02-20 experiment.

Figure :ref:`throughput-80-30-02-20-ap1` illustrates the throughput for AP1.

It is observed that the network reaches a throughput saturation of approximately
4.5 Mbps.

Throughput (AP2 only)
#####################

The throughput as offered load is increased is shown below for AP2 (network B).

.. _throughput-80-30-02-20-ap2:

.. figure:: figures/throughput-80-30-02-20-ap2.*
   :align: center 

   Throughput for AP2 for the 80-30-02-20 experiment.

Figure :ref:`throughput-80-30-02-20-ap2` illustrates the throughput for AP2.

It is observed that the network reaches a throughput saturation of approximately
4.5 Mbps.

Throughput (Both AP1 and AP2 on a single plot)
##############################################

Both plots are combined into one single plot below.

.. _throughput-80-30-02-20-both:

.. figure:: figures/throughput-80-30-02-20-both.*
   :align: center 

   Throughput for AP1 and AP2 for the 80-30-02-20 experiment.

Figure :ref:`throughput-80-30-02-20-both` illustrates the throughput for AP1 and AP2.

By combining system throuput for both networks onto a single plot, it is easiler to observe
that the both networks perform similary and reach a throughput saturation of approximately
4.5 Mbps.

Area Capacity (Both AP1 and AP2 on the same plot)
#################################################

The area capacity for both networks is plotted on a single plot below.

.. _area-capacity-80-30-02-20-both:

.. figure:: figures/area-capacity-80-30-02-20-both.*
   :align: center 

   Area capacity for AP1 and AP2 for the 80-30-02-20 experiment.

Figure :ref:`area-capacity-80-30-02-20-both` illustrates the area capacity for AP1 and AP2.

It is observed that the shapes of the curves in the area capacity plots generally follow
the corresponding shapes of the throughput curves for these two networks.  This is because
the areas are identical for both networks into which the nodes have been placed.

Spectrum Efficiency (Both AP1 and AP2 on the same plot)
#######################################################

The spectrum efficiency for both networks is plotted on a single plot below.

.. _spectrum-efficiency-80-30-02-20-both:

.. figure:: figures/spectrum-efficiency-80-30-02-20-both.*
   :align: center 

   Spectrum efficiency for AP1 and AP2 for the 80-30-02-20 experiment.

Figure :ref:`spectrum-efficiency-80-30-02-20-both` illustrates the spectrum efficiency for AP1 and AP2.

It is observed that the shapes of the curves in the spectrum efficiency plots generally follow
the corresponding shapes of the throughput curves for these two networks.  This is because
the areas are identical for both networks into which the nodes have been placed, and the 
operating frequency (in Hz) is also identical for both networks, which 
simulate 802.11ac networks.


Scenario 20-10-02-05
====================

This scenario differs from the previous one by reducing the separation distance
between the two APs, reducing the number of STAs per BSS, and reducing the radius
into which the STAs are placed near their corresponding AP.  While several variables
are thus changed, the evaluation of the scenario provides a comparison of performance
between two differnt scenario configurations.

In this scenario, two networks (A and B) each have one AP that are placed
20m apart.  Each network has 5 STAs dropped within a radius of 10m from
the AP.

The node positions for the 20-10-02-05 scenario is given below.

.. _positions-20-10-02-05:

.. figure:: figures/spatial-reuse-positions-experiments-20-10-02-05-0006-0000.*
   :align: center 

   Node positions for the 20-10-02-05 experiment.

Figure :ref:`positions-20-10-02-05` illustrates the node positions.

In contrast to the 80-30-02-20 scenario, we can see that each 
network is separated by a shorter distance such that interference 
between networks may occur.  We might expect communications effects
between them would impact their performance, as, for example, the offered load
is increased and the networks reach saturation levels.

Throughput (Both AP1 and AP2 on the same plot)
##############################################

The throughput as offered load is increased is shown below for both networks.

.

.. _throughput-20-10-02-05-both:

.. figure:: figures/throughput-20-10-02-05-both.*
   :align: center 

   Throughput for AP1 for the 20-10-02-05 experiment.

Figure :ref:`throughput-20-10-02-05-both` illustrates the throughput for AP1 and AP2.

It is observed that the networks reach a throughput saturation of approximately
2.8 Mbps.  This is a much lower throughput than in the earlier example where
the APs were at a greater separation distance (80m vs. 20m), even though the number
of STAs in each network was also greater (20 vs. 5).  In effect, the closer proximity
of the nodes and the overlap of the networks degrades communications effectiveness,
as indicated by the reduced throughput.

Area Capacity (Both AP1 and AP2 on the same plot)
#################################################

The area capacity for both networks is plotted on a single plot below.

.. _area-capacity-20-10-02-05-both:

.. figure:: figures/area-capacity-20-10-02-05-both.*
   :align: center 

   Area capacity for AP1 and AP2 for the 20-10-02-05 experiment.

Figure :ref:`area-capacity-20-10-02-05-both` illustrates the area capacity for AP1 and AP2.

It is observed that the shapes of the curves in the area capacity plots generally follow
the corresponding shapes of the throughput curves for these two networks.  This is because
the areas are identical for both networks into which the nodes have been placed.

Spectrum Efficiency (Both AP1 and AP2 on the same plot)
#######################################################

The spectrum efficiency for both networks is plotted on a single plot below.

.. _spectrum-efficiency-20-10-02-05-both:

.. figure:: figures/spectrum-efficiency-20-10-02-05-both.*
   :align: center 

   Spectrum efficiency for AP1 and AP2 for the 20-10-02-05 experiment.

Figure :ref:`spectrum-efficiency-20-10-02-05-both` illustrates the spectrum efficiency for AP1 and AP2.

It is observed that the shapes of the curves in the spectrum efficiency plots generally follow
the corresponding shapes of the throughput curves for these two networks.  This is because
the areas are identical for both networks into which the nodes have been placed, and the 
operating frequency (in Hz) is also identical for both networks, which 
simulate 802.11ac networks.

Area Capacity (Both Scenarios)
##############################

In order to show an example of the performance comparison between the two scenarios,
the area capacity for both scenarios is plotted on a single plot below.

.. _area-capacity-combined1:

.. figure:: figures/area-capacity-combined1.*
   :align: center 

   Area capacity for scenarios 80-30-02-20 and 20-10-02-05.

Figure :ref:`area-capacity-combined1` illustrates the area capacity for both scenarios.

It is observed that the scenario 80-30-02-20 has a significantly higher area capacity
than the 20-10-02-05 scenario.


Sensitivity Analysis
====================

This section presents results of the sensitiviy of key parameters (distance, radius,
and number of nodes per network) to the resulting area capacity, for scenarios with
two BSSs.

Distance Sensitivity
####################

The area capacity for several scenarios is plotted on a single plot below.  The
selected scenarios use the same values of r=20, nBss=2, and n=10, while varying
the distance, d={20, 40, 60, 80}.

.. _area-capacity-distance:

.. figure:: figures/area-capacity-distance.*
   :align: center 

   Area capacity for scenarios where the distance between APs varies.

Figure :ref:`area-capacity-distance` illustrates the area capacity for scenarios 
where the distance varies.

It is observed that area capacity decreases as the distances between APs decreases.
In effect, the closer that the two APs are to one another, the more likely there is
to be collisions among transmitting nodes that reduces performance for both networks.


Dropping Radius Sensitivity
###########################

The area capacity for several scenarios is plotted on a single plot below.  The
selected scenarios use the same values of d=80, nBss=2, and n=10, while varying
the dropping radius, r={10, 20, 30}.

.. _area-capacity-radius:

.. figure:: figures/area-capacity-radius.*
   :align: center 

   Area capacity for scenarios where the dropping radius for the positioning of STAs varies.

Figure :ref:`area-capacity-radius` illustrates the area capacity for scenarios 
where the radius varies.

It is observed that area capacity increases as the radius decreases.  In effect, for the
parametric sensitivies given here, the closer that the same number of nodes are placed to
their AP thus reduces the transmission collisions of one network with its neighboring network,
resuling in increased performance.


Number of STAs Sensitivity
##########################

The area capacity for several scenarios is plotted on a single plot below.  The
selected scenarios use the same values of d=80, r=20, and nBss=2, while varying
the number of STAs, n={5, 10, 20}.

.. _area-capacity-nSTAs:

.. figure:: figures/area-capacity-nSTAs.*
   :align: center 

   Area capacity for scenarios where the number of STAs per network varies.

Figure :ref:`area-capacity-nSTAs` illustrates the area capacity for scenarios 
where the number of STAs varies.

It is observed that the impact to area capacity is negligible when the network
is not saturated.  As the network reaches saturation, the area capacity generally
increases as the number of STAs per network increases.


Other things to do:
##########################

* the above scenario simulations can be extended to handle cases with 3 or 4 BSSs

* add latency plots

Parametric Studies
==================

In the nexgt sections, we discuss parametric studies that have been conducted to
examine the behaviors of the spatial reuse and coexistence features of
802.11ax.  We first describe the framework used to represent the scenarios
in order to conduct simulation experiments for these studies.  Then we 
present the results of those studies.

The following three parametric studies are conducted:

* Study 1 -- All nodes operate as 802.11ac (baseline)

* Study 2 -- All nodes operate as 802.11ax with OBSS_PD features enabled.

* Study 3 -- Several BSSs inter-operate using mixtures of 802.11ac, 802.11ax,
and LAA nodes.

Study 1 - 802.11ac (baseline)
=============================

The objective of the configuration of this scenario is "to capture the issues 
and be representative of real-world deployments with high density of 
APs and STAs" [TGax15]_.  

This simulation study configures the node placements and network simulation
parameters according to those defined in [TGax15]_ (see Scenario 3 - Indoor
BSSs Scenario).  However, to collect baseline measures, Study 1 configures
all nodes to operate in 802.11ac mode, whereas Study 2 evaluates 802.11ax 
features.

Topology / Environment Description
##################################

BSSs are place a regular and symmetric grid as in Figure X
for spatial resue with reuse frequency of N=3.

The node positions for n=5 STAs per BSS in Study 1 are given below.

.. _positions-test-study1:

.. figure:: figures/spatial-reuse-positions-test-study1.*
   :align: center 

   Node positions for n=5 STAs per BSS in Study 1.

Figure :ref:`positions-test-study1` illustrates the node positions 
for n=5 STAs per BSS in Study 1.

The BSSs are arranged with one BSS of high interest centered in the 
topology, with six other BSSs surrounding the high-interest BSS and
arranged in a hexagonal pattern.  As outlined in [TGax15]_ for the
Indoor Small BSSs Scenario, STAs are associated with each BSS within 
a radius of r=10m.  

The figure shown illustrates a scenario in which each BSS has 5 STAs
allocated randomly within its dropping radius (solid lines).  Dashed 
lines indicate nominal Carrier Sense Range (CSR) limits of 15m.

Parameters
##########

In this section, we desribe the parameters used for Study 1, and we
describe any deviations from those described in [TGax15]_ Scenario 3.

Topology
########

* BSSs - BSSs are placed in a regular and symmetric grid as shown 
in the Figure above, with reuse frequency N=3.  Each hexagon of the grid 
is modeled as a circle with radius r=10m into which the positions of the 
STAs are uniformly distributed.

* Reuse Frequency - N=3.

* AP location - APs are placed at the center of each circle.

* Inter BSS Distance (ICD) - BSSs are separated from one another 
by an inter BSS distance (ICD) of 2 * h, where 
h = sqrt(r^2-r^2/4) = 17.32m.

* AP antenna height - a 2D model is assumed, and heights of nodes, including
the AP antenna height, are not modeled.

* STA locations - STAs are uniformly distributed throughout the circle 
(radius r) of each BSS.

* Number of STAs - the number of STAs is varied for this study from 5 to 40 
in steps of 5.

Channel Model
#############

* Fading model - the TGac channel model D NLOS as mentioned in [TGax15]_ is 
not modeled in this study.

* Pathloss model - the pathloss model as given in [TGax15]_ for the Indoor 
Small BSSs (Scenario 3) is modeled.

* Shadowing - Log-normal with 5 dB standard deviation, iid across all links.

PHY Parameters
##############

* AP TX power - 20 dBm per antenna.

* STA TX power - 15 dBm per antenna.

* MCS - the ns-3 ideal channel model is used.  This is a minor deviation from the
[TGax15]_ described scenario of MCS0 or MCS7 only.

* antennas - (SISO) one antenna is modeled per AP and per STA.  This is a deviation from
the parameters described in [TGax15]_.

* TX gain - 0 dBm

* RX gain - 0 dBm

ns-3 parameters:

* Maximum supported TX spatial streams - 1

* Maximum supported RX spatial streams - 1

MAC Parameters
##############

* Access protocol parameters - EDCA with default EDCA parameter set.

* Primary channels - All BSS at 5GHz with 20 MHz BSS with reuse 3.  Assignment
of 20 MHz bands is a deviation from the parameters described in [TGax11].

* Aggregation - A-MPDU aggregation size of 3140.

* RTS/CTS Threshold - no RTS/CTS.

* Association - STAs are associated with the BSS for the circle into which the 
STA has been dropped.  This varies from the association scheme described in [TGax15]_,
although the resulting number of associated STAs per BSS remains the same.

Traffic Model
#############

The modeling of traffic differs from the approach described in [TGax15]_,
in which several different classes of traffic are described.  Instead, an
approach is used that allows the mix of uplink and downlink traffic and the
packet sizes to be specified.

The total offered load of the entire system is specified in Mbps.  This total
load is then allocated into uplink and downlink portions.  For example, assuming
a total offfered load of 5.0 Mbps with an allocation of 90% uplink and 10% downlink,
then the uplink traffic is 5.0 x 90% = 4.5 Mbps and the downlink traffic is 
5.0 x 10% = 0.5 Mbps.

The total load is allocated per node.  For example, if the total uplink traffic
is 4.5 Mbps and there are 10 STAs, then the traffic flow per link is 4.5 Mbps / 10 
= 0.45 Mbps per uplink.

Separate payload sizes (i.e., application layer packet lengths) are specified for
the uplink payload and the downlink payload sizes.  These payload sizes and the 
allocated traffic are used to determine the transmission interval per packet.

All traffic is assumed a Constant Bit Rate (CBR) and transmissions use UDP datagrams.

* Uplink payload size - 1500 bytes

* Downlink payload size - 300 bytes

Conducting the Study 1 Experiments
##################################

To conduct the exeriments for Study 1, a bash script is used to repeatedly
run the ns-3 spatial-reuse.cc script.  

Parameters are fixed as described above, with the exception of the number of
STAs.

The total offered load for each BSS is a balanced load, with the same load
for each BSS.  The offered load is increased from 1 Mbps, 
in steps of 1 Mbps, until the simulation shows that the (central) BSS of 
interest becomes saturated.

The traffic mix is 90% uplink and 10% dowblink, with a uplink 
payload size of 1500 bytes, and an downlink payload size of 300 bytes.

The number of STAs, n, is varied from 5 to 40 in steps of 5.

All nodes in Study 1 use 802.11ac.

Results are collected in terms of the one, central BSS of high-interest
(i.e., the "red" BSS area within the figure above).

Performance metrics collected include the following (using the same definitions
as given earlier):

* system throughput

* area capacity

* spectrum efficiency

* latency

To run the Study 1 scenarios, plot results, and transfer those results to the 
documentation figures, the following scripts should be executed in order:

1) run-spatial-reuse-study1.sh - this generates a script file "./study1.sh".

2) ./study1.sh - this is the script generate by (1), and should be run to generate all results.

3) make-data-files-study1.sh - this creates data files for plotting, from the set of all simulation results.

4) plot-study1.sh - this generates plots of results.

5) plot-study1-ecdf.sh - this generates additional plots (ECDFs) from results.

6) copy-study1-plots-to-doc-figures.sh - this copies a subset of plots generated into
the doc/figures folder, for inclusion of those results into the documentation.

Note that any modifications that are made to change the values of n in the 
study1 scripts above must be applied to both the 'run-spatial-reuse-study1.sh' 
script and also the 'plot-study1.sh'script.  The first script runs the ns-3 
simulations and creates data files that are used by the second script to 
create plots.  Specifically, note that the following items
appears in both scripts and thus must be kept synchronized:

for n in 5 10 15 20 25 30 35 ; do

Note that this line appears once in the 'run-spatial-reuse-study1.sh' script
and appears several times in the 'plot-study1.sh' script because there are
several different plots that are produced by the 'plot-study1.sh' script from
the full set of simulation results that are generated by the 
'run-spatial-reuse-study1.sh' script.

The values of n that are specified in the 'plot-study1.sh' script must be 
a subset of the values that are specified in the 'run-spatial-reuse-study1.sh' 
script.

Furthermore, changes to the offererdLoad parameter in the 'run-spatial-reuse-study1.sh'
script may require the axis ranges at the top of the 'plot-study1.sh'
script to be adjusted.

For example, if the offered load is changed to the following:

for offeredLoad in 1.5 2.0 2.5 3.0 ; do

then the x-axis values of the following may need to be adjusted, to match
the range specified by the offeredLoad parameter

THROUGHPUT_RANGE="[1:6][1:6]"

AREA_CAPCITY_RANGE="[1:6][0:0.02]"

SPECTRUM_EFFICIENCY_RANGE="[1:6][0:0.002]"

STUDY1_SPECTRUM_EFFICIENCY_RANGE="[1.0:3.0][0:0.00050]"

After the above have been executed, the doc/figures folder should contain the newly
generated plots for inclusion in the documentation.  The documentation can then be
regenerated, e.g.:

cd doc

make latexpdf

Study 1 Results
===============

The figures in the section were generated using the ns-3-dev-11ax 
repository as of commit changeset 8d5cd31b78fb862d7e9280ca3bc7468306ab6168.

System Throughput
#################

The throughput as offered load is increased is shown below for 
the center BSS of interest.  A separate line is plotted for each value
of the number of STAs (e.g., n=5, n=10, n=15, ..., n=40).

.

.. _throughput-study1:

.. figure:: figures/throughput-study1.*
   :align: center 

   Throughput for the BSS of interest in the center of the Study 1 parametric study.

Figure :ref:`throughput-study1` illustrates the system throughput for BSS1.

It is observed that system throughput increases as offered load increases, and plateaus
generally in the range of 1.2 - 2.2 Mbps.  Furthermore, system throughput decreases
as the number of STAs, n, per BSS increases.

Distribution of Node Contributions to Downlink Throughput
#########################################################

Total throughput of the system is divided into uplink and downlink portions.
The downlink portions (from the AP to  the STAs) are then further divided equally
among the STAs.  For example, for a total system throughput target of 0.6 Mbps
per BSS that is divided into 90% uplink and 10% downlink, then 0.6 Mbps x 0.1 = 0.06 Mbps
is allocated in downlink traffic.  If there are 5 STAs in the BSS, then the AP 
attemps to deliver 0.06 Mbps / 5 = 0.012 Mbps from the AP to each of the 5 STAs.
However, the packet performance measure of each STA, in terms of actual downlink 
throughput, may vary from one STA to another.  The per-STA performance of the 
ability to successfully receive packets is a measure that can provide insights
into the distribution of the behaviors among the nodes.

The ECDF of node contributions to downlink throughput for the offered load of
2 Mbps is shown below for cases of n=5 and n=20 nodes.

.

.. _study1-05-ecdf:

.. figure:: figures/study1-1732-10-05-2-180.0-20.0-ap1-ecdf.*
   :align: center 

   ECDF of node contributions to downlink throughput for n=5 nodes and offered load of 2 Mbps.

Figure :ref:`study1-05-ecdf` illustrates the ECDF of node contributions to downlink throughput for n=5 nodes.

It is observed that 100% of all n=5 STAs contributed the same amount of throughput per node, 
at approximately 0.04 Mbps per node, and that no STAs delivered 0 packets.  This indicates that
the network is not saturated, and packet delivery occurs with high success rates across all nodes.
.

.. _study1-20-ecdf:

.. figure:: figures/study1-1732-10-20-2-180.0-20.0-ap1-ecdf.*
   :align: center 

   ECDF of node contributions to downlink throughput for n=20 nodes and offered load of 2 Mbps.

Figure :ref:`study1-20-ecdf` illustrates the ECDF of node contributions to downlink throughput for n=20 nodes.

It is observed that the majority of STAs contributed throughput per node of approximately 0.01 Mbps per node,
and that a small number of STAs delivered 0 packets.  This indicates that packet delivery occurs successfully
for most nodes. 

Area Capacity
#############

The area capacity as offered load is increased is shown below for 
the center BSS of interest.  A separate line is plotted for each value
of the number of STAs (e.g., n=5, n=10, n=15, ..., n=40).

.

.. _area-capacity-study1:

.. figure:: figures/area-capacity-study1.*
   :align: center 

   Area capacity for the BSS of interest in the center of the Study 1 parametric study.

Figure :ref:`area-capacity-study1` illustrates the system throughput for BSS1.

It is observed that area capacity increases as offered load increases, and plateaus
generally in the range of 0.0025 - 0.007 Mbps / m^2.  Furthermore, area capacity decreases
as the number of STAs, n, per BSS increases.

Spectrum Efficiency
###################

The spectrum efficiency as offered load is increased is shown below for 
the center BSS of interest.  A separate line is plotted for each value
of the number of STAs (e.g., n=5, n=10, n=15, ..., n=40).

.

.. _spectrum-efficiency-study1:

.. figure:: figures/spectrum-efficiency-study1.*
   :align: center 

   Spectrum efficiency for the BSS of interest in the center of the Study 1 parametric study.

Figure :ref:`spectrum-efficiency-study1` illustrates the system throughput for BSS1.

It is observed that spectrum efficiency increases as offered load increases, and plateaus
generally in the range of 0.0002 - 0.0004 Mbps / Hz / m^2.  Furthermore, spectrum efficiency decreases
as the number of STAs, n, per BSS increases.

Airtime utilization
###################

The airtime utilization as offered load is increased is shown below for 
the center BSS of interest.  A separate line is plotted for each value
of the number of STAs (e.g., n=5, n=10, n=15, ..., n=40).

.

.. _airtime-utilization-study1:

.. figure:: figures/airtime-utilization-study1.*
   :align: center 

   Airtime utilization for the BSS of interest in the center of the Study 1 parametric study.

Figure :ref:`airtime-utilization-study1` illustrates the airtime utilization for BSS1.

Latency
#######

(Here there will be a figure of the average latency as a function of 
offered load.  There will be a separate line for each combination of 
n STAs - i.e., n=5, n=10, n=15, ..., n=40).

Noise and Signal Distributions
##############################

The ECDF of the signal level received at the AP of the BSS of interest is
shown below, for the scenario of n=20 nodes, and offered load of 2 Mbps.

.

.. _study1-20-ap1-signal:

.. figure:: figures/spatial-reuse-rx-sniff-study1-1732-10-20-2-180.0-20.0-ap1-signal.*
   :align: center 

   ECDF of the signal level at the AP for the BSS of interest in the center of the Study 1 parametric study, with n=20 STAs per BSS.

Figure :ref:`study1-20-ap1-signal` illustrates the ECDF of the signal levels received for BSS1 with n=20 STAs per BSS..

It is observed that TBD...

The ECDF of the noise level at the AP of the BSS of interest is
shown below, for the scenario of n=20 nodes, and offered load of 2 Mbps.

.

.. _study1-20-ap1-noise:

.. figure:: figures/spatial-reuse-rx-sniff-study1-1732-10-20-2-180.0-20.0-ap1-noise.*
   :align: center 

   ECDF of the noise level at the AP for the BSS of interest in the center of the Study 1 parametric study, with n=20 STAs per BSS..

Figure :ref:`study1-20-ap1-signal` illustrates the ECDF of the noise levels at BSS1 with n=20 STAs per BSS.

It is observed that TBD...

Study 2 - 802.11ax
==================

Study 2 repeats the experiments of Study 1, with the following
changes:

* standard - all nodes operate using 802.11ax

* BSS color - each BSS is assigned its own unique color.  For
example, BSS #1 uses BSS color 1, BSS #2 uses BSS color 2, etc.

* OBSS_PD level - as an additional sensitivity study, this value 
is varied from -82 dB to -62 dB, in steps of 5 dB.

Conducting the Study 2 Experiments
##################################

To run the Study 2 scenarios, plot results, and transfer those results to the
documentation figures, the following scripts should be executed in order:

1) run-spatial-reuse-study2.sh - this generates a script file "./study2.sh".

2) ./study2.sh - this is the script generate by (2), and should be run to generate all results.

3) make-data-files-study2.sh - this creates data files for plotting, from the set of all simulation results.

4) plot-study2a.sh - this generates study2a plots of results.

5) plot-study2b.sh - this generates study2b plots of results.

6) plot-study2-ecdf.sh - this generates additional plots (ECDFs) from results.

7) copy-study2-plots-to-doc-figures.sh - this copies a subset of plots generated into
the doc/figures folder, for inclusion of those results into the documentation.

After the above have been executed, the doc/figures folder should contain the newly
generated plots for inclusion in the documentation.  The documentation can then be
regenerated, e.g.:

cd doc

make latexpdf

Study 2 Results
===============

The figures in the section were generated using the ns-3-dev-11ax
repository as of commit changeset 003396927f1fa07b3681af0066c50436be1ec952.

System Throughput
#################

The throughput with OBSS_PD level -82 dBm as offered load is increased is shown below for
the center BSS of interest.  A separate line is plotted for each value
of the number of STAs (e.g., n=5, n=10, n=15, ..., n=40).

.

.. _throughput-study2a-82:

.. figure:: figures/throughput-study2a-82.*
   :align: center

   Throughput with OBSS_PD level -82 dBm for the BSS of interest in the center of the Study 2 parametric study.

Figure :ref:`throughput-study2a-82` illustrates the system throughput for BSS1.

It is observed that TBD...

The throughput with OBSS_PD level -62 dBm as offered load is increased is shown below for
the center BSS of interest.  A separate line is plotted for each value
of the number of STAs (e.g., n=5, n=10, n=15, ..., n=40).

.

.. _throughput-study2a-62:

.. figure:: figures/throughput-study2a-62.*
   :align: center

Throughput with OBSS_PD level -62 dBm for the BSS of interest in the center of the Study 2 parametric study.

Figure :ref:`throughput-study2a-62` illustrates the system throughput for BSS1.

It is observed that TBD...

Area Capacity
#############

The area capacity with OBSS_PD level -82 dBm as offered load is increased is shown below for
the center BSS of interest.  A separate line is plotted for each value
of the number of STAs (e.g., n=5, n=10, n=15, ..., n=40).

.

.. _area-capacity-study2a-82:

.. figure:: figures/area-capacity-study2a-82.*
   :align: center

   Area capacity with OBSS_PD level -82 dBm for the BSS of interest in the center of the Study 2 parametric study.

Figure :ref:`area-capacity-study2a-82` illustrates the system throughput for BSS1.

It is observed that TBD...

The area capacity with OBSS_PD level -62 dBm as offered load is increased is shown below for
the center BSS of interest.  A separate line is plotted for each value
of the number of STAs (e.g., n=5, n=10, n=15, ..., n=40).

.

.. _area-capacity-study2a-62:

.. figure:: figures/area-capacity-study2a-62.*
   :align: center

Area capacity with OBSS_PD level -62 dBm for the BSS of interest in the center of the Study 2 parametric study.

Figure :ref:`area-capacity-study2a-62` illustrates the system throughput for BSS1.

It is observed that TBD...

Spectrum Efficiency
###################

The spectrum efficiency with OBSS_PD level -82 dBm as offered load is increased is shown below for
the center BSS of interest.  A separate line is plotted for each value
of the number of STAs (e.g., n=5, n=10, n=15, ..., n=40).

.

.. _spectrum-efficiency-study2a-82:

.. figure:: figures/spectrum-efficiency-study2a-82.*
   :align: center

   Spectrum efficiency with OBSS_PD level -82 dBm for the BSS of interest in the center of the Study 2 parametric study.

Figure :ref:`spectrum-efficiency-study2a-82` illustrates the system throughput for BSS1.

It is observed that TBD...

The spectrum efficiency with OBSS_PD level -62 dBm as offered load is increased is shown below for
the center BSS of interest.  A separate line is plotted for each value
of the number of STAs (e.g., n=5, n=10, n=15, ..., n=40).

.

.. _spectrum-efficiency-study2a-62:

.. figure:: figures/spectrum-efficiency-study2a-62.*
   :align: center

Spectrum efficiency with OBSS_PD level -62 dBm for the BSS of interest in the center of the Study 2 parametric study.

Figure :ref:`spectrum-efficiency-study2a-62` illustrates the system throughput for BSS1.

It is observed that TBD...

Airtime utilization
###################

The airtime utilization with OBSS_PD level -82 dBm as offered load is increased is shown below for
the center BSS of interest.  A separate line is plotted for each value
of the number of STAs (e.g., n=5, n=10, n=15, ..., n=40).

.

.. _airtime-utilization-study2a-82:

.. figure:: figures/airtime-utilization-study2a-82.*
   :align: center

   Airtime utilization with OBSS_PD level -82 dBm for the BSS of interest in the center of the Study 2 parametric study.

Figure :ref:`airtime-utilization-study2a-82` illustrates the airtime utilization for BSS1.

It is observed that TBD...

The airtime utilization with OBSS_PD level -62 dBm as offered load is increased is shown below for
the center BSS of interest.  A separate line is plotted for each value
of the number of STAs (e.g., n=5, n=10, n=15, ..., n=40).

.

.. _airtime-utilization-study2a-62:

.. figure:: figures/airtime-utilization-study2a-62.*
   :align: center

   Airtime utilization with OBSS_PD level -62 dBm for the BSS of interest in the center of the Study 2 parametric study.

Figure :ref:`airtime-utilization-study2a-62` illustrates the airtime utilization for BSS1.

It is observed that TBD...

Latency
#######

(Here there will be a figure of the average latency as a function of 
offered load.  There will be a separate line for each combination of 
n STAs - i.e., n=5, n=10, n=15, ..., n=40).

Study 3 - Coexistence
=====================

Study 3 examines coexistence of combinations of 802.11ac, 802.11ac, and
LAA networks.

The topology of Study 3 is similar to that of Study 1 and Study 2,
and is illustrated below.

The node positions for Study 3 are given below.

.. _positions-test-study3:

.. figure:: figures/spatial-reuse-positions-test-study3.*
   :align: center 

   Node positions for Study 3.

Figure :ref:`positions-test-study3` illustrates the node positions 
for Study 3.

The system consists of 7 BSSs, with the BSS of interest (red) in the 
center.  This BSS is surrounded by a set of BSSs placed in a 
hexagonal pattern around it.  For illustrative purposes, the 
surrounding BSSs in the figure are shown in one of two colors, blue
or black, to indicate the placement and allocation of different
types of neighboring networks.

Study 3 thus consists of simulation where BSS of interest is an
802.11ac network, while the mix of neighboring networks are varied
(i.e., how many BSS are "blue" and how many are "black" ).

The following topology is studied:

* BSS #1 - the BSS off interest (center) - 802.11ax
* BSS #2 - 802.11ac
* BSS #3 - 802.11ax
* BSS #4 - 802.11ax
* BSS #5 - 802.11ac
* BSS #6 - 802.11ax coexisting with LAA
* BSS #7 - 802.11ax

BSS #6 is an 802.11ax network with an overlaid LAA network.  The ratio of nodes 
within that network is allocated as (X% TBD) as 802.11ax nodes and (Y% TBD) as
LAA nodes.

The simulation paramters for each network are determined as follows:

* 802.11ac - "best performing" parameters as identified by Study 1.

* 802.1asx - "best performing" parameter as identified by Study 2.

* LAA - TBD


Study 3 Results
===============

System Throughput
#################

(Here there will be a figure of the system throughput as a function of 
offered load.  There will be a separate line for each combination of 
n STAs - i.e., n=5, n=10, n=15, ..., n=40).

Area Capacity
#############

(Here there will be a figure of the area capacity as a function of 
offered load.  There will be a separate line for each combination of 
n STAs - i.e., n=5, n=10, n=15, ..., n=40).

Spectrum Efficiency
###################

(Here there will be a figure of the spectrum efficiency as a function of 
offered load.  There will be a separate line for each combination of 
n STAs - i.e., n=5, n=10, n=15, ..., n=40).

Latency
#######

(Here there will be a figure of the average latency as a function of 
offered load.  There will be a separate line for each combination of 
n STAs - i.e., n=5, n=10, n=15, ..., n=40).

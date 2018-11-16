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

In this chapter we provide some initial simulation results obtained to evaluate 
the WiFi He features, especially spatial reuse and coexistence of 802.11ac, 
802.11ax, and/or LTE networks. We first describe the framework used to launch
simulation scenarios and process the results.  Then we present the results for
some selected scenarios.

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

When more than one network is simulated, the APs are separate from one
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
=======================================

The nomenclature we use to define a scenario takes the form of:

d-r-nBss-n-uplink-downlink

For example, scenario 80-30-02-05-0003-0000 means that the
networks are separated by d=80m, with STAs dropped into each network
within a radius of r=30m, and there are nBss=2 networks each with
n=5 STAs with a total uplink traffic load of 3 Mbps from the APs towards
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
target destinations, measured in Mbps.

Throughput is a common term used in network performance evaluations. 
However, throughput alone does not completely capture the effects of
spatial resuse features.  For example, if networks A and B both 
give a throughput of 2 Mbps, this alone does not reflect the number
and location of nodes within each network.  Thus, a concept of density
also applied to throughput helps to describe the network performance.

We consider further two additional metrics:

1. Area capacity - throughput divided by the area within which the
nodes of the network have been placed, measured in Mbps / m^2.

2. Spectrum efficiency - throughput per Hertz divide by the area within
which the nodes of the netwowrk have been placed, measured in 
Mbps / Hz / m^2.

Examples of these metrics with further explanation are given below.

Example Scenarios
=================

As an example comparison, we consider two of these scenarios:

1. 80-30-02-20 and
2. 20-10-02-05

The first scenario separates the two networks by a distance of 80m, and 
places 20 nodes within a radius of 30m around each AP, while the second
scenario reducing the AP separation to 20m and also reduces the number
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

It is observed that the network reached a throughput saturation of approximately
3.8 Mbps.

Throughput (AP2 only)
#####################

The throughput as offered load is increased is shown below for AP2 (network B).

.. _throughput-80-30-02-20-ap2:

.. figure:: figures/throughput-80-30-02-20-ap2.*
   :align: center 

   Throughput for AP2 for the 80-30-02-20 experiment.

Figure :ref:`throughput-80-30-02-20-ap2` illustrates the throughput for AP2.

It is observed that the network reached a throughput saturation of approximately
3.95 Mbps.

Throughput (Both AP1 and AP2 on a single plot)
##############################################

Both plots are combined into one single plot below.

.. _throughput-80-30-02-20-both:

.. figure:: figures/throughput-80-30-02-20-both.*
   :align: center 

   Throughput for AP1 and AP2 for the 80-30-02-20 experiment.

Figure :ref:`throughput-80-30-02-20-both` illustrates the throughput for AP1 and AP2.

Area Capacity (Both AP1 and AP2 on the same plot)
#################################################

The area capacity for both networks is plotted on a single plot below.

.. _area-capacity-80-30-02-20-both:

.. figure:: figures/area-capacity-80-30-02-20-both.*
   :align: center 

   Area capacity for AP1 and AP2 for the 80-30-02-20 experiment.

Figure :ref:`area-capacity-80-30-02-20-both` illustrates the area capacity for AP1 and AP2.

It is observed that the shapes of the curves in the area capacity generally plots follow
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
simluate 802.11ac networks.


Scenario 20-10-02-05
====================

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
2.6 Mbps.  This is a much lower throughput than in the earlier example where
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


Spectrum Efficiency (Both AP1 and AP2 on the same plot)
#######################################################

The spectrum efficiency for both networks is plotted on a single plot below.

.. _spectrum-efficiency-20-10-02-05-both:

.. figure:: figures/spectrum-efficiency-20-10-02-05-both.*
   :align: center 

   Spectrum efficiency for AP1 and AP2 for the 20-10-02-05 experiment.

Figure :ref:`spectrum-efficiency-20-10-02-05-both` illustrates the spectrum efficiency for AP1 and AP2.


Area Capacity (Both Scenarios)
##############################

The area capacity for both scenarios is plotted on a single plot below.

.. _area-capacity-combined1:

.. figure:: figures/area-capacity-combined1.*
   :align: center 

   Area capacity for scenarios 80-30-02-20 and 20-10-02-05.

Figure :ref:`area-capacity-combined1` illustrates the area capacity for both scenarios.

It is observed that the scenario 80-30-02-20 have a significantly higher area capacity
than the 20-10-02-05 scenario.


Sensitivity Analysis
====================

This section presents results of the sensitiviy of key parameters (distance, radius,
and number of nodes per network) to the resulting area capacity.

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

It is observed that area capacity decreases as the radius increases.


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
===================

* extend / repeat with 3 BSS, 4 BSS

* add latency plots

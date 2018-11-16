Introduction
***************

.. include:: replace.txt
.. highlight:: cpp

.. heading hierarchy:
   ------------- Chapter
   ************* Section (#.#)
   ============= Subsection (#.#.#)
   ############# Paragraph (no number)

Overview
========

The document provides validation and initial testing results of the
ns-3 WiFi-HE implementation.

The WiFi-HE module implements 802.11ax features that intend to improve
network performance through spatial-reuse.


Motivating Example
==================

As a motivating example, consider a scenario is which two networks are
located near one another such that communications interference between them
may occur.  Such interference may degrade the network performance of one or
both networks.  Using the WiFi-HE within network simulations helps 
researchers understand the anticipated effects of the interactions among
different features and the impacts they have on network performance.

For illustrative purposes, consider two networks, each with one AP and such
that the APs are positioned 80m apart from one another.  Each network 
has n=10 STAs.  The STAs are randomly placed within a circle centered 
around the AP.  A pictorial plot of the node positions is given below, 
where the circle radius is 30m.

.. _positions-80-30-02-10:

.. figure:: figures/spatial-reuse-positions-experiments-80-30-02-10-0006-0000.*
   :align: center

   Illustrative node positions for a scenario with 2 APs separared by 80m, which 10 nodes per network.

Figure :ref:`positions-80-30-02-10` illustrates the node positions
of the scenario.

We can see that each network is separated by distance such that interference
between networks should be minimized.  We would expect each network to behave
similarly, and that communications effect between them would not generally
impact their performance, as, for example, the offered load is increased and
the networks reach saturation levels.

This leads to several research questions that generally evaluate the impact
to network performance by varying one or more simluation parameters.  

As one example, consider the above scenario where the radius of the circle is 
reduced from r1=30m (as shown in the plot) to r2=10m.  By reducing the
radius into which STAs are placed, the resulting scenario decreases the 
area in which the nodes operate and increases node density.


Dropping Radius Sensitivity
###########################

The two corresponding scenarios (r1=30m and r2=10m) are evaluated in simulations
and the resulting area capacities are compared below.

Area capacity is a performance metric that measures throughput dividied by area.
Given that the two scenarios differ in their operational areas, this metric is
useful for comparison of network performance.

The area capacity for the two scenarios is plotted on a single plot below.  The
selected scenarios use the same values of d=80, nBss=2, and n=10, while varying
the dropping radius, r={10, 30}.

.. _area-capacity-radius-v2:

.. figure:: figures/area-capacity-radius-v2.*
   :align: center

   Area capacity for scenarios where the dropping radius for the positioning of STAs varies.

Figure :ref:`area-capacity-radius-v2` illustrates the area capacity for scenarios
where the radius varies.

It is observed that area capacity of the second scenario (r2=10m) are significantly
higher than the first scenario (r1=30m).  In fact, the area capacitiy is higher by
an order of magnitude.  This implies that either dense placement of nodes, or nodes
placed closer to an AP, improves network throughput (per m^2).

Scope And Limitations
=====================


Spatial Reuse
=============



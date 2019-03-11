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
ns-3 WiFi 802.11ax features, with focus on spatial reuse.

This document is written in Python Sphinx markup and is maintained in
the ns-3 folder ``src/wifi-he/doc``.  

As described in [Sta16]_, 802.11ax has features to improve overlapping BSS
(OBSS) operation in dense environments, including changes to deferral rules
and CCA levels.  STAs are able to detect whether the received frame is an
inter-BSS or an intra-BSS frame by using the new BSS color field carried
in the 11ax preamble, and under prescribed conditions, to use a different 
OBSS PD level that is above the minimum receive sensitivity level, and
possibly a reduced transmit power, to send a frame while the channel is 
occupied by the inter-BSS frame.

This phase of the project is focused towards a basic dense-mode deployment
simulation scenario with a number of (overlapping) BSS, each of which
can be configured as 802.11ac or 802.11ax BSS. 
In such a scenario, the hypothesis that 802.11ax can 
lead to greater system performance can be tested.

The software corresponding to this report resides in a private
Bitbucket repository, although portions of the underlying 802.11ax
implementation are in the process of being migrated to the mainline
ns-3 repository.

Outline
=======

* Section 2 provides an overview of the design of the spatial reuse
  extensions.

* Section 3 provides instructions on how to run the ns-3 programs.

* Section 4 provides validation results of the underlying ns-3 Wi-Fi
  models based on TGax calibration scenarios.

* Section 5 provides in-depth results from different parametric studies.

* Section 6 describes open issues and future work.

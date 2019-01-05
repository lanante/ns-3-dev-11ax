.. include:: replace.txt
.. highlight:: cpp

.. heading hierarchy:
   ------------- Chapter
   ************* Section (#.#)
   ============= Subsection (#.#.#)
   ############# Paragraph (no number)

Open Issues and Future Work
***************************
Open issues are tracked in the Bitbucket issue tracker.  Below is a summary
of open issues as of changeset aa56aab1 (Jan 3, 2019).

Open Issues in the underlying Wi-Fi PHY or MAC model
====================================================

The Bianchi validation tests would benefit from plots showing the 
theoretical results on the plots vs. the simulation results. (issue #73)

Document AP(s) in Bianchi test (issue #69)

Some Calibration Box5 scenarios crash ns-3 (issue #43)

Block ack frame format (issue #80)

Reported throughput by script is inconsistent with pcap observed throughput
(issue #74)

Plot throughput results for Bianchi uplink scenario as N is varied
(issue #61)

Enable Time trace callback from MacLow to capture AMPDU and Block Ack
events, for calibration checkpoints (issue #12)

Open issues in the OBSS_PD model support
========================================

There are no known issues in the operation of the ConstantObssPdAlgorithm,
but there is an older implementation of a beacon-based RSSI algorithm
that is implemented in the StaWifiMac class code that should be ported
to the new framework.

Open issues in the spatial reuse scenarios
==========================================

Merge study1.sh and study2.sh into single script; study2.sh does not presently
work (issue #82).

obss_pd script failing on Mac (issue #81)

Rework scripts to avoid files in top-level directory (#77)

Plots are generally based on one iteration of each simulation scenario, 
and results will vary as the random number generator seed value is 
changed and scenarios are rerun.  An averaging of multiple iterations 
will smooth out some irregularities of the plots.

Regression testing on scenario scripts is needed (issue #65)

study1 throughput plots still show irregularities (issue #64)

Fix or cleanup disable wifi-he tests (issue #56)

Plot the CDF of latency per flow (issue #33).

Integrate and test LAA code (issue #5)

Channel model D integration (issue #4)

Propagation models need updating, documentation, testing (issue #1)

Implement additional wish-list of spatial reuse scenario features 
and metrics (issue #10, #14)

Reduce drop radius to r=10 as some nodes on edge are not heard by AP (issue #45)

Non-random downlink throughput distribution (#28)

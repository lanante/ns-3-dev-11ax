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
of open issues as of changeset 91d7fd4e24da998d284b8c434a24bbfecc4d17ae (February 11, 2019).

Open Issues and Future Work in the underlying Wi-Fi model
=========================================================

Channel model D integration (issue #4)

Enable Time trace callback from MacLow to capture AMPDU and Block Ack
events, for calibration checkpoints (issue #12)

Fix or cleanup disable wifi-he tests (issue #56)

Plot throughput results for Bianchi uplink scenario as N is varied (issue #61)

Document AP(s) in Bianchi test (issue #69)

The Bianchi validation tests would benefit from plots showing the 
theoretical results on the plots vs. the simulation results. (issue #73)

Block ack frame format (issue #80)

OBSS_PD algorithm should check for backoff before imposing power restriction (issue #107)

Open Issues and Future Work in the spatial reuse scenarios
==========================================================

Plot the CDF of latency per flow (issue #33)

Implement additional wish-list of spatial reuse scenario features
and metrics (issues #10 #14)

Merge study1.sh and study2.sh into single script; study2.sh does not presently
work (issue #82)

Spatial-reuse should be re-run with inter BSS distance of 34.64m instead of 17.32m (issue #110)

Open Issues and Future Work in the other scenarios
==================================================

Propagation models need updating, documentation, testing (issue #1)

Integrate and test LAA code (issue #5)

Some Calibration Box5 scenarios crash ns-3 (issue #43)

Rework scripts to avoid files in top-level directory ((issue #77)

pngcairo not readily available on MacOS (issue #78)

unequal split of the UL throughput for calibration tests (issue #86)

Indoor small BSS and residential scenarios give lower results than expected  (issue #87)

Get lower throughput than expected for 80-30-02-20 experiments (issue #88)

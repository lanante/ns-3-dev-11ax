Open Issues and Future Work
***************

.. include:: replace.txt
.. highlight:: cpp

.. heading hierarchy:
   ------------- Chapter
   ************* Section (#.#)
   ============= Subsection (#.#.#)
   ############# Paragraph (no number)

Open Issues
===========

Throughput results show some irregularities. These include:

1. Study 1 omits the data points for n=35 and n=40 STA per BSS, as 
the scenarios crash ns-3.

2. Throughput is calculated based on packet transmissions that have 
started before all ADDBA has been established.

3. Throughput of saturated links are lower than theoretical predictions 
due to current implementation of Block Ack Requests (BARs) in ns-3.  
Solutions have been proposed.

4. Plots are generally based on one iteration of each simluation scenario, 
and results will vary as the random number generator seed value is 
changed and scenarios are rerun.  An averaging of multiple iterations 
will smooth out some irregularities of the plots.

The Bianchi validation tests would benefit from plots showing the 
theoretical results on the plots vs. the simulation results.

Some Calibration Box5 scenarios crash ns-3

Future Work
===========

Plot the CDF of latency per flow.




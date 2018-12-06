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
comparable results from WiFi and LTE modules.  Then we describes the test suites
that are provided in order to validate the proper funcationality and correct
simulation output of the modeule.

Calibration
===========

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


Test 1a - MAC overhead w/o RTS/CTS
##################################

The following parameters further refine the Test 1a calibration scenario.


* ``test`` = calibration-test1a

* ``d`` = 1000

* ``r`` = 50

* ``n`` = 1

* ``uplink`` = 10.0

* ``downlink`` = 0.0

* ``txRange`` = 54

Results
#######

The node positions for the calibration test1a scenario is given below.

.. _positions-calibration-test1a:

.. figure:: figures/spatial-reuse-positions-calibration-test1a.*
   :align: center

   Node positions for the calibration test 1a.

Figure :ref:`positions-calibration-test1a` illustrates the node positions
of the calibration test 1a scenario.

We can see that TBD

The ECDF of the signals received by AP1 is given below.

.. _calibration-test1a-ap1-signal:

.. figure:: figures/spatial-reuse-rx-sniff-calibration-test1a-ap1-signal.*
   :align: center

   ECDF of the signals received by AP1 for the calibration test 1a.

Figure :ref:`calibration-test1a-ap1-signal` illustrates the ECDF of the signals
received by AP1 for the calibration test 1a scenario.

We can see that TBD


The ECDF of the signals received by AP2 is given below.

.. _calibration-test1a-ap2-signal:

.. figure:: figures/spatial-reuse-rx-sniff-calibration-test1a-ap2-signal.*
   :align: center

   ECDF of the signals received by AP2 for the calibration test 1a.

Figure :ref:`calibration-test1a-ap2-signal` illustrates the ECDF of the signals
received by AP2 for the calibration test 1a scenario.

We can see that TBD


The ECDF of the noise levels received by AP1 is given below.

.. _calibration-test1a-ap1-noise:

.. figure:: figures/spatial-reuse-rx-sniff-calibration-test1a-ap1-noise.*
   :align: center

   ECDF of the noise level received by AP1 for the calibration test 1a.

Figure :ref:`calibration-test1a-ap1-noise` illustrates the ECDF of the noise levels
received by AP1 for the calibration test 1a scenario.

We can see that TBD


The ECDF of the noise levels received by AP2 is given below.

.. _calibration-test1a-ap2-noise:

.. figure:: figures/spatial-reuse-rx-sniff-calibration-test1a-ap2-noise.*
   :align: center

   ECDF of the noise levels received by AP2 for the calibration test 1a.

Figure :ref:`calibration-test1a-ap2-noise` illustrates the ECDF of the noise levels
received by AP2 for the calibration test 1a scenario.

We can see that TBD


Test 1b - MAC overhead w/ RTS/CTS
##################################

Test 1b is nearly identical to Test 1a, except that RTS/CTS is enabled
in Test 1b, whereas it is disabled in Test 1a.

The following parameters further refine the Test 1b calibration scenario.

* ``test`` = calibration-test1b

* ``d`` = 1000

* ``r`` = 50

* ``n`` = 1

* ``uplink`` = 10.0

* ``downlink`` = 0.0

* ``enableRts`` = 1

* ``txRange`` = 54

Results
#######

The node positions for the calibration test1b scenario is given below.

.. _positions-calibration-test1b:

.. figure:: figures/spatial-reuse-positions-calibration-test1b.*
   :align: center

   Node positions for the calibration test 1b.

Figure :ref:`positions-calibration-test1b` illustrates the node positions
of the calibration test 1b scenario.

We can see that TBD

The ECDF of the signals received by AP1 is given below.

.. _calibration-test1b-ap1-signal:

.. figure:: figures/spatial-reuse-rx-sniff-calibration-test1b-ap1-signal.*
   :align: center

   ECDF of the signals received by AP1 for the calibration test 1b.

Figure :ref:`calibration-test1b-ap1-signal` illustrates the ECDF of the signals
received by AP1 for the calibration test 1b scenario.

We can see that TBD


The ECDF of the signals received by AP2 is given below.

.. _calibration-test1b-ap2-signal:

.. figure:: figures/spatial-reuse-rx-sniff-calibration-test1b-ap2-signal.*
   :align: center

   ECDF of the signals received by AP2 for the calibration test 1b.

Figure :ref:`calibration-test1b-ap2-signal` illustrates the ECDF of the signals
received by AP2 for the calibration test 1b scenario.

We can see that TBD


The ECDF of the noise levels received by AP1 is given below.

.. _calibration-test1b-ap1-noise:

.. figure:: figures/spatial-reuse-rx-sniff-calibration-test1b-ap1-noise.*
   :align: center

   ECDF of the noise level received by AP1 for the calibration test 1b.

Figure :ref:`calibration-test1b-ap1-noise` illustrates the ECDF of the noise levels
received by AP1 for the calibration test 1b scenario.

We can see that TBD


The ECDF of the noise levels received by AP2 is given below.

.. _calibration-test1b-ap2-noise:

.. figure:: figures/spatial-reuse-rx-sniff-calibration-test1b-ap2-noise.*
   :align: center

   ECDF of the noise levels received by AP2 for the calibration test 1b.

Figure :ref:`calibration-test1b-ap2-noise` illustrates the ECDF of the noise levels
received by AP2 for the calibration test 1b scenario.

We can see that TBD


Validation
==========

TBD

.. include:: replace.txt
.. highlight:: cpp

.. heading hierarchy:
   ------------- Chapter
   ************* Section (#.#)
   ============= Subsection (#.#.#)
   ############# Paragraph (no number)

Design
******

Spatial Reuse Design
====================

|ns3| support for 802.11ax (High Efficiency (HE) WLAN) ([Sta16]_) spatial
reuse features are found in the ``src/wifi/model`` module.

However, scripts to evaluate the performance of these extensions are found
in the ``src/wifi-he/scripts`` directory.  These scripts exist in a separate
module because we originally thought that 11ax would reside in a separate
module from ns-3's ``wifi`` module, but later we made the decision that
802.11ax extensions should reside in the main ``wifi`` module.

The implementation includes the following support:

1. A new ``HeConfiguration`` object to manage 802.11ax-specific configuration,
   in particular, BSS color parameter.

2. Extensions to the ``WifiTxVector`` object to carry the BSS color.

3. Extensions to the ``WifiPhy`` receive methods to model the reception
   of a notional HE preamble and PHY header, and to convey BSS color and
   other fields to a separate ``ObssPdAlgorithm`` object.

4. The ``ObssPdAlgorithm`` base class allows for different algorithms to
   be implemented to a common interface.  The default algorithm is a
   ``ConstantObssPdAlgorithm``. There exists also an implementation of a
   ``BeaconRssiObssPdAlgorithm`` that does not support transmit power
   control; it is implemented in the ``ns3::StaWifiMac`` class but will
   be moved to a subclass of ``ObssPdAlgorithm``.

HE PHY preamble and header reception
====================================

Once the PHY is done with receiving preambles, headers as well as training fields and is about to start receiving the payload,
it checks whether the packet under reception is HE. In this case, it reads the BSS color from the HE SIG field and triggers a trace
source named ``EndOfHePreamble``. OBSS PD algorithms can connect to this trace source to be notified when reception of a HE packet has started.

OBSS PD Algorithm
=================

``ObssPdAlgorithm`` is the base class of OBSS PD algorithms.
It implements the common functionalities. First, it makes sure the necessary callbacks are setup.
Second, when a PHY reset is requested by the algorithm, it performs the computation to determine the TX power
limitation and informs the PHY object.

Constant OBSS PD Algorithm
==========================

Constant OBSS PD algorithm is a simple OBSS PD algorithm implemmented in the ``ConstantObssPdAlgorithm`` class.

Once a HE preamble and its header have been received by the PHY, ``ConstantObssPdAlgorithm::
ReceiveHeSig`` is triggered.
The algorithm then checks whether this is an OBSS frame by comparing its own BSS color with the BSS color of the received preamble.
If this is an OBSS frame, it compare the received RSSI with its configured OBSS PD level value. The PHY then gets reset to IDLE
state in case the received RSSI is lower than that constant OBSS PD level value, and is informed about a TX power limitations.
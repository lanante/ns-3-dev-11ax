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

1. A new ``HeConfiguration`` object to manage 802.11ax-specific configuration;
   in particular, BSS color and OBSS_PD-related parameters.

2. Extensions to the ``WifiPreamble`` object to carry the BSS color.

3. Extensions to the ``WifiPhy`` receive methods to model the reception
   of a notional HE-SIG-A field, and to convey BSS color and other fields
   to a separate ``ObssPdAlgorithm`` object.

4. The ``ObssPdAlgorithm`` base class allows for different algorithms to
   be implemented to a common interface.  The initial algorithm is a 
   ``ConstantObssPdAlgorithm``
  

Constant OBSS_PD Algorithm
==========================

placeholder

Scope And Limitations
=====================

placeholder


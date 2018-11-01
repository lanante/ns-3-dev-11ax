.. include:: replace.txt
.. highlight:: cpp

.. heading hierarchy:
   ------------- Chapter
   ************* Section (#.#)
   ============= Subsection (#.#.#)
   ############# Paragraph (no number)

Voice Application
-----------------

The class ns3::VoiceApplication implements a simple model for a voice-like
data transfer over |ns3|.  The goal of this application has not been to
provide a sophisticated statistical model of actual voice traffic patterns
from, for example, a variable bit rate codec.  Instead, this application
focuses on requirements defined in 3GPP for voice flows, and focuses in
particular on measuring latency statistics observed on the receiving
side of a flow.

Model Description
*****************

The following describes the basic requirements that prompted this model,
and following this, we describe the overall design.

3GPP TR36.889 states:

  For VoIP capacity evaluations, the following performance metrics need to be considered:

  * VoIP system capacity in form of the maximum number of satisfied users supported per cell in downlink and uplink.
  * System capacity is defined as the number of users in the cell when more than [95%] of the users are satisfied.
  * A VoIP user is in outage (not satisfied) if [98%] radio interface tail latency of the user is greater than [50 ms].  This assumes an end-to-end delay below [200 ms] for mobile-to-mobile communications.

and later it states:

  Optional: Mixed traffic model with each UE carrying only VoIP traffic or only FTP traffic in the Wi-Fi network that is not replaced by LAA.

  * Two UEs with VoIP traffic in addition to UEs with FTP traffic
  * The VoIP traffic model is based on G.729A (data rate is 24 kbps)
  * Packet inter-arrival time: 20 ms
  * Packet size: 60 bytes (payload plus IP header overhead)
  * Voice activity is assumed to be 100%. Statistics are independently reported
    in each direction
  * No associated control plane traffic is modelled
  * For DL+UL coexistence evaluations the voice activity of the VoIP users is 50% for both DL and UL.
  * For DL+UL coexistence evaluations for each VoIP user, On and Off periods of length X (e.g., X = 5) second alternates with each other in such a way that both DL and UL are not active at the same time.

Design
======

Briefly describe the software design of the model and how it fits into
the existing ns-3 architecture.

Scope and Limitations
=====================

What can the model do?  What can it not do?  Please use this section to
describe the scope and limitations of the model.

References
==========

Add academic citations here, such as if you published a paper on this
model, or if readers should read a particular specification or other work.

Usage
*****

This section is principally concerned with the usage of your model, using
the public API.  Focus first on most common usage patterns, then go
into more advanced topics.

Building New Module
===================

Include this subsection only if there are special build instructions or
platform limitations.

Helpers
=======

What helper API will users typically use?  Describe it here.

Attributes
==========

What classes hold attributes, and what are the key ones worth mentioning?

Output
======

What kind of data does the model generate?  What are the key trace
sources?   What kind of logging output can be enabled?

Advanced Usage
==============

Go into further details (such as using the API outside of the helpers)
in additional sections, as needed.

Examples
========

What examples using this new code are available?  Describe them here.

Troubleshooting
===============

Add any tips for avoiding pitfalls, etc.

Validation
**********

Describe how the model has been tested/validated.  What tests run in the
test suite?  How much API and code is covered by the tests?  Again,
references to outside published work may help here.


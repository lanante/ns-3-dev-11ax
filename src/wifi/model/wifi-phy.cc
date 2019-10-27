/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Sébastien Deronne <sebastien.deronne@gmail.com>
 */

#include <algorithm>
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/mobility-model.h"
#include "ns3/random-variable-stream.h"
#include "ns3/error-model.h"
#include "wifi-phy.h"
#include "ampdu-tag.h"
#include "wifi-utils.h"
#include "sta-wifi-mac.h"
#include "frame-capture-model.h"
#include "preamble-detection-model.h"
#include "wifi-radio-energy-model.h"
#include "error-rate-model.h"
#include "wifi-net-device.h"
#include "ht-configuration.h"
#include "he-configuration.h"
#include "mpdu-aggregator.h"
#include "wifi-psdu.h"
#include "wifi-ppdu.h"
#include "ap-wifi-mac.h"
#include "wifi-phy-header.h"
#include "channel-bonding-manager.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WifiPhy");

/****************************************************************
 *       The actual WifiPhy class
 ****************************************************************/

NS_OBJECT_ENSURE_REGISTERED (WifiPhy);

uint64_t WifiPhy::m_globalPpduUid = 0;

/**
 * This table maintains the mapping of valid ChannelNumber to
 * Frequency/ChannelWidth pairs.  If you want to make a channel applicable
 * to all standards, then you may use the WIFI_PHY_STANDARD_UNSPECIFIED
 * standard to represent this, as a wildcard.  If you want to limit the
 * configuration of a particular channel/frequency/width to a particular
 * standard(s), then you can specify one or more such bindings.
 */
WifiPhy::ChannelToFrequencyWidthMap WifiPhy::m_channelToFrequencyWidth =
{
  // 802.11b uses width of 22, while OFDM modes use width of 20
  { std::make_pair (1, WIFI_PHY_STANDARD_80211b), std::make_pair (2412, 22) },
  { std::make_pair (1, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2412, 20) },
  { std::make_pair (2, WIFI_PHY_STANDARD_80211b), std::make_pair (2417, 22) },
  { std::make_pair (2, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2417, 20) },
  { std::make_pair (3, WIFI_PHY_STANDARD_80211b), std::make_pair (2422, 22) },
  { std::make_pair (3, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2422, 20) },
  { std::make_pair (4, WIFI_PHY_STANDARD_80211b), std::make_pair (2427, 22) },
  { std::make_pair (4, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2427, 20) },
  { std::make_pair (5, WIFI_PHY_STANDARD_80211b), std::make_pair (2432, 22) },
  { std::make_pair (5, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2432, 20) },
  { std::make_pair (6, WIFI_PHY_STANDARD_80211b), std::make_pair (2437, 22) },
  { std::make_pair (6, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2437, 20) },
  { std::make_pair (7, WIFI_PHY_STANDARD_80211b), std::make_pair (2442, 22) },
  { std::make_pair (7, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2442, 20) },
  { std::make_pair (8, WIFI_PHY_STANDARD_80211b), std::make_pair (2447, 22) },
  { std::make_pair (8, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2447, 20) },
  { std::make_pair (9, WIFI_PHY_STANDARD_80211b), std::make_pair (2452, 22) },
  { std::make_pair (9, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2452, 20) },
  { std::make_pair (10, WIFI_PHY_STANDARD_80211b), std::make_pair (2457, 22) },
  { std::make_pair (10, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2457, 20) },
  { std::make_pair (11, WIFI_PHY_STANDARD_80211b), std::make_pair (2462, 22) },
  { std::make_pair (11, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2462, 20) },
  { std::make_pair (12, WIFI_PHY_STANDARD_80211b), std::make_pair (2467, 22) },
  { std::make_pair (12, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2467, 20) },
  { std::make_pair (13, WIFI_PHY_STANDARD_80211b), std::make_pair (2472, 22) },
  { std::make_pair (13, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (2472, 20) },
  // Only defined for 802.11b
  { std::make_pair (14, WIFI_PHY_STANDARD_80211b), std::make_pair (2484, 22) },

  // Now the 5GHz channels; UNSPECIFIED for 802.11a/n/ac/ax channels
  // 20 MHz channels
  { std::make_pair (36, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5180, 20) },
  { std::make_pair (40, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5200, 20) },
  { std::make_pair (44, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5220, 20) },
  { std::make_pair (48, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5240, 20) },
  { std::make_pair (52, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5260, 20) },
  { std::make_pair (56, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5280, 20) },
  { std::make_pair (60, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5300, 20) },
  { std::make_pair (64, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5320, 20) },
  { std::make_pair (100, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5500, 20) },
  { std::make_pair (104, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5520, 20) },
  { std::make_pair (108, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5540, 20) },
  { std::make_pair (112, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5560, 20) },
  { std::make_pair (116, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5580, 20) },
  { std::make_pair (120, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5600, 20) },
  { std::make_pair (124, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5620, 20) },
  { std::make_pair (128, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5640, 20) },
  { std::make_pair (132, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5660, 20) },
  { std::make_pair (136, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5680, 20) },
  { std::make_pair (140, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5700, 20) },
  { std::make_pair (144, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5720, 20) },
  { std::make_pair (149, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5745, 20) },
  { std::make_pair (153, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5765, 20) },
  { std::make_pair (157, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5785, 20) },
  { std::make_pair (161, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5805, 20) },
  { std::make_pair (165, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5825, 20) },
  { std::make_pair (169, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5845, 20) },
  { std::make_pair (173, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5865, 20) },
  { std::make_pair (177, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5885, 20) },
  { std::make_pair (181, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5905, 20) },
  // 40 MHz channels
  { std::make_pair (38, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5190, 40) },
  { std::make_pair (46, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5230, 40) },
  { std::make_pair (54, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5270, 40) },
  { std::make_pair (62, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5310, 40) },
  { std::make_pair (102, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5510, 40) },
  { std::make_pair (110, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5550, 40) },
  { std::make_pair (118, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5590, 40) },
  { std::make_pair (126, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5630, 40) },
  { std::make_pair (134, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5670, 40) },
  { std::make_pair (142, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5710, 40) },
  { std::make_pair (151, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5755, 40) },
  { std::make_pair (159, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5795, 40) },
  { std::make_pair (167, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5835, 40) },
  { std::make_pair (175, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5875, 40) },
  // 80 MHz channels
  { std::make_pair (42, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5210, 80) },
  { std::make_pair (58, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5290, 80) },
  { std::make_pair (106, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5530, 80) },
  { std::make_pair (122, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5610, 80) },
  { std::make_pair (138, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5690, 80) },
  { std::make_pair (155, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5775, 80) },
  { std::make_pair (171, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5855, 80) },
  // 160 MHz channels
  { std::make_pair (50, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5250, 160) },
  { std::make_pair (114, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5570, 160) },
  { std::make_pair (163, WIFI_PHY_STANDARD_UNSPECIFIED), std::make_pair (5815, 160) },

  // 802.11p (10 MHz channels at the 5.855-5.925 band
  { std::make_pair (172, WIFI_PHY_STANDARD_80211_10MHZ), std::make_pair (5860, 10) },
  { std::make_pair (174, WIFI_PHY_STANDARD_80211_10MHZ), std::make_pair (5870, 10) },
  { std::make_pair (176, WIFI_PHY_STANDARD_80211_10MHZ), std::make_pair (5880, 10) },
  { std::make_pair (178, WIFI_PHY_STANDARD_80211_10MHZ), std::make_pair (5890, 10) },
  { std::make_pair (180, WIFI_PHY_STANDARD_80211_10MHZ), std::make_pair (5900, 10) },
  { std::make_pair (182, WIFI_PHY_STANDARD_80211_10MHZ), std::make_pair (5910, 10) },
  { std::make_pair (184, WIFI_PHY_STANDARD_80211_10MHZ), std::make_pair (5920, 10) }
};

TypeId
WifiPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WifiPhy")
    .SetParent<Object> ()
    .SetGroupName ("Wifi")
    .AddAttribute ("Frequency",
                   "The operating center frequency (MHz)",
                   UintegerValue (0),
                   MakeUintegerAccessor (&WifiPhy::GetFrequency,
                                         &WifiPhy::SetFrequency),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("ChannelWidth",
                   "Whether 5MHz, 10MHz, 20MHz, 22MHz, 40MHz, 80 MHz or 160 MHz.",
                   UintegerValue (20),
                   MakeUintegerAccessor (&WifiPhy::GetChannelWidth,
                                         &WifiPhy::SetChannelWidth),
                   MakeUintegerChecker<uint16_t> (5, 160))
    .AddAttribute ("ChannelNumber",
                   "If set to non-zero defined value, will control Frequency and ChannelWidth assignment",
                   UintegerValue (0),
                   MakeUintegerAccessor (&WifiPhy::SetChannelNumber,
                                         &WifiPhy::GetChannelNumber),
                   MakeUintegerChecker<uint8_t> (0, 196))
    .AddAttribute ("PrimaryChannelNumber",
                   "Select the primary 20 MHz channel if a channel width of 40, 80 or 160 MHz is used",
                   UintegerValue (36),
                   MakeUintegerAccessor (&WifiPhy::SetPrimaryChannelNumber,
                                         &WifiPhy::GetPrimaryChannelNumber),
                   MakeUintegerChecker<uint8_t> (0, 181))
    .AddAttribute ("EnergyDetectionThreshold",
                   "The energy of a received signal should be higher than "
                   "this threshold (dbm) to allow the PHY layer to detect the signal.",
                   DoubleValue (-101.0),
                   MakeDoubleAccessor (&WifiPhy::SetEdThreshold),
                   MakeDoubleChecker<double> (),
                   TypeId::DEPRECATED, "Replaced by RxSensitivity.")
    .AddAttribute ("RxSensitivity",
                   "The energy of a received signal should be higher than "
                   "this threshold (dBm) for the PHY to detect the signal.",
                   DoubleValue (-101.0),
                   MakeDoubleAccessor (&WifiPhy::SetRxSensitivity,
                                       &WifiPhy::GetRxSensitivity),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("CcaEdThreshold",
                   "The energy of a non Wi-Fi received signal should be higher than "
                   "this threshold (dbm) to allow the PHY layer to declare CCA BUSY state. "
                   "This check is performed on the 20 MHz primary channel only.",
                   DoubleValue (-62.0),
                   MakeDoubleAccessor (&WifiPhy::SetCcaEdThreshold,
                                       &WifiPhy::GetCcaEdThreshold),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxGain",
                   "Transmission gain (dB).",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&WifiPhy::SetTxGain,
                                       &WifiPhy::GetTxGain),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RxGain",
                   "Reception gain (dB).",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&WifiPhy::SetRxGain,
                                       &WifiPhy::GetRxGain),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxPowerLevels",
                   "Number of transmission power levels available between "
                   "TxPowerStart and TxPowerEnd included.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&WifiPhy::m_nTxPower),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("TxPowerEnd",
                   "Maximum available transmission level (dbm).",
                   DoubleValue (16.0206),
                   MakeDoubleAccessor (&WifiPhy::SetTxPowerEnd,
                                       &WifiPhy::GetTxPowerEnd),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxPowerStart",
                   "Minimum available transmission level (dbm).",
                   DoubleValue (16.0206),
                   MakeDoubleAccessor (&WifiPhy::SetTxPowerStart,
                                       &WifiPhy::GetTxPowerStart),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RxNoiseFigure",
                   "Loss (dB) in the Signal-to-Noise-Ratio due to non-idealities in the receiver."
                   " According to Wikipedia (http://en.wikipedia.org/wiki/Noise_figure), this is "
                   "\"the difference in decibels (dB) between"
                   " the noise output of the actual receiver to the noise output of an "
                   " ideal receiver with the same overall gain and bandwidth when the receivers "
                   " are connected to sources at the standard noise temperature T0 (usually 290 K)\".",
                   DoubleValue (7),
                   MakeDoubleAccessor (&WifiPhy::SetRxNoiseFigure),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("State",
                   "The state of the PHY layer.",
                   PointerValue (),
                   MakePointerAccessor (&WifiPhy::m_state),
                   MakePointerChecker<WifiPhyStateHelper> ())
    .AddAttribute ("ChannelSwitchDelay",
                   "Delay between two short frames transmitted on different frequencies.",
                   TimeValue (MicroSeconds (250)),
                   MakeTimeAccessor (&WifiPhy::m_channelSwitchDelay),
                   MakeTimeChecker ())
    .AddAttribute ("Antennas",
                   "The number of antennas on the device.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&WifiPhy::GetNumberOfAntennas,
                                         &WifiPhy::SetNumberOfAntennas),
                   MakeUintegerChecker<uint8_t> (1, 8))
    .AddAttribute ("MaxSupportedTxSpatialStreams",
                   "The maximum number of supported TX spatial streams."
                   "This parameter is only valuable for 802.11n/ac/ax STAs and APs.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&WifiPhy::GetMaxSupportedTxSpatialStreams,
                                         &WifiPhy::SetMaxSupportedTxSpatialStreams),
                   MakeUintegerChecker<uint8_t> (1, 8))
    .AddAttribute ("MaxSupportedRxSpatialStreams",
                   "The maximum number of supported RX spatial streams."
                   "This parameter is only valuable for 802.11n/ac/ax STAs and APs.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&WifiPhy::GetMaxSupportedRxSpatialStreams,
                                         &WifiPhy::SetMaxSupportedRxSpatialStreams),
                   MakeUintegerChecker<uint8_t> (1, 8))
    .AddAttribute ("ShortGuardEnabled",
                   "Whether or not short guard interval is enabled for HT/VHT transmissions."
                   "This parameter is only valuable for 802.11n/ac/ax STAs and APs.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&WifiPhy::GetShortGuardInterval,
                                        &WifiPhy::SetShortGuardInterval),
                   MakeBooleanChecker (),
                   TypeId::DEPRECATED, "Use the HtConfiguration instead")
    .AddAttribute ("GuardInterval",
                   "Whether 800ns, 1600ns or 3200ns guard interval is used for HE transmissions."
                   "This parameter is only valuable for 802.11ax STAs and APs.",
                   TimeValue (NanoSeconds (3200)),
                   MakeTimeAccessor (&WifiPhy::GetGuardInterval,
                                     &WifiPhy::SetGuardInterval),
                   MakeTimeChecker (NanoSeconds (800), NanoSeconds (3200)),
                   TypeId::DEPRECATED, "Use the HeConfiguration instead")
    .AddAttribute ("GreenfieldEnabled",
                   "Whether or not Greenfield is enabled."
                   "This parameter is only valuable for 802.11n STAs and APs.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&WifiPhy::GetGreenfield,
                                        &WifiPhy::SetGreenfield),
                   MakeBooleanChecker (),
                   TypeId::DEPRECATED, "Use the HtConfiguration instead")
    .AddAttribute ("ShortPlcpPreambleSupported",
                   "Whether or not short PLCP preamble is supported."
                   "This parameter is only valuable for 802.11b STAs and APs."
                   "Note: 802.11g APs and STAs always support short PLCP preamble.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&WifiPhy::GetShortPlcpPreambleSupported,
                                        &WifiPhy::SetShortPlcpPreambleSupported),
                   MakeBooleanChecker ())
    .AddAttribute ("FrameCaptureModel",
                   "Ptr to an object that implements the frame capture model",
                   PointerValue (),
                   MakePointerAccessor (&WifiPhy::m_frameCaptureModel),
                   MakePointerChecker <FrameCaptureModel> ())
    .AddAttribute ("PreambleDetectionModel",
                   "Ptr to an object that implements the preamble detection model",
                   PointerValue (),
                   MakePointerAccessor (&WifiPhy::m_preambleDetectionModel),
                   MakePointerChecker <PreambleDetectionModel> ())
    .AddAttribute ("PostReceptionErrorModel",
                   "An optional packet error model can be added to the receive "
                   "packet process after any propagation-based (SNR-based) error "
                   "models have been applied. Typically this is used to force "
                   "specific packet drops, for testing purposes.",
                   PointerValue (),
                   MakePointerAccessor (&WifiPhy::m_postReceptionErrorModel),
                   MakePointerChecker<ErrorModel> ())
    .AddAttribute ("ChannelBondingManager",
                   "Ptr to the channel bonding manager",
                   PointerValue (),
                   MakePointerAccessor (&WifiPhy::m_channelBondingManager),
                   MakePointerChecker <ChannelBondingManager> ())
    .AddTraceSource ("PhyTxBegin",
                     "Trace source indicating a packet "
                     "has begun transmitting over the channel medium",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyTxBeginTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PhyTxEnd",
                     "Trace source indicating a packet "
                     "has been completely transmitted over the channel. "
                     "NOTE: the only official WifiPhy implementation "
                     "available to this date never fires "
                     "this trace source.",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyTxEndTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PhyTxDrop",
                     "Trace source indicating a packet "
                     "has been dropped by the device during transmission",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyTxDropTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PhyRxBegin",
                     "Trace source indicating a packet "
                     "has begun being received from the channel medium "
                     "by the device",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyRxBeginTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PhyRxPayloadBegin",
                     "Trace source indicating the reception of the "
                     "payload of a PPDU has begun",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyRxPayloadBeginTrace),
                     "ns3::WifiPhy::PhyRxPayloadBeginTracedCallback")
    .AddTraceSource ("PhyRxEnd",
                     "Trace source indicating a packet "
                     "has been completely received from the channel medium "
                     "by the device",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyRxEndTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PhyRxDrop",
                     "Trace source indicating a packet "
                     "has been dropped by the device during reception",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyRxDropTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MonitorSnifferRx",
                     "Trace source simulating a wifi device in monitor mode "
                     "sniffing all received frames",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyMonitorSniffRxTrace),
                     "ns3::WifiPhy::MonitorSnifferRxTracedCallback")
    .AddTraceSource ("MonitorSnifferTx",
                     "Trace source simulating the capability of a wifi device "
                     "in monitor mode to sniff all frames being transmitted",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyMonitorSniffTxTrace),
                     "ns3::WifiPhy::MonitorSnifferTxTracedCallback")
    .AddTraceSource ("EndOfHePreamble",
                     "Trace source indicating the end of the 802.11ax preamble (after training fields)",
                     MakeTraceSourceAccessor (&WifiPhy::m_phyEndOfHePreambleTrace),
                     "ns3::WifiPhy::EndOfHePreambleTracedCallback")
  ;
  return tid;
}

WifiPhy::WifiPhy ()
  : m_txMpduReferenceNumber (0xffffffff),
    m_rxMpduReferenceNumber (0xffffffff),
    m_endPlcpRxEvent (),
    m_currentEvent (0),
    m_currentHeTbPpduUid (UINT64_MAX),
    m_previouslyRxPpduUid (UINT64_MAX),
    m_standard (WIFI_PHY_STANDARD_UNSPECIFIED),
    m_isConstructed (false),
    m_channelCenterFrequency (0),
    m_initialFrequency (0),
    m_frequencyChannelNumberInitialized (false),
    m_channelWidth (0),
    m_channelAccessRequested (false),
    m_txSpatialStreams (0),
    m_rxSpatialStreams (0),
    m_channelNumber (0),
    m_initialChannelNumber (0),
    m_wifiRadioEnergyModel (0),
    m_timeLastPreambleDetected (Seconds (0)),
    m_ofdmaStarted (false)
{
  NS_LOG_FUNCTION (this);
  m_random = CreateObject<UniformRandomVariable> ();
  m_state = CreateObject<WifiPhyStateHelper> ();
}

WifiPhy::~WifiPhy ()
{
  NS_LOG_FUNCTION (this);
}

void
WifiPhy::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  for (auto & endRxEvent : m_endRxEvents)
    {
      endRxEvent.Cancel ();
    }
  m_endRxEvents.clear ();
  m_endPlcpRxEvent.Cancel ();
  for (auto & endPreambleDetectionEvent : m_endPreambleDetectionEvents)
    {
      endPreambleDetectionEvent.Cancel ();
    }
  m_endPreambleDetectionEvents.clear ();
  m_device = 0;
  m_mobility = 0;
  m_state->Dispose ();
  m_state = 0;
  m_wifiRadioEnergyModel = 0;
  m_postReceptionErrorModel = 0;
  m_deviceRateSet.clear ();
  m_deviceMcsSet.clear ();
  m_mcsIndexMap.clear ();
  m_currentPreambleEvents.clear ();
}

void
WifiPhy::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_isConstructed = true;
  m_state->Initialize ();
  if (m_frequencyChannelNumberInitialized == true)
    {
      NS_LOG_DEBUG ("Frequency already initialized");
      return;
    }
  InitializeFrequencyChannelNumber ();
}

Ptr<WifiPhyStateHelper>
WifiPhy::GetState (void) const
{
  return m_state;
}

void
WifiPhy::SetReceiveOkCallback (RxOkCallback callback)
{
  m_state->SetReceiveOkCallback (callback);
}

void
WifiPhy::SetReceiveErrorCallback (RxErrorCallback callback)
{
  m_state->SetReceiveErrorCallback (callback);
}

void
WifiPhy::RegisterListener (WifiPhyListener *listener)
{
  m_state->RegisterListener (listener);
}

void
WifiPhy::UnregisterListener (WifiPhyListener *listener)
{
  m_state->UnregisterListener (listener);
}

void
WifiPhy::SetCapabilitiesChangedCallback (Callback<void> callback)
{
  m_capabilitiesChangedCallback = callback;
}

void
WifiPhy::InitializeFrequencyChannelNumber (void)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (m_frequencyChannelNumberInitialized == false, "Initialization called twice");

  // If frequency has been set to a non-zero value during attribute
  // construction phase, the frequency and channel width will drive the
  // initial configuration.  If frequency has not been set, but both
  // standard and channel number have been set, that pair will instead
  // drive the configuration, and frequency and channel number will be
  // aligned
  if (m_initialFrequency != 0)
    {
      SetFrequency (m_initialFrequency);
    }
  else if (m_initialChannelNumber != 0 && GetStandard () != WIFI_PHY_STANDARD_UNSPECIFIED)
    {
      SetChannelNumber (m_initialChannelNumber);
    }
  else if (m_initialChannelNumber != 0 && GetStandard () == WIFI_PHY_STANDARD_UNSPECIFIED)
    {
      NS_FATAL_ERROR ("Error, ChannelNumber " << +GetChannelNumber () << " was set by user, but neither a standard nor a frequency");
    }
  m_frequencyChannelNumberInitialized = true;
}

void
WifiPhy::SetEdThreshold (double threshold)
{
  SetRxSensitivity (threshold);
}

void
WifiPhy::SetRxSensitivity (double threshold)
{
  NS_LOG_FUNCTION (this << threshold);
  m_rxSensitivityW = DbmToW (threshold);
}

double
WifiPhy::GetRxSensitivity (void) const
{
  return WToDbm (m_rxSensitivityW);
}

void
WifiPhy::SetCcaEdThreshold (double threshold)
{
  NS_LOG_FUNCTION (this << threshold);
  m_ccaEdThresholdW = DbmToW (threshold);
}

double
WifiPhy::GetCcaEdThreshold (void) const
{
  return WToDbm (m_ccaEdThresholdW);
}

void
WifiPhy::AddCcaEdThresholdSecondary (double threshold)
{
  NS_LOG_FUNCTION (this << threshold);
  auto it = std::find (m_ccaEdThresholdsSecondaryW.begin (), m_ccaEdThresholdsSecondaryW.end (), DbmToW (threshold));
  if (it == m_ccaEdThresholdsSecondaryW.end ())
    {
      m_ccaEdThresholdsSecondaryW.push_back (DbmToW (threshold));
    }
}

void
WifiPhy::RemoveCcaEdThresholdSecondary (double threshold)
{
  NS_LOG_FUNCTION (this << threshold);
  auto it = std::find (m_ccaEdThresholdsSecondaryW.begin (), m_ccaEdThresholdsSecondaryW.end (), DbmToW (threshold));
  if (it != m_ccaEdThresholdsSecondaryW.end ())
    {
      m_ccaEdThresholdsSecondaryW.erase (it);
    }
}

double
WifiPhy::GetDefaultCcaEdThresholdSecondary (void) const
{
  return m_ccaEdThresholdsSecondaryW.front ();
}

void
WifiPhy::SetRxNoiseFigure (double noiseFigureDb)
{
  NS_LOG_FUNCTION (this << noiseFigureDb);
  m_interference.SetNoiseFigure (DbToRatio (noiseFigureDb));
  m_interference.SetNumberOfReceiveAntennas (GetNumberOfAntennas ());
}

void
WifiPhy::SetTxPowerStart (double start)
{
  NS_LOG_FUNCTION (this << start);
  m_txPowerBaseDbm = start;
}

double
WifiPhy::GetTxPowerStart (void) const
{
  return m_txPowerBaseDbm;
}

void
WifiPhy::SetTxPowerEnd (double end)
{
  NS_LOG_FUNCTION (this << end);
  m_txPowerEndDbm = end;
}

double
WifiPhy::GetTxPowerEnd (void) const
{
  return m_txPowerEndDbm;
}

void
WifiPhy::SetNTxPower (uint8_t n)
{
  NS_LOG_FUNCTION (this << +n);
  m_nTxPower = n;
}

uint8_t
WifiPhy::GetNTxPower (void) const
{
  return m_nTxPower;
}

void
WifiPhy::SetTxGain (double gain)
{
  NS_LOG_FUNCTION (this << gain);
  m_txGainDb = gain;
}

double
WifiPhy::GetTxGain (void) const
{
  return m_txGainDb;
}

void
WifiPhy::SetRxGain (double gain)
{
  NS_LOG_FUNCTION (this << gain);
  m_rxGainDb = gain;
}

double
WifiPhy::GetRxGain (void) const
{
  return m_rxGainDb;
}

void
WifiPhy::SetGreenfield (bool greenfield)
{
  NS_LOG_FUNCTION (this << greenfield);
  Ptr<WifiNetDevice> device = DynamicCast<WifiNetDevice> (GetDevice ());
  if (device)
    {
      Ptr<HtConfiguration> htConfiguration = device->GetHtConfiguration ();
      if (htConfiguration)
        {
          htConfiguration->SetGreenfieldSupported (greenfield);
        }
    }
  m_greenfield = greenfield;
}

bool
WifiPhy::GetGreenfield (void) const
{
  Ptr<WifiNetDevice> device = DynamicCast<WifiNetDevice> (GetDevice ());
  if (device)
    {
      Ptr<HtConfiguration> htConfiguration = device->GetHtConfiguration ();
      if (htConfiguration)
        {
          return htConfiguration->GetGreenfieldSupported ();
        }
    }
  return m_greenfield;
}

void
WifiPhy::SetShortGuardInterval (bool shortGuardInterval)
{
  NS_LOG_FUNCTION (this << shortGuardInterval);
  Ptr<WifiNetDevice> device = DynamicCast<WifiNetDevice> (GetDevice ());
  if (device)
    {
      Ptr<HtConfiguration> htConfiguration = device->GetHtConfiguration ();
      if (htConfiguration)
        {
          htConfiguration->SetShortGuardIntervalSupported (shortGuardInterval);
        }
    }
  m_shortGuardInterval = shortGuardInterval;
}

bool
WifiPhy::GetShortGuardInterval (void) const
{
  Ptr<WifiNetDevice> device = DynamicCast<WifiNetDevice> (GetDevice ());
  if (device)
    {
      Ptr<HtConfiguration> htConfiguration = device->GetHtConfiguration ();
      if (htConfiguration)
        {
          return htConfiguration->GetShortGuardIntervalSupported ();
        }
    }
  return m_shortGuardInterval;
}

void
WifiPhy::SetGuardInterval (Time guardInterval)
{
  NS_LOG_FUNCTION (this << guardInterval);
  NS_ASSERT (guardInterval == NanoSeconds (800) || guardInterval == NanoSeconds (1600) || guardInterval == NanoSeconds (3200));
  Ptr<WifiNetDevice> device = DynamicCast<WifiNetDevice> (GetDevice ());
  if (device)
    {
      Ptr<HeConfiguration> heConfiguration = device->GetHeConfiguration ();
      if (heConfiguration)
        {
          heConfiguration->SetGuardInterval (guardInterval);
        }
    }
  m_guardInterval = guardInterval;
}

Time
WifiPhy::GetGuardInterval (void) const
{
  Ptr<WifiNetDevice> device = DynamicCast<WifiNetDevice> (GetDevice ());
  if (device)
    {
      Ptr<HeConfiguration> heConfiguration = device->GetHeConfiguration ();
      if (heConfiguration)
        {
          return heConfiguration->GetGuardInterval ();
        }
    }
  return m_guardInterval;
}

void
WifiPhy::SetShortPlcpPreambleSupported (bool enable)
{
  NS_LOG_FUNCTION (this << enable);
  m_shortPreamble = enable;
}

bool
WifiPhy::GetShortPlcpPreambleSupported (void) const
{
  return m_shortPreamble;
}

void
WifiPhy::SetDevice (const Ptr<NetDevice> device)
{
  m_device = device;
  //TODO: to be removed once deprecated API is cleaned up
  Ptr<HtConfiguration> htConfiguration = DynamicCast<WifiNetDevice> (device)->GetHtConfiguration ();
  if (htConfiguration)
    {
      htConfiguration->SetShortGuardIntervalSupported (m_shortGuardInterval);
      htConfiguration->SetGreenfieldSupported (m_greenfield);
    }
  Ptr<HeConfiguration> heConfiguration = DynamicCast<WifiNetDevice> (device)->GetHeConfiguration ();
  if (heConfiguration)
    {
      heConfiguration->SetGuardInterval (m_guardInterval);
    }
}

Ptr<NetDevice>
WifiPhy::GetDevice (void) const
{
  return m_device;
}

void
WifiPhy::SetMobility (const Ptr<MobilityModel> mobility)
{
  m_mobility = mobility;
}

Ptr<MobilityModel>
WifiPhy::GetMobility (void) const
{
  if (m_mobility != 0)
    {
      return m_mobility;
    }
  else
    {
      return m_device->GetNode ()->GetObject<MobilityModel> ();
    }
}

void
WifiPhy::SetErrorRateModel (const Ptr<ErrorRateModel> rate)
{
  m_interference.SetErrorRateModel (rate);
  m_interference.SetNumberOfReceiveAntennas (GetNumberOfAntennas ());
}

void
WifiPhy::SetPostReceptionErrorModel (const Ptr<ErrorModel> em)
{
  NS_LOG_FUNCTION (this << em);
  m_postReceptionErrorModel = em;
}

void
WifiPhy::SetFrameCaptureModel (const Ptr<FrameCaptureModel> model)
{
  m_frameCaptureModel = model;
}

void
WifiPhy::SetPreambleDetectionModel (const Ptr<PreambleDetectionModel> model)
{
  m_preambleDetectionModel = model;
}

void
WifiPhy::SetWifiRadioEnergyModel (const Ptr<WifiRadioEnergyModel> wifiRadioEnergyModel)
{
  m_wifiRadioEnergyModel = wifiRadioEnergyModel;
}

void
WifiPhy::SetChannelBondingManager (const Ptr<ChannelBondingManager> manager)
{
  m_channelBondingManager = manager;
  m_channelBondingManager->SetPhy (this);
}

void
WifiPhy::SetPifs (Time pifs)
{
  m_pifs = pifs;
}

Time
WifiPhy::GetPifs (void) const
{
  return m_pifs;
}

double
WifiPhy::GetPowerDbm (uint8_t power) const
{
  NS_ASSERT (m_txPowerBaseDbm <= m_txPowerEndDbm);
  NS_ASSERT (m_nTxPower > 0);
  double dbm;
  if (m_nTxPower > 1)
    {
      dbm = m_txPowerBaseDbm + power * (m_txPowerEndDbm - m_txPowerBaseDbm) / (m_nTxPower - 1);
    }
  else
    {
      NS_ASSERT_MSG (m_txPowerBaseDbm == m_txPowerEndDbm, "cannot have TxPowerEnd != TxPowerStart with TxPowerLevels == 1");
      dbm = m_txPowerBaseDbm;
    }
  return dbm;
}

Time
WifiPhy::GetChannelSwitchDelay (void) const
{
  return m_channelSwitchDelay;
}

double
WifiPhy::CalculateSnr (WifiTxVector txVector, double ber) const
{
  return m_interference.GetErrorRateModel ()->CalculateSnr (txVector, ber);
}

void
WifiPhy::ConfigureDefaultsForStandard (WifiPhyStandard standard)
{
  NS_LOG_FUNCTION (this << standard);
  switch (standard)
    {
    case WIFI_PHY_STANDARD_80211a:
      SetChannelWidth (20);
      SetFrequency (5180);
      // Channel number should be aligned by SetFrequency () to 36
      NS_ASSERT (GetChannelNumber () == 36);
      break;
    case WIFI_PHY_STANDARD_80211b:
      SetChannelWidth (22);
      SetFrequency (2412);
      // Channel number should be aligned by SetFrequency () to 1
      NS_ASSERT (GetChannelNumber () == 1);
      SetPrimaryChannelNumber (1); //TODO: remove once primary channels is updated when channel number has changed
      break;
    case WIFI_PHY_STANDARD_80211g:
      SetChannelWidth (20);
      SetFrequency (2412);
      // Channel number should be aligned by SetFrequency () to 1
      NS_ASSERT (GetChannelNumber () == 1);
      SetPrimaryChannelNumber (1); //TODO: remove once primary channels is updated when channel number has changed
      break;
    case WIFI_PHY_STANDARD_80211_10MHZ:
      SetChannelWidth (10);
      SetFrequency (5860);
      // Channel number should be aligned by SetFrequency () to 172
      NS_ASSERT (GetChannelNumber () == 172);
      SetPrimaryChannelNumber (172); //TODO: remove once primary channels is updated when channel number has changed
      break;
    case WIFI_PHY_STANDARD_80211_5MHZ:
      SetChannelWidth (5);
      SetFrequency (5860);
      // Channel number should be aligned by SetFrequency () to 0
      NS_ASSERT (GetChannelNumber () == 0);
      SetPrimaryChannelNumber (0); //TODO: remove once primary channels is updated when channel number has changed
      break;
    case WIFI_PHY_STANDARD_holland:
      SetChannelWidth (20);
      SetFrequency (5180);
      // Channel number should be aligned by SetFrequency () to 36
      NS_ASSERT (GetChannelNumber () == 36);
      break;
    case WIFI_PHY_STANDARD_80211n_2_4GHZ:
      SetChannelWidth (20);
      SetFrequency (2412);
      // Channel number should be aligned by SetFrequency () to 1
      NS_ASSERT (GetChannelNumber () == 1);
      SetPrimaryChannelNumber (1); //TODO: remove once primary channels is updated when channel number has changed
      break;
    case WIFI_PHY_STANDARD_80211n_5GHZ:
      SetChannelWidth (20);
      SetFrequency (5180);
      // Channel number should be aligned by SetFrequency () to 36
      NS_ASSERT (GetChannelNumber () == 36);
      break;
    case WIFI_PHY_STANDARD_80211ac:
      SetChannelWidth (80);
      SetFrequency (5210);
      // Channel number should be aligned by SetFrequency () to 42
      NS_ASSERT (GetChannelNumber () == 42);
      break;
    case WIFI_PHY_STANDARD_80211ax_2_4GHZ:
      SetChannelWidth (20);
      SetFrequency (2412);
      // Channel number should be aligned by SetFrequency () to 1
      NS_ASSERT (GetChannelNumber () == 1);
      SetPrimaryChannelNumber (1); //TODO: remove once primary channels is updated when channel number has changed
      break;
    case WIFI_PHY_STANDARD_80211ax_5GHZ:
      SetChannelWidth (80);
      SetFrequency (5210);
      // Channel number should be aligned by SetFrequency () to 42
      NS_ASSERT (GetChannelNumber () == 42);
      break;
    case WIFI_PHY_STANDARD_UNSPECIFIED:
    default:
      NS_LOG_WARN ("Configuring unspecified standard; performing no action");
      break;
    }
}

void
WifiPhy::Configure80211a (void)
{
  NS_LOG_FUNCTION (this);

  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate6Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate9Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate12Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate18Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate24Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate36Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate48Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate54Mbps ());
}

void
WifiPhy::Configure80211b (void)
{
  NS_LOG_FUNCTION (this);

  m_deviceRateSet.push_back (WifiPhy::GetDsssRate1Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetDsssRate2Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetDsssRate5_5Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetDsssRate11Mbps ());
}

void
WifiPhy::Configure80211g (void)
{
  NS_LOG_FUNCTION (this);
  Configure80211b ();

  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate6Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate9Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate12Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate18Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate24Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate36Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate48Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetErpOfdmRate54Mbps ());
}

void
WifiPhy::Configure80211_10Mhz (void)
{
  NS_LOG_FUNCTION (this);

  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate3MbpsBW10MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate4_5MbpsBW10MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate6MbpsBW10MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate9MbpsBW10MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate12MbpsBW10MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate18MbpsBW10MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate24MbpsBW10MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate27MbpsBW10MHz ());
}

void
WifiPhy::Configure80211_5Mhz (void)
{
  NS_LOG_FUNCTION (this);

  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate1_5MbpsBW5MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate2_25MbpsBW5MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate3MbpsBW5MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate4_5MbpsBW5MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate6MbpsBW5MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate9MbpsBW5MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate12MbpsBW5MHz ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate13_5MbpsBW5MHz ());
}

void
WifiPhy::ConfigureHolland (void)
{
  NS_LOG_FUNCTION (this);

  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate6Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate12Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate18Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate36Mbps ());
  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate54Mbps ());
}

void
WifiPhy::PushMcs (WifiMode mode)
{
  NS_LOG_FUNCTION (this << mode);

  WifiModulationClass modulation = mode.GetModulationClass ();
  NS_ASSERT (modulation == WIFI_MOD_CLASS_HT || modulation == WIFI_MOD_CLASS_VHT
             || modulation == WIFI_MOD_CLASS_HE);

  m_mcsIndexMap[modulation][mode.GetMcsValue ()] = m_deviceMcsSet.size ();
  m_deviceMcsSet.push_back (mode);
}

void
WifiPhy::RebuildMcsMap (void)
{
  NS_LOG_FUNCTION (this);
  m_mcsIndexMap.clear ();
  uint8_t index = 0;
  for (auto& mode : m_deviceMcsSet)
    {
      m_mcsIndexMap[mode.GetModulationClass ()][mode.GetMcsValue ()] = index++;
    }
}

void
WifiPhy::ConfigureHtDeviceMcsSet (void)
{
  NS_LOG_FUNCTION (this);

  bool htFound = false;
  for (std::vector<uint8_t>::size_type i = 0; i < m_bssMembershipSelectorSet.size (); i++)
    {
      if (m_bssMembershipSelectorSet[i] == HT_PHY)
        {
          htFound = true;
          break;
        }
    }
  if (htFound)
    {
      // erase all HtMcs modes from deviceMcsSet
      std::size_t index = m_deviceMcsSet.size () - 1;
      for (std::vector<WifiMode>::reverse_iterator rit = m_deviceMcsSet.rbegin (); rit != m_deviceMcsSet.rend (); ++rit, --index)
        {
          if (m_deviceMcsSet[index].GetModulationClass () == WIFI_MOD_CLASS_HT)
            {
              m_deviceMcsSet.erase (m_deviceMcsSet.begin () + index);
            }
        }
      RebuildMcsMap ();
      PushMcs (WifiPhy::GetHtMcs0 ());
      PushMcs (WifiPhy::GetHtMcs1 ());
      PushMcs (WifiPhy::GetHtMcs2 ());
      PushMcs (WifiPhy::GetHtMcs3 ());
      PushMcs (WifiPhy::GetHtMcs4 ());
      PushMcs (WifiPhy::GetHtMcs5 ());
      PushMcs (WifiPhy::GetHtMcs6 ());
      PushMcs (WifiPhy::GetHtMcs7 ());
      if (GetMaxSupportedTxSpatialStreams () > 1)
        {
          PushMcs (WifiPhy::GetHtMcs8 ());
          PushMcs (WifiPhy::GetHtMcs9 ());
          PushMcs (WifiPhy::GetHtMcs10 ());
          PushMcs (WifiPhy::GetHtMcs11 ());
          PushMcs (WifiPhy::GetHtMcs12 ());
          PushMcs (WifiPhy::GetHtMcs13 ());
          PushMcs (WifiPhy::GetHtMcs14 ());
          PushMcs (WifiPhy::GetHtMcs15 ());
        }
      if (GetMaxSupportedTxSpatialStreams () > 2)
        {
          PushMcs (WifiPhy::GetHtMcs16 ());
          PushMcs (WifiPhy::GetHtMcs17 ());
          PushMcs (WifiPhy::GetHtMcs18 ());
          PushMcs (WifiPhy::GetHtMcs19 ());
          PushMcs (WifiPhy::GetHtMcs20 ());
          PushMcs (WifiPhy::GetHtMcs21 ());
          PushMcs (WifiPhy::GetHtMcs22 ());
          PushMcs (WifiPhy::GetHtMcs23 ());
        }
      if (GetMaxSupportedTxSpatialStreams () > 3)
        {
          PushMcs (WifiPhy::GetHtMcs24 ());
          PushMcs (WifiPhy::GetHtMcs25 ());
          PushMcs (WifiPhy::GetHtMcs26 ());
          PushMcs (WifiPhy::GetHtMcs27 ());
          PushMcs (WifiPhy::GetHtMcs28 ());
          PushMcs (WifiPhy::GetHtMcs29 ());
          PushMcs (WifiPhy::GetHtMcs30 ());
          PushMcs (WifiPhy::GetHtMcs31 ());
        }
    }
}

void
WifiPhy::Configure80211n (void)
{
  NS_LOG_FUNCTION (this);
  if (Is2_4Ghz (GetFrequency ()))
    {
      Configure80211b ();
      Configure80211g ();
    }
  if (Is5Ghz (GetFrequency ()))
    {
      Configure80211a ();
    }
  m_bssMembershipSelectorSet.push_back (HT_PHY);
  ConfigureHtDeviceMcsSet ();
}

void
WifiPhy::Configure80211ac (void)
{
  NS_LOG_FUNCTION (this);
  Configure80211n ();

  PushMcs (WifiPhy::GetVhtMcs0 ());
  PushMcs (WifiPhy::GetVhtMcs1 ());
  PushMcs (WifiPhy::GetVhtMcs2 ());
  PushMcs (WifiPhy::GetVhtMcs3 ());
  PushMcs (WifiPhy::GetVhtMcs4 ());
  PushMcs (WifiPhy::GetVhtMcs5 ());
  PushMcs (WifiPhy::GetVhtMcs6 ());
  PushMcs (WifiPhy::GetVhtMcs7 ());
  PushMcs (WifiPhy::GetVhtMcs8 ());
  PushMcs (WifiPhy::GetVhtMcs9 ());

  m_bssMembershipSelectorSet.push_back (VHT_PHY);
}

void
WifiPhy::Configure80211ax (void)
{
  NS_LOG_FUNCTION (this);
  if (Is5Ghz (GetFrequency ()))
    {
      Configure80211ac ();
    }
  else
    {
      Configure80211n ();
    }

  PushMcs (WifiPhy::GetHeMcs0 ());
  PushMcs (WifiPhy::GetHeMcs1 ());
  PushMcs (WifiPhy::GetHeMcs2 ());
  PushMcs (WifiPhy::GetHeMcs3 ());
  PushMcs (WifiPhy::GetHeMcs4 ());
  PushMcs (WifiPhy::GetHeMcs5 ());
  PushMcs (WifiPhy::GetHeMcs6 ());
  PushMcs (WifiPhy::GetHeMcs7 ());
  PushMcs (WifiPhy::GetHeMcs8 ());
  PushMcs (WifiPhy::GetHeMcs9 ());
  PushMcs (WifiPhy::GetHeMcs10 ());
  PushMcs (WifiPhy::GetHeMcs11 ());

  m_bssMembershipSelectorSet.push_back (HE_PHY);
}

bool
WifiPhy::DefineChannelNumber (uint8_t channelNumber, WifiPhyStandard standard, uint16_t frequency, uint16_t channelWidth)
{
  NS_LOG_FUNCTION (this << +channelNumber << standard << frequency << channelWidth);
  ChannelNumberStandardPair p = std::make_pair (channelNumber, standard);
  ChannelToFrequencyWidthMap::const_iterator it;
  it = m_channelToFrequencyWidth.find (p);
  if (it != m_channelToFrequencyWidth.end ())
    {
      NS_LOG_DEBUG ("channel number/standard already defined; returning false");
      return false;
    }
  FrequencyWidthPair f = std::make_pair (frequency, channelWidth);
  m_channelToFrequencyWidth[p] = f;
  return true;
}

uint8_t
WifiPhy::FindChannelNumberForFrequencyWidth (uint16_t frequency, uint16_t width) const
{
  NS_LOG_FUNCTION (this << frequency << width);
  bool found = false;
  FrequencyWidthPair f = std::make_pair (frequency, width);
  ChannelToFrequencyWidthMap::const_iterator it = m_channelToFrequencyWidth.begin ();
  while (it != m_channelToFrequencyWidth.end ())
    {
      if (it->second == f)
        {
          found = true;
          break;
        }
      ++it;
    }
  if (found)
    {
      NS_LOG_DEBUG ("Found, returning " << +it->first.first);
      return (it->first.first);
    }
  else
    {
      NS_LOG_DEBUG ("Not found, returning 0");
      return 0;
    }
}

void
WifiPhy::ConfigureChannelForStandard (WifiPhyStandard standard)
{
  NS_LOG_FUNCTION (this << standard);
  // If the user has configured both Frequency and ChannelNumber, Frequency
  // takes precedence
  if (GetFrequency () != 0)
    {
      // If Frequency is already set, then see whether a ChannelNumber can
      // be found that matches Frequency and ChannelWidth. If so, configure
      // the ChannelNumber to that channel number. If not, set ChannelNumber to zero.
      NS_LOG_DEBUG ("Frequency set; checking whether a channel number corresponds");
      uint8_t channelNumberSearched = FindChannelNumberForFrequencyWidth (GetFrequency (), GetChannelWidth ());
      if (channelNumberSearched)
        {
          NS_LOG_DEBUG ("Channel number found; setting to " << +channelNumberSearched);
          SetChannelNumber (channelNumberSearched);
        }
      else
        {
          NS_LOG_DEBUG ("Channel number not found; setting to zero");
          SetChannelNumber (0);
        }
    }
  else if (GetChannelNumber () != 0)
    {
      // If the channel number is known for this particular standard or for
      // the unspecified standard, configure using the known values;
      // otherwise, this is a configuration error
      NS_LOG_DEBUG ("Configuring for channel number " << +GetChannelNumber ());
      FrequencyWidthPair f = GetFrequencyWidthForChannelNumberStandard (GetChannelNumber (), standard);
      if (f.first == 0)
        {
          // the specific pair of number/standard is not known
          NS_LOG_DEBUG ("Falling back to check WIFI_PHY_STANDARD_UNSPECIFIED");
          f = GetFrequencyWidthForChannelNumberStandard (GetChannelNumber (), WIFI_PHY_STANDARD_UNSPECIFIED);
        }
      if (f.first == 0)
        {
          NS_FATAL_ERROR ("Error, ChannelNumber " << +GetChannelNumber () << " is unknown for this standard");
        }
      else
        {
          NS_LOG_DEBUG ("Setting frequency to " << f.first << "; width to " << +f.second);
          SetFrequency (f.first);
          SetChannelWidth (f.second);
        }
    }
}

void
WifiPhy::ConfigureStandard (WifiPhyStandard standard)
{
  NS_LOG_FUNCTION (this << standard);
  m_standard = standard;
  m_isConstructed = true;
  if (m_frequencyChannelNumberInitialized == false)
    {
      InitializeFrequencyChannelNumber ();
    }
  if (GetFrequency () == 0 && GetChannelNumber () == 0)
    {
      ConfigureDefaultsForStandard (standard);
    }
  else
    {
      // The user has configured either (or both) Frequency or ChannelNumber
      ConfigureChannelForStandard (standard);
    }
  switch (standard)
    {
    case WIFI_PHY_STANDARD_80211a:
      Configure80211a ();
      break;
    case WIFI_PHY_STANDARD_80211b:
      Configure80211b ();
      break;
    case WIFI_PHY_STANDARD_80211g:
      Configure80211g ();
      break;
    case WIFI_PHY_STANDARD_80211_10MHZ:
      Configure80211_10Mhz ();
      break;
    case WIFI_PHY_STANDARD_80211_5MHZ:
      Configure80211_5Mhz ();
      break;
    case WIFI_PHY_STANDARD_holland:
      ConfigureHolland ();
      break;
    case WIFI_PHY_STANDARD_80211n_2_4GHZ:
    case WIFI_PHY_STANDARD_80211n_5GHZ:
      Configure80211n ();
      AddCcaEdThresholdSecondary (-62.0);
      break;
    case WIFI_PHY_STANDARD_80211ac:
      Configure80211ac ();
      AddCcaEdThresholdSecondary (-72.0);
      break;
    case WIFI_PHY_STANDARD_80211ax_2_4GHZ:
    case WIFI_PHY_STANDARD_80211ax_5GHZ:
      Configure80211ax ();
      AddCcaEdThresholdSecondary (-72.0);
      break;
    case WIFI_PHY_STANDARD_UNSPECIFIED:
    default:
      NS_ASSERT (false);
      break;
    }
}

WifiPhyStandard
WifiPhy::GetStandard (void) const
{
  return m_standard;
}

void
WifiPhy::SetFrequency (uint16_t frequency)
{
  NS_LOG_FUNCTION (this << frequency);
  if (m_isConstructed == false)
    {
      NS_LOG_DEBUG ("Saving frequency configuration for initialization");
      m_initialFrequency = frequency;
      return;
    }
  if (GetFrequency () == frequency)
    {
      NS_LOG_DEBUG ("No frequency change requested");
      return;
    }
  if (frequency == 0)
    {
      DoFrequencySwitch (0);
      NS_LOG_DEBUG ("Setting frequency and channel number to zero");
      m_channelCenterFrequency = 0;
      m_channelNumber = 0;
      return;
    }
  // If the user has configured both Frequency and ChannelNumber, Frequency
  // takes precedence.  Lookup the channel number corresponding to the
  // requested frequency.
  uint8_t nch = FindChannelNumberForFrequencyWidth (frequency, GetChannelWidth ());
  if (nch != 0)
    {
      NS_LOG_DEBUG ("Setting frequency " << frequency << " corresponds to channel " << +nch);
      if (DoFrequencySwitch (frequency))
        {
          NS_LOG_DEBUG ("Channel frequency switched to " << frequency << "; channel number to " << +nch);
          m_channelCenterFrequency = frequency;
          m_channelNumber = nch;
        }
      else
        {
          NS_LOG_DEBUG ("Suppressing reassignment of frequency");
        }
    }
  else
    {
      NS_LOG_DEBUG ("Channel number is unknown for frequency " << frequency);
      if (DoFrequencySwitch (frequency))
        {
          NS_LOG_DEBUG ("Channel frequency switched to " << frequency << "; channel number to " << 0);
          m_channelCenterFrequency = frequency;
          m_channelNumber = 0;
        }
      else
        {
          NS_LOG_DEBUG ("Suppressing reassignment of frequency");
        }
    }
  //FIXME: in case the user changes the frequency, the primary channel should be updated accordingly
}

uint16_t
WifiPhy::GetFrequency (void) const
{
  return m_channelCenterFrequency;
}

void
WifiPhy::SetChannelWidth (uint16_t channelwidth)
{
  NS_LOG_FUNCTION (this << channelwidth);
  NS_ASSERT_MSG (channelwidth == 5 || channelwidth == 10 || channelwidth == 20 || channelwidth == 22 || channelwidth == 40 || channelwidth == 80 || channelwidth == 160, "wrong channel width value");
  bool changed = (m_channelWidth == channelwidth);
  m_channelWidth = channelwidth;
  AddSupportedChannelWidth (channelwidth);
  if (changed && !m_capabilitiesChangedCallback.IsNull ())
    {
      m_capabilitiesChangedCallback ();
    }
  //FIXME: Update frequency and channel number accordingly
}

uint16_t
WifiPhy::GetChannelWidth (void) const
{
  return m_channelWidth;
}

uint16_t
WifiPhy::GetUsableChannelWidth (WifiMode mode)
{
  if (GetChannelWidth () >= 40)
    {
      NS_ASSERT_MSG (m_channelBondingManager, "Channel bonding can only be used if a channel bonding manager has been set!");
      return m_channelBondingManager->GetUsableChannelWidth (mode);
    }
  return GetChannelWidth ();
}

void
WifiPhy::SetNumberOfAntennas (uint8_t antennas)
{
  NS_ASSERT_MSG (antennas > 0 && antennas <= 4, "unsupported number of antennas");
  m_numberOfAntennas = antennas;
  m_interference.SetNumberOfReceiveAntennas (antennas);
}

uint8_t
WifiPhy::GetNumberOfAntennas (void) const
{
  return m_numberOfAntennas;
}

void
WifiPhy::SetMaxSupportedTxSpatialStreams (uint8_t streams)
{
  NS_ASSERT (streams <= GetNumberOfAntennas ());
  bool changed = (m_txSpatialStreams == streams);
  m_txSpatialStreams = streams;
  ConfigureHtDeviceMcsSet ();
  if (changed && !m_capabilitiesChangedCallback.IsNull ())
    {
      m_capabilitiesChangedCallback ();
    }
}

uint8_t
WifiPhy::GetMaxSupportedTxSpatialStreams (void) const
{
  return m_txSpatialStreams;
}

void
WifiPhy::SetMaxSupportedRxSpatialStreams (uint8_t streams)
{
  NS_ASSERT (streams <= GetNumberOfAntennas ());
  bool changed = (m_rxSpatialStreams == streams);
  m_rxSpatialStreams = streams;
  if (changed && !m_capabilitiesChangedCallback.IsNull ())
    {
      m_capabilitiesChangedCallback ();
    }
}

uint8_t
WifiPhy::GetMaxSupportedRxSpatialStreams (void) const
{
  return m_rxSpatialStreams;
}

uint8_t
WifiPhy::GetNBssMembershipSelectors (void) const
{
  return static_cast<uint8_t> (m_bssMembershipSelectorSet.size ());
}

uint8_t
WifiPhy::GetBssMembershipSelector (uint8_t selector) const
{
  return m_bssMembershipSelectorSet[selector];
}

void
WifiPhy::AddSupportedChannelWidth (uint16_t width)
{
  NS_LOG_FUNCTION (this << width);
  for (std::vector<uint32_t>::size_type i = 0; i != m_supportedChannelWidthSet.size (); i++)
    {
      if (m_supportedChannelWidthSet[i] == width)
        {
          return;
        }
    }
  NS_LOG_FUNCTION ("Adding " << width << " to supported channel width set");
  m_supportedChannelWidthSet.push_back (width);
}

std::vector<uint16_t>
WifiPhy::GetSupportedChannelWidthSet (void) const
{
  return m_supportedChannelWidthSet;
}

WifiPhy::FrequencyWidthPair
WifiPhy::GetFrequencyWidthForChannelNumberStandard (uint8_t channelNumber, WifiPhyStandard standard) const
{
  ChannelNumberStandardPair p = std::make_pair (channelNumber, standard);
  FrequencyWidthPair f = m_channelToFrequencyWidth[p];
  return f;
}

void
WifiPhy::SetChannelNumber (uint8_t nch)
{
  NS_LOG_FUNCTION (this << +nch);
  if (m_isConstructed == false)
    {
      NS_LOG_DEBUG ("Saving channel number configuration for initialization");
      m_initialChannelNumber = nch;
      return;
    }
  if (GetChannelNumber () == nch)
    {
      NS_LOG_DEBUG ("No channel change requested");
      return;
    }
  if (nch == 0)
    {
      // This case corresponds to when there is not a known channel
      // number for the requested frequency.  There is no need to call
      // DoChannelSwitch () because DoFrequencySwitch () should have been
      // called by the client
      NS_LOG_DEBUG ("Setting channel number to zero");
      m_channelNumber = 0;
      return;
    }

  // First make sure that the channel number is defined for the standard
  // in use
  FrequencyWidthPair f = GetFrequencyWidthForChannelNumberStandard (nch, GetStandard ());
  if (f.first == 0)
    {
      f = GetFrequencyWidthForChannelNumberStandard (nch, WIFI_PHY_STANDARD_UNSPECIFIED);
    }
  if (f.first != 0)
    {
      if (DoChannelSwitch (nch))
        {
          NS_LOG_DEBUG ("Setting frequency to " << f.first << "; width to " << +f.second);
          m_channelCenterFrequency = f.first;
          SetChannelWidth (f.second);
          m_channelNumber = nch;
        }
      else
        {
          // Subclass may have suppressed (e.g. waiting for state change)
          NS_LOG_DEBUG ("Channel switch suppressed");
        }
    }
  else
    {
      NS_FATAL_ERROR ("Frequency not found for channel number " << +nch);
    }
  //FIXME: in case the user changes the channel, the primary channel should be updated accordingly
}

uint8_t
WifiPhy::GetChannelNumber (void) const
{
  return m_channelNumber;
}

void
WifiPhy::SetPrimaryChannelNumber (uint8_t nch)
{
  NS_LOG_FUNCTION (this << +nch);
  if (GetChannelWidth () < 20)
    {
      m_primaryChannelNumber = GetChannelNumber ();
      return;
    }
  auto it = m_channelToFrequencyWidth.find (std::make_pair (nch, WIFI_PHY_STANDARD_UNSPECIFIED));
  if ((it == m_channelToFrequencyWidth.end ()) || (it->second.second >= 40))
    {
      NS_FATAL_ERROR ("Invalid primary 20 MHz channel: " << +nch);
    }
  m_primaryChannelNumber = nch;
}

uint8_t
WifiPhy::GetPrimaryChannelNumber (void) const
{
  return m_primaryChannelNumber;
}

uint16_t
WifiPhy::GetCenterFrequencyForChannelWidth (uint16_t currentWidth) const
{
  NS_LOG_FUNCTION (this << currentWidth);
  uint16_t centerFrequencyForSupportedWidth = GetFrequency ();
  uint16_t supportedWidth = GetChannelWidth ();
  if (currentWidth != supportedWidth)
    {
      uint8_t index = GetPrimaryBandIndex (currentWidth);
      centerFrequencyForSupportedWidth = GetCenterFrequency (GetFrequency (), supportedWidth, currentWidth, index);
    }
  return centerFrequencyForSupportedWidth;
}

bool
WifiPhy::DoChannelSwitch (uint8_t nch)
{
  m_powerRestricted = false;
  m_channelAccessRequested = false;
  m_currentEvent = 0;
  m_currentPreambleEvents.clear ();
  if (!IsInitialized ())
    {
      //this is not channel switch, this is initialization
      NS_LOG_DEBUG ("initialize to channel " << +nch);
      return true;
    }

  NS_ASSERT (!IsStateSwitching ());
  switch (GetPhyState ())
    {
    case WifiPhyState::RX:
      NS_LOG_DEBUG ("drop packet because of channel switching while reception");
      m_endPlcpRxEvent.Cancel ();
      for (auto & endRxEvent : m_endRxEvents)
        {
          endRxEvent.Cancel ();
        }
      m_endRxEvents.clear ();
      for (auto & endPreambleDetectionEvent : m_endPreambleDetectionEvents)
        {
          endPreambleDetectionEvent.Cancel ();
        }
      m_endPreambleDetectionEvents.clear ();
      goto switchChannel;
      break;
    case WifiPhyState::TX:
      NS_LOG_DEBUG ("channel switching postponed until end of current transmission");
      Simulator::Schedule (GetDelayUntilIdle (), &WifiPhy::SetChannelNumber, this, nch);
      break;
    case WifiPhyState::CCA_BUSY:
    case WifiPhyState::IDLE:
        {
          for (auto & endPreambleDetectionEvent : m_endPreambleDetectionEvents)
            {
              endPreambleDetectionEvent.Cancel ();
            }
          m_endPreambleDetectionEvents.clear ();
          for (auto & endRxEvent : m_endRxEvents)
            {
              endRxEvent.Cancel ();
            }
          m_endRxEvents.clear ();
        }
      goto switchChannel;
      break;
    case WifiPhyState::SLEEP:
      NS_LOG_DEBUG ("channel switching ignored in sleep mode");
      break;
    default:
      NS_ASSERT (false);
      break;
    }

  return false;

switchChannel:

  NS_LOG_DEBUG ("switching channel " << +GetChannelNumber () << " -> " << +nch);
  uint16_t primaryChannelWidth = GetChannelWidth () >= 40 ? 20 : GetChannelWidth ();
  auto primaryBand = GetBand (primaryChannelWidth, GetPrimaryBandIndex (primaryChannelWidth));
  m_state->SwitchToChannelSwitching (GetChannelSwitchDelay (), primaryBand, GetCcaEdThreshold ());
  m_interference.EraseEvents ();
  /*
   * Needed here to be able to correctly sensed the medium for the first
   * time after the switching. The actual switching is not performed until
   * after m_channelSwitchDelay. Packets received during the switching
   * state are added to the event list and are employed later to figure
   * out the state of the medium after the switching.
   */
  return true;
}

bool
WifiPhy::DoFrequencySwitch (uint16_t frequency)
{
  m_powerRestricted = false;
  m_channelAccessRequested = false;
  m_currentEvent = 0;
  m_currentPreambleEvents.clear ();
  if (!IsInitialized ())
    {
      //this is not channel switch, this is initialization
      NS_LOG_DEBUG ("start at frequency " << frequency);
      return true;
    }

  NS_ASSERT (!IsStateSwitching ());
  switch (GetPhyState ())
    {
    case WifiPhyState::RX:
      NS_LOG_DEBUG ("drop packet because of channel/frequency switching while reception");
      m_endPlcpRxEvent.Cancel ();
      for (auto & endRxEvent : m_endRxEvents)
        {
          endRxEvent.Cancel ();
        }
      m_endRxEvents.clear ();
      for (auto & endPreambleDetectionEvent : m_endPreambleDetectionEvents)
        {
          endPreambleDetectionEvent.Cancel ();
        }
      m_endPreambleDetectionEvents.clear ();
      goto switchFrequency;
      break;
    case WifiPhyState::TX:
      NS_LOG_DEBUG ("channel/frequency switching postponed until end of current transmission");
      Simulator::Schedule (GetDelayUntilIdle (), &WifiPhy::SetFrequency, this, frequency);
      break;
    case WifiPhyState::CCA_BUSY:
    case WifiPhyState::IDLE:
      for (auto & endPreambleDetectionEvent : m_endPreambleDetectionEvents)
        {
          endPreambleDetectionEvent.Cancel ();
        }
      m_endPreambleDetectionEvents.clear ();
      for (auto & endRxEvent : m_endRxEvents)
        {
          endRxEvent.Cancel ();
        }
      m_endRxEvents.clear ();
      goto switchFrequency;
      break;
    case WifiPhyState::SLEEP:
      NS_LOG_DEBUG ("frequency switching ignored in sleep mode");
      break;
    default:
      NS_ASSERT (false);
      break;
    }

  return false;

switchFrequency:

  NS_LOG_DEBUG ("switching frequency " << GetFrequency () << " -> " << frequency);
  uint16_t primaryChannelWidth = GetChannelWidth () >= 40 ? 20 : GetChannelWidth ();
  auto primaryBand = GetBand (primaryChannelWidth, GetPrimaryBandIndex (primaryChannelWidth));
  m_state->SwitchToChannelSwitching (GetChannelSwitchDelay (), primaryBand, GetCcaEdThreshold ());
  m_interference.EraseEvents ();
  /*
   * Needed here to be able to correctly sensed the medium for the first
   * time after the switching. The actual switching is not performed until
   * after m_channelSwitchDelay. Packets received during the switching
   * state are added to the event list and are employed later to figure
   * out the state of the medium after the switching.
   */
  return true;
}

void
WifiPhy::SetSleepMode (void)
{
  NS_LOG_FUNCTION (this);
  m_powerRestricted = false;
  m_channelAccessRequested = false;
  switch (GetPhyState ())
    {
    case WifiPhyState::TX:
      NS_LOG_DEBUG ("setting sleep mode postponed until end of current transmission");
      Simulator::Schedule (GetDelayUntilIdle (), &WifiPhy::SetSleepMode, this);
      break;
    case WifiPhyState::RX:
      NS_LOG_DEBUG ("setting sleep mode postponed until end of current reception");
      Simulator::Schedule (GetDelayUntilIdle (), &WifiPhy::SetSleepMode, this);
      break;
    case WifiPhyState::SWITCHING:
      NS_LOG_DEBUG ("setting sleep mode postponed until end of channel switching");
      Simulator::Schedule (GetDelayUntilIdle (), &WifiPhy::SetSleepMode, this);
      break;
    case WifiPhyState::CCA_BUSY:
    case WifiPhyState::IDLE:
    {
      NS_LOG_DEBUG ("setting sleep mode");
      uint16_t primaryChannelWidth = GetChannelWidth () >= 40 ? 20 : GetChannelWidth ();
      auto primaryBand = GetBand (primaryChannelWidth, GetPrimaryBandIndex (primaryChannelWidth));
      m_state->SwitchToSleep (primaryBand, GetCcaEdThreshold ());
      break;
    }
    case WifiPhyState::SLEEP:
      NS_LOG_DEBUG ("already in sleep mode");
      break;
    default:
      NS_ASSERT (false);
      break;
    }
}

void
WifiPhy::SetOffMode (void)
{
  NS_LOG_FUNCTION (this);
  m_powerRestricted = false;
  m_channelAccessRequested = false;
  m_endPlcpRxEvent.Cancel ();
  for (auto & endRxEvent : m_endRxEvents)
    {
      endRxEvent.Cancel ();
    }
  m_endRxEvents.clear ();
  for (auto & endPreambleDetectionEvent : m_endPreambleDetectionEvents)
    {
      endPreambleDetectionEvent.Cancel ();
    }
  m_endPreambleDetectionEvents.clear ();
  uint16_t primaryChannelWidth = GetChannelWidth () >= 40 ? 20 : GetChannelWidth ();
  auto primaryBand = GetBand (primaryChannelWidth, GetPrimaryBandIndex (primaryChannelWidth));
  m_state->SwitchToOff (primaryBand, GetCcaEdThreshold ());
}

void
WifiPhy::ResumeFromSleep (void)
{
  NS_LOG_FUNCTION (this);
  m_currentPreambleEvents.clear ();
  switch (GetPhyState ())
    {
    case WifiPhyState::TX:
    case WifiPhyState::RX:
    case WifiPhyState::IDLE:
    case WifiPhyState::CCA_BUSY:
    case WifiPhyState::SWITCHING:
      {
        NS_LOG_DEBUG ("not in sleep mode, there is nothing to resume");
        break;
      }
    case WifiPhyState::SLEEP:
      {
        NS_LOG_DEBUG ("resuming from sleep mode");
        uint16_t channelWidth = GetChannelWidth ();
        for (uint8_t i = 0; i < std::max (1, channelWidth / 20); i++)
          {
            auto band = GetBand (((channelWidth >= 40) ? 20 : channelWidth), i);
            bool isPrimary = (i == GetPrimaryBandIndex (((channelWidth >= 40) ? 20 : channelWidth)));
            if (isPrimary)
              {
                Time delayUntilCcaEnd = m_interference.GetEnergyDuration (m_ccaEdThresholdW, band);
                m_state->SwitchFromSleep (delayUntilCcaEnd, band, isPrimary, GetCcaEdThreshold ());
              }
            else
              {
                for (auto const& ccaEdThresholdSecondaryW : m_ccaEdThresholdsSecondaryW)
                  {
                    Time delayUntilCcaEnd = m_interference.GetEnergyDuration (ccaEdThresholdSecondaryW, band);
                    m_state->SwitchFromSleep (delayUntilCcaEnd, band, isPrimary, WToDbm (ccaEdThresholdSecondaryW));
                  }
              }
          }
        break;
      }
    default:
      {
        NS_ASSERT (false);
        break;
      }
    }
}

void
WifiPhy::ResumeFromOff (void)
{
  NS_LOG_FUNCTION (this);
  switch (GetPhyState ())
    {
    case WifiPhyState::TX:
    case WifiPhyState::RX:
    case WifiPhyState::IDLE:
    case WifiPhyState::CCA_BUSY:
    case WifiPhyState::SWITCHING:
    case WifiPhyState::SLEEP:
      {
        NS_LOG_DEBUG ("not in off mode, there is nothing to resume");
        break;
      }
    case WifiPhyState::OFF:
      {
        NS_LOG_DEBUG ("resuming from off mode");
        uint16_t channelWidth = GetChannelWidth ();
        for (uint8_t i = 0; i < std::max (1, channelWidth / 20); i++)
          {
            uint16_t primaryChannelWidth = (channelWidth >= 40) ? 20 : channelWidth;
            auto band = GetBand (primaryChannelWidth, i);
            bool isPrimary = (i == GetPrimaryBandIndex (primaryChannelWidth));
            if (isPrimary)
              {
                Time delayUntilCcaEnd = m_interference.GetEnergyDuration (m_ccaEdThresholdW, band);
                m_state->SwitchFromOff (delayUntilCcaEnd, band, isPrimary, GetCcaEdThreshold ());
              }
            else
              {
                for (auto const& ccaEdThresholdSecondaryW : m_ccaEdThresholdsSecondaryW)
                  {
                    Time delayUntilCcaEnd = m_interference.GetEnergyDuration (ccaEdThresholdSecondaryW, band);
                    m_state->SwitchFromOff (delayUntilCcaEnd, band, isPrimary, WToDbm (ccaEdThresholdSecondaryW));
                  }
              }
          }
        break;
      }
    default:
      {
        NS_ASSERT (false);
        break;
      }
    }
}

WifiMode
WifiPhy::GetHtPlcpHeaderMode ()
{
  return WifiPhy::GetHtMcs0 ();
}

WifiMode
WifiPhy::GetVhtPlcpHeaderMode ()
{
  return WifiPhy::GetVhtMcs0 ();
}

WifiMode
WifiPhy::GetHePlcpHeaderMode ()
{
  return WifiPhy::GetHeMcs0 ();
}

WifiMode
WifiPhy::GetHeSigBMode (WifiTxVector txVector)
{
  NS_ABORT_MSG_IF (txVector.GetPreambleType () != WIFI_PREAMBLE_HE_MU, "HE-SIG-B only available for HE MU");
  uint8_t smallestMcs = 5; //maximum MCS for HE-SIG-B
  for (auto & info : txVector.GetHeMuUserInfoMap ())
    {
      smallestMcs = std::min (smallestMcs, info.second.mcs.GetMcsValue ());
    }
  switch (smallestMcs) //GetVhtMcs (mcs) is not static
  {
    case 0:
      return WifiPhy::GetVhtMcs0 ();
    case 1:
      return WifiPhy::GetVhtMcs1 ();
    case 2:
      return WifiPhy::GetVhtMcs2 ();
    case 3:
      return WifiPhy::GetVhtMcs3 ();
    case 4:
      return WifiPhy::GetVhtMcs4 ();
    case 5:
    default:
      return WifiPhy::GetVhtMcs5 ();
  }
}

Time
WifiPhy::GetPreambleDetectionDuration (void)
{
  return MicroSeconds (4);
}

Time
WifiPhy::GetPlcpTrainingSymbolDuration (WifiTxVector txVector)
{
  uint8_t Ndltf, Neltf;
  //We suppose here that STBC = 0.
  //If STBC > 0, we need a different mapping between Nss and Nltf (IEEE 802.11n-2012 standard, page 1682).
  uint8_t nss = txVector.GetNssMax (); //so as to cover also HE MU case (see section 27.3.10.10 of IEEE P802.11ax/D4.0)
  if (nss < 3)
    {
      Ndltf = nss;
    }
  else if (nss < 5)
    {
      Ndltf = 4;
    }
  else if (nss < 7)
    {
      Ndltf = 6;
    }
  else
    {
      Ndltf = 8;
    }

  if (txVector.GetNess () < 3)
    {
      Neltf = txVector.GetNess ();
    }
  else
    {
      Neltf = 4;
    }

  switch (txVector.GetPreambleType ())
    {
    case WIFI_PREAMBLE_HT_MF:
      return MicroSeconds (4 + (4 * Ndltf) + (4 * Neltf));
    case WIFI_PREAMBLE_HT_GF:
      return MicroSeconds ((4 * Ndltf) + (4 * Neltf));
    case WIFI_PREAMBLE_VHT_SU:
    case WIFI_PREAMBLE_VHT_MU:
      return MicroSeconds (4 + (4 * Ndltf));
    case WIFI_PREAMBLE_HE_SU:
    case WIFI_PREAMBLE_HE_MU:
    case WIFI_PREAMBLE_HE_TB:
      return MicroSeconds (4 + (8 * Ndltf));
    default:
      return MicroSeconds (0);
    }
}

Time
WifiPhy::GetPlcpHtSigHeaderDuration (WifiPreamble preamble)
{
  switch (preamble)
    {
    case WIFI_PREAMBLE_HT_MF:
    case WIFI_PREAMBLE_HT_GF:
      //HT-SIG
      return MicroSeconds (8);
    default:
      //no HT-SIG for non HT
      return MicroSeconds (0);
    }
}

Time
WifiPhy::GetPlcpSigA1Duration (WifiPreamble preamble)
{
  switch (preamble)
    {
    case WIFI_PREAMBLE_VHT_SU:
    case WIFI_PREAMBLE_HE_SU:
    case WIFI_PREAMBLE_VHT_MU:
    case WIFI_PREAMBLE_HE_MU:
    case WIFI_PREAMBLE_HE_TB:
      //VHT-SIG-A1 and HE-SIG-A1
      return MicroSeconds (4);
    default:
      // no SIG-A1
      return MicroSeconds (0);
    }
}

Time
WifiPhy::GetPlcpSigA2Duration (WifiPreamble preamble)
{
  switch (preamble)
    {
    case WIFI_PREAMBLE_VHT_SU:
    case WIFI_PREAMBLE_HE_SU:
    case WIFI_PREAMBLE_VHT_MU:
    case WIFI_PREAMBLE_HE_MU:
      //VHT-SIG-A2 and HE-SIG-A2
      return MicroSeconds (4);
    default:
      // no SIG-A2
      return MicroSeconds (0);
    }
}

Time
WifiPhy::GetPlcpSigBDuration (WifiTxVector txVector)
{
  if (txVector.GetPreambleType () == WIFI_PREAMBLE_HE_MU) //See section 27.3.10.8 of IEEE 802.11ax draft 4.0.
    {
      /*
       * Compute the number of bits used by common field.
       * Assume that compression bit in HE-SIG-A is not set (i.e. not
       * full band MU-MIMO); the field is present.
       */
      uint16_t bw = txVector.GetChannelWidth ();
      std::size_t commonFieldSize = 4 /* CRC */ + 6 /* tail */;
      if (bw <= 40)
        {
          commonFieldSize += 8; //only one allocation subfield
        }
      else
        {
          commonFieldSize += 8 * (bw / 40) /* one allocation field per 40 MHz */ + 1 /* center RU */;
        }

      /*
       * Compute the number of bits used by user-specific field.
       * MU-MIMO is not supported; only one station per RU.
       * The user-specific field is composed of N user block fields
       * spread over each corresponding HE-SIG-B content channel.
       * Each user block field contains either two or one users' data
       * (the latter being for odd number of stations per content channel).
       * Padding will be handled further down in the code.
       */
      std::pair<std::size_t, std::size_t> numStaPerContentChannel = txVector.GetNumRusPerHeSigBContentChannel ();
      std::size_t maxNumStaPerContentChannel = std::max (numStaPerContentChannel.first, numStaPerContentChannel.second);
      std::size_t maxNumUserBlockFields = maxNumStaPerContentChannel / 2; //handle last user block with single user, if any, further down
      std::size_t userSpecificFieldSize = maxNumUserBlockFields * (2 * 21 /* user fields (2 users) */ + 4 /* tail */ + 6 /* CRC */);
      if (maxNumStaPerContentChannel % 2 != 0)
        {
          userSpecificFieldSize += 21 /* last user field */ + 4 /* CRC */ + 6 /* tail */;
        }

      /*
       * Compute duration of HE-SIG-B considering that padding
       * is added up to the next OFDM symbol.
       * Nss = 1 and GI = 800 ns for HE-SIG-B.
       */
      Time symbolDuration = MicroSeconds (4);
      double numDataBitsPerSymbol = GetHeSigBMode (txVector).GetDataRate (20, 800, 1) * symbolDuration.GetNanoSeconds () / 1e9;
      double numSymbols = ceil ((commonFieldSize + userSpecificFieldSize) / numDataBitsPerSymbol);

      return FemtoSeconds (static_cast<uint64_t> (numSymbols * symbolDuration.GetFemtoSeconds ()));
    }
  else if (txVector.GetPreambleType () == WIFI_PREAMBLE_VHT_MU)
    {
      return MicroSeconds (4);
    }
  else
    {
      // no SIG-B
      return MicroSeconds (0);
    }
}

WifiMode
WifiPhy::GetPlcpHeaderMode (WifiTxVector txVector)
{
  WifiPreamble preamble = txVector.GetPreambleType ();
  switch (preamble)
    {
    case WIFI_PREAMBLE_LONG:
    case WIFI_PREAMBLE_SHORT:
      {
        switch (txVector.GetMode ().GetModulationClass ())
          {
            case WIFI_MOD_CLASS_OFDM:
              {
                switch (txVector.GetChannelWidth ())
                  {
                    case 5:
                      return WifiPhy::GetOfdmRate1_5MbpsBW5MHz ();
                    case 10:
                      return WifiPhy::GetOfdmRate3MbpsBW10MHz ();
                    case 20:
                    default:
                      //(Section 18.3.2 "PLCP frame format"; IEEE Std 802.11-2012)
                      //actually this is only the first part of the PlcpHeader,
                      //because the last 16 bits of the PlcpHeader are using the
                      //same mode of the payload
                      return WifiPhy::GetOfdmRate6Mbps ();
                  }
              }
            case WIFI_MOD_CLASS_ERP_OFDM:
              return WifiPhy::GetErpOfdmRate6Mbps ();
            case WIFI_MOD_CLASS_DSSS:
            case WIFI_MOD_CLASS_HR_DSSS:
              {
                if (preamble == WIFI_PREAMBLE_LONG || txVector.GetMode () == WifiPhy::GetDsssRate1Mbps ())
                  {
                    //(Section 16.2.3 "PLCP field definitions" and Section 17.2.2.2 "Long PPDU format"; IEEE Std 802.11-2012)
                    return WifiPhy::GetDsssRate1Mbps ();
                  }
                else
                  {
                    //(Section 17.2.2.3 "Short PPDU format"; IEEE Std 802.11-2012)
                    return WifiPhy::GetDsssRate2Mbps ();
                  }
              }
            default:
              NS_FATAL_ERROR ("unsupported modulation class");
              return WifiMode ();
          }
      }
    case WIFI_PREAMBLE_HT_MF:
    case WIFI_PREAMBLE_HT_GF:
    case WIFI_PREAMBLE_VHT_SU:
    case WIFI_PREAMBLE_VHT_MU:
    case WIFI_PREAMBLE_HE_SU:
    case WIFI_PREAMBLE_HE_ER_SU:
    case WIFI_PREAMBLE_HE_MU:
    case WIFI_PREAMBLE_HE_TB:
      return WifiPhy::GetOfdmRate6Mbps ();
    default:
      NS_FATAL_ERROR ("unsupported preamble type");
      return WifiMode ();
    }
}

Time
WifiPhy::GetPlcpHeaderDuration (WifiTxVector txVector)
{
  WifiPreamble preamble = txVector.GetPreambleType ();
  switch (txVector.GetPreambleType ())
    {
    case WIFI_PREAMBLE_LONG:
    case WIFI_PREAMBLE_SHORT:
      {
        switch (txVector.GetMode ().GetModulationClass ())
          {
          case WIFI_MOD_CLASS_OFDM:
            {
              switch (txVector.GetChannelWidth ())
                {
                case 20:
                default:
                  //(Section 18.3.3 "PLCP preamble (SYNC))" and Figure 18-4 "OFDM training structure"; IEEE Std 802.11-2012)
                  //also (Section 18.3.2.4 "Timing related parameters" Table 18-5 "Timing-related parameters"; IEEE Std 802.11-2012)
                  //We return the duration of the SIGNAL field only, since the
                  //SERVICE field (which strictly speaking belongs to the PLCP
                  //header, see Section 18.3.2 and Figure 18-1) is sent using the
                  //payload mode.
                  return MicroSeconds (4);
                case 10:
                  //(Section 18.3.2.4 "Timing related parameters" Table 18-5 "Timing-related parameters"; IEEE Std 802.11-2012)
                  return MicroSeconds (8);
                case 5:
                  //(Section 18.3.2.4 "Timing related parameters" Table 18-5 "Timing-related parameters"; IEEE Std 802.11-2012)
                  return MicroSeconds (16);
                }
            }
          case WIFI_MOD_CLASS_ERP_OFDM:
            return MicroSeconds (4);
          case WIFI_MOD_CLASS_DSSS:
          case WIFI_MOD_CLASS_HR_DSSS:
            {
              if ((preamble == WIFI_PREAMBLE_SHORT) && (txVector.GetMode ().GetDataRate (22) > 1000000))
                {
                  //(Section 17.2.2.3 "Short PPDU format" and Figure 17-2 "Short PPDU format"; IEEE Std 802.11-2012)
                  return MicroSeconds (24);
                }
              else
                {
                  //(Section 17.2.2.2 "Long PPDU format" and Figure 17-1 "Short PPDU format"; IEEE Std 802.11-2012)
                  return MicroSeconds (48);
                }
            }
          default:
            NS_FATAL_ERROR ("modulation class is not matching the preamble type");
            return MicroSeconds (0);
          }
      }
    case WIFI_PREAMBLE_HT_MF:
    case WIFI_PREAMBLE_VHT_SU:
    case WIFI_PREAMBLE_VHT_MU:
      //L-SIG
      return MicroSeconds (4);
    case WIFI_PREAMBLE_HE_SU:
    case WIFI_PREAMBLE_HE_ER_SU:
    case WIFI_PREAMBLE_HE_MU:
    case WIFI_PREAMBLE_HE_TB:
      //LSIG + R-LSIG
      return MicroSeconds (8);
    case WIFI_PREAMBLE_HT_GF:
      return MicroSeconds (0);
    default:
      NS_FATAL_ERROR ("unsupported preamble type");
      return MicroSeconds (0);
    }
}

Time
WifiPhy::GetStartOfPacketDuration (WifiTxVector txVector)
{
  return MicroSeconds (4);
}

Time
WifiPhy::GetPlcpPreambleDuration (WifiTxVector txVector)
{
  WifiPreamble preamble = txVector.GetPreambleType ();
  switch (txVector.GetPreambleType ())
    {
    case WIFI_PREAMBLE_LONG:
    case WIFI_PREAMBLE_SHORT:
      {
        switch (txVector.GetMode ().GetModulationClass ())
          {
            case WIFI_MOD_CLASS_OFDM:
              {
                switch (txVector.GetChannelWidth ())
                  {
                    case 20:
                    default:
                      //(Section 18.3.3 "PLCP preamble (SYNC))" Figure 18-4 "OFDM training structure"
                      //also Section 18.3.2.3 "Modulation-dependent parameters" Table 18-4 "Modulation-dependent parameters"; IEEE Std 802.11-2012)
                      return MicroSeconds (16);
                    case 10:
                      //(Section 18.3.3 "PLCP preamble (SYNC))" Figure 18-4 "OFDM training structure"
                      //also Section 18.3.2.3 "Modulation-dependent parameters" Table 18-4 "Modulation-dependent parameters"; IEEE Std 802.11-2012)
                      return MicroSeconds (32);
                    case 5:
                      //(Section 18.3.3 "PLCP preamble (SYNC))" Figure 18-4 "OFDM training structure"
                      //also Section 18.3.2.3 "Modulation-dependent parameters" Table 18-4 "Modulation-dependent parameters"; IEEE Std 802.11-2012)
                      return MicroSeconds (64);
                  }
              }
            case WIFI_MOD_CLASS_ERP_OFDM:
              return MicroSeconds (16);
            case WIFI_MOD_CLASS_DSSS:
            case WIFI_MOD_CLASS_HR_DSSS:
              {
                if ((preamble == WIFI_PREAMBLE_SHORT) && (txVector.GetMode ().GetDataRate (22) > 1000000))
                  {
                    //(Section 17.2.2.3 "Short PPDU format)" Figure 17-2 "Short PPDU format"; IEEE Std 802.11-2012)
                    return MicroSeconds (72);
                  }
                else
                  {
                    //(Section 17.2.2.2 "Long PPDU format)" Figure 17-1 "Long PPDU format"; IEEE Std 802.11-2012)
                    return MicroSeconds (144);
                  }
              }
            default:
              NS_FATAL_ERROR ("modulation class is not matching the preamble type");
              return MicroSeconds (0);
          }
      }
    case WIFI_PREAMBLE_HT_MF:
    case WIFI_PREAMBLE_VHT_SU:
    case WIFI_PREAMBLE_VHT_MU:
    case WIFI_PREAMBLE_HE_SU:
    case WIFI_PREAMBLE_HE_ER_SU:
    case WIFI_PREAMBLE_HE_MU:
    case WIFI_PREAMBLE_HE_TB:
      //L-STF + L-LTF
      return MicroSeconds (16);
    case WIFI_PREAMBLE_HT_GF:
      //HT-GF-STF + HT-LTF1
      return MicroSeconds (16);
    default:
      NS_FATAL_ERROR ("unsupported preamble type");
      return MicroSeconds (0);
    }
}

Time
WifiPhy::GetPayloadDuration (uint32_t size, WifiTxVector txVector, uint16_t frequency, MpduType mpdutype, uint16_t staId)
{
  uint32_t totalAmpduSize;
  double totalAmpduNumSymbols;
  return GetPayloadDuration (size, txVector, frequency, mpdutype, false, totalAmpduSize, totalAmpduNumSymbols, staId);
}

Time
WifiPhy::GetPayloadDuration (uint32_t size, WifiTxVector txVector, uint16_t frequency, MpduType mpdutype,
                             bool incFlag, uint32_t &totalAmpduSize, double &totalAmpduNumSymbols,
                             uint16_t staId)
{
  WifiMode payloadMode = txVector.GetMode (staId);
  NS_LOG_FUNCTION (size << payloadMode);

  double stbc = 1;
  if (txVector.IsStbc ()
      && (payloadMode.GetModulationClass () == WIFI_MOD_CLASS_HT
          || payloadMode.GetModulationClass () == WIFI_MOD_CLASS_VHT))
    {
      stbc = 2;
    }

  double Nes = 1;
  //todo: improve logic to reduce the number of if cases
  //todo: extend to NSS > 4 for VHT rates
  if (payloadMode == GetHtMcs21 ()
      || payloadMode == GetHtMcs22 ()
      || payloadMode == GetHtMcs23 ()
      || payloadMode == GetHtMcs28 ()
      || payloadMode == GetHtMcs29 ()
      || payloadMode == GetHtMcs30 ()
      || payloadMode == GetHtMcs31 ())
    {
      Nes = 2;
    }
  if (payloadMode.GetModulationClass () == WIFI_MOD_CLASS_VHT)
    {
      if (txVector.GetChannelWidth () == 40
          && txVector.GetNss (staId) == 3
          && payloadMode.GetMcsValue () >= 8)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 80
          && txVector.GetNss (staId) == 2
          && payloadMode.GetMcsValue () >= 7)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 80
          && txVector.GetNss (staId) == 3
          && payloadMode.GetMcsValue () >= 7)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 80
          && txVector.GetNss (staId) == 3
          && payloadMode.GetMcsValue () == 9)
        {
          Nes = 3;
        }
      if (txVector.GetChannelWidth () == 80
          && txVector.GetNss (staId) == 4
          && payloadMode.GetMcsValue () >= 4)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 80
          && txVector.GetNss (staId) == 4
          && payloadMode.GetMcsValue () >= 7)
        {
          Nes = 3;
        }
      if (txVector.GetChannelWidth () == 160
          && payloadMode.GetMcsValue () >= 7)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss (staId) == 2
          && payloadMode.GetMcsValue () >= 4)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss (staId) == 2
          && payloadMode.GetMcsValue () >= 7)
        {
          Nes = 3;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss (staId) == 3
          && payloadMode.GetMcsValue () >= 3)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss (staId) == 3
          && payloadMode.GetMcsValue () >= 5)
        {
          Nes = 3;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss (staId) == 3
          && payloadMode.GetMcsValue () >= 7)
        {
          Nes = 4;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss (staId) == 4
          && payloadMode.GetMcsValue () >= 2)
        {
          Nes = 2;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss (staId) == 4
          && payloadMode.GetMcsValue () >= 4)
        {
          Nes = 3;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss (staId) == 4
          && payloadMode.GetMcsValue () >= 5)
        {
          Nes = 4;
        }
      if (txVector.GetChannelWidth () == 160
          && txVector.GetNss (staId) == 4
          && payloadMode.GetMcsValue () >= 7)
        {
          Nes = 6;
        }
    }

  Time symbolDuration = Seconds (0);
  switch (payloadMode.GetModulationClass ())
    {
    case WIFI_MOD_CLASS_OFDM:
    case WIFI_MOD_CLASS_ERP_OFDM:
      {
        //(Section 18.3.2.4 "Timing related parameters" Table 18-5 "Timing-related parameters"; IEEE Std 802.11-2012
        //corresponds to T_{SYM} in the table)
        switch (txVector.GetChannelWidth ())
          {
          case 20:
          default:
            symbolDuration = MicroSeconds (4);
            break;
          case 10:
            symbolDuration = MicroSeconds (8);
            break;
          case 5:
            symbolDuration = MicroSeconds (16);
            break;
          }
        break;
      }
    case WIFI_MOD_CLASS_HT:
    case WIFI_MOD_CLASS_VHT:
      {
        //if short GI data rate is used then symbol duration is 3.6us else symbol duration is 4us
        //In the future has to create a stationmanager that only uses these data rates if sender and receiver support GI
        uint16_t gi = txVector.GetGuardInterval ();
        NS_ASSERT (gi == 400 || gi == 800);
        symbolDuration = NanoSeconds (3200 + gi);
      }
      break;
    case WIFI_MOD_CLASS_HE:
      {
        //if short GI data rate is used then symbol duration is 3.6us else symbol duration is 4us
        //In the future has to create a stationmanager that only uses these data rates if sender and receiver support GI
        uint16_t gi = txVector.GetGuardInterval ();
        NS_ASSERT (gi == 800 || gi == 1600 || gi == 3200);
        symbolDuration = NanoSeconds (12800 + gi);
      }
      break;
    default:
      break;
    }

  double numDataBitsPerSymbol = payloadMode.GetDataRate (txVector, staId) * symbolDuration.GetNanoSeconds () / 1e9;

  double numSymbols = 0;
  if (mpdutype == FIRST_MPDU_IN_AGGREGATE)
    {
      //First packet in an A-MPDU
      numSymbols = (stbc * (16 + size * 8.0 + 6 * Nes) / (stbc * numDataBitsPerSymbol));
      if (incFlag == 1)
        {
          totalAmpduSize += size;
          totalAmpduNumSymbols += numSymbols;
        }
    }
  else if (mpdutype == MIDDLE_MPDU_IN_AGGREGATE)
    {
      //consecutive packets in an A-MPDU
      numSymbols = (stbc * size * 8.0) / (stbc * numDataBitsPerSymbol);
      if (incFlag == 1)
        {
          totalAmpduSize += size;
          totalAmpduNumSymbols += numSymbols;
        }
    }
  else if (mpdutype == LAST_MPDU_IN_AGGREGATE)
    {
      //last packet in an A-MPDU
      uint32_t totalSize = totalAmpduSize + size;
      numSymbols = lrint (stbc * ceil ((16 + totalSize * 8.0 + 6 * Nes) / (stbc * numDataBitsPerSymbol)));
      NS_ASSERT (totalAmpduNumSymbols <= numSymbols);
      numSymbols -= totalAmpduNumSymbols;
      if (incFlag == 1)
        {
          totalAmpduSize = 0;
          totalAmpduNumSymbols = 0;
        }
    }
  else if (mpdutype == NORMAL_MPDU || mpdutype == SINGLE_MPDU)
    {
      //Not an A-MPDU or single MPDU (i.e. the current payload contains both service and padding)
      //The number of OFDM symbols in the data field when BCC encoding
      //is used is given in equation 19-32 of the IEEE 802.11-2016 standard.
      numSymbols = lrint (stbc * ceil ((16 + size * 8.0 + 6.0 * Nes) / (stbc * numDataBitsPerSymbol)));
    }
  else
    {
      NS_FATAL_ERROR ("Unknown MPDU type");
    }

  switch (payloadMode.GetModulationClass ())
    {
    case WIFI_MOD_CLASS_OFDM:
    case WIFI_MOD_CLASS_ERP_OFDM:
      {
        //Add signal extension for ERP PHY
        if (payloadMode.GetModulationClass () == WIFI_MOD_CLASS_ERP_OFDM)
          {
            return FemtoSeconds (static_cast<uint64_t> (numSymbols * symbolDuration.GetFemtoSeconds ())) + MicroSeconds (6);
          }
        else
          {
            return FemtoSeconds (static_cast<uint64_t> (numSymbols * symbolDuration.GetFemtoSeconds ()));
          }
      }
    case WIFI_MOD_CLASS_HT:
    case WIFI_MOD_CLASS_VHT:
      {
        if (payloadMode.GetModulationClass () == WIFI_MOD_CLASS_HT && Is2_4Ghz (frequency)
            && (mpdutype == NORMAL_MPDU || mpdutype == SINGLE_MPDU || mpdutype == LAST_MPDU_IN_AGGREGATE)) //at 2.4 GHz
          {
            return FemtoSeconds (static_cast<uint64_t> (numSymbols * symbolDuration.GetFemtoSeconds ())) + MicroSeconds (6);
          }
        else //at 5 GHz
          {
            return FemtoSeconds (static_cast<uint64_t> (numSymbols * symbolDuration.GetFemtoSeconds ()));
          }
      }
    case WIFI_MOD_CLASS_HE:
      {
        if (Is2_4Ghz (frequency)
            && ((mpdutype == NORMAL_MPDU || mpdutype == SINGLE_MPDU || mpdutype == LAST_MPDU_IN_AGGREGATE))) //at 2.4 GHz
          {
            return FemtoSeconds (static_cast<uint64_t> (numSymbols * symbolDuration.GetFemtoSeconds ())) + MicroSeconds (6);
          }
        else //at 5 GHz
          {
            return FemtoSeconds (static_cast<uint64_t> (numSymbols * symbolDuration.GetFemtoSeconds ()));
          }
      }
    case WIFI_MOD_CLASS_DSSS:
    case WIFI_MOD_CLASS_HR_DSSS:
      return MicroSeconds (lrint (ceil ((size * 8.0) / (payloadMode.GetDataRate (22) / 1.0e6))));
    default:
      NS_FATAL_ERROR ("unsupported modulation class");
      return MicroSeconds (0);
    }
}

Time
WifiPhy::CalculatePlcpPreambleAndHeaderDuration (WifiTxVector txVector)
{
  WifiPreamble preamble = txVector.GetPreambleType ();
  Time duration = GetPlcpPreambleDuration (txVector)
    + GetPlcpHeaderDuration (txVector)
    + GetPlcpHtSigHeaderDuration (preamble)
    + GetPlcpSigA1Duration (preamble)
    + GetPlcpSigA2Duration (preamble)
    + GetPlcpTrainingSymbolDuration (txVector)
    + GetPlcpSigBDuration (txVector);
  return duration;
}

Time
WifiPhy::CalculateTxDuration (uint32_t size, WifiTxVector txVector, uint16_t frequency, uint16_t staId)
{
  Time duration = CalculatePlcpPreambleAndHeaderDuration (txVector)
    + GetPayloadDuration (size, txVector, frequency, NORMAL_MPDU, staId);
  NS_ASSERT (duration.IsStrictlyPositive ());
  return duration;
}

Time
WifiPhy::CalculateTxDuration (WifiPsduMap psduMap, WifiTxVector txVector, uint16_t frequency)
{
  Time maxDuration = Seconds (0);
  for (auto & staIdPsdu : psduMap)
    {
      if (txVector.GetPreambleType () == WIFI_PREAMBLE_HE_MU)
        {
          WifiTxVector::HeMuUserInfoMap userInfoMap = txVector.GetHeMuUserInfoMap ();
          NS_ABORT_MSG_IF (userInfoMap.find (staIdPsdu.first) == userInfoMap.end (), "STA-ID in psduMap (" << staIdPsdu.first << ") should be referenced in txVector");
        }
      Time current = CalculateTxDuration (staIdPsdu.second->GetSize (), txVector, frequency,
                                          staIdPsdu.first);
      if (current > maxDuration)
        {
          maxDuration = current;
        }
    }
  NS_ASSERT (maxDuration.IsStrictlyPositive ());
  return maxDuration;
}

void
WifiPhy::NotifyTxBegin (WifiPsduMap psdus, double txPowerW)
{
  for (auto const& psdu : psdus)
    {
      for (auto& mpdu : *PeekPointer (psdu.second))
        {
          Ptr<Packet> pdu = mpdu->GetProtocolDataUnit ();
          m_phyTxBeginTrace (pdu, txPowerW);
          CopyByteTags (pdu, mpdu->GetPacket ()); //mainly so that netanim's byte tags may be handed over to all receivers
        }
    }
}

void
WifiPhy::NotifyTxEnd (Ptr<const WifiPsdu> psdu)
{
  for (auto& mpdu : *PeekPointer (psdu))
    {
      m_phyTxEndTrace (mpdu->GetProtocolDataUnit ());
    }
}

void
WifiPhy::NotifyTxDrop (Ptr<const WifiPsdu> psdu)
{
  for (auto& mpdu : *PeekPointer (psdu))
    {
      m_phyTxDropTrace (mpdu->GetProtocolDataUnit ());
    }
}

void
WifiPhy::NotifyRxBegin (Ptr<const WifiPsdu> psdu, RxPowerWattPerChannelBand rxPowersW)
{
  if (psdu)
    {
      for (auto& mpdu : *PeekPointer (psdu))
        {
          m_phyRxBeginTrace (mpdu->GetProtocolDataUnit (), rxPowersW);
        }
    }
}

void
WifiPhy::NotifyRxEnd (Ptr<const WifiPsdu> psdu)
{
  if (psdu)
    {
      for (auto& mpdu : *PeekPointer (psdu))
        {
          m_phyRxEndTrace (mpdu->GetProtocolDataUnit ());
        }
    }
}

void
WifiPhy::NotifyRxDrop (Ptr<const WifiPsdu> psdu, WifiPhyRxfailureReason reason)
{
  if (psdu)
    {
      for (auto& mpdu : *PeekPointer (psdu))
        {
          m_phyRxDropTrace (mpdu->GetProtocolDataUnit (), reason);
        }
    }
}

void
WifiPhy::NotifyMonitorSniffRx (Ptr<const WifiPsdu> psdu, uint16_t channelFreqMhz, WifiTxVector txVector,
                               SignalNoiseDbm signalNoise, std::vector<bool> statusPerMpdu)
{
  MpduInfo aMpdu;
  if (psdu->IsAggregate ())
    {
      //Expand A-MPDU
      NS_ABORT_MSG_IF (!txVector.IsAggregation (), "TxVector with aggregate flag expected here according to PSDU");
      aMpdu.mpduRefNumber = ++m_rxMpduReferenceNumber;
      size_t nMpdus = psdu->GetNMpdus ();
      NS_ABORT_MSG_IF (statusPerMpdu.size () != nMpdus, "Should have one reception status per MPDU");
      aMpdu.type = (psdu->IsSingle ()) ? SINGLE_MPDU : FIRST_MPDU_IN_AGGREGATE;
      for (size_t i = 0; i < nMpdus;)
        {
          if (statusPerMpdu.at (i)) //packet received without error, hand over to sniffer
            {
              m_phyMonitorSniffRxTrace (psdu->GetAmpduSubframe (i), channelFreqMhz, txVector, aMpdu, signalNoise);
            }
          ++i;
          aMpdu.type = (i == (nMpdus - 1)) ? LAST_MPDU_IN_AGGREGATE : MIDDLE_MPDU_IN_AGGREGATE;
        }
    }
  else
    {
      aMpdu.type = NORMAL_MPDU;
      NS_ABORT_MSG_IF (statusPerMpdu.size () != 1, "Should have one reception status for normal MPDU");
      m_phyMonitorSniffRxTrace (psdu->GetPacket (), channelFreqMhz, txVector, aMpdu, signalNoise);
    }
}

void
WifiPhy::NotifyMonitorSniffTx (Ptr<const WifiPsdu> psdu, uint16_t channelFreqMhz, WifiTxVector txVector)
{
  MpduInfo aMpdu;
  if (psdu->IsAggregate ())
    {
      //Expand A-MPDU
      NS_ABORT_MSG_IF (!txVector.IsAggregation (), "TxVector with aggregate flag expected here according to PSDU");
      aMpdu.mpduRefNumber = ++m_rxMpduReferenceNumber;
      size_t nMpdus = psdu->GetNMpdus ();
      aMpdu.type = (psdu->IsSingle ()) ? SINGLE_MPDU: FIRST_MPDU_IN_AGGREGATE;
      for (size_t i = 0; i < nMpdus;)
        {
          m_phyMonitorSniffTxTrace (psdu->GetAmpduSubframe (i), channelFreqMhz, txVector, aMpdu);
          ++i;
          aMpdu.type = (i == (nMpdus - 1)) ? LAST_MPDU_IN_AGGREGATE : MIDDLE_MPDU_IN_AGGREGATE;
        }
    }
  else
    {
      aMpdu.type = NORMAL_MPDU;
      m_phyMonitorSniffTxTrace (psdu->GetPacket (), channelFreqMhz, txVector, aMpdu);
    }
}

void
WifiPhy::NotifyEndOfHePreamble (HePreambleParameters params)
{
  m_phyEndOfHePreambleTrace (params);
}

void
WifiPhy::Send (WifiPsduMap psdus, WifiTxVector txVector)
{
  NS_LOG_FUNCTION (this << psdus << txVector);
  /* Transmission can happen if:
   *  - we are syncing on a packet. It is the responsibility of the
   *    MAC layer to avoid doing this but the PHY does nothing to
   *    prevent it.
   *  - we are idle
   */
  NS_ASSERT (!IsStateTx () && !IsStateSwitching ());

  if (txVector.GetNssMax () > GetMaxSupportedTxSpatialStreams ())
    {
      NS_FATAL_ERROR ("Unsupported number of spatial streams!");
    }

  if (IsStateSleep ())
    {
      NS_LOG_DEBUG ("Dropping packet because in sleep mode");
      for (auto const& psdu : psdus)
        {
          NotifyTxDrop (psdu.second);
        }
      return;
    }
  
  Time txDuration;
  if (txVector.GetPreambleType () == WIFI_PREAMBLE_HE_TB)
    {
      NS_ASSERT (txVector.GetLength () > 0);
      txDuration = ConvertLSigLengthToHeTbPpduDuration (txVector.GetLength (), txVector, GetFrequency ());
    }
  else
    {
      txDuration = CalculateTxDuration (psdus, txVector, GetFrequency ());
    }

  if (!m_endPreambleDetectionEvents.empty () || m_currentEvent != 0)
    {
      MaybeCcaBusy (); //needed to keep busy state afterwards
    }

  for (auto & endPreambleDetectionEvent : m_endPreambleDetectionEvents)
    {
      endPreambleDetectionEvent.Cancel ();
    }
  m_endPreambleDetectionEvents.clear ();
  if (m_endPlcpRxEvent.IsRunning ())
    {
      m_endPlcpRxEvent.Cancel ();
    }
  for (auto & endRxEvent : m_endRxEvents)
    {
      endRxEvent.Cancel ();
    }
  m_endRxEvents.clear ();
  if (IsStateRx ())
    {
      m_interference.NotifyRxEnd (Simulator::Now ());
    }
  m_currentEvent = 0;
  m_currentPreambleEvents.clear ();

  if (m_powerRestricted)
    {
      NS_LOG_DEBUG ("Transmitting with power restriction");
    }
  else
    {
      NS_LOG_DEBUG ("Transmitting without power restriction");
    }

  NotifyTxBegin (psdus, DbmToW (GetTxPowerForTransmission (txVector) + GetTxGain ()));
  NotifyMonitorSniffTx (psdus.begin ()->second, GetFrequency (), txVector); //TODO: fix for MU
  uint16_t primaryChannelWidth = GetChannelWidth () >= 40 ? 20 : GetChannelWidth ();
  auto primaryBand = GetBand (primaryChannelWidth, GetPrimaryBandIndex (primaryChannelWidth));
  m_state->SwitchToTx (txDuration, psdus, GetPowerDbm (txVector.GetTxPowerLevel ()), txVector, primaryBand, GetCcaEdThreshold ());

  if (IsStateOff ())
    {
      NS_LOG_DEBUG ("Transmission canceled because device is OFF");
      return;
    }

  uint64_t uid;
  if (txVector.GetPreambleType () == WIFI_PREAMBLE_HE_TB)
    {
      //Use UID of PPDU containing trigger frame to identify resulting HE TB PPDUs, since the latter should immediately follow the former
      NS_ASSERT (m_previouslyRxPpduUid != UINT64_MAX);
      uid = m_previouslyRxPpduUid;
    }
  else
    {
      uid = m_globalPpduUid++;
    }
  m_previouslyRxPpduUid = UINT64_MAX; //reset to use it only once
  Ptr<WifiPpdu> ppdu = Create<WifiPpdu> (psdus, txVector, txDuration, GetCenterFrequencyForChannelWidth (txVector.GetChannelWidth ()), uid);

  if (m_wifiRadioEnergyModel != 0 && m_wifiRadioEnergyModel->GetMaximumTimeInState (WifiPhyState::TX) < txDuration)
    {
      ppdu->SetTruncatedTx ();
    }

  StartTx (ppdu, txVector.GetTxPowerLevel ()); //now that the content of the TXVECTOR is stored in the WifiPpdu through PHY headers, the method calling StartTx has to specify the TX power level to use upon transmission

  m_channelAccessRequested = false;
  m_powerRestricted = false;

  Simulator::Schedule (txDuration, &WifiPhy::Reset, this);
}

void
WifiPhy::Reset (void)
{
  NS_LOG_FUNCTION (this);
  m_currentPreambleEvents.clear ();
  m_currentEvent = 0;
  m_ofdmaStarted = false;
}

void
WifiPhy::StartReceiveHeader (Ptr<Event> event)
{
  NS_LOG_FUNCTION (this << *event);
  NS_ASSERT (!IsStateRx ());
  NS_ASSERT (m_endPlcpRxEvent.IsExpired ());
  NS_ASSERT (m_endRxEvents.empty ());

  double maxRxPowerW = 0;
  Ptr<Event> maxEvent;
  NS_ASSERT (!m_currentPreambleEvents.empty());
  uint16_t primaryChannelWidth = GetChannelWidth () >= 40 ? 20 : GetChannelWidth ();
  auto primaryband = GetBand (primaryChannelWidth, GetPrimaryBandIndex (primaryChannelWidth));
  for (auto preambleEvent : m_currentPreambleEvents)
  {
    double rxPowerW = preambleEvent.second->GetRxPowerW (primaryband);
    if (rxPowerW > maxRxPowerW)
      {
        maxRxPowerW = rxPowerW;
        maxEvent = preambleEvent.second;
      }
  }

  if (maxEvent != event)
    {
      NS_LOG_DEBUG ("Receiver got a stronger packet with UID " << maxEvent->GetPpdu ()->GetUid () << " during preamble detection: drop packet with UID " << event->GetPpdu ()->GetUid ());
      NotifyRxDrop (GetAddressedPsduInPpdu (event->GetPpdu ()), PREAMBLE_DETECTION_PACKET_SWITCH);
      auto it = m_currentPreambleEvents.find (event->GetPpdu ()->GetUid ());
      m_currentPreambleEvents.erase (it);
      return;
    }

  m_currentEvent = event;

  //This is needed to cleanup the m_firstPowerPerBand so that the first power corresponds to the power at the start of the PPDU
  m_interference.NotifyRxEnd (m_currentEvent->GetStartTime ());
  //Make sure InterferenceHelper keeps recording events
  m_interference.NotifyRxStart ();

  double snr = m_interference.CalculateSnr (m_currentEvent, primaryChannelWidth, primaryband);
  NS_LOG_DEBUG ("SNR(dB)=" << RatioToDb (snr) << " at start of legacy PHY header");

  Time headerPayloadDuration = m_currentEvent->GetStartTime () + m_currentEvent->GetPpdu ()->GetTxDuration () - Simulator::Now ();
  if (!m_preambleDetectionModel || (m_preambleDetectionModel->IsPreambleDetected (m_currentEvent->GetRxPowerW (primaryband), snr, primaryChannelWidth)))
    {
      for (auto & endPreambleDetectionEvent : m_endPreambleDetectionEvents)
        {
          endPreambleDetectionEvent.Cancel ();
        }
      m_endPreambleDetectionEvents.clear ();

      for (auto it = m_currentPreambleEvents.begin (); it != m_currentPreambleEvents.end (); )
        {
          if (it->second != m_currentEvent)
            {
              NS_LOG_DEBUG ("Drop packet with UID " << it->first << " arrived at time " << it->second->GetStartTime ());
              NotifyRxDrop (GetAddressedPsduInPpdu (it->second->GetPpdu ()), PREAMBLE_DETECTION_PACKET_SWITCH);
              it = m_currentPreambleEvents.erase (it);
            }
          else
            {
              it++;
            }
        }
  
      auto primaryBand = GetBand (primaryChannelWidth, GetPrimaryBandIndex (primaryChannelWidth));
      m_state->SwitchToRx (headerPayloadDuration, primaryBand, GetCcaEdThreshold ());
      NotifyRxBegin (GetAddressedPsduInPpdu (m_currentEvent->GetPpdu ()), m_currentEvent->GetRxPowerWPerBand ());

      m_timeLastPreambleDetected = Simulator::Now ();

      WifiTxVector txVector = m_currentEvent->GetTxVector ();

      if (txVector.GetPreambleType () == WIFI_PREAMBLE_HT_GF)
        {
          //No legacy PHY header for HT GF
          Time remainingPreambleHeaderDuration = CalculatePlcpPreambleAndHeaderDuration (txVector) - (Simulator::Now () - m_currentEvent->GetStartTime ());
          m_endPlcpRxEvent = Simulator::Schedule (remainingPreambleHeaderDuration, &WifiPhy::StartReceivePayload, this, m_currentEvent);
        }
      else
        {
          //Schedule end of legacy PHY header
          Time remainingPreambleAndLegacyHeaderDuration = GetPlcpPreambleDuration (txVector) + GetPlcpHeaderDuration (txVector) - (Simulator::Now () - m_currentEvent->GetStartTime ());
          m_endPlcpRxEvent = Simulator::Schedule (remainingPreambleAndLegacyHeaderDuration, &WifiPhy::ContinueReceiveHeader, this, m_currentEvent);
        }
    }
  else
    {
      NS_LOG_DEBUG ("Drop packet because PHY preamble detection failed");
      NotifyRxDrop (GetAddressedPsduInPpdu (m_currentEvent->GetPpdu ()), PREAMBLE_DETECT_FAILURE);
      for (auto it = m_currentPreambleEvents.begin (); it != m_currentPreambleEvents.end (); ++it)
        {
          if (it->second == m_currentEvent)
            {
              it = m_currentPreambleEvents.erase (it);
              break;
            }
        }
      if (m_currentPreambleEvents.empty())
        {
          //Do not erase events if there are still pending preamble events to be processed
          m_interference.NotifyRxEnd (Simulator::Now ());
        }
      m_currentEvent = 0;
    }
  // Like CCA-SD, CCA-ED is governed by the 4μs CCA window to flag CCA-BUSY
  // for any received signal greater than the CCA-ED threshold.
  MaybeCcaBusy ();
}

void
WifiPhy::ContinueReceiveHeader (Ptr<Event> event)
{
  NS_LOG_FUNCTION (this << *event);
  NS_ASSERT (IsStateRx ());
  NS_ASSERT (m_endPlcpRxEvent.IsExpired ());

  uint16_t channelWidth;
  if (event->GetTxVector ().GetChannelWidth () >= 40)
    {
      channelWidth = 20; //calculate PER on the 20 MHz primary channel for PHY headers
    }
  else
    {
      channelWidth = event->GetTxVector ().GetChannelWidth ();
    }
  InterferenceHelper::SnrPer snrPer = m_interference.CalculateLegacyPhyHeaderSnrPer (event, GetBand (channelWidth, GetPrimaryBandIndex (channelWidth)));

  NS_LOG_DEBUG ("SNR(dB)=" << RatioToDb (snrPer.snr) << ", PER=" << snrPer.per);
  if (m_random->GetValue () > snrPer.per) //legacy PHY header reception succeeded
    {
      NS_LOG_DEBUG ("Received legacy PHY header");
      WifiTxVector txVector = event->GetTxVector ();
      Time remainingPreambleHeaderDuration = CalculatePlcpPreambleAndHeaderDuration (txVector) - GetPlcpPreambleDuration (txVector) - GetPlcpHeaderDuration (txVector);
      m_endPlcpRxEvent = Simulator::Schedule (remainingPreambleHeaderDuration, &WifiPhy::StartReceivePayload, this, event);
    }
  else //legacy PHY header reception failed
    {
      NS_LOG_DEBUG ("Abort reception because legacy PHY header reception failed");
      AbortCurrentReception (L_SIG_FAILURE);
      m_currentPreambleEvents.clear ();
      MaybeCcaBusy ();
    }
}

void
WifiPhy::StartReceivePreamble (Ptr<WifiPpdu> ppdu, RxPowerWattPerChannelBand rxPowersW)
{
  WifiTxVector txVector = ppdu->GetTxVector ();
  //The total RX power corresponds to the sum of the power in each involved 20 MHz band
  std::size_t nBands = std::max (txVector.GetChannelWidth () / 20, 1);
  std::size_t i = 0;
  double rxPowerW = 0.0;
  for (auto const& rxPowerPerBand : rxPowersW)
    {
      rxPowerW += rxPowerPerBand.second;
      i++;
      if (i == nBands)
        {
          break;
        }
    }
  NS_LOG_FUNCTION (this << *ppdu << rxPowerW);
  Time rxDuration = ppdu->GetTxDuration ();

  Ptr<Event> event;
  //We store all incoming preamble events, and a decision is made at the end of the preamble detection window.
  //If a preamble is received after the preamble detection window, it is stored anyway because this is needed for HE TB PPDUs in
  //order to properly update the received power in InterferenceHelper. The map is cleaned anyway at the end of the current reception.
  if (ppdu->IsUlMu ())
    {
      auto it = m_currentPreambleEvents.find (ppdu->GetUid ());
      if (it != m_currentPreambleEvents.end ())
        {
          NS_LOG_DEBUG ("Received another HE TB PPDU for UID " << ppdu->GetUid () << " from STA-ID " << ppdu->GetStaId () << " and BSS color " << +txVector.GetBssColor ());
          event = it->second;
          if (Simulator::Now () - event->GetStartTime () > NanoSeconds (400))
            {
              //Section 27.3.14.3 from 802.11ax Draft 4.0: Pre-correction accuracy requirements.
              //A STA that transmits an HE TB PPDU, non-HT PPDU, or non-HT duplicate PPDU in response to a triggering PPDU
              //shall ensure that the transmission start time of the HE TB PPDU, non-HT PPDU, or non-HT duplicate PPDU is
              //within ±0.4 µs + 16 µs from the end, at the STA’s antenna connector, of the last OFDM symbol of the triggering
              //PPDU (if it contains no PE field) or of the PE field of the triggering PPDU (if the PE field is present).
              //As a result, if an HE TB PPDU arrives later than 0.4 µs, it is added as an interference but PPDU is dropped.
              event = m_interference.Add (ppdu, txVector, rxDuration, rxPowersW);
              NS_LOG_DEBUG ("Drop packet because not received within the 400ns window");
              NotifyRxDrop (GetAddressedPsduInPpdu (ppdu), NOT_ALLOWED);
            }
          else
            {
              //Update received power of the event associated to that UL MU transmission
              m_interference.UpdateEvent (event, rxPowersW);
            }
          if ((GetPhyState () == WifiPhyState::RX) && (m_currentEvent->GetPpdu ()->GetUid () != ppdu->GetUid ()))
            {
              NS_LOG_DEBUG ("Drop packet because already in Rx");
              NotifyRxDrop (GetAddressedPsduInPpdu (ppdu), NOT_ALLOWED);
            }
          return;
        }
      else
        {
          NS_LOG_DEBUG ("Received a new HE TB PPDU for UID " << ppdu->GetUid () << " from STA-ID " << ppdu->GetStaId () << " and BSS color " << +txVector.GetBssColor ());
          event = m_interference.Add (ppdu, txVector, CalculatePlcpPreambleAndHeaderDuration (txVector), rxPowersW); //the OFDMA part of the transmission will be added later on
          m_currentPreambleEvents.insert ({ppdu->GetUid (), event});
        }
    }
  else
    {
      event = m_interference.Add (ppdu, txVector, rxDuration, rxPowersW);
      NS_ASSERT (m_currentPreambleEvents.find (ppdu->GetUid ()) == m_currentPreambleEvents.end ());
      m_currentPreambleEvents.insert ({ppdu->GetUid (), event});
    }

  if (GetChannelWidth () >= 40)
    {
      auto primaryChannelband = GetBand (20, GetPrimaryBandIndex (20));
      double rxPowerPrimaryChannelW = event->GetRxPowerW (primaryChannelband);
      if (WToDbm (rxPowerPrimaryChannelW) < GetRxSensitivity ())
        {
          NS_LOG_INFO ("Received signal in primary channel too weak to process: " << WToDbm (rxPowerPrimaryChannelW) << " dBm");
          //TODO: drop packet?
          MaybeCcaBusy (); //secondary channel shall maybe switch to CCA_BUSY
          for (auto it = m_currentPreambleEvents.begin (); it != m_currentPreambleEvents.end (); ++it)
          {
            if (it->second == event)
              {
                it = m_currentPreambleEvents.erase (it);
                break;
              }
          }
          return;
        }
      if ((txVector.GetChannelWidth () < GetChannelWidth ()) && (ppdu->GetFrequency () != GetCenterFrequencyForChannelWidth (txVector.GetChannelWidth ())))
        {
          NS_LOG_INFO ("Received a " << txVector.GetChannelWidth () << " MHz PPDU in a secondary " << txVector.GetChannelWidth () << " MHz channel: do not proceed with reception");
          //TODO: drop packet?
          MaybeCcaBusy (); //secondary channel shall maybe switch to CCA_BUSY
          for (auto it = m_currentPreambleEvents.begin (); it != m_currentPreambleEvents.end (); ++it)
          {
            if (it->second == event)
              {
                it = m_currentPreambleEvents.erase (it);
                break;
              }
          }
          return;
        }
    }

  if (GetPhyState () == WifiPhyState::OFF)
    {
      NS_LOG_DEBUG ("Cannot start RX because device is OFF");
      return;
    }

  if (ppdu->IsTruncatedTx ())
    {
      NS_LOG_DEBUG ("Packet reception stopped because transmitter has been switched off");
      return;
    }

  Time endRx = Simulator::Now () + rxDuration;
  switch (GetPhyState ())
    {
    case WifiPhyState::SWITCHING:
      NS_LOG_DEBUG ("drop packet because of channel switching");
      NotifyRxDrop (GetAddressedPsduInPpdu (ppdu), NOT_ALLOWED);
      /*
       * Packets received on the upcoming channel are added to the event list
       * during the switching state. This way the medium can be correctly sensed
       * when the device listens to the channel for the first time after the
       * switching e.g. after channel switching, the channel may be sensed as
       * busy due to other devices' tramissions started before the end of
       * the switching.
       */
      if (endRx > Simulator::Now () + GetDelayUntilIdle ())
        {
          //that packet will be noise _after_ the completion of the channel switching.
          MaybeCcaBusy ();
          return;
        }
      break;
    case WifiPhyState::RX:
      if (m_frameCaptureModel != 0
          && m_frameCaptureModel->IsInCaptureWindow (m_timeLastPreambleDetected)
          && m_frameCaptureModel->CaptureNewFrame (m_currentEvent, event))
        {
          AbortCurrentReception (FRAME_CAPTURE_PACKET_SWITCH);
          NS_LOG_DEBUG ("Switch to new packet");
          StartRx (event);
        }
      else
        {
          NS_LOG_DEBUG ("Drop packet because already in Rx");
          NotifyRxDrop (GetAddressedPsduInPpdu (ppdu), NOT_ALLOWED);
          if (m_currentEvent == 0)
            {
              /*
               * We are here because the non-legacy PHY header has not been successfully received.
               * The PHY is kept in RX state for the duration of the PPDU, but EndReceive function is
               * not called when the reception of the PPDU is finished, which is responsible to clear
               * m_currentPreambleEvents. As a result, m_currentPreambleEvents should be cleared here.
               */
              m_currentPreambleEvents.clear ();
            }
          if (endRx > Simulator::Now () + GetDelayUntilIdle ())
            {
              //that packet will be noise _after_ the reception of the currently-received packet.
              MaybeCcaBusy ();
              return;
            }
        }
      break;
    case WifiPhyState::TX:
      NS_LOG_DEBUG ("Drop packet because already in Tx");
      NotifyRxDrop (GetAddressedPsduInPpdu (ppdu), NOT_ALLOWED);
      if (endRx > Simulator::Now () + GetDelayUntilIdle ())
        {
          //that packet will be noise _after_ the transmission of the currently-transmitted packet.
          MaybeCcaBusy ();
          return;
        }
      break;
    case WifiPhyState::CCA_BUSY:
    case WifiPhyState::IDLE:
      StartRx (event);
      break;
    case WifiPhyState::SLEEP:
      NS_LOG_DEBUG ("Drop packet because in sleep mode");
      NotifyRxDrop (GetAddressedPsduInPpdu (ppdu), NOT_ALLOWED);
      break;
    default:
      NS_FATAL_ERROR ("Invalid WifiPhy state.");
      break;
    }
}

Time
WifiPhy::GetDelayUntilCcaEnd (double ccaThreshold, WifiSpectrumBand band)
{
  return m_interference.GetEnergyDuration (ccaThreshold, band);
}

void
WifiPhy::MaybeCcaBusy ()
{
  //We are here because we have received the first bit of a packet and we are
  //not going to be able to synchronize on it
  //In this model, CCA becomes busy when the aggregation of all signals as
  //tracked by the InterferenceHelper class is higher than the CcaBusyThreshold
  uint16_t channelWidth = GetChannelWidth ();
  for (uint8_t i = 0; i < std::max (1, channelWidth / 20); i++)
    {
      uint16_t primaryChannelWidth = (channelWidth >= 40) ? 20 : channelWidth;
      auto band = GetBand (primaryChannelWidth, i);
      bool isPrimary = (i == GetPrimaryBandIndex (primaryChannelWidth));
      if (isPrimary)
        {
          Time delayUntilCcaEnd = GetDelayUntilCcaEnd (m_ccaEdThresholdW, band);
          if (!delayUntilCcaEnd.IsZero ())
            {
              NS_LOG_DEBUG ("Calling SwitchMaybeToCcaBusy for channel band " << +i << " for " << delayUntilCcaEnd.As (Time::S));
              m_state->SwitchMaybeToCcaBusy (delayUntilCcaEnd, band, isPrimary, GetCcaEdThreshold ());
            }
        }
      else
        {
          for (auto const& ccaEdThresholdSecondaryW : m_ccaEdThresholdsSecondaryW)
            {
              Time delayUntilCcaEnd = GetDelayUntilCcaEnd (ccaEdThresholdSecondaryW, band);
              if (!delayUntilCcaEnd.IsZero ())
                {
                  NS_LOG_DEBUG ("Calling SwitchMaybeToCcaBusy for channel band " << +i << " for " << delayUntilCcaEnd.As (Time::S) << " using threshold " << WToDbm (ccaEdThresholdSecondaryW));
                  m_state->SwitchMaybeToCcaBusy (delayUntilCcaEnd, band, isPrimary, WToDbm (ccaEdThresholdSecondaryW));
                }
            }
        }
    }
}

void
WifiPhy::StartReceiveOfdmaPayload (Ptr<WifiPpdu> ppdu, RxPowerWattPerChannelBand rxPowersW)
{
  //The total RX power corresponds to the maximum over all the bands
  auto it = std::max_element (rxPowersW.begin (), rxPowersW.end (),
    [] (const std::pair<WifiSpectrumBand, double>& p1, const std::pair<WifiSpectrumBand, double>& p2) {
      return p1.second < p2.second;
    });
  NS_LOG_FUNCTION (this << *ppdu << it->second);
  WifiTxVector txVector = ppdu->GetTxVector ();
  Time payloadDuration = ppdu->GetTxDuration () - CalculatePlcpPreambleAndHeaderDuration (txVector);
  Ptr<Event> event = m_interference.Add (ppdu, txVector, payloadDuration, rxPowersW, !m_ofdmaStarted);
  Ptr<const WifiPsdu> psdu = GetAddressedPsduInPpdu (ppdu);
  if (psdu->GetNMpdus () > 1)
    {
      ScheduleEndOfMpdus (event);
    }
  m_endRxEvents.push_back (Simulator::Schedule (payloadDuration, &WifiPhy::EndReceive, this, event));
  m_ofdmaStarted = true;
  m_signalNoiseMap.insert ({std::make_pair (ppdu->GetUid (), ppdu->GetStaId ()), SignalNoiseDbm ()});
  m_statusPerMpduMap.insert ({std::make_pair (ppdu->GetUid (), ppdu->GetStaId ()), std::vector<bool> ()});
}

void
WifiPhy::StartReceivePayload (Ptr<Event> event)
{
  NS_LOG_FUNCTION (this << *event);
  NS_ASSERT (m_endPlcpRxEvent.IsExpired ());
  bool canReceivePayload = false;
  Ptr<const WifiPpdu> ppdu = event->GetPpdu ();
  WifiTxVector txVector = event->GetTxVector ();
  WifiModulationClass modulation = ppdu->GetModulation ();

  if (txVector.GetPreambleType () == WIFI_PREAMBLE_HE_TB)
    {
      Ptr<WifiNetDevice> device = DynamicCast<WifiNetDevice> (GetDevice ());
      bool isAp = (DynamicCast<ApWifiMac> (device->GetMac ()) != 0);
      if (!isAp)
        {
          NS_LOG_INFO ("Ignore UL-OFDMA (OFDMA part of HE TB PPDU) received by STA");
          for (auto it = m_currentPreambleEvents.begin (); it != m_currentPreambleEvents.end (); ++it)
          {
            if (it->second == event)
              {
                it = m_currentPreambleEvents.erase (it);
                break;
              }
          }
          if (m_currentPreambleEvents.empty ())
            {
              Reset ();
            }
          m_interference.NotifyRxEnd (Simulator::Now ());
          return;
        }
    }
  
  if ((modulation == WIFI_MOD_CLASS_HT) || (modulation == WIFI_MOD_CLASS_VHT) || (modulation == WIFI_MOD_CLASS_HE))
    {
      uint16_t channelWidth;
      if (txVector.GetChannelWidth () >= 40)
        {
          channelWidth = 20; //calculate PER on the 20 MHz primary channel for PHY headers
        }
      else
        {
          channelWidth = txVector.GetChannelWidth ();
        }
      InterferenceHelper::SnrPer snrPer = m_interference.CalculateNonLegacyPhyHeaderSnrPer (event, GetBand (channelWidth, GetPrimaryBandIndex (channelWidth)));
      NS_LOG_DEBUG ("SNR(dB)=" << RatioToDb (snrPer.snr) << ", PER=" << snrPer.per);
      canReceivePayload = (m_random->GetValue () > snrPer.per);
    }
  else
    {
      //If we are here, this means legacy PHY header was already successfully received
      canReceivePayload = true;
    }
  if (canReceivePayload) //plcp reception succeeded
    {
      Ptr<const WifiPsdu> psdu = GetAddressedPsduInPpdu (ppdu);
      if (psdu)
        {
          uint16_t staId = GetStaId (ppdu);
          WifiMode txMode = txVector.GetMode (staId);
          uint8_t nss = txVector.GetNssMax();
          if (txVector.GetPreambleType () == WIFI_PREAMBLE_HE_MU)
            {
              uint16_t staId = GetStaId (m_currentEvent->GetPpdu ());
              for (auto info : txVector.GetHeMuUserInfoMap ())
                {
                  if (info.first == staId)
                    {
                      nss = info.second.nss; //no need to look at other PSDUs
                      break;
                    }
                }
            }
          if (nss > GetMaxSupportedRxSpatialStreams ())
            {
              NS_LOG_DEBUG ("Packet reception could not be started because not enough RX antennas");
              AbortCurrentReception (UNSUPPORTED_SETTINGS);
              MaybeCcaBusy ();
              return;
            }
          else if ((txVector.GetChannelWidth () >= 40) && (txVector.GetChannelWidth () > GetChannelWidth ()))
            {
              NS_LOG_DEBUG ("Packet reception could not be started because not enough channel width");
              AbortCurrentReception (UNSUPPORTED_SETTINGS);
              MaybeCcaBusy ();
              return;
            }
          else if (!IsModeSupported (txMode) && !IsMcsSupported (txMode))
            {
              NS_LOG_DEBUG ("Drop packet because it was sent using an unsupported mode (" << txMode << ")");
              AbortCurrentReception (UNSUPPORTED_SETTINGS);
              MaybeCcaBusy ();
              return;
            }
          else
            {
              NS_LOG_DEBUG ("Receiving PSDU");
              m_signalNoiseMap.insert ({std::make_pair (ppdu->GetUid (), staId), SignalNoiseDbm ()});
              m_statusPerMpduMap.insert ({std::make_pair (ppdu->GetUid (), staId), std::vector<bool> ()});
              if (psdu->GetNMpdus () > 1)
                {
                  ScheduleEndOfMpdus (event);
                }
              Time payloadDuration = ppdu->GetTxDuration () - CalculatePlcpPreambleAndHeaderDuration (txVector);
              m_phyRxPayloadBeginTrace (txVector, payloadDuration); //this callback (equivalent to PHY-RXSTART primitive) is triggered only if headers have been correctly decoded and that the mode within is supported
              if (txVector.GetPreambleType () == WIFI_PREAMBLE_HE_TB)
                {
                  m_currentHeTbPpduUid = ppdu->GetUid ();
                  //EndReceive is scheduled by StartReceiveOfdmaPayload
                }
              else
                {
                  m_endRxEvents.push_back (Simulator::Schedule (payloadDuration, &WifiPhy::EndReceive, this, event));
                }
            }
        }
      else
        {
          NS_ASSERT (ppdu->IsMu ());
          NS_LOG_DEBUG ("No PSDU addressed to that PHY in the received MU PPDU");
          m_interference.NotifyRxEnd (m_currentEvent->GetEndTime ());
          m_currentEvent = 0;
          m_currentPreambleEvents.clear ();
          return;
        }
      if (modulation == WIFI_MOD_CLASS_HE)
        {
          HePreambleParameters params;
          WifiSpectrumBands bands;
          if (txVector.IsMu ())
            {
              uint16_t staId = GetStaId (ppdu);
              auto band = GetRuBand (txVector, staId);
              bands.push_back (band);
            }
          else
            {
              uint8_t index = 0;
              uint16_t channelWidth = txVector.GetChannelWidth ();
              if (channelWidth < GetChannelWidth ())
                {
                  index = GetPrimaryBandIndex (channelWidth);
                }
              auto band = GetBand (channelWidth, index);
              if (channelWidth < 40)
                {
                  bands.push_back (band);
                }
              else
                {
                  for (uint8_t i = 0; i < (channelWidth / 20); i++)
                    {
                      band = GetBand (20, i);
                      bands.push_back (band);
                    }
                }
            }
          params.rssiW = event->GetRxPowerW (bands);
          params.bssColor = txVector.GetBssColor ();
          NotifyEndOfHePreamble (params);
        }
    }
  else //plcp reception failed
    {
      NS_LOG_DEBUG ("Drop packet because non-legacy PHY header reception failed");
      NotifyRxDrop (GetAddressedPsduInPpdu (event->GetPpdu ()), SIG_A_FAILURE);
      m_interference.NotifyRxEnd (m_currentEvent->GetEndTime ());
      m_currentEvent = 0;
      m_currentPreambleEvents.clear ();
    }
}

void
WifiPhy::ScheduleEndOfMpdus (Ptr<Event> event)
{
  NS_LOG_FUNCTION (this << *event);
  Ptr<const WifiPpdu> ppdu = event->GetPpdu ();
  Ptr<const WifiPsdu> psdu = GetAddressedPsduInPpdu (ppdu);
  WifiTxVector txVector = event->GetTxVector ();
  uint16_t staId = GetStaId (ppdu);
  Time endOfMpduDuration = NanoSeconds (0);
  Time relativeStart = NanoSeconds (0);
  Time psduDuration = ppdu->GetTxDuration () - CalculatePlcpPreambleAndHeaderDuration (txVector);
  Time remainingAmpduDuration = psduDuration;
  MpduType mpdutype = FIRST_MPDU_IN_AGGREGATE;
  uint32_t totalAmpduSize = 0;
  double totalAmpduNumSymbols = 0.0;
  size_t nMpdus = psdu->GetNMpdus ();
  auto mpdu = psdu->begin ();
  for (size_t i = 0; i < nMpdus && mpdu != psdu->end (); ++mpdu)
    {
      Time mpduDuration = GetPayloadDuration (psdu->GetAmpduSubframeSize (i), txVector,
                                              GetFrequency (), mpdutype, true, totalAmpduSize,
                                              totalAmpduNumSymbols, staId);

      remainingAmpduDuration -= mpduDuration;
      if (i == (nMpdus - 1) && !remainingAmpduDuration.IsZero ()) //no more MPDU coming
        {
          mpduDuration += remainingAmpduDuration; //apply a correction just in case rounding had induced slight shift
        }

      endOfMpduDuration += mpduDuration;
      m_endOfMpduEvents.push_back (Simulator::Schedule (endOfMpduDuration, &WifiPhy::EndOfMpdu, this, event, Create<WifiPsdu> (*mpdu, false), i, relativeStart, mpduDuration));

      //Prepare next iteration
      ++i;
      relativeStart += mpduDuration;
      mpdutype = (i == (nMpdus - 1)) ? LAST_MPDU_IN_AGGREGATE : MIDDLE_MPDU_IN_AGGREGATE;
    }
}

void
WifiPhy::EndOfMpdu (Ptr<Event> event, Ptr<const WifiPsdu> psdu, size_t mpduIndex, Time relativeStart, Time mpduDuration)
{
  NS_LOG_FUNCTION (this << *event << mpduIndex << relativeStart << mpduDuration);
  Ptr<const WifiPpdu> ppdu = event->GetPpdu ();
  WifiTxVector txVector = event->GetTxVector ();
  uint16_t staId = GetStaId (ppdu);
  uint16_t channelWidth = std::min (GetChannelWidth (), event->GetTxVector ().GetChannelWidth ());
  WifiSpectrumBand band;
  WifiSpectrumBands bands;
  double snr;
  if (txVector.IsMu ())
    {
      band = GetRuBand (txVector, staId);
      channelWidth = HeRu::GetBandwidth (txVector.GetRu (staId).ruType);
      snr = m_interference.CalculateSnr (event, channelWidth, band);
    }
  else
    {
      uint8_t index = 0;
      if (channelWidth < GetChannelWidth ())
        {
          index = GetPrimaryBandIndex (channelWidth);
        }
      band = GetBand (channelWidth, index);
      if (channelWidth < 40)
        {
          bands.push_back (band);
        }
      else
        {
          for (uint8_t i = 0; i < (channelWidth / 20); i++)
            {
              band = GetBand (20, i);
              bands.push_back (band);
            }
        }
      snr = m_interference.CalculateEffectiveSnr (event, channelWidth, bands);
    }
  std::pair<bool, SignalNoiseDbm> rxInfo = GetReceptionStatus (psdu, event, staId, relativeStart, mpduDuration);
  NS_LOG_DEBUG ("Extracted MPDU #" << mpduIndex << ": duration: " << mpduDuration.GetNanoSeconds () << "ns" <<
                ", correct reception: " << rxInfo.first << ", Signal/Noise: " << rxInfo.second.signal << "/" << rxInfo.second.noise << "dBm");

  auto signalNoiseIt = m_signalNoiseMap.find (std::make_pair (ppdu->GetUid (), staId));
  NS_ASSERT (signalNoiseIt != m_signalNoiseMap.end ());
  signalNoiseIt->second = rxInfo.second;

  RxSignalInfo rxSignalInfo;
  rxSignalInfo.snr = snr;
  rxSignalInfo.rssi = rxInfo.second.signal;

  auto statusPerMpduIt = m_statusPerMpduMap.find (std::make_pair (ppdu->GetUid (), staId));
  NS_ASSERT (statusPerMpduIt != m_statusPerMpduMap.end ());
  statusPerMpduIt->second.push_back (rxInfo.first);

  if (rxInfo.first)
    {
      m_state->ContinueRxNextMpdu (Copy (psdu), rxSignalInfo, txVector);
    }
}

void
WifiPhy::EndReceive (Ptr<Event> event)
{
  Ptr<const WifiPpdu> ppdu = event->GetPpdu ();
  WifiTxVector txVector = event->GetTxVector ();
  Time psduDuration = ppdu->GetTxDuration () - CalculatePlcpPreambleAndHeaderDuration (txVector);
  NS_LOG_FUNCTION (this << *event << psduDuration);
  NS_ASSERT (event->GetEndTime () == Simulator::Now ());
  uint16_t staId = GetStaId (ppdu);
  uint16_t channelWidth = std::min (GetChannelWidth (), event->GetTxVector ().GetChannelWidth ());
  WifiSpectrumBand band;
  WifiSpectrumBands bands;
  double snr;
  if (txVector.IsMu ())
    {
      band = GetRuBand (txVector, staId);
      channelWidth = HeRu::GetBandwidth (txVector.GetRu (staId).ruType);
      snr = m_interference.CalculateSnr (event, channelWidth, band);
    }
  else
    {
      uint8_t index = 0;
      if (channelWidth < GetChannelWidth ())
        {
          index = GetPrimaryBandIndex (channelWidth);
        }
      band = GetBand (channelWidth, index);
      if (channelWidth < 40)
        {
          bands.push_back (band);
        }
      else
        {
          for (uint8_t i = 0; i < (channelWidth / 20); i++)
            {
              band = GetBand (20, i);
              bands.push_back (band);
            }
        }
      snr = m_interference.CalculateEffectiveSnr (event, channelWidth, bands);
    }
  Ptr<const WifiPsdu> psdu = GetAddressedPsduInPpdu (ppdu);
  auto signalNoiseIt = m_signalNoiseMap.find (std::make_pair (ppdu->GetUid (), staId));
  NS_ASSERT (signalNoiseIt != m_signalNoiseMap.end ());
  auto statusPerMpduIt = m_statusPerMpduMap.find (std::make_pair (ppdu->GetUid (), staId));
  NS_ASSERT (statusPerMpduIt != m_statusPerMpduMap.end ());
  if (psdu->GetNMpdus () == 1)
    {
      //We do not enter here for A-MPDU since this is done in WifiPhy::EndOfMpdu
      std::pair<bool, SignalNoiseDbm> rxInfo = GetReceptionStatus (psdu, event, staId, NanoSeconds (0), psduDuration);
      signalNoiseIt->second = rxInfo.second;
      statusPerMpduIt->second.push_back (rxInfo.first);
    }

if (std::count(statusPerMpduIt->second.begin (), statusPerMpduIt->second.end (), true))
   {
      //At least one MPDU has been successfully received
      NotifyMonitorSniffRx (psdu, GetFrequency (), txVector, signalNoiseIt->second, statusPerMpduIt->second);
      RxSignalInfo rxSignalInfo;
      rxSignalInfo.snr = snr;
      rxSignalInfo.rssi = signalNoiseIt->second.signal; //same information for all MPDUs
      m_state->SwitchFromRxEndOk (Copy (psdu), rxSignalInfo, txVector, staId, statusPerMpduIt->second);
      m_previouslyRxPpduUid = event->GetPpdu ()->GetUid (); //store UID only if reception is successful (b/c otherwise trigger won't be read by MAC layer)
    }
  else
    {
      m_state->SwitchFromRxEndError (Copy (psdu), snr);
    }

  if (ppdu->IsUlMu ())
    {
      for (auto it = m_endRxEvents.begin (); it != m_endRxEvents.end (); )
        {
          if (it->IsExpired ())
            {
              it = m_endRxEvents.erase (it);
            }
          else
            {
              it++;
            }
        }
      if (m_endRxEvents.empty ())
        {
          //We got the last PPDU of the UL-OFDMA transmission
          m_interference.NotifyRxEnd (Simulator::Now ());
          m_signalNoiseMap.clear ();
          m_statusPerMpduMap.clear ();
          for (const auto & endOfMpduEvent : m_endOfMpduEvents)
            {
              NS_ASSERT (endOfMpduEvent.IsExpired ());
            }
          m_endOfMpduEvents.clear ();
          Reset ();
        }
    }
  else
    {
      m_interference.NotifyRxEnd (Simulator::Now ());
      m_currentEvent = 0;
      m_currentPreambleEvents.clear ();
      m_endRxEvents.clear ();
      m_signalNoiseMap.clear ();
      m_statusPerMpduMap.clear ();
      for (const auto & endOfMpduEvent : m_endOfMpduEvents)
        {
          NS_ASSERT (endOfMpduEvent.IsExpired ());
        }
      m_endOfMpduEvents.clear ();
    }
  MaybeCcaBusy ();
}

std::pair<bool, SignalNoiseDbm>
WifiPhy::GetReceptionStatus (Ptr<const WifiPsdu> psdu, Ptr<Event> event, uint16_t staId,
                             Time relativeMpduStart, Time mpduDuration)
{
  NS_LOG_FUNCTION (this << *psdu << *event << staId << relativeMpduStart << mpduDuration);
  uint16_t channelWidth = std::min (GetChannelWidth (), event->GetTxVector ().GetChannelWidth ());
  WifiTxVector txVector = event->GetTxVector ();
  WifiSpectrumBand band;
  WifiSpectrumBands bands;
  if (txVector.IsMu ())
    {
      band = GetRuBand (txVector, staId);
      channelWidth = HeRu::GetBandwidth (txVector.GetRu (staId).ruType);
      bands.push_back (band);
    }
  else
    {
      uint8_t index = 0;
      if (channelWidth < GetChannelWidth ())
        {
          index = GetPrimaryBandIndex (channelWidth);
        }
      band = GetBand (channelWidth, index);
      if (channelWidth < 40)
        {
          bands.push_back (band);
        }
      else
        {
          for (uint8_t i = 0; i < (channelWidth / 20); i++)
            {
              band = GetBand (20, i);
              bands.push_back (band);
            }
        }
    }
  InterferenceHelper::SnrPer snrPer = m_interference.CalculatePayloadSnrPer (event, channelWidth, bands, staId, std::make_pair (relativeMpduStart, relativeMpduStart + mpduDuration));

  WifiMode mode = event->GetTxVector ().GetMode (staId);
  NS_LOG_DEBUG ("rate=" << (mode.GetDataRate (event->GetTxVector (), staId)) <<
                ", SNR(dB)=" << RatioToDb (snrPer.snr) << ", PER=" << snrPer.per << ", size=" << psdu->GetSize () <<
                ", relativeStart = " << relativeMpduStart.GetNanoSeconds () << "ns, duration = " << mpduDuration.GetNanoSeconds () << "ns");

  // There are two error checks: PER and receive error model check.
  // PER check models is typical for Wi-Fi and is based on signal modulation;
  // Receive error model is optional, if we have an error model and
  // it indicates that the packet is corrupt, drop the packet.
  SignalNoiseDbm signalNoise;
  signalNoise.signal = WToDbm (event->GetRxPowerW (bands));
  signalNoise.noise = WToDbm (event->GetRxPowerW (bands) / snrPer.snr);
  if (m_random->GetValue () > snrPer.per &&
      !(m_postReceptionErrorModel && m_postReceptionErrorModel->IsCorrupt (psdu->GetPacket ()->Copy ())))
    {
      NS_LOG_DEBUG ("Reception succeeded: " << psdu);
      NotifyRxEnd (psdu);
      return std::make_pair (true, signalNoise);
    }
  else
    {
      NS_LOG_DEBUG ("Reception failed: " << psdu);
      NotifyRxDrop (psdu, ERRONEOUS_FRAME);
      return std::make_pair (false, signalNoise);
    }
}

WifiSpectrumBand
WifiPhy::GetRuBand (WifiTxVector txVector, uint16_t staId)
{
  NS_ASSERT (txVector.IsMu ());
  WifiSpectrumBand band;
  HeRu::RuSpec ru = txVector.GetRu (staId);
  uint16_t channelWidth = txVector.GetChannelWidth ();
  NS_ASSERT (channelWidth <= GetChannelWidth ());
  HeRu::SubcarrierGroup group = HeRu::GetSubcarrierGroup (channelWidth, ru.ruType, ru.index);
  HeRu::SubcarrierRange range = std::make_pair (group.front ().first, group.back ().second);
  band = ConvertHeRuSubcarriers (channelWidth, range);
  return band;
}

WifiSpectrumBand
WifiPhy::ConvertHeRuSubcarriers (uint16_t channelWidth, HeRu::SubcarrierRange range) const
{
  NS_ASSERT_MSG (false, "802.11ax can only be used with SpectrumWifiPhy");
  WifiSpectrumBand convertedSubcarriers;
  return convertedSubcarriers;
}

void
WifiPhy::EndReceiveInterBss (void)
{
  NS_LOG_FUNCTION (this);
  if (!m_channelAccessRequested)
    {
      m_powerRestricted = false;
    }
}

void
WifiPhy::NotifyChannelAccessRequested (void)
{
  NS_LOG_FUNCTION (this);
  m_channelAccessRequested = true;
}

// Clause 15 rates (DSSS)

WifiMode
WifiPhy::GetDsssRate1Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DsssRate1Mbps",
                                     WIFI_MOD_CLASS_DSSS,
                                     true,
                                     WIFI_CODE_RATE_UNDEFINED,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetDsssRate2Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DsssRate2Mbps",
                                     WIFI_MOD_CLASS_DSSS,
                                     true,
                                     WIFI_CODE_RATE_UNDEFINED,
                                     4);
  return mode;
}


// Clause 18 rates (HR/DSSS)

WifiMode
WifiPhy::GetDsssRate5_5Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DsssRate5_5Mbps",
                                     WIFI_MOD_CLASS_HR_DSSS,
                                     true,
                                     WIFI_CODE_RATE_UNDEFINED,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetDsssRate11Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("DsssRate11Mbps",
                                     WIFI_MOD_CLASS_HR_DSSS,
                                     true,
                                     WIFI_CODE_RATE_UNDEFINED,
                                     256);
  return mode;
}


// Clause 19.5 rates (ERP-OFDM)

WifiMode
WifiPhy::GetErpOfdmRate6Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate6Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetErpOfdmRate9Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate9Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetErpOfdmRate12Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate12Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetErpOfdmRate18Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate18Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetErpOfdmRate24Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate24Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetErpOfdmRate36Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate36Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetErpOfdmRate48Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate48Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     false,
                                     WIFI_CODE_RATE_2_3,
                                     64);
  return mode;
}

WifiMode
WifiPhy::GetErpOfdmRate54Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("ErpOfdmRate54Mbps",
                                     WIFI_MOD_CLASS_ERP_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     64);
  return mode;
}


// Clause 17 rates (OFDM)

WifiMode
WifiPhy::GetOfdmRate6Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate6Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate9Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate9Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate12Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate12Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate18Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate18Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate24Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate24Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate36Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate36Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate48Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate48Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_2_3,
                                     64);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate54Mbps ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate54Mbps",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     64);
  return mode;
}


// 10 MHz channel rates

WifiMode
WifiPhy::GetOfdmRate3MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate3MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate4_5MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate4_5MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate6MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate6MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate9MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate9MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate12MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate12MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate18MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate18MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate24MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate24MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_2_3,
                                     64);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate27MbpsBW10MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate27MbpsBW10MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     64);
  return mode;
}


// 5 MHz channel rates

WifiMode
WifiPhy::GetOfdmRate1_5MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate1_5MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate2_25MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate2_25MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     2);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate3MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate3MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate4_5MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate4_5MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     4);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate6MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate6MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     true,
                                     WIFI_CODE_RATE_1_2,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate9MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate9MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     16);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate12MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate12MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_2_3,
                                     64);
  return mode;
}

WifiMode
WifiPhy::GetOfdmRate13_5MbpsBW5MHz ()
{
  static WifiMode mode =
    WifiModeFactory::CreateWifiMode ("OfdmRate13_5MbpsBW5MHz",
                                     WIFI_MOD_CLASS_OFDM,
                                     false,
                                     WIFI_CODE_RATE_3_4,
                                     64);
  return mode;
}


// Clause 20

WifiMode
WifiPhy::GetHtMcs0 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs0", 0, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs1 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs1", 1, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs2 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs2", 2, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs3 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs3", 3, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs4 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs4", 4, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs5 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs5", 5, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs6 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs6", 6, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs7 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs7", 7, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs8 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs8", 8, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs9 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs9", 9, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs10 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs10", 10, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs11 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs11", 11, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs12 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs12", 12, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs13 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs13", 13, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs14 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs14", 14, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs15 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs15", 15, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs16 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs16", 16, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs17 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs17", 17, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs18 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs18", 18, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs19 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs19", 19, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs20 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs20", 20, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs21 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs21", 21, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs22 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs22", 22, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs23 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs23", 23, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs24 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs24", 24, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs25 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs25", 25, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs26 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs26", 26, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs27 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs27", 27, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs28 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs28", 28, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs29 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs29", 29, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs30 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs30", 30, WIFI_MOD_CLASS_HT);
  return mcs;
}

WifiMode
WifiPhy::GetHtMcs31 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HtMcs31", 31, WIFI_MOD_CLASS_HT);
  return mcs;
}


// Clause 22

WifiMode
WifiPhy::GetVhtMcs0 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs0", 0, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs1 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs1", 1, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs2 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs2", 2, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs3 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs3", 3, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs4 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs4", 4, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs5 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs5", 5, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs6 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs6", 6, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs7 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs7", 7, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs8 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs8", 8, WIFI_MOD_CLASS_VHT);
  return mcs;
}

WifiMode
WifiPhy::GetVhtMcs9 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("VhtMcs9", 9, WIFI_MOD_CLASS_VHT);
  return mcs;
}

// Clause 26

WifiMode
WifiPhy::GetHeMcs0 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs0", 0, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs1 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs1", 1, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs2 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs2", 2, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs3 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs3", 3, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs4 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs4", 4, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs5 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs5", 5, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs6 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs6", 6, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs7 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs7", 7, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs8 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs8", 8, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs9 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs9", 9, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs10 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs10", 10, WIFI_MOD_CLASS_HE);
  return mcs;
}

WifiMode
WifiPhy::GetHeMcs11 ()
{
  static WifiMode mcs =
    WifiModeFactory::CreateWifiMcs ("HeMcs11", 11, WIFI_MOD_CLASS_HE);
  return mcs;
}

bool
WifiPhy::IsModeSupported (WifiMode mode) const
{
  for (uint8_t i = 0; i < GetNModes (); i++)
    {
      if (mode == GetMode (i))
        {
          return true;
        }
    }
  return false;
}

bool
WifiPhy::IsMcsSupported (WifiMode mcs) const
{
  WifiModulationClass modulation = mcs.GetModulationClass ();
  if (modulation == WIFI_MOD_CLASS_HT || modulation == WIFI_MOD_CLASS_VHT
      || modulation == WIFI_MOD_CLASS_HE)
    {
      return IsMcsSupported (modulation, mcs.GetMcsValue ());
    }
  return false;
}

bool
WifiPhy::IsMcsSupported (WifiModulationClass mc, uint8_t mcs) const
{
  if (m_mcsIndexMap.find (mc) == m_mcsIndexMap.end ())
    {
      return false;
    }
  if (m_mcsIndexMap.at (mc).find (mcs) == m_mcsIndexMap.at (mc).end ())
    {
      return false;
    }
  return true;
}

uint8_t
WifiPhy::GetNModes (void) const
{
  return static_cast<uint8_t> (m_deviceRateSet.size ());
}

WifiMode
WifiPhy::GetMode (uint8_t mode) const
{
  return m_deviceRateSet[mode];
}

uint8_t
WifiPhy::GetNMcs (void) const
{
  return static_cast<uint8_t> (m_deviceMcsSet.size ());
}

WifiMode
WifiPhy::GetMcs (uint8_t mcs) const
{
  return m_deviceMcsSet[mcs];
}

WifiMode
WifiPhy::GetMcs (WifiModulationClass modulation, uint8_t mcs) const
{
  NS_ABORT_MSG_IF (!IsMcsSupported (modulation, mcs), "Unsupported MCS");
  uint8_t index = m_mcsIndexMap.at (modulation).at (mcs);
  NS_ASSERT (index < m_deviceMcsSet.size ());
  WifiMode mode = m_deviceMcsSet[index];
  NS_ASSERT (mode.GetModulationClass () == modulation);
  NS_ASSERT (mode.GetMcsValue () == mcs);
  return mode;
}

WifiMode
WifiPhy::GetHtMcs (uint8_t mcs)
{
  WifiMode mode;
  switch (mcs)
    {
      case 0:
        mode = WifiPhy::GetHtMcs0 ();
        break;
      case 1:
        mode = WifiPhy::GetHtMcs1 ();
        break;
      case 2:
        mode = WifiPhy::GetHtMcs2 ();
        break;
      case 3:
        mode = WifiPhy::GetHtMcs3 ();
        break;
      case 4:
        mode = WifiPhy::GetHtMcs4 ();
        break;
      case 5:
        mode = WifiPhy::GetHtMcs5 ();
        break;
      case 6:
        mode = WifiPhy::GetHtMcs6 ();
        break;
      case 7:
        mode = WifiPhy::GetHtMcs7 ();
        break;
      case 8:
        mode = WifiPhy::GetHtMcs8 ();
        break;
      case 9:
        mode = WifiPhy::GetHtMcs9 ();
        break;
      case 10:
        mode = WifiPhy::GetHtMcs10 ();
        break;
      case 11:
        mode = WifiPhy::GetHtMcs11 ();
        break;
      case 12:
        mode = WifiPhy::GetHtMcs12 ();
        break;
      case 13:
        mode = WifiPhy::GetHtMcs13 ();
        break;
      case 14:
        mode = WifiPhy::GetHtMcs14 ();
        break;
      case 15:
        mode = WifiPhy::GetHtMcs15 ();
        break;
      case 16:
        mode = WifiPhy::GetHtMcs16 ();
        break;
      case 17:
        mode = WifiPhy::GetHtMcs17 ();
        break;
      case 18:
        mode = WifiPhy::GetHtMcs18 ();
        break;
      case 19:
        mode = WifiPhy::GetHtMcs19 ();
        break;
      case 20:
        mode = WifiPhy::GetHtMcs20 ();
        break;
      case 21:
        mode = WifiPhy::GetHtMcs21 ();
        break;
      case 22:
        mode = WifiPhy::GetHtMcs22 ();
        break;
      case 23:
        mode = WifiPhy::GetHtMcs23 ();
        break;
      case 24:
        mode = WifiPhy::GetHtMcs24 ();
        break;
      case 25:
        mode = WifiPhy::GetHtMcs25 ();
        break;
      case 26:
        mode = WifiPhy::GetHtMcs26 ();
        break;
      case 27:
        mode = WifiPhy::GetHtMcs27 ();
        break;
      case 28:
        mode = WifiPhy::GetHtMcs28 ();
        break;
      case 29:
        mode = WifiPhy::GetHtMcs29 ();
        break;
      case 30:
        mode = WifiPhy::GetHtMcs30 ();
        break;
      case 31:
        mode = WifiPhy::GetHtMcs31 ();
        break;
      default:
        NS_ABORT_MSG ("Invalid HT MCS");
        break;
    }
  return mode;
}

WifiMode
WifiPhy::GetVhtMcs (uint8_t mcs)
{
  WifiMode mode;
  switch (mcs)
    {
      case 0:
        mode = WifiPhy::GetVhtMcs0 ();
        break;
      case 1:
        mode = WifiPhy::GetVhtMcs1 ();
        break;
      case 2:
        mode = WifiPhy::GetVhtMcs2 ();
        break;
      case 3:
        mode = WifiPhy::GetVhtMcs3 ();
        break;
      case 4:
        mode = WifiPhy::GetVhtMcs4 ();
        break;
      case 5:
        mode = WifiPhy::GetVhtMcs5 ();
        break;
      case 6:
        mode = WifiPhy::GetVhtMcs6 ();
        break;
      case 7:
        mode = WifiPhy::GetVhtMcs7 ();
        break;
      case 8:
        mode = WifiPhy::GetVhtMcs8 ();
        break;
      case 9:
        mode = WifiPhy::GetVhtMcs9 ();
        break;
      default:
        NS_ABORT_MSG ("Invalid VHT MCS");
        break;
    }
  return mode;
}

WifiMode
WifiPhy::GetHeMcs (uint8_t mcs)
{
  WifiMode mode;
  switch (mcs)
    {
      case 0:
        mode = WifiPhy::GetHeMcs0 ();
        break;
      case 1:
        mode = WifiPhy::GetHeMcs1 ();
        break;
      case 2:
        mode = WifiPhy::GetHeMcs2 ();
        break;
      case 3:
        mode = WifiPhy::GetHeMcs3 ();
        break;
      case 4:
        mode = WifiPhy::GetHeMcs4 ();
        break;
      case 5:
        mode = WifiPhy::GetHeMcs5 ();
        break;
      case 6:
        mode = WifiPhy::GetHeMcs6 ();
        break;
      case 7:
        mode = WifiPhy::GetHeMcs7 ();
        break;
      case 8:
        mode = WifiPhy::GetHeMcs8 ();
        break;
      case 9:
        mode = WifiPhy::GetHeMcs9 ();
        break;
      case 10:
        mode = WifiPhy::GetHeMcs10 ();
        break;
      case 11:
        mode = WifiPhy::GetHeMcs11 ();
        break;
      default:
        NS_ABORT_MSG ("Invalid HE MCS");
        break;
    }
  return mode;
}

bool
WifiPhy::IsStateCcaBusy (void)
{
  uint16_t primaryChannelWidth = GetChannelWidth () >= 40 ? 20 : GetChannelWidth ();
  auto primaryBand = GetBand (primaryChannelWidth, GetPrimaryBandIndex (primaryChannelWidth));
  return m_state->IsStateCcaBusy (primaryBand, GetCcaEdThreshold ());
}

bool
WifiPhy::IsStateIdle (void)
{
  uint16_t primaryChannelWidth = GetChannelWidth () >= 40 ? 20 : GetChannelWidth ();
  auto primaryBand = GetBand (primaryChannelWidth, GetPrimaryBandIndex (primaryChannelWidth));
  return m_state->IsStateIdle (primaryBand, GetCcaEdThreshold ());
}

bool
WifiPhy::IsStateRx (void)
{
  uint16_t primaryChannelWidth = GetChannelWidth () >= 40 ? 20 : GetChannelWidth ();
  auto primaryBand = GetBand (primaryChannelWidth, GetPrimaryBandIndex (primaryChannelWidth));
  return m_state->IsStateRx (primaryBand, GetCcaEdThreshold ());
}

bool
WifiPhy::IsStateTx (void)
{
  uint16_t primaryChannelWidth = GetChannelWidth () >= 40 ? 20 : GetChannelWidth ();
  auto primaryBand = GetBand (primaryChannelWidth, GetPrimaryBandIndex (primaryChannelWidth));
  return m_state->IsStateTx (primaryBand, GetCcaEdThreshold ());
}

bool
WifiPhy::IsStateSwitching (void)
{
  uint16_t primaryChannelWidth = GetChannelWidth () >= 40 ? 20 : GetChannelWidth ();
  auto primaryBand = GetBand (primaryChannelWidth, GetPrimaryBandIndex (primaryChannelWidth));
  return m_state->IsStateSwitching (primaryBand, GetCcaEdThreshold ());
}

bool
WifiPhy::IsStateSleep (void)
{
  uint16_t primaryChannelWidth = GetChannelWidth () >= 40 ? 20 : GetChannelWidth ();
  auto primaryBand = GetBand (primaryChannelWidth, GetPrimaryBandIndex (primaryChannelWidth));
  return m_state->IsStateSleep (primaryBand, GetCcaEdThreshold ());
}

bool
WifiPhy::IsStateOff (void)
{
  uint16_t primaryChannelWidth = GetChannelWidth () >= 40 ? 20 : GetChannelWidth ();
  auto primaryBand = GetBand (primaryChannelWidth, GetPrimaryBandIndex (primaryChannelWidth));
  return m_state->IsStateOff (primaryBand, GetCcaEdThreshold ());
}

Time
WifiPhy::GetDelayUntilIdle (void)
{
  uint16_t primaryChannelWidth = GetChannelWidth () >= 40 ? 20 : GetChannelWidth ();
  auto primaryBand = GetBand (primaryChannelWidth, GetPrimaryBandIndex (primaryChannelWidth));
  return m_state->GetDelayUntilIdle (primaryBand, GetCcaEdThreshold ());
}

Time
WifiPhy::GetLastRxStartTime (void) const
{
  return m_state->GetLastRxStartTime ();
}

Time
WifiPhy::GetDelaySinceChannelIsIdle (uint16_t channelWidth, double ccaThreshold)
{
  NS_ASSERT (channelWidth <= GetChannelWidth ());
  Time delaySinceIdle = Simulator::Now ();
  uint8_t nBands = channelWidth / 20;
  uint8_t index = (GetPrimaryBandIndex (20) / nBands);
  uint8_t startIndex = index * nBands;
  uint8_t stopIndex = startIndex + nBands;
  for (uint8_t i = startIndex; i < stopIndex; i++)
    {
      auto band = GetBand (((channelWidth >= 40) ? 20 : channelWidth), i);
      delaySinceIdle = std::min (delaySinceIdle, m_state->GetDelaySinceIdle (band, ccaThreshold));
    }
  return delaySinceIdle;
}

bool
WifiPhy::IsStateIdle (uint16_t channelWidth, double ccaThreshold)
{
  NS_ASSERT (channelWidth <= GetChannelWidth ());
  if (GetChannelWidth () < 40)
    {
      auto band = GetBand (channelWidth, 0);
      return m_state->IsStateIdle (band, ccaThreshold);
    }
  bool idle = true;
  uint8_t nBands = channelWidth / 20;
  uint8_t index = (GetPrimaryBandIndex (20) / nBands);
  uint8_t startIndex = index * nBands;
  uint8_t stopIndex = startIndex + nBands;
  for (uint8_t i = startIndex; i < stopIndex; i++)
    {
      auto band = GetBand (20, i);
      idle &= m_state->IsStateIdle (band, ccaThreshold);
    }
  return idle;
}

WifiPhyState
WifiPhy::GetPhyState (void)
{
  uint16_t primaryChannelWidth = GetChannelWidth () >= 40 ? 20 : GetChannelWidth ();
  auto primaryBand = GetBand (primaryChannelWidth, GetPrimaryBandIndex (primaryChannelWidth));
  return m_state->GetState (primaryBand, GetCcaEdThreshold ());
}

void
WifiPhy::AbortCurrentReception (WifiPhyRxfailureReason reason)
{
  NS_LOG_FUNCTION (this << reason);
  for (auto & endPreambleDetectionEvent : m_endPreambleDetectionEvents)
    {
      endPreambleDetectionEvent.Cancel ();
    }
  m_endPreambleDetectionEvents.clear ();
  if (m_endPlcpRxEvent.IsRunning ())
    {
      m_endPlcpRxEvent.Cancel ();
    }
  for (auto & endRxEvent : m_endRxEvents)
    {
      endRxEvent.Cancel ();
    }
  m_endRxEvents.clear ();
  NotifyRxDrop (GetAddressedPsduInPpdu (m_currentEvent->GetPpdu ()), reason);
  m_interference.NotifyRxEnd (Simulator::Now ());
  bool is_failure = (reason != OBSS_PD_CCA_RESET);
  m_state->SwitchFromRxAbort (is_failure);
  for (auto it = m_currentPreambleEvents.begin (); it != m_currentPreambleEvents.end (); ++it)
    {
      if (it->second == m_currentEvent)
        {
          it = m_currentPreambleEvents.erase (it);
          break;
        }
    }
  m_currentEvent = 0;
}

void
WifiPhy::ResetCca (bool powerRestricted, double txPowerMaxSiso, double txPowerMaxMimo)
{
  NS_LOG_FUNCTION (this << powerRestricted << txPowerMaxSiso << txPowerMaxMimo);
  m_powerRestricted = powerRestricted;
  m_txPowerMaxSiso = txPowerMaxSiso;
  m_txPowerMaxMimo = txPowerMaxMimo;
  NS_ASSERT ((m_currentEvent->GetEndTime () - Simulator::Now ()).IsPositive ());
  Simulator::Schedule (m_currentEvent->GetEndTime () - Simulator::Now (), &WifiPhy::EndReceiveInterBss, this);
  AbortCurrentReception (OBSS_PD_CCA_RESET);
}

double
WifiPhy::GetTxPowerForTransmission (WifiTxVector txVector) const
{
  NS_LOG_FUNCTION (this << m_powerRestricted);
  if (!m_powerRestricted)
    {
      return GetPowerDbm (txVector.GetTxPowerLevel ());
    }
  else
    {
      if (txVector.GetNssMax () > 1)
        {
          return std::min (m_txPowerMaxMimo, GetPowerDbm (txVector.GetTxPowerLevel ()));
        }
      else
        {
          return std::min (m_txPowerMaxSiso, GetPowerDbm (txVector.GetTxPowerLevel ()));
        }
    }
}

void
WifiPhy::StartRx (Ptr<Event> event)
{
  double rxPowerW = event->GetRxPowerW ();
  NS_LOG_FUNCTION (this << *event << rxPowerW);

  NS_LOG_DEBUG ("sync to signal (power=" << rxPowerW << "W)");
  m_interference.NotifyRxStart (); //We need to notify it now so that it starts recording events

  m_endPreambleDetectionEvents.push_back (Simulator::Schedule (GetPreambleDetectionDuration (), &WifiPhy::StartReceiveHeader, this, event));
}

Ptr<const WifiPsdu>
WifiPhy::GetAddressedPsduInPpdu (Ptr<const WifiPpdu> ppdu) const
{
  Ptr<const WifiPsdu> psdu;
  if (!ppdu->IsMu ())
    {
      psdu = ppdu->GetPsdu ();
    }
  else if (ppdu->IsUlMu ())
    {
      uint8_t bssColor = 0;
      Ptr<WifiNetDevice> device = DynamicCast<WifiNetDevice> (GetDevice ());
      if (device)
        {
          Ptr<HeConfiguration> heConfiguration = device->GetHeConfiguration ();
          if (heConfiguration)
            {
              UintegerValue bssColorAttribute;
              heConfiguration->GetAttribute ("BssColor", bssColorAttribute);
              bssColor = bssColorAttribute.Get ();
            }
        }
      psdu = ppdu->GetPsdu (bssColor);
    }
  else //DL MU
    {
      uint8_t bssColor = 0;
      Ptr<WifiNetDevice> device = DynamicCast<WifiNetDevice> (GetDevice ());
      if (device)
        {
          Ptr<HeConfiguration> heConfiguration = device->GetHeConfiguration ();
          if (heConfiguration)
            {
              UintegerValue bssColorAttribute;
              heConfiguration->GetAttribute ("BssColor", bssColorAttribute);
              bssColor = bssColorAttribute.Get ();
            }
        }
      psdu = ppdu->GetPsdu (bssColor, GetStaId (ppdu));
    }
    return psdu;
}

uint16_t
WifiPhy::GetStaId (const Ptr<const WifiPpdu> ppdu) const
{
  uint16_t staId = SU_STA_ID;
  if (ppdu->IsUlMu ())
    {
      staId = ppdu->GetStaId ();
    }
  else if (ppdu->IsDlMu ())
    {
      Ptr<WifiNetDevice> device = DynamicCast<WifiNetDevice> (GetDevice ());
      if (device)
        {
          Ptr<StaWifiMac> mac = DynamicCast<StaWifiMac> (device->GetMac ());
          if (mac && mac->IsAssociated ())
            {
              return mac->GetAssociationId ();
            }
        }
    }
  return staId;
}

WifiSpectrumBand
WifiPhy::GetBand (uint16_t /*bandWidth*/, uint8_t /*bandIndex*/)
{
  WifiSpectrumBand band;
  band.first = 0;
  band.second = 0;
  return band;
}

uint8_t
WifiPhy::GetPrimaryBandIndex (uint16_t currentWidth) const
{
  uint8_t index = 0;
  uint16_t supportedWidth = GetChannelWidth ();
  if (currentWidth == supportedWidth)
    {
      return index;
    }
  if (supportedWidth == 40)
    {
      index = (GetPrimaryChannelNumber () <= GetChannelNumber ()) ? 0 : 1;
    }
  else if (supportedWidth == 80)
    {
      uint8_t channelNumber80 = GetChannelNumber ();
      uint8_t channelNumberLow40 = 0;
      uint8_t channelNumberHigh40 = 0;
      for (auto const& channelToFrequencyWidth : m_channelToFrequencyWidth)
        {
          if (channelToFrequencyWidth.second.second == 40)
            {
              if (channelToFrequencyWidth.first.first < channelNumber80)
                {
                  channelNumberLow40 = channelToFrequencyWidth.first.first;
                }
              else
                {
                  channelNumberHigh40 = channelToFrequencyWidth.first.first;
                  break;
                }
            }
        }
      NS_ASSERT (channelNumberLow40 != 0 && channelNumberHigh40 != 0);
      if (currentWidth == 40)
        {
          index = (GetPrimaryChannelNumber () <= GetChannelNumber ()) ? 0 : 1;
        }
      else if (currentWidth == 20)
        {
          if (GetPrimaryChannelNumber () <= channelNumberLow40)
            {
              index = 0;
            }
          else if (GetPrimaryChannelNumber () <= channelNumber80)
            {
              index = 1;
            }
          else if (GetPrimaryChannelNumber () <= channelNumberHigh40)
            {
              index = 2;
            }
          else
            {
              index = 3;
            }
        }
    }
  else if (supportedWidth == 160)
    {
      uint8_t channelNumber160 = GetChannelNumber ();
      uint8_t channelNumberLow80 = 0;
      uint8_t channelNumberHigh80 = 0;
      for (auto const& channelToFrequencyWidth : m_channelToFrequencyWidth)
        {
          if (channelToFrequencyWidth.second.second == 80)
            {
              if (channelToFrequencyWidth.first.first < channelNumber160)
                {
                  channelNumberLow80 = channelToFrequencyWidth.first.first;
                }
              else
                {
                  channelNumberHigh80 = channelToFrequencyWidth.first.first;
                  break;
                }
            }
        }
      NS_ASSERT (channelNumberLow80 != 0 && channelNumberHigh80 != 0);
      uint8_t channelNumberLow80Low40 = 0;
      uint8_t channelNumberLow80High40 = 0;
      uint8_t channelNumberHigh80Low40 = 0;
      uint8_t channelNumberHigh80High40 = 0;
      for (auto const& channelToFrequencyWidth : m_channelToFrequencyWidth)
        {
          if (channelToFrequencyWidth.second.second == 40)
            {
              if (channelToFrequencyWidth.first.first < channelNumberLow80)
                {
                  channelNumberLow80Low40 = channelToFrequencyWidth.first.first;
                }
              else if (channelNumberLow80High40 == 0)
                {
                  channelNumberLow80High40 = channelToFrequencyWidth.first.first;
                }
              if (channelToFrequencyWidth.first.first < channelNumberHigh80)
                {
                  channelNumberHigh80Low40 = channelToFrequencyWidth.first.first;
                }
              else if (channelNumberHigh80High40 == 0)
                {
                  channelNumberHigh80High40 = channelToFrequencyWidth.first.first;
                }
            }
        }
      NS_ASSERT (channelNumberLow80Low40 != 0 && channelNumberLow80High40 != 0 && channelNumberHigh80Low40 != 0 && channelNumberHigh80High40 != 0);
      if (currentWidth == 80)
        {
          index = (GetPrimaryChannelNumber () <= GetChannelNumber ()) ? 0 : 1;
        }
      else if (currentWidth == 40)
        {
          if (GetPrimaryChannelNumber () <= channelNumberLow80)
            {
              index = 0;
            }
          else if (GetPrimaryChannelNumber () <= channelNumber160)
            {
              index = 1;
            }
          else if (GetPrimaryChannelNumber () <= channelNumberHigh80)
            {
              index = 2;
            }
          else
            {
              index = 3;
            }
        }
      else if (currentWidth == 20)
        {
          if (GetPrimaryChannelNumber () <= channelNumberLow80Low40)
            {
              index = 0;
            }
          else if (GetPrimaryChannelNumber () <= channelNumberLow80)
            {
              index = 1;
            }
          else if (GetPrimaryChannelNumber () <= channelNumberLow80High40)
            {
              index = 2;
            }
          else if (GetPrimaryChannelNumber () <= channelNumber160)
            {
              index = 3;
            }
          else if (GetPrimaryChannelNumber () <= channelNumberHigh80Low40)
            {
              index = 4;
            }
          else if (GetPrimaryChannelNumber () <= channelNumberHigh80)
            {
              index = 5;
            }
          else if (GetPrimaryChannelNumber () <= channelNumberHigh80High40)
            {
              index = 6;
            }
          else
            {
              index = 7;
            }
        }
    }
  return index;
}

uint16_t
WifiPhy::ConvertHeTbPpduDurationToLSigLength (Time ppduDuration, uint16_t frequency)
{
  uint8_t sigExtension = 0;
  if (Is2_4Ghz (frequency))
    {
      sigExtension = 6;
    }
  uint8_t m = 2; //HE TB PPDU so m is set to 2
  uint16_t length = ((ceil ((static_cast<double> (ppduDuration.GetNanoSeconds () - (20 * 1000) - (sigExtension * 1000)) / 1000) / 4.0) * 3) - 3 - m);
  return length;
}

Time
WifiPhy::ConvertLSigLengthToHeTbPpduDuration (uint16_t length, WifiTxVector txVector, uint16_t frequency)
{
  Time tSymbol = NanoSeconds (12800 + txVector.GetGuardInterval ());
  Time preambleDuration = CalculatePlcpPreambleAndHeaderDuration (txVector);
  uint8_t sigExtension = 0;
  if (Is2_4Ghz (frequency))
    {
      sigExtension = 6;
    }
  uint8_t m = 2; //HE TB PPDU so m is set to 2
  //Equation 27-11 of IEEE P802.11ax/D4.0
  Time calculatedDuration = MicroSeconds (((ceil (static_cast<double> (length + 3 + m) / 3)) * 4) + 20 + sigExtension);
  uint32_t nSymbols = floor (static_cast<double> ((calculatedDuration - preambleDuration).GetNanoSeconds () - (sigExtension * 1000)) / tSymbol.GetNanoSeconds ());
  Time ppduDuration = preambleDuration + (nSymbols * tSymbol) + MicroSeconds (sigExtension);
  return ppduDuration;
}

int64_t
WifiPhy::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_random->SetStream (stream);
  return 1;
}

std::ostream& operator<< (std::ostream& os, WifiPhyState state)
{
  switch (state)
    {
    case WifiPhyState::IDLE:
      return (os << "IDLE");
    case WifiPhyState::CCA_BUSY:
      return (os << "CCA_BUSY");
    case WifiPhyState::TX:
      return (os << "TX");
    case WifiPhyState::RX:
      return (os << "RX");
    case WifiPhyState::SWITCHING:
      return (os << "SWITCHING");
    case WifiPhyState::SLEEP:
      return (os << "SLEEP");
    case WifiPhyState::OFF:
      return (os << "OFF");
    default:
      NS_FATAL_ERROR ("Invalid WifiPhy state");
      return (os << "INVALID");
    }
}

std::ostream& operator<< (std::ostream& os, RxSignalInfo rxSignalInfo)
{
  os << "SNR:" << RatioToDb (rxSignalInfo.snr) << " dB"
     << ", RSSI:" << rxSignalInfo.rssi << " dBm";
  return os;
}

} //namespace ns3

namespace {

/**
 * Constructor class
 */
static class Constructor
{
public:
  Constructor ()
  {
    ns3::WifiPhy::GetDsssRate1Mbps ();
    ns3::WifiPhy::GetDsssRate2Mbps ();
    ns3::WifiPhy::GetDsssRate5_5Mbps ();
    ns3::WifiPhy::GetDsssRate11Mbps ();
    ns3::WifiPhy::GetErpOfdmRate6Mbps ();
    ns3::WifiPhy::GetErpOfdmRate9Mbps ();
    ns3::WifiPhy::GetErpOfdmRate12Mbps ();
    ns3::WifiPhy::GetErpOfdmRate18Mbps ();
    ns3::WifiPhy::GetErpOfdmRate24Mbps ();
    ns3::WifiPhy::GetErpOfdmRate36Mbps ();
    ns3::WifiPhy::GetErpOfdmRate48Mbps ();
    ns3::WifiPhy::GetErpOfdmRate54Mbps ();
    ns3::WifiPhy::GetOfdmRate6Mbps ();
    ns3::WifiPhy::GetOfdmRate9Mbps ();
    ns3::WifiPhy::GetOfdmRate12Mbps ();
    ns3::WifiPhy::GetOfdmRate18Mbps ();
    ns3::WifiPhy::GetOfdmRate24Mbps ();
    ns3::WifiPhy::GetOfdmRate36Mbps ();
    ns3::WifiPhy::GetOfdmRate48Mbps ();
    ns3::WifiPhy::GetOfdmRate54Mbps ();
    ns3::WifiPhy::GetOfdmRate3MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate4_5MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate6MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate9MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate12MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate18MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate24MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate27MbpsBW10MHz ();
    ns3::WifiPhy::GetOfdmRate1_5MbpsBW5MHz ();
    ns3::WifiPhy::GetOfdmRate2_25MbpsBW5MHz ();
    ns3::WifiPhy::GetOfdmRate3MbpsBW5MHz ();
    ns3::WifiPhy::GetOfdmRate4_5MbpsBW5MHz ();
    ns3::WifiPhy::GetOfdmRate6MbpsBW5MHz ();
    ns3::WifiPhy::GetOfdmRate9MbpsBW5MHz ();
    ns3::WifiPhy::GetOfdmRate12MbpsBW5MHz ();
    ns3::WifiPhy::GetOfdmRate13_5MbpsBW5MHz ();
    ns3::WifiPhy::GetHtMcs0 ();
    ns3::WifiPhy::GetHtMcs1 ();
    ns3::WifiPhy::GetHtMcs2 ();
    ns3::WifiPhy::GetHtMcs3 ();
    ns3::WifiPhy::GetHtMcs4 ();
    ns3::WifiPhy::GetHtMcs5 ();
    ns3::WifiPhy::GetHtMcs6 ();
    ns3::WifiPhy::GetHtMcs7 ();
    ns3::WifiPhy::GetHtMcs8 ();
    ns3::WifiPhy::GetHtMcs9 ();
    ns3::WifiPhy::GetHtMcs10 ();
    ns3::WifiPhy::GetHtMcs11 ();
    ns3::WifiPhy::GetHtMcs12 ();
    ns3::WifiPhy::GetHtMcs13 ();
    ns3::WifiPhy::GetHtMcs14 ();
    ns3::WifiPhy::GetHtMcs15 ();
    ns3::WifiPhy::GetHtMcs16 ();
    ns3::WifiPhy::GetHtMcs17 ();
    ns3::WifiPhy::GetHtMcs18 ();
    ns3::WifiPhy::GetHtMcs19 ();
    ns3::WifiPhy::GetHtMcs20 ();
    ns3::WifiPhy::GetHtMcs21 ();
    ns3::WifiPhy::GetHtMcs22 ();
    ns3::WifiPhy::GetHtMcs23 ();
    ns3::WifiPhy::GetHtMcs24 ();
    ns3::WifiPhy::GetHtMcs25 ();
    ns3::WifiPhy::GetHtMcs26 ();
    ns3::WifiPhy::GetHtMcs27 ();
    ns3::WifiPhy::GetHtMcs28 ();
    ns3::WifiPhy::GetHtMcs29 ();
    ns3::WifiPhy::GetHtMcs30 ();
    ns3::WifiPhy::GetHtMcs31 ();
    ns3::WifiPhy::GetVhtMcs0 ();
    ns3::WifiPhy::GetVhtMcs1 ();
    ns3::WifiPhy::GetVhtMcs2 ();
    ns3::WifiPhy::GetVhtMcs3 ();
    ns3::WifiPhy::GetVhtMcs4 ();
    ns3::WifiPhy::GetVhtMcs5 ();
    ns3::WifiPhy::GetVhtMcs6 ();
    ns3::WifiPhy::GetVhtMcs7 ();
    ns3::WifiPhy::GetVhtMcs8 ();
    ns3::WifiPhy::GetVhtMcs9 ();
    ns3::WifiPhy::GetHeMcs0 ();
    ns3::WifiPhy::GetHeMcs1 ();
    ns3::WifiPhy::GetHeMcs2 ();
    ns3::WifiPhy::GetHeMcs3 ();
    ns3::WifiPhy::GetHeMcs4 ();
    ns3::WifiPhy::GetHeMcs5 ();
    ns3::WifiPhy::GetHeMcs6 ();
    ns3::WifiPhy::GetHeMcs7 ();
    ns3::WifiPhy::GetHeMcs8 ();
    ns3::WifiPhy::GetHeMcs9 ();
    ns3::WifiPhy::GetHeMcs10 ();
    ns3::WifiPhy::GetHeMcs11 ();
  }
} g_constructor; ///< the constructor

}

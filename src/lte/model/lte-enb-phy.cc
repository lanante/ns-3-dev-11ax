/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 TELEMATICS LAB, DEE - Politecnico di Bari
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
 * Author: Giuseppe Piro  <g.piro@poliba.it>
 *         Marco Miozzo <mmiozzo@cttc.es>
 */

#include <ns3/object-factory.h>
#include <ns3/log.h>
#include <cfloat>
#include <cmath>
#include <fstream>
#include <ns3/simulator.h>
#include <ns3/attribute-accessor-helper.h>
#include <ns3/double.h>


#include "lte-enb-phy.h"
#include "lte-ue-phy.h"
#include "lte-net-device.h"
#include "lte-spectrum-value-helper.h"
#include "lte-control-messages.h"
#include "lte-enb-net-device.h"
#include "lte-ue-rrc.h"
#include "lte-enb-mac.h"
#include <ns3/lte-common.h>
#include <ns3/lte-vendor-specific-parameters.h>

// WILD HACK for the inizialization of direct eNB-UE ctrl messaging
#include <ns3/node-list.h>
#include <ns3/node.h>
#include <ns3/lte-ue-net-device.h>
#include <ns3/pointer.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteEnbPhy");

NS_OBJECT_ENSURE_REGISTERED (LteEnbPhy);

/**
 * Duration of the data portion of a DL subframe.
 * Equals to "TTI length * (11/14) - margin".
 * Data portion is fixed to 11 symbols out of the available 14 symbols.
 * 1 nanosecond margin is added to avoid overlapping simulator events.
 */
static const Time DL_DATA_DURATION = NanoSeconds (785714 -1);

/**
 * Delay from the start of a DL subframe to transmission of the data portion.
 * Equals to "TTI length * (3/14)".
 * Control portion is fixed to 3 symbols out of the available 14 symbols.
 */
static const Time DL_CTRL_DELAY_FROM_SUBFRAME_START = NanoSeconds (214286);

////////////////////////////////////////
// member SAP forwarders
////////////////////////////////////////

/// \todo SetBandwidth() and SetCellId() can be removed.
class EnbMemberLteEnbPhySapProvider : public LteEnbPhySapProvider
{
public:
  /**
   * Constructor
   *
   * \param phy the ENB Phy
   */
  EnbMemberLteEnbPhySapProvider (LteEnbPhy* phy);

  // inherited from LteEnbPhySapProvider
  virtual void SendMacPdu (Ptr<Packet> p);
  virtual void SendLteControlMessage (Ptr<LteControlMessage> msg);
  virtual uint8_t GetMacChTtiDelay ();
  /**
   * Set bandwidth function
   *
   * \param ulBandwidth the UL bandwidth
   * \param dlBandwidth the DL bandwidth
   */
  virtual void SetBandwidth (uint8_t ulBandwidth, uint8_t dlBandwidth);
  /**
   * Set Cell ID function
   *
   * \param cellId the cell ID
   */
  virtual void SetCellId (uint16_t cellId);


private:
  LteEnbPhy* m_phy; ///< the ENB Phy
};

EnbMemberLteEnbPhySapProvider::EnbMemberLteEnbPhySapProvider (LteEnbPhy* phy) : m_phy (phy)
{

}

void
EnbMemberLteEnbPhySapProvider::SendMacPdu (Ptr<Packet> p)
{
  m_phy->DoSendMacPdu (p);
}

void
EnbMemberLteEnbPhySapProvider::SetBandwidth (uint8_t ulBandwidth, uint8_t dlBandwidth)
{
  m_phy->DoSetBandwidth (ulBandwidth, dlBandwidth);
}

void
EnbMemberLteEnbPhySapProvider::SetCellId (uint16_t cellId)
{
  m_phy->DoSetCellId (cellId);
}

void
EnbMemberLteEnbPhySapProvider::SendLteControlMessage (Ptr<LteControlMessage> msg)
{
  m_phy->DoSendLteControlMessage (msg);
}

uint8_t
EnbMemberLteEnbPhySapProvider::GetMacChTtiDelay ()
{
  return (m_phy->DoGetMacChTtiDelay ());
}


////////////////////////////////////////
// generic LteEnbPhy methods
////////////////////////////////////////



LteEnbPhy::LteEnbPhy ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("This constructor should not be called");
}

LteEnbPhy::LteEnbPhy (Ptr<LteSpectrumPhy> dlPhy, Ptr<LteSpectrumPhy> ulPhy)
  : LtePhy (dlPhy, ulPhy),
    m_enbPhySapUser (0),
    m_enbCphySapUser (0),
    m_nrFrames (0),
    m_nrSubFrames (0),
    m_srsPeriodicity (0),
    m_srsStartTime (Seconds (0)),
    m_currentSrsOffset (0),
    m_interferenceSampleCounter (0),
    m_grantTimeout (Seconds(0)),
    m_isWaitingForChannelAccessGrant (false)
{
  m_enbPhySapProvider = new EnbMemberLteEnbPhySapProvider (this);
  m_enbCphySapProvider = new MemberLteEnbCphySapProvider<LteEnbPhy> (this);
  m_harqPhyModule = Create <LteHarqPhy> ();
  m_downlinkSpectrumPhy->SetHarqPhyModule (m_harqPhyModule);
  m_uplinkSpectrumPhy->SetHarqPhyModule (m_harqPhyModule);
  m_reservationSignal = false;
  m_channelAccessManager = 0;
  m_ctrlMsgCounter = 0;
  m_drsReservationSignal = false;
}

TypeId
LteEnbPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LteEnbPhy")
    .SetParent<LtePhy> ()
    .SetGroupName("Lte")
    .AddConstructor<LteEnbPhy> ()
    .AddAttribute ("TxPower",
                   "Transmission power in dBm",
                   DoubleValue (30.0),
                   MakeDoubleAccessor (&LteEnbPhy::SetTxPower, 
                                       &LteEnbPhy::GetTxPower),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("NoiseFigure",
                   "Loss (dB) in the Signal-to-Noise-Ratio due to "
                   "non-idealities in the receiver.  According to Wikipedia "
                   "(http://en.wikipedia.org/wiki/Noise_figure), this is "
                   "\"the difference in decibels (dB) between"
                   " the noise output of the actual receiver to "
                   "the noise output of an ideal receiver with "
                   "the same overall gain and bandwidth when the receivers "
                   "are connected to sources at the standard noise "
                   "temperature T0.\"  In this model, we consider T0 = 290K.",
                   DoubleValue (5.0),
                   MakeDoubleAccessor (&LteEnbPhy::SetNoiseFigure, 
                                       &LteEnbPhy::GetNoiseFigure),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MacToChannelDelay",
                   "The delay in TTI units that occurs between "
                   "a scheduling decision in the MAC and the actual "
                   "start of the transmission by the PHY. This is "
                   "intended to be used to model the latency of real PHY "
                   "and MAC implementations.",
                   UintegerValue (2),
                   MakeUintegerAccessor (&LteEnbPhy::SetMacChDelay, 
                                         &LteEnbPhy::GetMacChDelay),
                   MakeUintegerChecker<uint8_t> ())
    .AddTraceSource ("ReportUeSinr",
                     "Report UEs' averaged linear SINR",
                     MakeTraceSourceAccessor (&LteEnbPhy::m_reportUeSinr),
                     "ns3::LteEnbPhy::ReportUeSinrTracedCallback")
    .AddAttribute ("UeSinrSamplePeriod",
                   "The sampling period for reporting UEs' SINR stats.",
                   UintegerValue (1),  /// \todo In what unit is this?
                   MakeUintegerAccessor (&LteEnbPhy::m_srsSamplePeriod),
                   MakeUintegerChecker<uint16_t> ())
    .AddTraceSource ("ReportInterference",
                     "Report linear interference power per PHY RB",
                     MakeTraceSourceAccessor (&LteEnbPhy::m_reportInterferenceTrace),
                     "ns3::LteEnbPhy::ReportInterferenceTracedCallback")
    .AddAttribute ("InterferenceSamplePeriod",
                   "The sampling period for reporting interference stats",
                   UintegerValue (1),  /// \todo In what unit is this?
                   MakeUintegerAccessor (&LteEnbPhy::m_interferenceSamplePeriod),
                   MakeUintegerChecker<uint16_t> ())
    .AddTraceSource ("DlPhyTransmission",
                     "DL transmission PHY layer statistics.",
                     MakeTraceSourceAccessor (&LteEnbPhy::m_dlPhyTransmission),
                     "ns3::PhyTransmissionStatParameters::TracedCallback")
    .AddAttribute ("DlSpectrumPhy",
                   "The downlink LteSpectrumPhy associated to this LtePhy",
                   TypeId::ATTR_GET,
                   PointerValue (),
                   MakePointerAccessor (&LteEnbPhy::GetDlSpectrumPhy),
                   MakePointerChecker <LteSpectrumPhy> ())
    .AddAttribute ("UlSpectrumPhy",
                   "The uplink LteSpectrumPhy associated to this LtePhy",
                   TypeId::ATTR_GET,
                   PointerValue (),
                   MakePointerAccessor (&LteEnbPhy::GetUlSpectrumPhy),
                   MakePointerChecker <LteSpectrumPhy> ())
    .AddAttribute ("MibPeriod",
                   "The period for sending MIB, number of TTIs between two consecutive mib messages",
                   UintegerValue (10),
                   MakeUintegerAccessor (&LteEnbPhy::m_mibPeriod),
                   MakeUintegerChecker<uint16_t> (10,160))
    .AddAttribute ("SibPeriod",
                   "The period for sending SIB, number of TTIs between two consecutive sib messages",
                   UintegerValue (20),
                   MakeUintegerAccessor (&LteEnbPhy::m_sibPeriod),
                   MakeUintegerChecker<uint16_t> (20,160))
    .AddAttribute ("DrsPeriod",
                   "The period for sending DRS, number of TTIs between two consecutive drs messages",
                   UintegerValue (40),
                   MakeUintegerAccessor (&LteEnbPhy::m_drsPeriod),
                   MakeUintegerChecker<uint16_t> (40,160))
    .AddAttribute ("DrsMessagesEnabled",
                   "If true generate and send drs messages, "
                   "if false do not generate drs messages",
                   BooleanValue (false),
                   MakeBooleanAccessor (&LteEnbPhy::m_drsMessagesEnabled),
                   MakeBooleanChecker ())
    .AddAttribute ("DisableMibAndSibStartupTime",
                   "Disable MIB and SIB messages after the provided time, "
                   "set to 0 to disable this feature",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&LteEnbPhy::m_disableMibAndSibStartupTime),
                   MakeTimeChecker ())
    .AddAttribute ("SaveCtrlAndRbStats",
                   "If true, generate and save to file statistics about ctrl messages and resource block usage at in dispose function, "
                   "if false, do not generate statistics",
                   BooleanValue (false),
                   MakeBooleanAccessor (&LteEnbPhy::m_generateCtrlAndRbStats),
                   MakeBooleanChecker ())
    .AddAttribute ("ImplWith2msDelay",
                   "It is meant to be used when channel access manager is set;"
                   "if true and channel access manager is set, scheduler will be stopped until channel access grant is obtained, thus there will be 2ms of delay between start of scheduling and transmitting (mac to phy delay)"
                   "if false and channel access manager is set, scheduler will schedule every subframe, but the data will be transmitted only when there is channel access.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&LteEnbPhy::m_channelAccessImplWith2msDelay),
                   MakeBooleanChecker ())
    .AddAttribute ("DropPackets",
                   "Meant to be used only when channel access manager is set and ChannelAccessImplWith2msDelay is set to false;"
                   "if true, drop packets when there is no channel access"
                   "if false save packets while channel access is not granted",
                   BooleanValue (false),
                   MakeBooleanAccessor (&LteEnbPhy::m_dropPackets),
                   MakeBooleanChecker ())
    .AddAttribute ("ChannelAccessManagerStartTime",
                   "Time at which will start to use channel access manager if available.",
                   TimeValue (Seconds (2)),
                   MakeTimeAccessor (&LteEnbPhy::m_channelAccessManagerStartTime),
                   MakeTimeChecker ())
    .AddTraceSource ("ReservationSignal",
                     "Reports that transmission of reservation signal started and it reports its duration",
                     MakeTraceSourceAccessor (&LteEnbPhy::m_reservationSignalTraceSource),
                     "ns3::LteEnbPhy::ReportReservationSignalTracedCallback")
    .AddTraceSource ("DataSent",
                     "Reports how many bytes was transmited.",
                     MakeTraceSourceAccessor (&LteEnbPhy::m_data),
                     "ns3::LteEnbPhy::DataTracedCallback")
    .AddTraceSource ("Txop",
                     "Reports that txop started and it reports its duration",
                     MakeTraceSourceAccessor (&LteEnbPhy::m_txopTraceSource),
                     "ns3::LteEnbPhy::ReportTxopTracedCallback")
    .AddTraceSource ("CtrlMsgTransmission",
                     "Control message transmission traces.",
                     MakeTraceSourceAccessor (&LteEnbPhy::m_ctrlMsgTransmission),
                     "ns3::CtrlMsgTransmissionTrace::TracedCallback")
  ;
  return tid;
}

std::string LteEnbPhy::PrintMessageType (int i)
{
  switch (i)
    {
    case LteControlMessage::DL_DCI: return "DL DCI";
    case LteControlMessage::UL_DCI: return "UL DCI";
    case LteControlMessage::DL_CQI: return "DL CQI";
    case LteControlMessage::UL_CQI: return "UL CQI";
    case LteControlMessage::BSR: return "BSR";
    case LteControlMessage::DL_HARQ: return "DL HARQ";
    case LteControlMessage::RACH_PREAMBLE: return "RACH_PREAMBLE";
    case LteControlMessage::RAR: return "RAR";
    case LteControlMessage::MIB: return "MIB";
    case LteControlMessage::SIB1: return "SIB1";
    case LteControlMessage::DRS: return "DRS";
    }
  return "";
};


LteEnbPhy::~LteEnbPhy ()
{
}

void
LteEnbPhy::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  if (m_generateCtrlAndRbStats)
    {
      // save statistics regarding ctrl messages, type and count
      std::ofstream outFileCtrlMessages;

      std::ostringstream outstring;
      outstring << "ctrl_messages" << m_cellId << ".txt";
      outFileCtrlMessages.open (outstring.str ().c_str (), std::ofstream::out | std::ofstream::app);
      outFileCtrlMessages.setf (std::ios_base::fixed);
      if (!outFileCtrlMessages.is_open ())
        {
          NS_LOG_ERROR ("Can't open file to save lte-enb-phy.cc ctrl messages stats");
          return;
        }
      outFileCtrlMessages << "Number of subframes used to send all CTRL messages:" << m_ctrlMsgCounter << " " << std::endl;
      for(std::map<int, int>::iterator i = m_ctrlTypesCount.begin (); i != m_ctrlTypesCount.end (); i++)
        {
          outFileCtrlMessages << PrintMessageType (i->first) <<  " ";
          outFileCtrlMessages << i->second << " " << std::endl;
        }
      outFileCtrlMessages.close ();

      // save statistics regarding resource block occupancy
      std::ofstream outFileRbOccupancy;
      std::ostringstream outstringRb;
      outstringRb << "rb_occupancy.txt" << m_cellId << ".txt";
      outFileRbOccupancy.open (outstringRb.str ().c_str (), std::ofstream::out | std::ofstream::app);
      outFileRbOccupancy.setf (std::ios_base::fixed);

      if (!outFileRbOccupancy.is_open ())
        {
          NS_LOG_ERROR ("Can't open file to save lte-enb-phy.cc resource blocks usage stats");
          return;
        }

      for (std::map<Time, double>::iterator it = m_logTimeToRbUsage.begin (); it != m_logTimeToRbUsage.end (); it++)
        {
          outFileRbOccupancy << it->first.GetSeconds () <<  " ";
          outFileRbOccupancy << (uint32_t)it->second << " " << std::endl;
        }
      outFileRbOccupancy.close ();
    }

  m_ueAttached.clear ();
  m_srsUeOffset.clear ();
  delete m_enbPhySapProvider;
  delete m_enbCphySapProvider;
  LtePhy::DoDispose ();
}

void
LteEnbPhy::DoInitialize ()
{
  NS_LOG_FUNCTION (this);

  NS_ABORT_MSG_IF (m_netDevice == nullptr, "LteEnbDevice is not available in LteEnbPhy");
  Ptr<Node> node = m_netDevice->GetNode ();
  NS_ABORT_MSG_IF (node == nullptr, "Node is not available in the LteNetDevice of LteEnbPhy");
  uint32_t nodeId = node->GetId ();

  //ScheduleWithContext() is needed here to set context for logs,
  //because Initialize() is called outside of Node::AddDevice().

  Simulator::ScheduleWithContext (nodeId, Seconds (0), &LteEnbPhy::StartFrame, this);

  Ptr<SpectrumValue> noisePsd = LteSpectrumValueHelper::CreateNoisePowerSpectralDensity (m_ulEarfcn, m_ulBandwidth, m_noiseFigure);
  m_uplinkSpectrumPhy->SetNoisePowerSpectralDensity (noisePsd);
  LtePhy::DoInitialize ();
}


void
LteEnbPhy::SetLteEnbPhySapUser (LteEnbPhySapUser* s)
{
  m_enbPhySapUser = s;
}

LteEnbPhySapProvider*
LteEnbPhy::GetLteEnbPhySapProvider ()
{
  return (m_enbPhySapProvider);
}

void
LteEnbPhy::SetLteEnbCphySapUser (LteEnbCphySapUser* s)
{
  NS_LOG_FUNCTION (this);
  m_enbCphySapUser = s;
}

LteEnbCphySapProvider*
LteEnbPhy::GetLteEnbCphySapProvider ()
{
  NS_LOG_FUNCTION (this);
  return (m_enbCphySapProvider);
}

void
LteEnbPhy::SetTxPower (double pow)
{
  NS_LOG_FUNCTION (this << pow);
  m_txPower = pow;
}

double
LteEnbPhy::GetTxPower () const
{
  NS_LOG_FUNCTION (this);
  return m_txPower;
}

int8_t
LteEnbPhy::DoGetReferenceSignalPower () const
{
  NS_LOG_FUNCTION (this);
  return m_txPower;
}

void
LteEnbPhy::SetNoiseFigure (double nf)
{
  NS_LOG_FUNCTION (this << nf);
  m_noiseFigure = nf;
}

double
LteEnbPhy::GetNoiseFigure () const
{
  NS_LOG_FUNCTION (this);
  return m_noiseFigure;
}

void
LteEnbPhy::SetMacChDelay (uint8_t delay)
{
  NS_LOG_FUNCTION (this);
  m_macChTtiDelay = delay;
  for (int i = 0; i < m_macChTtiDelay; i++)
    {
      Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
      m_packetBurstQueue.push_back (pb);
      std::list<Ptr<LteControlMessage> > l;
      m_controlMessagesQueue.push_back (l);
      std::list<UlDciLteControlMessage> l1;
      m_ulDciQueue.push_back (l1);
    }
  for (int i = 0; i < UL_PUSCH_TTIS_DELAY; i++)
    {
      std::list<UlDciLteControlMessage> l1;
      m_ulDciQueue.push_back (l1);
    }
}

uint8_t
LteEnbPhy::GetMacChDelay (void) const
{
  return (m_macChTtiDelay);
}

Ptr<LteSpectrumPhy>
LteEnbPhy::GetDlSpectrumPhy () const
{
  return m_downlinkSpectrumPhy;
}

Ptr<LteSpectrumPhy>
LteEnbPhy::GetUlSpectrumPhy () const
{
  return m_uplinkSpectrumPhy;
}

void
LteEnbPhy::SetUseReservationSignal (bool useReservationSignal)
{
  NS_LOG_FUNCTION (this << useReservationSignal);
  m_reservationSignal = useReservationSignal;
}

Time
LteEnbPhy::GetGrantTimeout () const
{
  return m_grantTimeout;
}

void
LteEnbPhy::SetChannelAccessManager (Ptr<LteChannelAccessManager> channelAccessManager)
{
  NS_LOG_FUNCTION (this);
  //TODO before changing channel access manager take the stateof the previous, e.g. channel access grant
  m_channelAccessManager = channelAccessManager;
  m_channelAccessManager->SetAccessGrantedCallback (MakeCallback (&LteEnbPhy::ReceiveAccessGranted, this));
  //RequestChannelAccess();
}

Ptr<LteChannelAccessManager>
LteEnbPhy::GetChannelAccessManager ()
{
  return m_channelAccessManager;
}

bool
LteEnbPhy::AddUePhy (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);
  std::set <uint16_t>::iterator it;
  it = m_ueAttached.find (rnti);
  if (it == m_ueAttached.end ())
    {
      m_ueAttached.insert (rnti);
      return (true);
    }
  else
    {
      NS_LOG_ERROR ("UE already attached");
      return (false);
    }
}

bool
LteEnbPhy::DeleteUePhy (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);
  std::set <uint16_t>::iterator it;
  it = m_ueAttached.find (rnti);
  if (it == m_ueAttached.end ())
    {
      NS_LOG_ERROR ("UE not attached");
      return (false);
    }
  else
    {
      m_ueAttached.erase (it);
      return (true);
    }
}



void
LteEnbPhy::DoSendMacPdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);
  SetMacPdu (p);
}

uint8_t
LteEnbPhy::DoGetMacChTtiDelay ()
{
  return (m_macChTtiDelay);
}


void
LteEnbPhy::PhyPduReceived (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);
  m_enbPhySapUser->ReceivePhyPdu (p);
}

void
LteEnbPhy::SetDownlinkSubChannels (std::vector<int> mask)
{
  NS_LOG_FUNCTION (this);
  m_listOfDownlinkSubchannel = mask;
  Ptr<SpectrumValue> txPsd = CreateTxPowerSpectralDensity ();
  m_downlinkSpectrumPhy->SetTxPowerSpectralDensity (txPsd);
}

void
LteEnbPhy::SetDownlinkSubChannelsWithPowerAllocation (std::vector<int> mask)
{
  NS_LOG_FUNCTION (this);
  m_listOfDownlinkSubchannel = mask;
  Ptr<SpectrumValue> txPsd = CreateTxPowerSpectralDensityWithPowerAllocation ();
  m_downlinkSpectrumPhy->SetTxPowerSpectralDensity (txPsd);
}

std::vector<int>
LteEnbPhy::GetDownlinkSubChannels (void)
{
  NS_LOG_FUNCTION (this);
  return m_listOfDownlinkSubchannel;
}

void
LteEnbPhy::GeneratePowerAllocationMap (uint16_t rnti, int rbId)
{
  NS_LOG_FUNCTION (this);
  double rbgTxPower = m_txPower;

  std::map<uint16_t, double>::iterator it = m_paMap.find (rnti);
  if (it != m_paMap.end ())
    {
      rbgTxPower = m_txPower + it->second;
    }

  m_dlPowerAllocationMap.insert (std::pair<int, double> (rbId, rbgTxPower));
}

Ptr<SpectrumValue>
LteEnbPhy::CreateTxPowerSpectralDensity ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("TX Power: " << m_txPower);

  Ptr<SpectrumValue> psd = LteSpectrumValueHelper::CreateTxPowerSpectralDensity (m_dlEarfcn, m_dlBandwidth, m_txPower, GetDownlinkSubChannels ());

  return psd;
}

Ptr<SpectrumValue>
LteEnbPhy::CreateTxPowerSpectralDensityWithPowerAllocation ()
{
  NS_LOG_FUNCTION (this);

  Ptr<SpectrumValue> psd = LteSpectrumValueHelper::CreateTxPowerSpectralDensity (m_dlEarfcn, m_dlBandwidth, m_txPower, m_dlPowerAllocationMap, GetDownlinkSubChannels ());

  return psd;
}


void
LteEnbPhy::CalcChannelQualityForUe (std::vector <double> sinr, Ptr<LteSpectrumPhy> ue)
{
  NS_LOG_FUNCTION (this);
}


void
LteEnbPhy::DoSendLteControlMessage (Ptr<LteControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);
  // queues the message (wait for MAC-PHY delay)
  SetControlMessages (msg);
}



void
LteEnbPhy::ReceiveLteControlMessage (Ptr<LteControlMessage> msg)
{
  NS_FATAL_ERROR ("Obsolete function");
  NS_LOG_FUNCTION (this << msg);
  m_enbPhySapUser->ReceiveLteControlMessage (msg);
}

void
LteEnbPhy::ReceiveLteControlMessageList (std::list<Ptr<LteControlMessage> > msgList)
{
  NS_LOG_FUNCTION (this);
  std::list<Ptr<LteControlMessage> >::iterator it;
  for (it = msgList.begin (); it != msgList.end (); it++)
    {
      switch ((*it)->GetMessageType ())
        {
        case LteControlMessage::RACH_PREAMBLE:
          {
            Ptr<RachPreambleLteControlMessage> rachPreamble = DynamicCast<RachPreambleLteControlMessage> (*it);
            m_enbPhySapUser->ReceiveRachPreamble (rachPreamble->GetRapId ());
          }
          break;
        case LteControlMessage::DL_CQI:
          {
            Ptr<DlCqiLteControlMessage> dlcqiMsg = DynamicCast<DlCqiLteControlMessage> (*it);
            CqiListElement_s dlcqi = dlcqiMsg->GetDlCqi ();
            // check whether the UE is connected
            if (m_ueAttached.find (dlcqi.m_rnti) != m_ueAttached.end ())
              {
                m_enbPhySapUser->ReceiveLteControlMessage (*it);
              }
          }
          break;
        case LteControlMessage::BSR:
          {
            Ptr<BsrLteControlMessage> bsrMsg = DynamicCast<BsrLteControlMessage> (*it);
            MacCeListElement_s bsr = bsrMsg->GetBsr ();
            // check whether the UE is connected
            if (m_ueAttached.find (bsr.m_rnti) != m_ueAttached.end ())
              {
                m_enbPhySapUser->ReceiveLteControlMessage (*it);
              }
          }
          break;
        case LteControlMessage::DL_HARQ:
          {
            Ptr<DlHarqFeedbackLteControlMessage> dlharqMsg = DynamicCast<DlHarqFeedbackLteControlMessage> (*it);
            DlInfoListElement_s dlharq = dlharqMsg->GetDlHarqFeedback ();
            // check whether the UE is connected
            if (m_ueAttached.find (dlharq.m_rnti) != m_ueAttached.end ())
              {
                m_enbPhySapUser->ReceiveLteControlMessage (*it);
              }
          }
          break;
        default:
          NS_FATAL_ERROR ("Unexpected LteControlMessage type");
          break;
        }
    }
}



void
LteEnbPhy::StartFrame (void)
{
  NS_LOG_FUNCTION (this);

  ++m_nrFrames;
  NS_LOG_INFO ("-----frame " << m_nrFrames << "-----");
  m_nrSubFrames = 0;

  if (m_disableMibAndSibStartupTime == Seconds (0) ||
      (m_disableMibAndSibStartupTime != Seconds (0) && Simulator::Now () <= m_disableMibAndSibStartupTime))
    {
      uint32_t period = m_mibPeriod / 10;
      if ((m_nrFrames % period) == 0)
        {
          // send MIB at beginning of every frame
          m_mib.systemFrameNumber = m_nrSubFrames;
          Ptr<MibLteControlMessage> mibMsg = Create<MibLteControlMessage> ();
          mibMsg->SetMib (m_mib);
          m_controlMessagesQueue.at (0).push_back (mibMsg);
        }
    }

  StartSubFrame ();
}

bool
LteEnbPhy::IsThereData ()
{
  return m_enbPhySapUser->IsThereData ();
}

void
LteEnbPhy::StartSubFrame (void)
{
  NS_LOG_FUNCTION (this);

  ++m_nrSubFrames;
  m_ttiBegin = Simulator::Now();

  // Piece of code that needs to be executed each subframe, no matter if we are are transmiting subframe or not

  /*
   * Send SIB1 at 6th subframe of every odd-numbered radio frame. This is
   * equivalent with Section 5.2.1.2 of 3GPP TS 36.331, where it is specified
   * "repetitions are scheduled in subframe #5 of all other radio frames for
   * which SFN mod 2 = 0," except that 3GPP counts frames and subframes starting
   * from 0, while ns-3 counts starting from 1.
   */

  if (m_disableMibAndSibStartupTime == Seconds (0) ||
      (m_disableMibAndSibStartupTime != Seconds (0) &&  Simulator::Now () <= m_disableMibAndSibStartupTime))
    {
      uint32_t sibPeriod = m_sibPeriod / 10;
      if ((m_nrSubFrames == 6) && ((m_nrFrames % sibPeriod) == 1))
        {
          Ptr<Sib1LteControlMessage> msg = Create<Sib1LteControlMessage> ();
          msg->SetSib1 (m_sib1);
          m_controlMessagesQueue.at (0).push_back (msg);
        }
    }

  if (m_drsMessagesEnabled)
    {
      uint32_t drsPeriod = m_drsPeriod/10;
      if ((m_nrSubFrames == 6) && ((m_nrFrames % drsPeriod) == 1))
        {
          Ptr<DrsLteControlMessage> msg = Create<DrsLteControlMessage> ();
          m_controlMessagesQueue.at (0).push_back (msg);
        }
    }

  /*
   * This code should be removed because the only variable affected is m_currentSrsOffset and is only used in CreateSrsCqiReport(). The main reason is when used
   * some channel acess transmission model that does not call StartSubFrame() for some time, can leave m_currentSrsOffset with old value, which can cause problems because
   * CreateSrsCqiReport() is called periodically by interference model.
   *
    if (m_srsPeriodicity>0)
       {
         // might be 0 in case the eNB has no UEs attached
         NS_ASSERT_MSG (m_nrFrames > 1, "the SRS index check code assumes that frameNo starts at 1");
         NS_ASSERT_MSG (m_nrSubFrames > 0 && m_nrSubFrames <= 10, "the SRS index check code assumes that subframeNo starts at 1");
         m_currentSrsOffset = (((m_nrFrames-1)*10 + (m_nrSubFrames-1)) % m_srsPeriodicity);
       }
   */

  NS_LOG_INFO ("-----sub frame " << m_nrSubFrames << "-----");
  m_harqPhyModule->SubframeIndication (m_nrFrames, m_nrSubFrames);

  // update info on TB to be received
  std::list<UlDciLteControlMessage> uldcilist = DequeueUlDci ();
  std::list<UlDciLteControlMessage>::iterator dciIt = uldcilist.begin ();
  NS_LOG_DEBUG (this << " eNB Expected TBs " << uldcilist.size ());
  for (dciIt = uldcilist.begin (); dciIt!=uldcilist.end (); dciIt++)
    {
      std::set <uint16_t>::iterator it2;
      it2 = m_ueAttached.find ((*dciIt).GetDci ().m_rnti);

      if (it2 == m_ueAttached.end ())
        {
          NS_LOG_ERROR ("UE not attached");
        }
      else
        {
          // send info of TB to LteSpectrumPhy 
          // translate to allocation map
          std::vector <int> rbMap;
          for (int i = (*dciIt).GetDci ().m_rbStart; i < (*dciIt).GetDci ().m_rbStart + (*dciIt).GetDci ().m_rbLen; i++)
            {
              rbMap.push_back (i);
            }
          m_uplinkSpectrumPhy->AddExpectedTb ((*dciIt).GetDci ().m_rnti, (*dciIt).GetDci ().m_ndi, (*dciIt).GetDci ().m_tbSize, (*dciIt).GetDci ().m_mcs, rbMap, 0 /* always SISO*/, 0 /* no HARQ proc id in UL*/, 0 /*evaluated by LteSpectrumPhy*/, false /* UL*/);
          if ((*dciIt).GetDci ().m_ndi==1)
            {
              NS_LOG_DEBUG (this << " RNTI " << (*dciIt).GetDci ().m_rnti << " NEW TB");
            }
          else
            {
              NS_LOG_DEBUG (this << " RNTI " << (*dciIt).GetDci ().m_rnti << " HARQ RETX");
            }
        }
    }

  // if there is no channel access manager transmit subframe
  if (m_channelAccessManager == 0 ||
      (m_channelAccessManager != 0 && Simulator::Now () < m_channelAccessManagerStartTime))
    {
      TransmitSubFrame ();
    }
  else  // if there is channel access manager
    {
      if (m_channelAccessImplWith2msDelay)
        {
          // if we have grant and there is no data in the buffers of scheduler
          if ((m_grantTimeout > m_ttiBegin + MilliSeconds (m_macChTtiDelay)) && (!IsThereData ()))
            {
              // reduce grant, but allow to PHY to transmit what MAC has alrady scheduled
              NS_LOG_INFO ("Grant reduced from: " << m_grantTimeout << " to:" << m_ttiBegin + MilliSeconds (m_macChTtiDelay));
              m_grantTimeout = m_ttiBegin + MilliSeconds (m_macChTtiDelay);
            }

          // check if channel access was requested, if not request it now.
          if (m_grantTimeout < m_ttiBegin && !m_isWaitingForChannelAccessGrant && IsThereData ())
            {
              // this may invoke immediate update of m_grantTimeout variable from ReceiveAccessGranted,
              // so it is very important at which point in code we invoke RequestChannelAccess
              RequestChannelAccess ();
            }

          // check again state of grant timeout variable, because it might got updated immediately after RequestChannelAccess
          if (m_grantTimeout - m_ttiBegin >= Seconds (GetTti ()))
            {
              TransmitSubFrame ();
              // because there is a delay between mac and phy, check if channel access will be granted in the future timestamp for which mac would be scheduling if SubframeIndication is triggered
              if (m_grantTimeout - m_ttiBegin >= MilliSeconds (m_macChTtiDelay) + Seconds (GetTti ()))
                {
                  // trigger the MAC
                  m_enbPhySapUser->SubframeIndication (m_nrFrames, m_nrSubFrames);
                }
            }
          else // no grant, continue to wait for a grant
            {
              // if there is nothing to be transmitted shift the queue
              if (!IsNonEmptyCtrlMessage () && (!IsNonEmptyPacketBurst ()))
                {
                  GetControlMessages ();
                  GetPacketBurst ();
                }
            }

          Simulator::Schedule (Seconds (GetTti ()), &LteEnbPhy::EndSubFrame, this);
          return;
        }
      else  // implementation 2 - without 2ms delay, scheduler is not being stopped
        {
          // check in queue if there are empty elements
          CheckQueues ();
          // if there is no data neither ctrl messages ready to be transmitted shift the queues and release the grant
          if (!IsNonEmptyCtrlMessage ())
            {
              UpdateQueues (m_dropPackets);
              m_grantTimeout = m_ttiBegin;
            }
          else  // there is something ready to be transmitted
            {
              // channel access granted
              if (m_grantTimeout > m_ttiBegin)
                {
                  // there is data, check if grant is enough to transmit CTRL or/and DATA, if not ask for new grant
                  if ((IsNonEmptyPacketBurst () && m_grantTimeout-m_ttiBegin < Seconds (GetTti ())) ||
                      (!IsNonEmptyPacketBurst () && IsNonEmptyCtrlMessage () && (m_grantTimeout - m_ttiBegin < DL_CTRL_DELAY_FROM_SUBFRAME_START)))
                    {
                      m_grantTimeout = m_ttiBegin;
                      RequestChannelAccess();
                    }
                }
              // channel access not granted
              else
                {
                  // if there is data, ask for channel access grant
                  if (!m_isWaitingForChannelAccessGrant)
                    {
                      RequestChannelAccess ();
                    }
                }

              // check again state of grant timeout variable, because it might got updated immediately after RequestChannelAccess
              if (m_grantTimeout - m_ttiBegin >=  Seconds (GetTti ()))
                {
                  NS_ASSERT_MSG (m_controlMessagesQueue.at (0).size (), " Transmit subframe should not be called when there is nothing to transmit.");
                  TransmitSubFrame ();
                }
              else // no grant, continue to wait for a grant
                {
                  if (m_dropPackets)
                    {
                      std::list<Ptr<LteControlMessage> > ctrlMsg = GetLteControlMessageVector ().at (0);
                      bool m_hasUlDci = false;

                      if (ctrlMsg.size () > 0)
                        {
                          std::list<Ptr<LteControlMessage> >::iterator it;
                          it = ctrlMsg.begin ();

                          while (it != ctrlMsg.end ())
                            {
                              Ptr<LteControlMessage> msg = (*it);
                              if (msg->GetMessageType () == LteControlMessage::UL_DCI)
                                {
                                  NS_LOG_DEBUG ("found UL-DCI");
                                  m_hasUlDci=true;
                                }
                              it++;
                            }
                        }
                      // we drop packet in case we have UL_DCI
                      if (m_hasUlDci)
                        {
                          NS_LOG_DEBUG ("Dropping PDU because its ctrl message contains UL_DCI");
                          UpdateQueues (true);
                        }
                      else
                        {
                          UpdateQueues (false);
                        }
                    }//end of if drop packet
                  else
                    {
                      // dont drop
                      UpdateQueues (false);
                    }
                }// end of if no grant
            } //end of else (there is ctrl or data available for transmission)
        }// end of else of implementation 2
    } // end of else if there is channel access manager

  // trigger the MAC
  m_enbPhySapUser->SubframeIndication (m_nrFrames, m_nrSubFrames);
  Simulator::Schedule (Seconds (GetTti ()),
                       &LteEnbPhy::EndSubFrame,
                       this);

}


void
LteEnbPhy::TransmitSubFrame (void)
{
  NS_LOG_FUNCTION (this);

  // process the current burst of control messages
  std::list<Ptr<LteControlMessage> > ctrlMsg = GetControlMessages ();
  m_ctrlMsgTransmission(ctrlMsg);
  m_dlDataRbMap.clear ();
  m_dlPowerAllocationMap.clear ();
  bool drsCtrlMessage = false;
  if (ctrlMsg.size () > 0)
    {
      std::list<Ptr<LteControlMessage> >::iterator it;
      it = ctrlMsg.begin ();
      while (it != ctrlMsg.end ())
        {
          Ptr<LteControlMessage> msg = (*it);
          if (msg->GetMessageType () == LteControlMessage::DL_DCI)
            {
              Ptr<DlDciLteControlMessage> dci = DynamicCast<DlDciLteControlMessage> (msg);
              // get the tx power spectral density according to DL-DCI(s)
              // translate the DCI to Spectrum framework
              uint32_t mask = 0x1;
              for (int i = 0; i < 32; i++)
                {
                  if (((dci->GetDci ().m_rbBitmap & mask) >> i) == 1)
                    {
                      for (int k = 0; k < GetRbgSize (); k++)
                        {
                          m_dlDataRbMap.push_back ((i * GetRbgSize ()) + k);
                          //NS_LOG_DEBUG(this << " [enb]DL-DCI allocated PRB " << (i*GetRbgSize()) + k);
                          GeneratePowerAllocationMap (dci->GetDci ().m_rnti, (i * GetRbgSize ()) + k );
                        }
                    }
                  mask = (mask << 1);
                }
              // fire trace of DL Tx PHY stats
              for (uint8_t i = 0; i < dci->GetDci ().m_mcs.size (); i++)
                {
                  PhyTransmissionStatParameters params;
                  params.m_cellId = m_cellId;
                  params.m_imsi = 0; // it will be set by DlPhyTransmissionCallback in LteHelper
                  params.m_timestamp = Simulator::Now ().GetMilliSeconds ();
                  params.m_rnti = dci->GetDci ().m_rnti;
                  params.m_txMode = 0; // TBD
                  params.m_layer = i;
                  params.m_mcs = dci->GetDci ().m_mcs.at (i);
                  params.m_size = dci->GetDci ().m_tbsSize.at (i);
                  params.m_rv = dci->GetDci ().m_rv.at (i);
                  params.m_ndi = dci->GetDci ().m_ndi.at (i);
                  params.m_ccId = m_componentCarrierId;
                  m_dlPhyTransmission (params);
                }

            }
          else if (msg->GetMessageType () == LteControlMessage::UL_DCI)
            {
              Ptr<UlDciLteControlMessage> dci = DynamicCast<UlDciLteControlMessage> (msg);
              QueueUlDci (*dci);
            }
          else if (msg->GetMessageType () == LteControlMessage::RAR)
            {
              Ptr<RarLteControlMessage> rarMsg = DynamicCast<RarLteControlMessage> (msg);
              for (std::list<RarLteControlMessage::Rar>::const_iterator it = rarMsg->RarListBegin (); it != rarMsg->RarListEnd (); ++it)
                {
                  if (it->rarPayload.m_grant.m_ulDelay == true)
                    {
                      NS_FATAL_ERROR (" RAR delay is not yet implemented");
                    }
                  UlGrant_s ulGrant = it->rarPayload.m_grant;
                  // translate the UL grant in a standard UL-DCI and queue it
                  UlDciListElement_s dci;
                  dci.m_rnti = ulGrant.m_rnti;
                  dci.m_rbStart = ulGrant.m_rbStart;
                  dci.m_rbLen = ulGrant.m_rbLen;
                  dci.m_tbSize = ulGrant.m_tbSize;
                  dci.m_mcs = ulGrant.m_mcs;
                  dci.m_hopping = ulGrant.m_hopping;
                  dci.m_tpc = ulGrant.m_tpc;
                  dci.m_cqiRequest = ulGrant.m_cqiRequest;
                  dci.m_ndi = 1;
                  UlDciLteControlMessage msg;
                  msg.SetDci (dci);
                  QueueUlDci (msg);
                }
            }
          else if (msg->GetMessageType () == LteControlMessage::DRS)
            {
              m_lastDrsMessageSent = Simulator::Now();
              drsCtrlMessage = true;
            }
          it++;
        }
    }

  NS_ASSERT_MSG (m_nrSubFrames > 0 && m_nrSubFrames <= 10, "code assumes subframe in 1..10");
  uint32_t absIndex = (m_nrFrames % 4) * 10 + ((m_nrSubFrames - 1) % 10);
  uint32_t absIndex2 = (m_nrFrames * 10 + (m_nrSubFrames - 1)) % 40;
  bool isAlmostBlankSubframe = m_absPattern[absIndex];

  if (isAlmostBlankSubframe) // Almost Blank Subframe
    {
      NS_LOG_LOGIC ("Almost Blank Subframe (ABS), absIndex=" << absIndex << ", absIndex2=" << absIndex2 << ", m_absPattern=" << m_absPattern);
      Ptr<PacketBurst> pb = GetPacketBurst ();
      NS_ASSERT_MSG (pb == 0, "the LTE MAC scheduler shall not allocate data transmission in an ABS");
    }
  else // regular subframe
    {
      std::list<Ptr<LteControlMessage> >::iterator it;
      it = ctrlMsg.begin ();
      while (it != ctrlMsg.end ())
        {
          Ptr<LteControlMessage> msg = (*it);
          if (m_ctrlTypesCount.find (msg->GetMessageType ()) == m_ctrlTypesCount.end ())
            {
              m_ctrlTypesCount.insert (std::make_pair<int, int> (msg->GetMessageType (), 1));
            }
          else
            {
              m_ctrlTypesCount.find (msg->GetMessageType ())->second++;
            }
          it++;
        }

      m_ctrlMsgCounter++;
      SendControlChannels (ctrlMsg);
      // schedule PDSCH transmission
      Ptr<PacketBurst> pb = GetPacketBurst ();
      if (pb)
        {
          Simulator::Schedule (DL_CTRL_DELAY_FROM_SUBFRAME_START, // ctrl frame fixed to 3 symbols
                               &LteEnbPhy::SendDataChannels,
                               this,pb);
        }
      else if (m_channelAccessManager && (Simulator::Now () > m_channelAccessManagerStartTime)) // check if to send reservation signal after ctrl
        {
          if (m_channelAccessImplWith2msDelay)
            {
              if (m_reservationSignal)
                {
                  Simulator::Schedule (DL_CTRL_DELAY_FROM_SUBFRAME_START, // ctrl frame fixed to 3 symbols
                                       &LteEnbPhy::DoSendReservationSignal,
                                       this);
                }
            }
          else
            {
              // in the case of drs messages we send reservation signal to simulate that drs message
              // lasts 1 subframe instead of only 3 symbols
              if (drsCtrlMessage && m_reservationSignal)
                {
                  NS_LOG_DEBUG ("eNB is transmitting DRS at: " << Simulator::Now ());
                  // Set m_drsReservationSignal to true so we do not fire the ReservationSignal trace again
                  // in DoSendReservationSignal. DoSendReservationSignal here is being called just to make
                  // DRS duration 1 msec, not to reserve the channel as we normally do if we get grant in
                  // the middle of subframe.
                  m_drsReservationSignal = true;
                  Simulator::Schedule (DL_CTRL_DELAY_FROM_SUBFRAME_START, // ctrl frame fixed to 3 symbols
                                       &LteEnbPhy::DoSendReservationSignal,
                                       this);
                }
              else
                {
                  // release the grant only when there is nothing waiting to be transmitted
                  if (m_controlMessagesQueue.at (0).size () == 0)
                    {
                      m_grantTimeout = m_ttiBegin + DL_CTRL_DELAY_FROM_SUBFRAME_START;
                    }
                }
            }
        }
    }
}

void
LteEnbPhy::SendControlChannels (std::list<Ptr<LteControlMessage> > ctrlMsgList)
{
  NS_LOG_FUNCTION (this << " eNB " << m_cellId << " start tx ctrl frame");
  // set the current tx power spectral density (full bandwidth)
  std::vector <int> dlRb;
  for (uint8_t i = 0; i < m_dlBandwidth; i++)
    {
      dlRb.push_back (i);
    }
  SetDownlinkSubChannels (dlRb);
  NS_LOG_LOGIC (this << " eNB start TX CTRL");
  bool pss = false;
  if ((m_nrSubFrames == 1) || (m_nrSubFrames == 6))
    {
      pss = true;
    }
  m_downlinkSpectrumPhy->StartTxDlCtrlFrame (ctrlMsgList, pss);

}

void
LteEnbPhy::SendDataChannels (Ptr<PacketBurst> pb)
{
  // update log file for RB usage
  m_logTimeToRbUsage.insert (std::make_pair<Time, int> (Simulator::Now (), m_dlDataRbMap.size ()));
  // set the current tx power spectral density
  SetDownlinkSubChannelsWithPowerAllocation (m_dlDataRbMap);
  // send the current burts of packets
  NS_LOG_LOGIC (this << " eNB start TX DATA");
  std::list<Ptr<LteControlMessage> > ctrlMsgList;
  ctrlMsgList.clear ();
  m_downlinkSpectrumPhy->StartTxDataFrame (pb, ctrlMsgList, DL_DATA_DURATION);

  m_data (pb->GetSize ());
}


void
LteEnbPhy::DoSendReservationSignal ()
{
  // set the current tx power spectral density (full bandwidth)
  NS_LOG_FUNCTION (this << " eNB: " << m_cellId << " start tx reserv signal at: " << Simulator::Now ().GetMilliSeconds () << " until: " << m_ttiBegin + Seconds (GetTti ()));
  // Since this is async call, check if we are already transmitting something. If there is already some transmission ongoing, then this reservation signal should not be sent
  if (!m_downlinkSpectrumPhy->IsStateIdle ())
  {
    NS_LOG_WARN ("Tried to send reservation signal when channel state is not IDLE.");
    return;
  }
  NS_ASSERT_MSG (m_channelAccessManager && (Simulator::Now () >= m_channelAccessManagerStartTime), "When m_channelAccessManager is not set, SendReservationSignal function should not be called.");

  std::vector <int> dlRb;
  for (uint8_t i = 0; i < m_dlBandwidth; i++)
    {
      dlRb.push_back (i);
    }
  SetDownlinkSubChannels (dlRb);
  NS_LOG_LOGIC (this << " eNB start TX RESERV SIGNAL");
  std::list<Ptr<LteControlMessage> > ctrlMsgList;
  ctrlMsgList.clear ();
  // similarly to when the data is sent, we reduce the duration for 1 nanoseconds to avoid overlapping simulator events.
  Time duration = m_ttiBegin + Seconds (GetTti ()) - Simulator::Now () - NanoSeconds (1);
  if (duration > Seconds (0))
  {
     m_downlinkSpectrumPhy->StartTxDataFrame (0, ctrlMsgList, duration);
     if(!m_drsReservationSignal)
       {
          m_reservationSignalTraceSource (Simulator::Now (), duration);
       }
     m_drsReservationSignal = false;
  }
}


void
LteEnbPhy::EndSubFrame (void)
{
  NS_LOG_FUNCTION (this << Simulator::Now ().GetSeconds ());
  if (m_nrSubFrames == 10)
    {
      Simulator::ScheduleNow (&LteEnbPhy::EndFrame, this);
    }
  else
    {
      Simulator::ScheduleNow (&LteEnbPhy::StartSubFrame, this);
    }
}


void
LteEnbPhy::EndFrame (void)
{
  NS_LOG_FUNCTION (this << Simulator::Now ().GetSeconds ());
  Simulator::ScheduleNow (&LteEnbPhy::StartFrame, this);
}

void
LteEnbPhy::RequestChannelAccess(void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (m_channelAccessManager != 0 && (Simulator::Now () >= m_channelAccessManagerStartTime), "Channel access manager not set!");
  m_isWaitingForChannelAccessGrant = true;
  m_channelAccessManager->RequestAccess ();
}

void
LteEnbPhy::ReceiveAccessGranted(Time grantDuration)
{
  NS_LOG_FUNCTION (this);
  // get duration of grant as number of TTIs
  m_grantTimeout = Simulator::Now () + grantDuration;
  m_txopTraceSource (Simulator::Now (), grantDuration, m_ttiBegin + Seconds(GetTti ()));
  m_isWaitingForChannelAccessGrant = false;
  // dont send reservation if we are at the beginning of the subframe
  if (m_ttiBegin != Simulator::Now ())
    {
      DoSendReservationSignal ();
    }
}

void 
LteEnbPhy::GenerateCtrlCqiReport (const SpectrumValue& sinr)
{
  NS_LOG_FUNCTION (this << sinr << Simulator::Now () << m_srsStartTime);
  // avoid processing SRSs sent with an old SRS configuration index
  if (Simulator::Now () > m_srsStartTime)
    {
      FfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi = CreateSrsCqiReport (sinr);
      m_enbPhySapUser->UlCqiReport (ulcqi);
    }
}

void
LteEnbPhy::GenerateDataCqiReport (const SpectrumValue& sinr)
{
  NS_LOG_FUNCTION (this << sinr);
  FfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi = CreatePuschCqiReport (sinr);
  m_enbPhySapUser->UlCqiReport (ulcqi);
}

void
LteEnbPhy::ReportInterference (const SpectrumValue& interf)
{
  NS_LOG_FUNCTION (this << interf);
  Ptr<SpectrumValue> interfCopy = Create<SpectrumValue> (interf);
  m_interferenceSampleCounter++;
  if (m_interferenceSampleCounter == m_interferenceSamplePeriod)
    {
      m_reportInterferenceTrace (m_cellId, interfCopy);
      m_interferenceSampleCounter = 0;
    }
}

void
LteEnbPhy::ReportRsReceivedPower (const SpectrumValue& power)
{
  // not used by eNB
}



FfMacSchedSapProvider::SchedUlCqiInfoReqParameters
LteEnbPhy::CreatePuschCqiReport (const SpectrumValue& sinr)
{
  NS_LOG_FUNCTION (this << sinr);
  Values::const_iterator it;
  FfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi;
  ulcqi.m_ulCqi.m_type = UlCqi_s::PUSCH;
  int i = 0;
  for (it = sinr.ConstValuesBegin (); it != sinr.ConstValuesEnd (); it++)
    {
      double sinrdb = 10 * std::log10 ((*it));
//       NS_LOG_DEBUG ("ULCQI RB " << i << " value " << sinrdb);
      // convert from double to fixed point notation Sxxxxxxxxxxx.xxx
      int16_t sinrFp = LteFfConverter::double2fpS11dot3 (sinrdb);
      ulcqi.m_ulCqi.m_sinr.push_back (sinrFp);
      i++;
    }
  return (ulcqi);
	
}


void
LteEnbPhy::DoSetBandwidth (uint8_t ulBandwidth, uint8_t dlBandwidth)
{
  NS_LOG_FUNCTION (this << (uint32_t) ulBandwidth << (uint32_t) dlBandwidth);
  m_ulBandwidth = ulBandwidth;
  m_dlBandwidth = dlBandwidth;

  static const int Type0AllocationRbg[4] = {
    10,     // RGB size 1
    26,     // RGB size 2
    63,     // RGB size 3
    110     // RGB size 4
  };  // see table 7.1.6.1-1 of 36.213
  for (int i = 0; i < 4; i++)
    {
      if (dlBandwidth < Type0AllocationRbg[i])
        {
          m_rbgSize = i + 1;
          break;
        }
    }
}

void 
LteEnbPhy::DoSetEarfcn (uint32_t ulEarfcn, uint32_t dlEarfcn)
{
  NS_LOG_FUNCTION (this << ulEarfcn << dlEarfcn);
  m_ulEarfcn = ulEarfcn;
  m_dlEarfcn = dlEarfcn;
}


void 
LteEnbPhy::DoAddUe (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);
 
  bool success = AddUePhy (rnti);
  NS_ASSERT_MSG (success, "AddUePhy() failed");

  // add default P_A value
  DoSetPa (rnti, 0);
}

void 
LteEnbPhy::DoRemoveUe (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);
 
  bool success = DeleteUePhy (rnti);
  NS_ASSERT_MSG (success, "DeleteUePhy() failed");

  // remove also P_A value
  std::map<uint16_t, double>::iterator it = m_paMap.find (rnti);
  if (it != m_paMap.end ())
    {
      m_paMap.erase (it);
    }

  //additional data to be removed
  m_uplinkSpectrumPhy->RemoveExpectedTb (rnti);
  //remove srs info to avoid trace errors
  std::map<uint16_t, uint16_t>::iterator sit = m_srsSampleCounterMap.find (rnti);
  if (sit != m_srsSampleCounterMap.end ())
    {
      m_srsSampleCounterMap.erase (rnti);
    }
  //remove DL_DCI message otherwise errors occur for m_dlPhyTransmission trace
  //remove also any UL_DCI message for the UE to be removed

  for (auto & ctrlMessageList : m_controlMessagesQueue)
    {
      std::list<Ptr<LteControlMessage> >::iterator ctrlMsgListIt = ctrlMessageList.begin ();
      while (ctrlMsgListIt != ctrlMessageList.end ())
        {
          Ptr<LteControlMessage> msg = (*ctrlMsgListIt);
          if (msg->GetMessageType () == LteControlMessage::DL_DCI)
            {
              auto dci = DynamicCast<DlDciLteControlMessage> (msg);
              if (dci->GetDci ().m_rnti == rnti)
                {
                  NS_LOG_INFO ("DL_DCI to be sent from cell id : "
                        << m_cellId << " to RNTI : " << rnti << " is deleted");
                  ctrlMsgListIt = ctrlMessageList.erase (ctrlMsgListIt);
                }
              else
                {
                  ++ctrlMsgListIt;
                }
            }
          else if (msg->GetMessageType () == LteControlMessage::UL_DCI)
            {
              auto dci = DynamicCast<UlDciLteControlMessage> (msg);
              if (dci->GetDci ().m_rnti == rnti)
                {
                  NS_LOG_INFO ("UL_DCI to be sent from cell id : "
                        << m_cellId << " to RNTI : " << rnti << " is deleted");
                  ctrlMsgListIt = ctrlMessageList.erase (ctrlMsgListIt);
                }
              else
                {
                  ++ctrlMsgListIt;
                }
            }
          else
            {
              ++ctrlMsgListIt;
            }
        }
    }

}

void
LteEnbPhy::DoSetPa (uint16_t rnti, double pa)
{
  NS_LOG_FUNCTION (this << rnti);

  std::map<uint16_t, double>::iterator it = m_paMap.find (rnti);

  if (it == m_paMap.end ())
    {
      m_paMap.insert (std::pair<uint16_t, double> (rnti, pa));
    }
  else
    {
      it->second = pa;
    }

}

FfMacSchedSapProvider::SchedUlCqiInfoReqParameters
LteEnbPhy::CreateSrsCqiReport (const SpectrumValue& sinr)
{
  NS_LOG_FUNCTION (this << sinr);
  Values::const_iterator it;
  FfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi;
  ulcqi.m_ulCqi.m_type = UlCqi_s::SRS;
  int i = 0;
  double srsSum = 0.0;
  for (it = sinr.ConstValuesBegin (); it != sinr.ConstValuesEnd (); it++)
    {
      double sinrdb = 10 * log10 ((*it));
      //       NS_LOG_DEBUG ("ULCQI RB " << i << " value " << sinrdb);
      // convert from double to fixed point notation Sxxxxxxxxxxx.xxx
      int16_t sinrFp = LteFfConverter::double2fpS11dot3 (sinrdb);
      srsSum += (*it);
      ulcqi.m_ulCqi.m_sinr.push_back (sinrFp);
      i++;
    }
  // Insert the user generated the srs as a vendor specific parameter
  // update currentSrsOffset
  m_currentSrsOffset = (((m_nrFrames - 1) * 10 + (m_nrSubFrames - 1)) % m_srsPeriodicity);
  NS_LOG_DEBUG (this << " ENB RX UL-CQI of " << m_srsUeOffset.at (m_currentSrsOffset));
  VendorSpecificListElement_s vsp;
  vsp.m_type = SRS_CQI_RNTI_VSP;
  vsp.m_length = sizeof(SrsCqiRntiVsp);
  Ptr<SrsCqiRntiVsp> rnti  = Create <SrsCqiRntiVsp> (m_srsUeOffset.at (m_currentSrsOffset));
  vsp.m_value = rnti;
  ulcqi.m_vendorSpecificList.push_back (vsp);
  // call SRS tracing method
  CreateSrsReport (m_srsUeOffset.at (m_currentSrsOffset),
                   (i > 0) ? (srsSum / i) : DBL_MAX);
  return (ulcqi);

}


void
LteEnbPhy::CreateSrsReport (uint16_t rnti, double srs)
{
  NS_LOG_FUNCTION (this << rnti << srs);
  std::map <uint16_t,uint16_t>::iterator it = m_srsSampleCounterMap.find (rnti);
  if (it==m_srsSampleCounterMap.end ())
    {
      // create new entry
      m_srsSampleCounterMap.insert (std::pair <uint16_t,uint16_t> (rnti, 0));
      it = m_srsSampleCounterMap.find (rnti);
    }
  (*it).second++;
  if ((*it).second == m_srsSamplePeriod)
    {
      m_reportUeSinr (m_cellId, rnti, srs, (uint16_t) m_componentCarrierId);
      (*it).second = 0;
    }
}

void
LteEnbPhy::DoSetTransmissionMode (uint16_t  rnti, uint8_t txMode)
{
  NS_LOG_FUNCTION (this << rnti << (uint16_t)txMode);
  // UL supports only SISO MODE
}

void
LteEnbPhy::QueueUlDci (UlDciLteControlMessage m)
{
  NS_LOG_FUNCTION (this);
  m_ulDciQueue.at (UL_PUSCH_TTIS_DELAY - 1).push_back (m);
}

std::list<UlDciLteControlMessage>
LteEnbPhy::DequeueUlDci (void)
{
  NS_LOG_FUNCTION (this);
  if (m_ulDciQueue.at (0).size ()>0)
    {
      std::list<UlDciLteControlMessage> ret = m_ulDciQueue.at (0);
      m_ulDciQueue.erase (m_ulDciQueue.begin ());
      std::list<UlDciLteControlMessage> l;
      m_ulDciQueue.push_back (l);
      return (ret);
    }
  else
    {
      m_ulDciQueue.erase (m_ulDciQueue.begin ());
      std::list<UlDciLteControlMessage> l;
      m_ulDciQueue.push_back (l);
      std::list<UlDciLteControlMessage> emptylist;
      return (emptylist);
    }
}

void
LteEnbPhy::DoSetSrsConfigurationIndex (uint16_t  rnti, uint16_t srcCi)
{
  NS_LOG_FUNCTION (this);
  uint16_t p = GetSrsPeriodicity (srcCi);
  if (p!=m_srsPeriodicity)
    {
      // resize the array of offset -> re-initialize variables
      m_srsUeOffset.clear ();
      m_srsUeOffset.resize (p, 0);
      m_srsPeriodicity = p;
      // inhibit SRS until RRC Connection Reconfiguration propagates
      // to UEs, otherwise we might be wrong in determining the UE who
      // actually sent the SRS (if the UE was using a stale SRS config)
      // if we use a static SRS configuration index, we can have a 0ms guard time
      m_srsStartTime = Simulator::Now () + MilliSeconds (m_macChTtiDelay) + MilliSeconds (0);
    }

  NS_LOG_DEBUG (this << " ENB SRS P " << m_srsPeriodicity << " RNTI " << rnti << " offset " << GetSrsSubframeOffset (srcCi) << " CI " << srcCi);
  std::map <uint16_t,uint16_t>::iterator it = m_srsCounter.find (rnti);
  if (it != m_srsCounter.end ())
    {
      (*it).second = GetSrsSubframeOffset (srcCi) + 1;
    }
  else
    {
      m_srsCounter.insert (std::pair<uint16_t, uint16_t> (rnti, GetSrsSubframeOffset (srcCi) + 1));
    }
  m_srsUeOffset.at (GetSrsSubframeOffset (srcCi)) = rnti;

}


void 
LteEnbPhy::DoSetMasterInformationBlock (LteRrcSap::MasterInformationBlock mib)
{
  NS_LOG_FUNCTION (this);
  m_mib = mib;
}


void
LteEnbPhy::DoSetSystemInformationBlockType1 (LteRrcSap::SystemInformationBlockType1 sib1)
{
  NS_LOG_FUNCTION (this);
  m_sib1 = sib1;
}


void
LteEnbPhy::DoSetAbsPattern (std::bitset<40> absPattern)
{
  NS_LOG_FUNCTION (this);
  m_absPattern = absPattern;
}


void
LteEnbPhy::SetHarqPhyModule (Ptr<LteHarqPhy> harq)
{
  m_harqPhyModule = harq;
}


void
LteEnbPhy::ReceiveLteUlHarqFeedback (UlInfoListElement_s mes)
{
  NS_LOG_FUNCTION (this);
  // forward to scheduler
  m_enbPhySapUser->UlInfoListElementHarqFeeback (mes);
}

};

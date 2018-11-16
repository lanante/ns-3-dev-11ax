/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2015 University of Washington
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
 * Authors: Nicola Baldo <nbaldo@cttc.es> and Tom Henderson <tomh@tomh.org>
 */

#include "scenario-helper.h"
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/internet-module.h>
#include <ns3/point-to-point-module.h>
#include <ns3/csma-module.h>
#include <ns3/lte-module.h>
#include <ns3/wifi-module.h>
#include <ns3/spectrum-module.h>
#include <ns3/applications-module.h>
#include <ns3/internet-module.h>
#include <ns3/internet-apps-module.h>
#include <ns3/propagation-module.h>
#include <ns3/config-store-module.h>
#include <ns3/flow-monitor-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-wifi-coexistence-helper.h>
#include <ns3/ff-mac-common.h>
#include <ns3/lbt-access-manager.h>

#ifndef UINT32_MAX
#define UINT32_MAX 4294967295U
#endif

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ScenarioHelper");

static const uint16_t VOICE_PORT = 16384; // a port sometimes used for RTP
static const uint16_t UDP_SERVER_PORT = 9; // discard port

// Used to assign fixed streams to random variables
static int64_t streamIndex = 1000;

// Global Values are used in place of command line arguments so that these
// values may be managed in the ns-3 ConfigStore system.
//
static ns3::GlobalValue g_serverStartTimeSeconds ("serverStartTimeSeconds",
                                                  "Server start time (seconds)",
                                                  ns3::DoubleValue (2),
                                                  ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_clientStartTimeSeconds ("clientStartTimeSeconds",
                                                  "Client start time (seconds)",
                                                  ns3::DoubleValue (2),
                                                  ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_serverLingerTimeSeconds ("serverLingerTimeSeconds",
                                                   "Server linger time (seconds)",
                                                   ns3::DoubleValue (1),
                                                   ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_simulationLingerTimeSeconds ("simulationLingerTimeSeconds",
                                                       "Simulation linger time (seconds)",
                                                       ns3::DoubleValue (1),
                                                       ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_remDir ("remDir",
                                  "directory where to save REM-related output files",
                                  ns3::StringValue ("./"),
                                  ns3::MakeStringChecker ());

static ns3::GlobalValue g_lbtChannelAccessManagerInstallTime ("lbtChannelAccessManagerInstallTime",
                                                              "LTE channel access manager install time (seconds)",
                                                              ns3::DoubleValue (2),
                                                              ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_logWifiRetries ("logWifiRetries",
                                              "Log when a data packet requires a retry",
                                              ns3::BooleanValue (false),
                                              ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_logWifiFailRetries ("logWifiFailRetries",
                                              "Log when a data packet fails to be retransmitted successfully and is discarded",
                                              ns3::BooleanValue (false),
                                              ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_logPhyNodeId ("logPhyNodeId",
                                        "Node index of specific node to filter logPhyArrivals (default will log all nodes)",
                                        ns3::UintegerValue (UINT32_MAX),
                                        ns3::MakeUintegerChecker<uint32_t> ());

static ns3::GlobalValue g_logTxopNodeId ("logTxopNodeId",
                                        "Node index of specific node to filter logTxops (default will log all nodes)",
                                        ns3::UintegerValue (UINT32_MAX),
                                        ns3::MakeUintegerChecker<uint32_t> ());

static ns3::GlobalValue g_logPhyArrivals ("logPhyArrivals",
                                          "Whether to write logfile of signal arrivals to SpectrumWifiPhy",
                                          ns3::BooleanValue (false),
                                          ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_logTxops ("logTxops",
                                          "Whether to write logfile of txop opportunities of eNb",
                                          ns3::BooleanValue (false),
                                          ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_logDataTx ("logDataTx",
                                          "Whether to write logfile when data is transmitted at eNb",
                                          ns3::BooleanValue (false),
                                          ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_logCwChanges ("logCwChanges",
                                        "Whether to write logfile of contention window changes",
                                        ns3::BooleanValue (false),
                                        ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_logCwNodeId ("logCwNodeId",
                                        "Node index of specific node to filter logCwChanges (default will log all nodes)",
                                        ns3::UintegerValue (UINT32_MAX),
                                        ns3::MakeUintegerChecker<uint32_t> ());

static ns3::GlobalValue g_logBackoffChanges ("logBackoffChanges",
                                             "Whether to write logfile of backoff values drawn",
                                             ns3::BooleanValue (false),
                                             ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_logBackoffNodeId ("logBackoffNodeId",
                                            "Node index of specific node to filter logBackoffChanges (default will log all nodes)",
                                            ns3::UintegerValue (UINT32_MAX),
                                            ns3::MakeUintegerChecker<uint32_t> ());

static ns3::GlobalValue g_logHarqFeedbackNodeId ("logHarqFeedbackNodeId",
                                                 "Node index of specific node to filter HARQ feedbacks (default will log all nodes)",
                                                 ns3::UintegerValue (UINT32_MAX),
                                                 ns3::MakeUintegerChecker<uint32_t> ());

static ns3::GlobalValue g_logBeaconNodeId ("logBeaconNodeId",
                                           "Node index of specific node to filter logBeaconArrivals (default will log all nodes)",
                                           ns3::UintegerValue (UINT32_MAX),
                                           ns3::MakeUintegerChecker<uint32_t> ());

static ns3::GlobalValue g_logBeaconArrivals ("logBeaconArrivals",
                                             "Whether to write logfile of beacon arrivals to STA devices",
                                             ns3::BooleanValue (false),
                                             ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_logCtrlSignals ("logCtrlSignals",
                                          "Whether to write logfile of CTRL signals sent by enbs",
                                          ns3::BooleanValue (false),
                                          ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_logHarqFeedbacks ("logHarqFeedback",
                                          "Whether to write logfile of HARQ feedbacks per each subframe.",
                                          ns3::BooleanValue (false),
                                          ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_logReservationSignals ("logReservationSignals",
                                                 "Whether to write logfile of reservation signals",
                                                 ns3::BooleanValue (false),
                                                 ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_logAssociationStats ("logAssociationStats",
                                               "Whether to write logfile of WiFi association and LAA/LTE eNB and UE association",
                                               ns3::BooleanValue (false),
                                               ns3::MakeBooleanChecker ());

// 75 Mb/s will saturate LAA and WiFi SISO 20 MHz
static const uint64_t UDP_SATURATION_RATE = 75000000;

static ns3::GlobalValue g_udpRate ("udpRate",
                                   "Data rate of UDP application",
                                   ns3::DataRateValue (DataRate (UDP_SATURATION_RATE)),
                                   ns3::MakeDataRateChecker ());

static ns3::GlobalValue g_udpSize ("udpPacketSize",
                                   "Packet size of UDP application",
                                   ns3::UintegerValue (1000),
                                   ns3::MakeUintegerChecker<uint32_t> ());

static ns3::GlobalValue g_spreadUdpLoad ("spreadUdpLoad",
                                         "optimization to try to spread a saturating UDP load across multiple UE and cells",
                                         ns3::BooleanValue (false),
                                         ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_wifiMacQueueMaxDelay ("wifiQueueMaxDelay",
                                         "change from default 500 ms to change the maximum queue dwell time for Wifi packets",
                                         ns3::UintegerValue (500),
                                         ns3::MakeUintegerChecker<uint32_t> ());

static ns3::GlobalValue g_wifiMacQueueMaxSize ("wifiQueueMaxSize",
                                         "change from default 400 packets to change the queue size for Wifi packets",
                                         ns3::QueueSizeValue (QueueSize ("400p")),
                                         ns3::MakeQueueSizeChecker ());

static ns3::GlobalValue g_mibPeriod ("mibPeriod",
                                     "periodicity to be set for MIB ctrl messages",
                                     ns3::UintegerValue (10),
                                     ns3::MakeUintegerChecker<uint32_t> (10,160));

static ns3::GlobalValue g_sibPeriod ("sibPeriod",
                                     "periodicity to be set for SIB1 ctrl messages",
                                     ns3::UintegerValue (20),
                                     ns3::MakeUintegerChecker<uint32_t> (20,160));

static ns3::GlobalValue g_drsPeriod ("drsPeriod",
                                     "periodicity to be set for DRS ctrl messages",
                                     ns3::UintegerValue (80),
                                     ns3::MakeUintegerChecker<uint32_t> (40,160));

static ns3::GlobalValue g_drsEnabled ("drsEnabled",
                                      "if true, drs ctrl messages will be generated"
                                      "if false, drs ctrl messages will not be generated",
                                      ns3::BooleanValue (true),
                                      ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_disableMibAndSibStartupTime ("disableMibAndSibStartupTime",
                                                       "the time at which to disable mib and sib control messages (seconds)",
                                                       ns3::DoubleValue (2),
                                                       ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_dropPackets ("dropPackets",
                                         "applicable to impl2; if true, Phy will drop packets upon failure to obtain channel access",
                                         ns3::BooleanValue (false),
                                         ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_lteChannelAccessImpl ("lteChannelAccessImpl",
                                         "if true, (impl1) it will be used lte channel access implemenation that stops scheduler when no channel access, thus has 2ms delay"
                                         "if false, (impl2) it will be used lte channel acces implementation that does not stop scheduler, always schedules, transmits only when "
                                         "there is channel access, can be used in combination with drop packet attribute",
                                         ns3::BooleanValue (false),
                                         ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_tcpRlcMode ("tcpRlcMode",
                                        "RLC_AM, RLC_UM",
                                        ns3::EnumValue (LteEnbRrc::RLC_AM_ALWAYS),
                                        ns3::MakeEnumChecker (LteEnbRrc::RLC_AM_ALWAYS, "RlcAm",
                                                              LteEnbRrc::RLC_UM_ALWAYS, "RlcUm"));

static ns3::GlobalValue g_rlcReportBufferStatusTimer ("rlcAmRbsTimer",
                                                      "Set value of ReportBufferStatusTimer attribute of RlcAm.",
                                                      ns3::UintegerValue (20),
                                                      ns3::MakeUintegerChecker<uint32_t> ());

// Parse context strings of the form "/NodeList/3/DeviceList/1/Mac/Assoc"
// to extract the NodeId
uint32_t
ContextToNodeId (std::string context)
{
  std::string sub = context.substr (10);  // skip "/NodeList/"
  uint32_t pos = sub.find ("/Device");
  NS_LOG_DEBUG ("Found NodeId " << atoi (sub.substr (0, pos).c_str ()));
  return atoi (sub.substr (0,pos).c_str ());
}

// Generate a 'fake' context id that can be parsed by ContextToNodeId ()
// For use in direct calls to "TraceConnect()" for LAA code, so that
// uniform ContextToNodeId() can be used in Wifi and LAA callbacks
std::string
NodeIdToContext (uint32_t nodeId)
{
  std::stringstream ss;
  ss << "/NodeList/";
  ss << nodeId;
  ss << "/DeviceList/";
  return ss.str ();
}

// Parse context strings of the form "/NodeList/3/DeviceList/1/Mac/Assoc"
// to extract the DeviceId
uint32_t
ContextToDeviceId (std::string context)
{
  uint32_t pos = context.find ("/Device");
  std::string sub = context.substr (pos + 12); // skip "/DeviceList/"
  pos = sub.find ("/Mac");
  NS_LOG_DEBUG ("Found DeviceId " << atoi (sub.substr (0, pos).c_str ()));
  return atoi (sub.substr (0,pos).c_str ());
}

std::string
CellConfigToString (enum Config_e config)
{
  if (config == WIFI)
    {
      return "Wi-Fi";
    }
  else if (config == LTE)
    {
      return "LTE";
    }
  else
    {
      return "LAA";
    }
}

std::string
CtrlSignalToString (std::list<Ptr<LteControlMessage> > ctrlMsg)
{
  std::string msgString="Unknown_CTRL";
  std::string tempString="Unknown_CTRL";;
  std::list<Ptr<LteControlMessage> >::iterator it;
  it = ctrlMsg.begin ();
  bool firstTime = true;
  while (it != ctrlMsg.end ())
    {
      Ptr<LteControlMessage> msg = (*it);
      switch (msg->GetMessageType())
      {
        case LteControlMessage::MIB:
          tempString="MIB";
          break;

        case LteControlMessage::SIB1:
          tempString="SIB1";
          break;

        case LteControlMessage::DRS:
          tempString="DRS";
          break;

        case LteControlMessage::DL_DCI:
          tempString="DL_DCI";
          break;

        case LteControlMessage::UL_DCI:
          tempString="UL_DCI";
          break;

        case LteControlMessage::BSR:
          tempString="BSR";
          break;

        case LteControlMessage::DL_HARQ:
          tempString="DL_HARQ";
          break;

        case LteControlMessage::RACH_PREAMBLE:
          tempString="RACH_PREAMBLE";
          break;

        case LteControlMessage::RAR:
          tempString="RAR";
          break;

        default:
          tempString="Unknown_CTRL";
          break;
      }

      if(firstTime)
        {
          msgString=tempString;
          firstTime = false;
        }
      else
        {
          msgString=msgString+":"+tempString;
        }
      ++it;
    }
  return msgString;
}


Ptr<Node>
MacAddressToNode (Mac48Address address)
{
  Ptr<Node> n;
  Ptr<NetDevice> nd;
  for (uint32_t i = 0; i < NodeContainer::GetGlobal ().GetN (); i++)
    {
      n = NodeContainer::GetGlobal ().Get (i);
      for (uint32_t j = 0; j < n->GetNDevices (); j++)
        {
          nd = n->GetDevice (j);
          Ptr<WifiNetDevice> wnd = nd->GetObject<WifiNetDevice> ();
          if (wnd != 0)
            {
              Address a = wnd->GetAddress ();
              Mac48Address mac = Mac48Address::ConvertFrom (a);
              if (address == mac)
                {
                  NS_LOG_DEBUG ("Found node " << n->GetId () << " for address " << address);
                  return n;
                }
            }
        }
    }
  return 0;
}

Ptr<WifiNetDevice>
FindFirstWifiNetDevice (Ptr<Node> ap)
{
  Ptr<WifiNetDevice> wifi;
  Ptr<NetDevice> nd;
  for (uint32_t i = 0; i < ap->GetNDevices (); i++)
    {
      nd = ap->GetDevice (i);
      wifi = DynamicCast<WifiNetDevice> (nd);
      if (wifi)
        {
          NS_LOG_DEBUG ("Found wifi device on interface " << i);
          return wifi;
        }
    }
  return 0;
}

Ptr<CsmaNetDevice>
FindFirstCsmaNetDevice (Ptr<Node> ap)
{
  Ptr<CsmaNetDevice> csma;
  Ptr<NetDevice> nd;
  for (uint32_t i = 0; i < ap->GetNDevices (); i++)
    {
      nd = ap->GetDevice (i);
      csma = DynamicCast<CsmaNetDevice> (nd);
      if (csma)
        {
          NS_LOG_DEBUG ("Found csma device on interface " << i);
          return csma;
        }
    }
  return 0;
}

Ptr<CsmaNetDevice>
GetClientDevice (Ptr<CsmaNetDevice> csma)
{
  for (uint32_t i = 0; i < csma->GetChannel ()->GetNDevices (); ++i)
    {
      Ptr<NetDevice> tmp = csma->GetChannel ()->GetDevice (i);
      Ptr<CsmaNetDevice> tmp2 = DynamicCast<CsmaNetDevice> (tmp);
      NS_ASSERT (tmp2);
      if (tmp2->GetNode ()->GetNDevices () == 2)
        {
          // APs will have 3 NetDevices (Wifi, Loopback, Csma)
          // client nodes will have only Loopback and Csma
          return tmp2;
        }
    }
  NS_ASSERT (false);
  return 0;
}

Ptr<PointToPointNetDevice>
GetRemoteDevice (Ptr<Node> ap, Ptr<PointToPointNetDevice> p2p)
{
  for (uint32_t i = 0; i < p2p->GetChannel ()->GetNDevices (); ++i)
    {
      Ptr<NetDevice> tmp = p2p->GetChannel ()->GetDevice (i);
      Ptr<PointToPointNetDevice> tmp2 = DynamicCast<PointToPointNetDevice> (tmp);
      if (tmp2 != p2p)
        {
          return tmp2;
        }
    }
  NS_ASSERT (false);
  return 0;
}


struct AssociationEvent
{
  Time m_time;
  uint32_t m_nodeId;
  std::string m_type;
  Mac48Address m_address;
  uint32_t m_apNodeId;
};

struct SignalArrival
{
  Time m_time;
  Time m_duration;
  bool m_wifi;
  uint32_t m_nodeId;
  uint32_t m_senderNodeId;
  double m_power;
};

struct TxopLog
{
  Time m_time;
  Time m_duration;
  uint32_t m_nodeId;
};

struct DataTx
{
  Time m_time;
  uint32_t m_nodeId;
  uint32_t m_size;
};

struct BeaconArrival
{
  Time m_time;
  Time m_interval;
  uint32_t m_nodeId;
};

struct CwChange
{
  Time m_time;
  uint32_t m_nodeId;
  uint32_t m_oldCw;
  uint32_t m_newCw;
};

struct BackoffChange
{
  Time m_time;
  uint32_t m_nodeId;
  uint32_t m_oldBackoff;
  uint32_t m_newBackoff;
};

struct HarqFeedbackLog
{
  Time m_time;
  uint32_t m_nodeId;
  uint32_t m_nackCount;
  uint32_t m_ackCount;
  std::vector<uint32_t> m_ueId;
};

struct ReservationSignalLog
{
  Time m_time;
  uint32_t m_nodeId;
  Time m_duration;
};

struct WifiRetriesLog
{
  Time m_time;
  uint32_t m_nodeId;
  Mac48Address m_dest;
};

struct VoiceRxLog
{
  Time m_time;
  uint32_t m_nodeId;
  uint32_t m_seqno;
  double m_latency;
};

struct CtrlSignalLog
{
  Time m_time;
  uint32_t m_nodeId;
  std::string m_ctrlType;
};


std::vector<AssociationEvent> g_associations;
std::vector<SignalArrival> g_arrivals;
std::vector<TxopLog> g_txopLogs;
std::vector<BeaconArrival> g_beaconArrivals;
std::vector<CwChange> g_cwChanges;
std::vector<BackoffChange> g_backoffChanges;
std::vector<HarqFeedbackLog> g_harqFeedbacks;
std::vector<ReservationSignalLog> g_reservationSingals;
std::vector<WifiRetriesLog> g_wifiFailRetries;
std::vector<WifiRetriesLog> g_wifiRetries;
std::vector<VoiceRxLog> g_voiceRxLog;
std::vector<DataTx> g_dataTxLogs;
std::vector<CtrlSignalLog> g_ctrlSignalLog;

double g_txopDurationCounter = 0;
double g_arrivalsDurationCounter = 0;


void
DeassociationLogging (std::string context, Mac48Address address)
{
  // We receive the context string of the STA that has just deassociated
  // and the BSSID of the AP in the 'address' parameter.

  NS_LOG_DEBUG ("Deassociation: " << context << " " << address);
  uint32_t myNodeId = ContextToNodeId (context);
  Ptr<Node> myNode = NodeContainer::GetGlobal ().Get (myNodeId);
  uint32_t myDeviceId = ContextToDeviceId (context);
  Ptr<Ipv4> myIp = myNode->GetObject<Ipv4> ();
  Ptr<NetDevice> myNd = myNode->GetDevice (myDeviceId);
  int32_t myIface = myIp->GetInterfaceForDevice (myNd);
  Ipv4InterfaceAddress myAddr = myIp->GetAddress (myIface, 0);
  NS_LOG_DEBUG ("Deassociating STA IP address is: " << myAddr.GetLocal ());
  Ptr<Node> ap = MacAddressToNode (address);
  AssociationEvent a;
  a.m_time = Simulator::Now ();
  a.m_nodeId = myNodeId;
  a.m_address = address;
  a.m_type = "deassociate";
  a.m_apNodeId=ap->GetId();
  g_associations.push_back (a);
}

void
VoiceRxCb (std::string context, Ptr<const Packet> packet)
{
  SeqTsHeader seqTs;
  packet->PeekHeader (seqTs);
  SequenceNumber32 currentSequenceNumber (seqTs.GetSeq ());
  Time sendTime = seqTs.GetTs ();
  double latencySample = Simulator::Now ().GetSeconds () - sendTime.GetSeconds ();
  VoiceRxLog entry;
  entry.m_time = Simulator::Now ();
  entry.m_nodeId = ContextToNodeId (context);
  entry.m_seqno = currentSequenceNumber.GetValue ();
  entry.m_latency = latencySample;
  g_voiceRxLog.push_back (entry);
}

void
WifiFailRetriesCb (std::string context, Mac48Address dest)
{
  WifiRetriesLog entry;
  entry.m_time = Simulator::Now ();
  entry.m_nodeId = ContextToNodeId (context);
  entry.m_dest = dest;
  g_wifiFailRetries.push_back (entry);
}

void
WifiRetriesCb (std::string context, Mac48Address dest)
{
  WifiRetriesLog entry;
  entry.m_time = Simulator::Now ();
  entry.m_nodeId = ContextToNodeId (context);
  entry.m_dest = dest;
  g_wifiRetries.push_back (entry);
}

void
CwChangeCb (std::string context, uint32_t oldVal, uint32_t newVal)
{
  CwChange cwchange;
  cwchange.m_time = Simulator::Now ();
  cwchange.m_nodeId = ContextToNodeId (context);
  cwchange.m_oldCw = oldVal;
  cwchange.m_newCw = newVal;
  g_cwChanges.push_back (cwchange);
}

void
BackoffChangeCb (std::string context, uint32_t oldVal, uint32_t newVal)
{
  BackoffChange bc;
  bc.m_time = Simulator::Now ();
  bc.m_nodeId = ContextToNodeId (context);
  bc.m_oldBackoff = oldVal;
  bc.m_newBackoff = newVal;
  g_backoffChanges.push_back (bc);
}

void
BeaconArrivalCb (std::string context, Time oldVal, Time newVal)
{
  BeaconArrival ba;
  ba.m_time = newVal;
  ba.m_interval = newVal - oldVal;
  ba.m_nodeId = ContextToNodeId (context);
  g_beaconArrivals.push_back (ba);
}

void
SignalCb (std::string context, bool wifi, uint32_t senderNodeId, double rxPowerDbm, Time rxDuration)
{
  SignalArrival arr;
  arr.m_time = Simulator::Now();
  arr.m_duration = rxDuration;
  arr.m_nodeId = ContextToNodeId (context);
  arr.m_senderNodeId = senderNodeId;
  arr.m_wifi = wifi;
  arr.m_power = rxPowerDbm;
  g_arrivals.push_back (arr);

  UintegerValue uintegerValue;
  GlobalValue::GetValueByName ("logPhyNodeId", uintegerValue);
  if ((uintegerValue.Get () == UINT32_MAX || uintegerValue.Get () == arr.m_nodeId) && !wifi)
    {
      UintegerValue uintegerValue;
      GlobalValue::GetValueByName ("logTxopNodeId", uintegerValue);
      if  (uintegerValue.Get () == arr.m_senderNodeId)
        {
          g_arrivalsDurationCounter += rxDuration.GetSeconds();
        }
    }

  NS_LOG_DEBUG (context << " " << wifi << " " << senderNodeId << " " << rxPowerDbm << " " << rxDuration.GetSeconds ()/1000.0);
}

void
CtrlSignalCb (std::string context, std::list<Ptr<LteControlMessage> > ctrlMsg)
{
  if (ctrlMsg.size () > 0)
  {
    CtrlSignalLog ctrlsig;
    ctrlsig.m_time=Simulator::Now();
    ctrlsig.m_nodeId=ContextToNodeId (context);
    ctrlsig.m_ctrlType=CtrlSignalToString(ctrlMsg);
    g_ctrlSignalLog.push_back (ctrlsig);

    NS_LOG_DEBUG (context << " " << ctrlsig.m_nodeId << " " <<ctrlsig.m_ctrlType<<" " << ctrlsig.m_time.GetSeconds ()/1000.0);
  }

}

void
TxopReceived (std::string context, Time startTime, Time duration, Time nextSubframeStarts)
{
  TxopLog txopLog;
  txopLog.m_time = Simulator::Now();
  txopLog.m_duration = duration;
  txopLog.m_nodeId = ContextToNodeId (context);
  g_txopLogs.push_back (txopLog);

  UintegerValue uintegerValue;
  GlobalValue::GetValueByName ("logTxopNodeId", uintegerValue);
  if (uintegerValue.Get () == txopLog.m_nodeId)
    {
      g_txopDurationCounter += duration.GetSeconds();
    }
  NS_LOG_DEBUG (context <<" " << txopLog.m_nodeId << " " << txopLog.m_time << " " << txopLog.m_duration.GetSeconds());
}

void
LteDataTxCallback (std::string context, uint32_t bytes)
{
  DataTx dataTxLog;
  dataTxLog.m_time = Simulator::Now();
  dataTxLog.m_nodeId = ContextToNodeId (context);
  dataTxLog.m_size = bytes;
  g_dataTxLogs.push_back(dataTxLog);
}


void
HarqFeedbackReceived (std::string context, std::vector<DlInfoListElement_s> m_dlInfoListReceived)
{
  HarqFeedbackLog harq;
  harq.m_time = Simulator::Now();
  harq.m_nodeId = ContextToNodeId(context);

  uint32_t nackCounter = 0;
  std::vector<uint8_t> harqFeedback;
   for (uint16_t i = 0; i < m_dlInfoListReceived.size(); i++)
     {
       for (uint8_t layer = 0; layer < m_dlInfoListReceived.at(i).m_harqStatus.size (); layer++)
         {
           if (m_dlInfoListReceived.at(i).m_harqStatus.at(layer) == DlInfoListElement_s::ACK)
             {
               harqFeedback.push_back(0);
               harq.m_ueId.push_back(m_dlInfoListReceived.at(i).m_rnti);
             }
           else if (m_dlInfoListReceived.at(i).m_harqStatus.at(layer) == DlInfoListElement_s::NACK)
             {
               harqFeedback.push_back(1);
               nackCounter++;
               harq.m_ueId.push_back(m_dlInfoListReceived.at(i).m_rnti);
             }
         }
     }

  if (harqFeedback.size())
    harq.m_ackCount = harqFeedback.size() - nackCounter;
  else
    harq.m_ackCount = 0;
  harq.m_nackCount = nackCounter;
  g_harqFeedbacks.push_back(harq);
  NS_LOG_DEBUG (context << " " << harq.m_nodeId << " " << nackCounter << " " << harqFeedback.size() - nackCounter);
}

void
ReservationSignalTraceReceived (std::string context, Time startTime, Time duration)
{
  ReservationSignalLog resSigLog;
  resSigLog.m_time = startTime;
  resSigLog.m_nodeId = ContextToNodeId (context);
  resSigLog.m_duration = duration;
  g_reservationSingals.push_back(resSigLog);
}

void
CwChangeConnect (Ptr<NetDevice> lteEnbNetDevice)
{
  Ptr<LbtAccessManager> lbtAccessManager = DynamicCast<LbtAccessManager>(lteEnbNetDevice->GetObject<LteEnbNetDevice>()->GetPhy()->GetChannelAccessManager());
  NS_ASSERT_MSG(lbtAccessManager!=0, "LbtAccessManager does not exist");
  std::string context = NodeIdToContext (lteEnbNetDevice->GetNode ()->GetId ());
  bool success = lbtAccessManager->TraceConnect("Cw", context, MakeCallback(&CwChangeCb));
  NS_ASSERT (success);
}

void
CwChangeDisconnect (Ptr<NetDevice> lteEnbNetDevice)
{
  Ptr<LbtAccessManager> lbtAccessManager = DynamicCast<LbtAccessManager>(lteEnbNetDevice->GetObject<LteEnbNetDevice>()->GetPhy()->GetChannelAccessManager());
  NS_ASSERT_MSG(lbtAccessManager!=0, "LbtAccessManager does not exist");
  std::string context = NodeIdToContext (lteEnbNetDevice->GetNode ()->GetId ());
  bool success = lbtAccessManager->TraceDisconnect("Cw", context, MakeCallback(&CwChangeCb));
  NS_ASSERT (success);
}

void
BackoffChangeConnect (Ptr<NetDevice> lteEnbNetDevice)
{
  Ptr<LbtAccessManager> lbtAccessManager = DynamicCast<LbtAccessManager>(lteEnbNetDevice->GetObject<LteEnbNetDevice>()->GetPhy()->GetChannelAccessManager());
  NS_ASSERT_MSG(lbtAccessManager!=0, "LbtAccessManager does not exist");
  std::string context = NodeIdToContext (lteEnbNetDevice->GetNode ()->GetId ());
  bool success = lbtAccessManager->TraceConnect("Backoff", context, MakeCallback(&BackoffChangeCb));
  NS_ASSERT (success);
}

void
BackoffChangeDisconnect (Ptr<NetDevice> lteEnbNetDevice)
{
  Ptr<LbtAccessManager> lbtAccessManager = DynamicCast<LbtAccessManager>(lteEnbNetDevice->GetObject<LteEnbNetDevice>()->GetPhy()->GetChannelAccessManager());
  NS_ASSERT_MSG(lbtAccessManager!=0, "LbtAccessManager does not exist");
  std::string context = NodeIdToContext (lteEnbNetDevice->GetNode ()->GetId ());
  bool success = lbtAccessManager->TraceDisconnect("Backoff", context, MakeCallback(&BackoffChangeCb));
  NS_ASSERT (success);
}

void
HarqFeedbackConnect (Ptr<NetDevice> lteEnbNetDevice)
{
  Ptr<LteEnbMac> lteEnbMac = lteEnbNetDevice->GetObject<LteEnbNetDevice>()->GetMac();
  NS_ASSERT_MSG(lteEnbMac!=0, "lteEnbMac does not exist");
  std::string context = NodeIdToContext (lteEnbNetDevice->GetNode ()->GetId ());
  bool success = lteEnbMac->TraceConnect("DlHarqFeedback", context, MakeCallback(&HarqFeedbackReceived));
  NS_ASSERT (success);
}

void
HarqFeedbackDisconnect (Ptr<NetDevice> lteEnbNetDevice)
{
  Ptr<LteEnbMac> lteEnbMac = lteEnbNetDevice->GetObject<LteEnbNetDevice>()->GetMac();
  NS_ASSERT_MSG(lteEnbMac!=0, "lteEnbMac does not exist");
  std::string context = NodeIdToContext (lteEnbNetDevice->GetNode ()->GetId ());
  bool success = lteEnbMac->TraceDisconnect("DlHarqFeedback", context, MakeCallback(&HarqFeedbackReceived));
  NS_ASSERT (success);
}

void
ReservationSignalTraceConnect (Ptr<NetDevice> lteEnbNetDevice)
{
  Ptr<LteEnbPhy> lteEnbPhy = lteEnbNetDevice->GetObject<LteEnbNetDevice>()->GetPhy();
  NS_ASSERT_MSG(lteEnbPhy!=0, "lteEnbPhy does not exist");
  std::string context = NodeIdToContext (lteEnbNetDevice->GetNode ()->GetId ());
  bool success = lteEnbPhy->TraceConnect("ReservationSignal", context, MakeCallback(&ReservationSignalTraceReceived));
  NS_ASSERT (success);
}

void
ReservationSignalTraceDisconnect (Ptr<NetDevice> lteEnbNetDevice)
{
  Ptr<LteEnbPhy> lteEnbPhy = lteEnbNetDevice->GetObject<LteEnbNetDevice>()->GetPhy();
  NS_ASSERT_MSG(lteEnbPhy!=0, "lteEnbPhy does not exist");
  std::string context = NodeIdToContext (lteEnbNetDevice->GetNode ()->GetId ());
  bool success = lteEnbPhy->TraceDisconnect("ReservationSignal", context, MakeCallback(&ReservationSignalTraceReceived));
  NS_ASSERT (success);
}

void
ScheduleWifiBackoffLogConnect (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/BE_Txop/BackoffTrace", MakeCallback (&BackoffChangeCb));
  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/BE_Txop/BackoffTrace", MakeCallback (&BackoffChangeCb));
}

void
ScheduleWifiBackoffLogDisconnect (void)
{
  Config::Disconnect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/BE_Txop/BackoffTrace", MakeCallback (&BackoffChangeCb));
  Config::Disconnect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/BE_Txop/BackoffTrace", MakeCallback (&BackoffChangeCb));
}

void
ScheduleCwChangesLogConnect (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/BE_Txop/CwTrace", MakeCallback (&CwChangeCb));
  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/BE_Txop/CwTrace", MakeCallback (&CwChangeCb));
}

void
ScheduleCwChangesLogDisconnect (void)
{
  Config::Disconnect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/BE_Txop/CwTrace", MakeCallback (&CwChangeCb));
  Config::Disconnect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/BE_Txop/CwTrace", MakeCallback (&CwChangeCb));
}

void
ScheduleWifiFailRetriesLogConnect (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/RemoteStationManager/MacTxFinalDataFailed", MakeCallback (&WifiFailRetriesCb));
}

void
ScheduleWifiFailRetriesLogDisconnect (void)
{
  Config::Disconnect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/RemoteStationManager/MacTxFinalDataFailed", MakeCallback (&WifiFailRetriesCb));
}

void
ScheduleWifiRetriesLogConnect (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/RemoteStationManager/MacTxDataFailed", MakeCallback (&WifiRetriesCb));
}

void
ScheduleWifiRetriesLogDisconnect (void)
{
  Config::Disconnect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/RemoteStationManager/MacTxDataFailed", MakeCallback (&WifiRetriesCb));
}

void
SchedulePhyLogConnect (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/$ns3::SpectrumWifiPhy/SignalArrival", MakeCallback (&SignalCb));
}

void
SchedulePhyLogDisconnect (void)
{
  Config::Disconnect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/$ns3::SpectrumWifiPhy/SignalArrival", MakeCallback (&SignalCb));
}

void
ScheduleTxopLogConnect (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::LteNetDevice/$ns3::LteEnbNetDevice/LteEnbPhy/Txop", MakeCallback (&TxopReceived));
}

void
ScheduleTxopLogDisconnect (void)
{
  Config::Disconnect ("/NodeList/*/DeviceList/*/$ns3::LteNetDevice/$ns3::LteEnbNetDevice/LteEnbPhy/Txop", MakeCallback (&TxopReceived));
}

void
ScheduleDataTxConnect (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbPhy/DataSent", MakeCallback (&LteDataTxCallback));
}

void
ScheduleDataTxDisconnect (void)
{
  Config::Disconnect ("/NodeList/*/DeviceList/*/LteEnbPhy/DataSent", MakeCallback (&LteDataTxCallback));
}

void
ScheduleBeaconLogConnect (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/Mac/$ns3::StaWifiMac/BeaconArrival", MakeCallback (&BeaconArrivalCb));
}

void
ScheduleBeaconLogDisconnect (void)
{
  Config::Disconnect ("/NodeList/*/DeviceList/*/Mac/$ns3::StaWifiMac/BeaconArrival", MakeCallback (&BeaconArrivalCb));
}

void
ScheduleCtrlSignalLogConnect (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbPhy/CtrlMsgTransmission", MakeCallback (&CtrlSignalCb));
}

void
ScheduleCtrlSignalLogDisconnect (void)
{
  Config::Disconnect ("/NodeList/*/DeviceList/*/LteEnbPhy/CtrlMsgTransmission", MakeCallback (&CtrlSignalCb));
}

void
ConfigureRouteForStation (std::string context, Mac48Address address)
{
  // We receive the context string of the STA that has just associated
  // and the BSSID of the AP in the 'address' parameter.
  // We need to install the IP address of the AP as this STA's default
  // route.  We need to install the IP address of the AP's point-to-point
  // interface with the client node, as a next hop host route to this STA.

  // Step 1: Obtain STA IP address
  NS_LOG_DEBUG ("ConfigureRouteForStation: " << context << " " << address);
  uint32_t myNodeId = ContextToNodeId (context);
  Ptr<Node> myNode = NodeContainer::GetGlobal ().Get (myNodeId);
  uint32_t myDeviceId = ContextToDeviceId (context);
  Ptr<Ipv4> myIp = myNode->GetObject<Ipv4> ();
  Ptr<NetDevice> myNd = myNode->GetDevice (myDeviceId);
  int32_t myIface = myIp->GetInterfaceForDevice (myNd);
  Ipv4InterfaceAddress myAddr = myIp->GetAddress (myIface, 0);
  NS_LOG_DEBUG ("STA IP address is: " << myAddr.GetLocal ());

  // Step 2: Install default route to AP on STA
  Ptr<Node> ap = MacAddressToNode (address);
  Ptr<WifiNetDevice> wifi = FindFirstWifiNetDevice (ap);
  Ptr<Ipv4> ip = ap->GetObject<Ipv4> ();
  int32_t iface = ip->GetInterfaceForDevice (wifi);
  Ipv4InterfaceAddress apAddr = ip->GetAddress (iface, 0);
  NS_LOG_DEBUG ("ConfigureRouteForStation ap addr: " << apAddr.GetLocal ());
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> myStaticRouting = ipv4RoutingHelper.GetStaticRouting (myNode->GetObject<Ipv4> ());
  myStaticRouting->SetDefaultRoute (apAddr.GetLocal (), myIface);
  NS_LOG_DEBUG ("Setting STA default to: " << apAddr.GetLocal () << " " << myIface);

  // Step 3: Install host route on client node
  Ptr<CsmaNetDevice> csma = FindFirstCsmaNetDevice (ap);
  iface = ip->GetInterfaceForDevice (csma);
  apAddr = ip->GetAddress (iface, 0);
  Ptr<CsmaNetDevice> remote = GetClientDevice (csma);
  ip = remote->GetNode ()->GetObject<Ipv4> ();
  iface = ip->GetInterfaceForDevice (remote);
  Ptr<Ipv4StaticRouting> clientStaticRouting = ipv4RoutingHelper.GetStaticRouting (remote->GetNode ()->GetObject<Ipv4> ());
  clientStaticRouting->AddHostRouteTo (myAddr.GetLocal (), apAddr.GetLocal (), iface);
  NS_LOG_DEBUG ("Setting client host route to " << myAddr.GetLocal () << " to nextHop " << apAddr << " " << iface);

  // Log the association event
  AssociationEvent a;
  a.m_time = Simulator::Now ();
  a.m_nodeId = myNodeId;
  a.m_address = address;
  a.m_type = "associate";
  a.m_apNodeId=ap->GetId();
  g_associations.push_back (a);
}

void
PrintFlowMonitorStats (Ptr<FlowMonitor> monitor, FlowMonitorHelper& flowmonHelper, double duration)
{
  // Print per-flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      if (t.sourcePort == VOICE_PORT || t.destinationPort == VOICE_PORT)
        {
          continue; //skip voice flow
        }
      std::stringstream protoStream;
      protoStream << (uint16_t) t.protocol;
      if (t.protocol == 6)
        {
          protoStream.str ("TCP");
        }
      if (t.protocol == 17)
        {
          protoStream.str ("UDP");
        }
      std::cout << "Flow " << i->first << " (" << t.sourceAddress << ":" << t.sourcePort << " -> " << t.destinationAddress << ":" << t.destinationPort << ") proto " << protoStream.str () << "\n";
      std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
      std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
      std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / duration / 1000 / 1000  << " Mbps\n";
      std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      if (i->second.rxPackets > 0)
        {
          // Measure the duration of the flow from receiver's perspective
          double rxDuration = i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ();
          std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / rxDuration / 1000 / 1000  << " Mbps\n";
          std::cout << "  Mean delay:  " << 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets << " ms\n";
          std::cout << "  Mean jitter:  " << 1000 * i->second.jitterSum.GetSeconds () / i->second.rxPackets  << " ms\n";
        }
      else
        {
          std::cout << "  Throughput:  0 Mbps\n";
          std::cout << "  Mean delay:  0 ms\n";
          std::cout << "  Mean jitter: 0 ms\n";
        }
      std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
    }
}

bool
MatchingFlow (Ipv4FlowClassifier::FiveTuple first, Ipv4FlowClassifier::FiveTuple second)
{
  if ((first.sourceAddress == second.destinationAddress) &&
      (first.destinationAddress == second.sourceAddress) &&
      (first.protocol == second.protocol) &&
      (first.sourcePort == second.destinationPort) &&
      (first.destinationPort == second.sourcePort))
    {
      return true;
    }
  return false;
}

struct FlowRecord
{
  FlowId flowId;
  Ipv4FlowClassifier::FiveTuple fiveTuple;
  FlowMonitor::FlowStats flowStats;
};

void
SaveCtrlSigStats (std::string filename, const std::vector<CtrlSignalLog> & ctrlsig)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::app);
  outFile.setf (std::ios_base::fixed);

  if(!outFile.is_open())
    {
     NS_LOG_ERROR("Can't open file " << filename);
    }

   outFile << "#time(s) nodeId CtrlType" << std::endl;

   for (std::vector<CtrlSignalLog>::size_type i = 0; i != ctrlsig.size (); i++)
    {
      outFile << ctrlsig[i].m_time.GetSeconds() << " ";
      outFile << ctrlsig[i].m_nodeId << " ";
      outFile << ctrlsig[i].m_ctrlType << std::endl;
    }
  outFile.close ();

}


void
SaveSpectrumPhyStats (std::string filename, const std::vector<SignalArrival> &arrivals)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::app);
  UintegerValue uintegerValue;
  GlobalValue::GetValueByName ("logPhyNodeId", uintegerValue);
  outFile.setf (std::ios_base::fixed);

  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  outFile << "#time(s) nodeId type sender endTime(s) duration(ms)     powerDbm" << std::endl;
  for (std::vector<SignalArrival>::size_type i = 0; i != arrivals.size (); i++)
    {
      if (uintegerValue.Get () == UINT32_MAX || uintegerValue.Get () == arrivals[i].m_nodeId)
        {
          outFile << std::setprecision (9) << std::fixed << arrivals[i].m_time.GetSeconds () <<  " ";
          outFile << arrivals[i].m_nodeId << " ";
          outFile << ((arrivals[i].m_wifi == true) ? "wifi " : " lte ");
          outFile << arrivals[i].m_senderNodeId << " ";
          outFile <<  arrivals[i].m_time.GetSeconds () + arrivals[i].m_duration.GetSeconds () << " ";
          outFile << arrivals[i].m_duration.GetSeconds () * 1000.0 << " " << arrivals[i].m_power << std::endl;
        }
    }
  outFile.close ();
}

void
SaveTxopStats (std::string filename, const std::vector<TxopLog> &txopLogs)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::app);
  UintegerValue uintegerValue;
  GlobalValue::GetValueByName ("logTxopNodeId", uintegerValue);
  outFile.setf (std::ios_base::fixed);

  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  outFile << "#time(s) nodeId endTime(s) duration(ms)" << std::endl;
  for (std::vector<TxopLog>::size_type i = 0; i != txopLogs.size (); i++)
    {
      if (uintegerValue.Get () == UINT32_MAX || uintegerValue.Get () == txopLogs[i].m_nodeId)
        {
          outFile << std::setprecision (9) << std::fixed << txopLogs[i].m_time.GetSeconds () <<  " ";
          outFile << txopLogs[i].m_nodeId << " ";
          outFile << txopLogs[i].m_time.GetSeconds () + txopLogs[i].m_duration.GetSeconds () << " ";
          outFile << txopLogs[i].m_duration.GetSeconds () * 1000.0 << " " << std::endl;
        }
    }
  outFile.close ();
}

void
SaveDataTxStats (std::string filename, const std::vector<DataTx> &dataTxLogs)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::app);
  UintegerValue uintegerValue;
  GlobalValue::GetValueByName ("logTxopNodeId", uintegerValue);
  outFile.setf (std::ios_base::fixed);

  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  outFile << "#time(s) nodeId endTime(s) duration(ms)" << std::endl;
  for (std::vector<DataTx>::size_type i = 0; i != dataTxLogs.size (); i++)
    {
      if (uintegerValue.Get () == UINT32_MAX || uintegerValue.Get () == dataTxLogs[i].m_nodeId)
        {
          outFile << std::setprecision (9) << std::fixed << dataTxLogs[i].m_time.GetSeconds () <<  " ";
          outFile << dataTxLogs[i].m_nodeId << " ";
          outFile << dataTxLogs[i].m_size << " " << std::endl;
        }
    }
  outFile.close ();
}

void
SaveCwStats (std::string filename, const std::vector<CwChange> &changes)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::app);
  UintegerValue uintegerValue;
  GlobalValue::GetValueByName ("logCwNodeId", uintegerValue);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  outFile.setf (std::ios_base::fixed);
  outFile << "#time(s) nodeId oldCw newCw" << std::endl;
  for (std::vector<CwChange>::size_type i = 0; i != changes.size (); i++)
    {
      if (uintegerValue.Get () == UINT32_MAX || uintegerValue.Get () == changes[i].m_nodeId)
        {
          outFile << std::setprecision (9) << std::fixed << changes[i].m_time.GetSeconds () <<  " ";
          outFile << changes[i].m_nodeId << " ";
          outFile << changes[i].m_oldCw << " ";
          outFile << changes[i].m_newCw << std::endl;
        }
    }
}

void
SaveFailRetriesStats (std::string filename, const std::vector<WifiRetriesLog> &entries)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::app);
  //UintegerValue uintegerValue;
  //GlobalValue::GetValueByName ("logBackoffNodeId", uintegerValue);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  outFile.setf (std::ios_base::fixed);
  outFile << "#time(s) nodeId dest" << std::endl;
  for (std::vector<WifiRetriesLog>::size_type i = 0; i != entries.size (); i++)
    {
      //if (uintegerValue.Get () == UINT32_MAX || uintegerValue.Get () == entries[i].m_nodeId)
        {
          outFile << std::setprecision (9) << std::fixed << entries[i].m_time.GetSeconds () <<  " ";
          outFile << entries[i].m_nodeId << " ";
          outFile << entries[i].m_dest << std::endl;
        }
    }
}

void
SaveRetriesStats (std::string filename, const std::vector<WifiRetriesLog> &entries)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::app);
  //UintegerValue uintegerValue;
  //GlobalValue::GetValueByName ("logBackoffNodeId", uintegerValue);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  outFile.setf (std::ios_base::fixed);
  outFile << "#time(s) nodeId dest" << std::endl;
  for (std::vector<WifiRetriesLog>::size_type i = 0; i != entries.size (); i++)
    {
      //if (uintegerValue.Get () == UINT32_MAX || uintegerValue.Get () == entries[i].m_nodeId)
        {
          outFile << std::setprecision (9) << std::fixed << entries[i].m_time.GetSeconds () <<  " ";
          outFile << entries[i].m_nodeId << " ";
          outFile << entries[i].m_dest << std::endl;
        }
    }
}

void
SaveVoiceStats (std::string filename, NodeContainer nodes, const std::vector<VoiceRxLog> &entries)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::app);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  outFile.setf (std::ios_base::fixed);
  outFile << "#time(s) nodeId seqno dest" << std::endl;
  for (std::vector<VoiceRxLog>::size_type i = 0; i != entries.size (); i++)
    {
      if (nodes.Contains (entries[i].m_nodeId))
        {
          outFile << std::setprecision (9) << std::fixed << entries[i].m_time.GetSeconds () <<  " ";
          outFile << entries[i].m_nodeId << " ";
          outFile << entries[i].m_seqno << " ";
          // milliseconds
          outFile << (1000 * entries[i].m_latency) << std::endl;
        }
    }
}

void
SaveVoiceSummaryStats (std::string filename, NodeContainer nodes)
{
  bool voiceSenderFound = false;
  Ptr<Node> sender = nodes.Get (0);
  uint32_t numSent = 0;
  for (uint32_t j = 0; j < sender->GetNApplications (); j++)
    {
      Ptr<Application> app = sender->GetApplication (j);
      Ptr<VoiceApplication> voiceApp = DynamicCast <VoiceApplication> (app);
      if (voiceApp)
        {
          voiceSenderFound = true;
          numSent = voiceApp->GetNumSent ();
        }
    }

  if (!voiceSenderFound && numSent)
    {
      NS_LOG_DEBUG ("No voice app found; returning");
      return;
    }
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::app);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  outFile.setf (std::ios_base::fixed);

  outFile << "#Id    sent    rcvd rcvd/sent lost delayed latencyMean latencyStddev" << std::endl;
  // Iterate over receiver applications to gather stats
  for (uint32_t i = 1; i < nodes.GetN (); i++)
    {
      Ptr<Node> n = nodes.Get (i);
      for (uint32_t j = 0; j < n->GetNApplications (); j++)
        {
          Ptr<Application> app = n->GetApplication (j);
          Ptr<VoiceApplication> voiceApp = DynamicCast<VoiceApplication> (app);
          if (voiceApp)
            {
              NS_LOG_DEBUG ("Found client Voice app on node " << n->GetId ());
              outFile << std::setw (3) << n->GetId ()
                      << std::setw (8) << numSent
                      << std::setw (8) << voiceApp->GetNumReceived ()
                      << std::setw (10) << std::setprecision (3) << std::fixed
                      << (1.0 * voiceApp->GetNumReceived ()) / numSent
                      << std::setw (5) << voiceApp->GetNumLost ()
                      << std::setw (8) << voiceApp->GetNumDelayed ()
                      // milliseconds
                      << std::setw (12) << voiceApp->GetLatencyMean () * 1000
                      << std::setw (14) << voiceApp->GetLatencyStddev () * 1000
                      << std::endl;
            }
        }
    }

}

void
SaveBackoffStats (std::string filename, const std::vector<BackoffChange> &changes)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::app);
  UintegerValue uintegerValue;
  GlobalValue::GetValueByName ("logBackoffNodeId", uintegerValue);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  outFile.setf (std::ios_base::fixed);
  outFile << "#time(s) nodeId oldBackoff newBackoff" << std::endl;
  for (std::vector<BackoffChange>::size_type i = 0; i != changes.size (); i++)
    {
      if (uintegerValue.Get () == UINT32_MAX || uintegerValue.Get () == changes[i].m_nodeId)
        {
          outFile << std::setprecision (9) << std::fixed << changes[i].m_time.GetSeconds () <<  " ";
          outFile << changes[i].m_nodeId << " ";
          outFile << changes[i].m_oldBackoff << " ";
          outFile << changes[i].m_newBackoff << std::endl;
        }
    }
}

void
SaveHarqFeedbacksStats (std::string filename, const std::vector<HarqFeedbackLog> &harqs)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::app);
  UintegerValue uintegerValue;
  GlobalValue::GetValueByName ("logHarqFeedbackNodeId", uintegerValue);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  outFile.setf (std::ios_base::fixed);
  outFile << "#time(s) nodeId acks nacks" << std::endl;
  for (std::vector<HarqFeedbackLog>::size_type i = 0; i != harqs.size (); i++)
    {
      if (uintegerValue.Get () == UINT32_MAX || uintegerValue.Get () == harqs[i].m_nodeId)
        {
          outFile << std::setprecision (9) << std::fixed << harqs[i].m_time.GetSeconds () <<  " ";
          outFile << harqs[i].m_nodeId << " ";
          outFile << harqs[i].m_ackCount << " ";
          outFile << harqs[i].m_nackCount << " ";

          if ((harqs[i].m_ackCount + harqs[i].m_nackCount)>0)
            {
             outFile <<std::setprecision(2)<<((double)harqs[i].m_nackCount/(double)(harqs[i].m_ackCount + harqs[i].m_nackCount))*100 <<"%";
            }
          else
            {
              outFile <<"0";
            }
        }
    /* It is possible to print RNTI of UEs that have reported harq feedback by enabling this code.
     * Currently is disabled because it is easier to parse files when all rows have the same number of columns.
       for(std::vector<uint32_t>::size_type j=0; j != harqs[i].m_ueId.size(); j++)
        {
          outFile<<" "<<harqs[i].m_ueId[j]<<" ";
         }
    */
      outFile<<std::endl;
    }
}

void
SaveReservationSingalStats (std::string filename, const std::vector<ReservationSignalLog> &resSigs)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::app);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  outFile.setf (std::ios_base::fixed);
  outFile << "#time(s) nodeId duration (msec)" << std::endl;
  for (std::vector<HarqFeedbackLog>::size_type i = 0; i != resSigs.size (); i++)
    {
      outFile << std::setprecision (9) << std::fixed << resSigs[i].m_time.GetSeconds () <<  " ";
      outFile << resSigs[i].m_nodeId << " ";
      outFile << std::setprecision (9) << std::fixed << resSigs[i].m_duration.GetSeconds ()*1000 <<  " ";
      outFile<<std::endl;
    }
}

void
SaveBeaconStats (std::string filename, const std::vector<BeaconArrival> &arrivals)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::app);
  outFile.setf (std::ios_base::fixed);
  UintegerValue uintegerValue;
  GlobalValue::GetValueByName ("logBeaconNodeId", uintegerValue);

  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  outFile << "#time(s)   interval(s) nodeId" << std::endl;
  for (std::vector<BeaconArrival>::size_type i = 0; i != arrivals.size (); i++)
    {
      if (uintegerValue.Get () == UINT32_MAX || uintegerValue.Get () == arrivals[i].m_nodeId)
        {
          outFile << std::setprecision (9) << std::fixed << arrivals[i].m_time.GetSeconds () <<  " ";
          outFile << arrivals[i].m_interval.GetSeconds () <<  " ";
          outFile << arrivals[i].m_nodeId << std::endl;
        }
    }
}

void
SaveTcpFlowMonitorStats (std::string filename, std::string simulationParams, Ptr<FlowMonitor> monitor, FlowMonitorHelper& flowmonHelper, double duration)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::app);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  outFile.setf (std::ios_base::fixed);

  std::map<FlowId, FlowRecord> tupleMap;
  typedef std::map<FlowId, FlowRecord>::iterator tupleMapI;
  typedef std::pair<FlowRecord, FlowRecord> FlowPair;
  std::vector<FlowPair> downlinkFlowList;

  // Print per-flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      if (t.sourcePort == VOICE_PORT || t.destinationPort == VOICE_PORT)
        {
          continue; //skip voice flow
        }
      bool found = false;
      for (tupleMapI j = tupleMap.begin (); j != tupleMap.end (); ++j)
        {
          if (MatchingFlow (t, j->second.fiveTuple))
            {
              // We have found a pair of flow records; insert the pair
              // into the flowList.  i->second and j->second contain the
              // flow stats.  Erase the j record from the tupleMap
              FlowRecord firstFlow;
              firstFlow.flowId = i->first;
              firstFlow.fiveTuple = t;
              firstFlow.flowStats = i->second;
              FlowRecord secondFlow = j->second;
              // For downlink, the smaller IP address is the source
              // of the traffic in these scenarios
              FlowPair flowPair;
              if (t.sourceAddress < j->second.fiveTuple.sourceAddress)
                {
                  flowPair = std::make_pair (firstFlow, secondFlow);
                }
              else
                {
                  flowPair = std::make_pair (secondFlow, firstFlow);
                }
              downlinkFlowList.push_back (flowPair);
              tupleMap.erase (j);
              found = true;
              break;
            }
        }
      if (found == false)
        {
          FlowRecord newFlow;
          newFlow.flowId = i->first;
          newFlow.fiveTuple = t;
          newFlow.flowStats = i->second;
          tupleMap.insert (std::pair<FlowId, FlowRecord> (i->first, newFlow));
        }
    }
  for (std::vector<FlowPair>::size_type idx = 0; idx != downlinkFlowList.size (); idx++)
    {
      // statistics for the downlink (data stream)
      FlowRecord record = downlinkFlowList[idx].first;
      outFile << record.flowId << " " << record.fiveTuple.sourceAddress << ":" << record.fiveTuple.sourcePort
              << " " << record.fiveTuple.destinationAddress << ":" << record.fiveTuple.destinationPort
              << " " << record.flowStats.txPackets
              << " " << record.flowStats.txBytes
              // Mb/s
              << " " << (record.flowStats.txBytes * 8.0 / duration) / 1e6;
      if (record.flowStats.rxPackets > 0)
        {
          // Measure the duration of the flow from receiver's perspective
          double rxDuration = record.flowStats.timeLastRxPacket.GetSeconds () - record.flowStats.timeFirstTxPacket.GetSeconds ();
          outFile << " " << record.flowStats.rxBytes
                  << " " << (record.flowStats.rxBytes * 8.0 / rxDuration) / 1e6
                  // milliseconds
                  << " " << 1000 * record.flowStats.delaySum.GetSeconds () / record.flowStats.rxPackets;
          outFile << " " << 1000 * record.flowStats.jitterSum.GetSeconds () / record.flowStats.rxPackets;
        }
      else
        {
          outFile << "  0" // rxBytes
                  << "  0" // throughput
                  << "  0" // delaySum
                  << "  0"; // jitterSum
        }
      outFile << " " << record.flowStats.rxPackets;

      outFile << " "; // add space separator
      // statistics for the uplink (ACK stream)
      record = downlinkFlowList[idx].second;
      outFile << record.flowId << " " << record.fiveTuple.sourceAddress << ":" << record.fiveTuple.sourcePort << " "
              << record.fiveTuple.destinationAddress << ":" << record.fiveTuple.destinationPort
              << " " << record.flowStats.txPackets
              << " " << record.flowStats.txBytes
              << " " << (record.flowStats.txBytes * 8.0 / duration ) / 1e6;
      if (record.flowStats.rxPackets > 0)
        {
          // Measure the duration of the flow from receiver's perspective
          double rxDuration = record.flowStats.timeLastRxPacket.GetSeconds () - record.flowStats.timeFirstTxPacket.GetSeconds ();
          outFile << " " << record.flowStats.rxBytes
                  // Mb/s
                  << " " << (record.flowStats.rxBytes * 8.0 / rxDuration) / 1e6
                  // milliseconds
                  << " " << 1000 * record.flowStats.delaySum.GetSeconds () / record.flowStats.rxPackets
                  << " " << 1000 * record.flowStats.jitterSum.GetSeconds () / record.flowStats.rxPackets;
        }
      else
        {
          outFile << "  0" // rxBytes
                  << "  0" // throughput
                  << "  0" // delaySum
                  << "  0"; // jitterSum
        }
      outFile << " " << record.flowStats.rxPackets;
      outFile << std::endl;
    }
  outFile.close ();
}

void
SaveUdpFlowMonitorStats (std::string filename, std::string simulationParams, Ptr<FlowMonitor> monitor, FlowMonitorHelper& flowmonHelper, double duration)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::app);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  outFile.setf (std::ios_base::fixed);

  // Print per-flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      if (t.sourcePort == VOICE_PORT || t.destinationPort == VOICE_PORT)
        {
          continue; //skip voice flow
        }
      NS_ASSERT_MSG (t.sourceAddress < t.destinationAddress ,
                 "Flow " << t.sourceAddress << ":" << t.sourcePort << " --> " << t.destinationAddress << ":" << t.destinationPort
                 << " is probably not downlink");

      outFile << i->first
              << " " << t.sourceAddress << ":" << t.sourcePort
              << " " << t.destinationAddress << ":" << t.destinationPort
              << " " << i->second.txPackets
              << " " << i->second.txBytes
              // Mb/s
              << " " << (i->second.txBytes * 8.0 / duration) / 1e6;
      if (i->second.rxPackets > 0)
        {
          // Measure the duration of the flow from receiver's perspective
          double rxDuration = i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ();
          // Mb/s
          outFile << " " << i->second.rxBytes
                  // Mb/s
                  << " " << (i->second.rxBytes * 8.0 / rxDuration) / 1e6
                  // milliseconds
                  << " " << 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets
                  << " " << 1000 * i->second.jitterSum.GetSeconds () / i->second.rxPackets;
        }
      else
        {
          outFile << "  0" // rxBytes
                  << "  0" // throughput
                  << "  0" // delaySum
                  << "  0"; // jitterSum
        }
      outFile << " " << i->second.rxPackets << std::endl;
    }
  outFile.close ();
}

void
SaveAssociationStats(std::string filename, const std::vector<AssociationEvent> &events, NodeContainer bsnodes, NodeContainer uenodes)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::app);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  outFile.setf (std::ios_base::fixed);

  outFile << "#--- List of WiFi associations ---" << std::endl;
  outFile << "#time(s) nodeId(STA) event MAC(AP) nodeId(AP)" << std::endl;

  for (std::vector<AssociationEvent>::size_type i = 0; i < events.size (); i++)
    {
       outFile<< std::setprecision (6) << std::fixed << events[i].m_time.GetSeconds () << " " << events[i].m_nodeId << " " << events[i].m_type << " " << events[i].m_address <<" "<<events[i].m_apNodeId<< std::endl;
    }

  outFile  << "\n#--- LAA/LTE eNB info ---" << std::endl;
  outFile<<"nodeId(eNB) IP(eNB) cellId(eNB) position(eNB)"<<std::endl;

  for (NodeContainer::Iterator it = bsnodes.Begin (); it != bsnodes.End (); ++it)
    {
      Ptr<Node> bsnode = *it;
      int nDevs = bsnode->GetNDevices ();

      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteEnbNetDevice> enbdev = bsnode->GetDevice (j)->GetObject <LteEnbNetDevice> ();
          if (enbdev)
            {
              Vector pos = bsnode->GetObject<MobilityModel> ()->GetPosition ();
              Ptr<Ipv4> ipv4nue = bsnode->GetObject<Ipv4>();
              Ipv4InterfaceAddress ipv4_int_addr_nenb = ipv4nue->GetAddress(1,0);
              Ipv4Address ip_addr_nenb = ipv4_int_addr_nenb.GetLocal();

              outFile<<bsnode->GetId ()<<" "<<ip_addr_nenb<<" "<<enbdev->GetCellId()<<" "<< pos.x << "," << pos.y<< std::endl;
           }
        }
    }

  outFile  << "\n#--- LAA/LTE UE info ---" << std::endl;
  outFile<<"nodeId(UE) IP(UE) Imsi(UE) cellId(UE) position(UE)"<<std::endl;

  for (NodeContainer::Iterator it = uenodes.Begin (); it != uenodes.End (); ++it)
    {
      Ptr<Node> uenode = *it;
      int nDevs = uenode->GetNDevices ();
      uint64_t lteUeImsi;
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteUeNetDevice> uedev = uenode->GetDevice (j)->GetObject <LteUeNetDevice> ();
          if (uedev)
            {
              lteUeImsi =uedev->GetImsi();
              Vector pos = uenode->GetObject<MobilityModel> ()->GetPosition ();
              Ptr<Ipv4> ipv4nue = uenode->GetObject<Ipv4>();
              Ptr<LteUeRrc> lteUeRrc = uedev->GetRrc();
              Ipv4InterfaceAddress ipv4_int_addr_nue = ipv4nue->GetAddress(1,0);
              Ipv4Address ip_addr_nue = ipv4_int_addr_nue.GetLocal();

              outFile<<uenode->GetId ()<<" "<<ip_addr_nue<<" "<<lteUeImsi<<" "<<lteUeRrc->GetCellId()<<" " << pos.x << "," << pos.y<< std::endl;

           }
         }
    }

  outFile.close ();
}

void
ConfigureLte (Ptr<LteHelper> lteHelper, Ptr<PointToPointEpcHelper> epcHelper, Ipv4AddressHelper& internetIpv4Helper, NodeContainer bsNodes, NodeContainer ueNodes, NodeContainer clientNodes, NetDeviceContainer& bsDevices, NetDeviceContainer& ueDevices, struct PhyParams phyParams, std::vector<LteSpectrumValueCatcher>& lteDlSinrCatcherVector, std::bitset<40> absPattern, Transport_e transport)
{
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (2000000));

  // For LTE, the client node needs to be connected only to the PGW/SGW node
  // The EpcHelper will then take care of connecting the PGW/SGW node to the eNBs
  Ptr<Node> clientNode = clientNodes.Get (0);
  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.0)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, clientNode);

  Ipv4InterfaceContainer internetIpIfaces = internetIpv4Helper.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  //Ipv4Address clientNodeAddr = internetIpIfaces.GetAddress (1);

  // make LTE and network reachable from the client node
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> clientNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting (clientNode->GetObject<Ipv4> ());
  clientNodeStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);



  // LTE configuration parametes
  lteHelper->SetSchedulerType ("ns3::PfFfMacScheduler");
  lteHelper->SetSchedulerAttribute ("UlCqiFilter", EnumValue (FfMacScheduler::PUSCH_UL_CQI));
  // LTE-U DL transmission @5180 MHz
  lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (255444));
  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (100));
  // needed for initial cell search
  lteHelper->SetUeDeviceAttribute ("DlEarfcn", UintegerValue (255444));
  // LTE calibration
  lteHelper->SetEnbAntennaModelType ("ns3::IsotropicAntennaModel");
  lteHelper->SetEnbAntennaModelAttribute ("Gain",   DoubleValue (phyParams.m_bsTxGain));
  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (phyParams.m_bsTxPower));
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (phyParams.m_ueTxPower));

  EnumValue enumValue;
  enum LteEnbRrc::LteEpsBearerToRlcMapping_t rlcMode = LteEnbRrc::RLC_AM_ALWAYS;
  if (GlobalValue::GetValueByNameFailSafe ("tcpRlcMode", enumValue))
    {
      rlcMode = (LteEnbRrc::LteEpsBearerToRlcMapping_t) enumValue.Get ();
    }

  switch (transport)
    {
    case TCP:
      Config::SetDefault ("ns3::LteEnbRrc::EpsBearerToRlcMapping", EnumValue (rlcMode));
      break;

    case UDP:
    case FTP:
    default:
      Config::SetDefault ("ns3::LteEnbRrc::EpsBearerToRlcMapping", EnumValue (LteEnbRrc::RLC_UM_ALWAYS));
      break;
    }

  // Create Devices and install them in the Nodes (eNBs and UEs)
  bsDevices = lteHelper->InstallEnbDevice (bsNodes);
  ueDevices = lteHelper->InstallUeDevice (ueNodes);

  // additional eNB-specific configuration
  for (uint32_t n = 0; n < bsDevices.GetN (); ++n)
    {
      Ptr<NetDevice> enbDevice = bsDevices.Get (n);
      Ptr<LteEnbNetDevice> enbLteDevice = enbDevice->GetObject<LteEnbNetDevice> ();
      enbLteDevice->GetRrc ()->SetAbsPattern (absPattern);
    }

  NetDeviceContainer ueLteDevs (ueDevices);


  // additional UE-specific configuration
  Ipv4InterfaceContainer clientIpIfaces;
  NS_ASSERT_MSG (lteDlSinrCatcherVector.empty (), "Must provide an empty lteDlSinCatcherVector");
  // side effect: will create LteSpectrumValueCatchers
  // note that nobody else should resize this vector otherwise callbacks will be using invalid pointers
  lteDlSinrCatcherVector.resize (ueNodes.GetN ());

  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ue = ueNodes.Get (u);
      Ptr<NetDevice> ueDevice = ueLteDevs.Get (u);
      Ptr<LteUeNetDevice> ueLteDevice = ueDevice->GetObject<LteUeNetDevice> ();

      // assign IP address to UEs
      Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevice));
      clientIpIfaces.Add (ueIpIface);
      // set the default gateway for the UE
      Ipv4StaticRoutingHelper ipv4RoutingHelper;
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

      // set up SINR monitoring
      Ptr<LtePhy> uePhy = ueLteDevice->GetPhy ()->GetObject<LtePhy> ();
      Ptr<LteAverageChunkProcessor> monitorLteChunkProcessor  = Create<LteAverageChunkProcessor> ();
      monitorLteChunkProcessor->AddCallback (MakeCallback (&LteSpectrumValueCatcher::ReportValue, &lteDlSinrCatcherVector.at (u)));
      uePhy->GetDownlinkSpectrumPhy ()->AddDataSinrChunkProcessor (monitorLteChunkProcessor);
    }

  // instruct all devices to attach using the LTE initial cell selection procedure
  lteHelper->Attach (ueDevices);
}

void
ConfigureLaa (Ptr<LteHelper> lteHelper, Ptr<PointToPointEpcHelper> epcHelper, Ipv4AddressHelper& internetIpv4Helper, NodeContainer bsNodes,
              NodeContainer ueNodes, NodeContainer clientNodes, NetDeviceContainer& bsDevices, NetDeviceContainer& ueDevices, struct PhyParams phyParams,
              std::vector<LteSpectrumValueCatcher>& lteDlSinrCatcherVector, std::bitset<40> absPattern, Transport_e transport, Time lbtChannelAccessManagerInstallTime)
{

  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (2000000));

  // For LTE, the client node needs to be connected only to the PGW/SGW node
  // The EpcHelper will then take care of connecting the PGW/SGW node to the eNBs

  Ptr<Node> clientNode;

  // uf there are client nodes configure link to them
  if (clientNodes.GetN () != 0)
    {
      clientNode = clientNodes.Get (0);
      Ptr<Node> pgw = epcHelper->GetPgwNode ();
      PointToPointHelper p2ph;
      p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
      p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
      p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.0)));
      NetDeviceContainer internetDevices = p2ph.Install (pgw, clientNode);

      Ipv4InterfaceContainer internetIpIfaces = internetIpv4Helper.Assign (internetDevices);
      // interface 0 is localhost, 1 is the p2p device
      //Ipv4Address clientNodeAddr = internetIpIfaces.GetAddress (1);

      // make LTE and network reachable from the client node
      Ipv4StaticRoutingHelper ipv4RoutingHelper;
      Ptr<Ipv4StaticRouting> clientNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting (clientNode->GetObject<Ipv4> ());
      clientNodeStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
    }


  // LTE configuration parametes
  lteHelper->SetSchedulerType ("ns3::PfFfMacScheduler");
  lteHelper->SetSchedulerAttribute ("UlCqiFilter", EnumValue (FfMacScheduler::PUSCH_UL_CQI));
  // LTE-U DL transmission @5180 MHz
  lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (255444));
  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (100));
  // needed for initial cell search
  lteHelper->SetUeDeviceAttribute ("DlEarfcn", UintegerValue (255444));
  // LTE calibration
  lteHelper->SetEnbAntennaModelType ("ns3::IsotropicAntennaModel");
  lteHelper->SetEnbAntennaModelAttribute ("Gain",   DoubleValue (phyParams.m_bsTxGain));
  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (phyParams.m_bsTxPower));
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (phyParams.m_ueTxPower));

  EnumValue enumValue;
  enum LteEnbRrc::LteEpsBearerToRlcMapping_t rlcMode = LteEnbRrc::RLC_AM_ALWAYS;
  if (GlobalValue::GetValueByNameFailSafe ("tcpRlcMode", enumValue))
    {
      rlcMode = (LteEnbRrc::LteEpsBearerToRlcMapping_t) enumValue.Get ();
    }

  switch (transport)
    {
    case TCP:
      Config::SetDefault ("ns3::LteEnbRrc::EpsBearerToRlcMapping", EnumValue (rlcMode));
      break;

    case UDP:
    case FTP:
    default:
      Config::SetDefault ("ns3::LteEnbRrc::EpsBearerToRlcMapping", EnumValue (LteEnbRrc::RLC_UM_ALWAYS));
      break;
    }

  // Create Devices and install them in the Nodes (eNBs and UEs)
  bsDevices = lteHelper->InstallEnbDevice (bsNodes);
  ueDevices = lteHelper->InstallUeDevice (ueNodes);


  // additional eNB-specific configuration
  /* for (uint32_t n = 0; n < bsDevices.GetN (); ++n)
     {
       Ptr<NetDevice> enbDevice = bsDevices.Get (n);
       Ptr<LteEnbNetDevice> enbLteDevice = enbDevice->GetObject<LteEnbNetDevice> ();
       enbLteDevice->GetRrc ()->SetAbsPattern (absPattern);
     }*/

  // Check if we need to instantiate LteWifiCoexistence helper
  if (GlobalValue::GetValueByNameFailSafe ("ChannelAccessManager", enumValue))
    {
      enum Config_ChannelAccessManager channelAccessManager = (Config_ChannelAccessManager) enumValue.Get ();

      if (channelAccessManager == BasicLbt ||
          channelAccessManager == OnlyListen ||
          channelAccessManager == Lbt ||
          channelAccessManager == DutyCycle)
        {
          Ptr<LteWifiCoexistenceHelper> laaWifiCoexistenceHelper = CreateObject<LteWifiCoexistenceHelper> ();
          laaWifiCoexistenceHelper->ConfigureEnbDevicesForLbt (bsDevices, phyParams);
          //Simulator::Schedule (lbtChannelAccessManagerInstallTime, &LteWifiCoexistenceHelper::ConfigureEnbDevicesForLbt, laaWifiCoexistenceHelper, bsDevices, phyParams);
        }
    }

  // if there are ue devices configure and  attach them
  if (ueDevices.GetN () != 0)
    {
      NetDeviceContainer ueLteDevs (ueDevices);
      // additional UE-specific configuration
      Ipv4InterfaceContainer clientIpIfaces;
      NS_ASSERT_MSG (lteDlSinrCatcherVector.empty (), "Must provide an empty lteDlSinCatcherVector");
      // side effect: will create LteSpectrumValueCatchers
      // note that nobody else should resize this vector otherwise callbacks will be using invalid pointers
      lteDlSinrCatcherVector.resize (ueNodes.GetN ());

      for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
        {
          Ptr<Node> ue = ueNodes.Get (u);
          Ptr<NetDevice> ueDevice = ueLteDevs.Get (u);
          Ptr<LteUeNetDevice> ueLteDevice = ueDevice->GetObject<LteUeNetDevice> ();

          // assign IP address to UEs
          Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevice));
          clientIpIfaces.Add (ueIpIface);
          // set the default gateway for the UE
          Ipv4StaticRoutingHelper ipv4RoutingHelper;
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

          // set up SINR monitoring
          Ptr<LtePhy> uePhy = ueLteDevice->GetPhy ()->GetObject<LtePhy> ();
          Ptr<LteAverageChunkProcessor> monitorLteChunkProcessor  = Create<LteAverageChunkProcessor> ();
          monitorLteChunkProcessor->AddCallback (MakeCallback (&LteSpectrumValueCatcher::ReportValue, &lteDlSinrCatcherVector.at (u)));
          uePhy->GetDownlinkSpectrumPhy ()->AddDataSinrChunkProcessor (monitorLteChunkProcessor);
        }
      // instruct all devices to attach using the LTE initial cell selection procedure
      lteHelper->Attach (ueDevices);
    }
}

NetDeviceContainer
ConfigureWifiAp (NodeContainer bsNodes, struct PhyParams phyParams, Ptr<SpectrumChannel> channel, Ssid ssid)
{
  QueueSizeValue queueSize;
  GlobalValue::GetValueByName ("wifiQueueMaxSize", queueSize);
  // Original
  // Config::SetDefault ("ns3::WifiMacQueue::MaxSize", queueSize);
  Config::SetDefault ("ns3::QueueBase::MaxSize", queueSize);
  UintegerValue uValue;
  GlobalValue::GetValueByName ("wifiQueueMaxDelay", uValue);
  Config::SetDefault ("ns3::WifiMacQueue::MaxDelay", TimeValue (MilliSeconds (uValue.Get ())));
  NetDeviceContainer apDevices;
  SpectrumWifiPhyHelper spectrumPhy = SpectrumWifiPhyHelper::Default ();
  spectrumPhy.SetChannel (channel);
  // Note that LTE eNB rxGain is implemented through AntennaModel at SpectrumChannel, contrary to Wi-Fi which is handled by SpectrumWifiPhy only.
  // In addition, LAA has an AdHocWifiMac station to perform LBT (set in ConfigureEnbDevicesForLbt), that should have BS gain properties (namely Rx).
  spectrumPhy.Set ("TxGain", DoubleValue (phyParams.m_bsTxGain));
  spectrumPhy.Set ("RxGain", DoubleValue (phyParams.m_bsRxGain));
  spectrumPhy.Set ("TxPowerStart", DoubleValue (phyParams.m_bsTxPower));
  spectrumPhy.Set ("TxPowerEnd", DoubleValue (phyParams.m_bsTxPower));
  spectrumPhy.Set ("RxNoiseFigure", DoubleValue (phyParams.m_bsNoiseFigure));
  spectrumPhy.Set ("Antennas", UintegerValue (2));
  spectrumPhy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (2));
  spectrumPhy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (2));
  spectrumPhy.SetPcapDataLinkType (SpectrumWifiPhyHelper::DLT_IEEE802_11_RADIO);

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  WifiMacHelper mac;

  spectrumPhy.Set ("ChannelWidth", UintegerValue (20));
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "EnableBeaconJitter", BooleanValue (true));

  for (uint32_t i = 0; i < bsNodes.GetN (); i++)
    {
      //uint32_t channelNumber = 36 + 4 * (i%4);
      uint32_t channelNumber = 36;
      spectrumPhy.Set ("ChannelNumber", UintegerValue (channelNumber));
      apDevices.Add (wifi.Install (spectrumPhy, mac, bsNodes.Get (i)));
    }

  BooleanValue booleanValue;
  bool found;
  found = GlobalValue::GetValueByNameFailSafe ("pcapEnabled", booleanValue);
  if (found && booleanValue.Get () == true)
    {
      spectrumPhy.EnablePcap ("laa-wifi-ap", apDevices);
    }
  found = GlobalValue::GetValueByNameFailSafe ("asciiEnabled", booleanValue);
  if (found && booleanValue.Get () == true)
    {
      AsciiTraceHelper ascii;
      std::string prefix = "laa-wifi-ap";
      spectrumPhy.EnableAscii (prefix, apDevices);
    }
  return apDevices;
}

NetDeviceContainer
ConfigureWifiSta (NodeContainer ueNodes, struct PhyParams phyParams, Ptr<SpectrumChannel> channel, Ssid ssid)
{
  QueueSizeValue queueSize;
  GlobalValue::GetValueByName ("wifiQueueMaxSize", queueSize);
  // Original
  // Config::SetDefault ("ns3::WifiMacQueue::MaxSize", queueSize);
  Config::SetDefault ("ns3::QueueBase::MaxSize", queueSize);
  UintegerValue uValue;
  GlobalValue::GetValueByName ("wifiQueueMaxDelay", uValue);
  Config::SetDefault ("ns3::WifiMacQueue::MaxDelay", TimeValue (MilliSeconds (uValue.Get ())));

  NetDeviceContainer staDevices;
  SpectrumWifiPhyHelper spectrumPhy = SpectrumWifiPhyHelper::Default ();
  spectrumPhy.SetChannel (channel);
  // Note that LTE eNB rxGain is implemented through AntennaModel at SpectrumChannel, contrary to Wi-Fi which is handled by SpectrumWifiPhy only
  // In addition, LAA has an AdHocWifiMac station to perform LBT (set in ConfigureEnbDevicesForLbt), that should have BS gain properties (namely Rx).
  spectrumPhy.Set ("TxGain", DoubleValue (phyParams.m_ueTxGain));
  spectrumPhy.Set ("RxGain", DoubleValue (phyParams.m_ueRxGain));
  spectrumPhy.Set ("TxPowerStart", DoubleValue (phyParams.m_ueTxPower));
  spectrumPhy.Set ("TxPowerEnd", DoubleValue (phyParams.m_ueTxPower));
  spectrumPhy.Set ("RxNoiseFigure", DoubleValue (phyParams.m_ueNoiseFigure));
  spectrumPhy.Set ("Antennas", UintegerValue (2));
  spectrumPhy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (2));
  spectrumPhy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (2));
  spectrumPhy.SetPcapDataLinkType (SpectrumWifiPhyHelper::DLT_IEEE802_11_RADIO);

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  WifiMacHelper mac;

  spectrumPhy.Set ("ChannelWidth", UintegerValue (20));
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  for (uint32_t i = 0; i < ueNodes.GetN (); i++)
    {
      //uint32_t channelNumber = 36 + 4 * (i%4);
      uint32_t channelNumber = 36;
      spectrumPhy.Set ("ChannelNumber", UintegerValue (channelNumber));
      staDevices.Add (wifi.Install (spectrumPhy, mac, ueNodes.Get (i)));
    }
  BooleanValue booleanValue;
  bool found;
  found = GlobalValue::GetValueByNameFailSafe ("pcapEnabled", booleanValue);
  if (found && booleanValue.Get () == true)
    {
      spectrumPhy.EnablePcap ("laa-wifi-sta", staDevices);
    }
  found = GlobalValue::GetValueByNameFailSafe ("asciiEnabled", booleanValue);
  if (found && booleanValue.Get () == true)
    {
      AsciiTraceHelper ascii;
      std::string prefix = "laa-wifi-sta";
      spectrumPhy.EnableAscii (prefix, staDevices);
    }
  return staDevices;
}

ApplicationContainer
ConfigureUdpServers (NodeContainer servers, Time startTime, Time stopTime)
{
  ApplicationContainer serverApps;
  uint32_t port = UDP_SERVER_PORT; // discard
  UdpServerHelper myServer (port);
  serverApps = myServer.Install (servers);
  serverApps.Start (startTime);
  serverApps.Stop (stopTime);
  return serverApps;
}

ApplicationContainer
ConfigureUdpClients (NodeContainer client, Ipv4InterfaceContainer servers, Time startTime, Time stopTime, Time interval)
{
  // Randomly distribute the start times
  Ptr<UniformRandomVariable> randomVariable = CreateObject<UniformRandomVariable> ();
  randomVariable->SetAttribute ("Max", DoubleValue (1.0));
  randomVariable->SetStream (streamIndex++);
  uint32_t remotePort = UDP_SERVER_PORT;
  ApplicationContainer clientApps;
  UdpClientHelper clientHelper (Address (), 0);
  clientHelper.SetAttribute ("MaxPackets", UintegerValue (1e6));
  UintegerValue packetSizeValue;
  GlobalValue::GetValueByName ("udpPacketSize", packetSizeValue);
  clientHelper.SetAttribute ("Interval", TimeValue (interval));
  clientHelper.SetAttribute ("PacketSize", packetSizeValue);
  clientHelper.SetAttribute ("RemotePort", UintegerValue (remotePort));

  ApplicationContainer pingApps;
  for (uint32_t i = 0; i < servers.GetN (); i++)
    {
      Ipv4Address ip = servers.GetAddress (i, 0);
      clientHelper.SetAttribute ("RemoteAddress", AddressValue (ip));
      clientApps.Add (clientHelper.Install (client));

      // Seed the ARP cache by pinging early in the simulation
      // This is a workaround until a static ARP capability is provided
      V4PingHelper ping (ip);
      pingApps.Add (ping.Install (client));
    }
  clientApps.StartWithJitter (startTime, randomVariable);
  clientApps.Stop (stopTime);
  // Add one or two pings for ARP at the beginnning of the simulation
  pingApps.Start (Seconds (1) + Seconds (randomVariable->GetValue ()));
  pingApps.Stop (Seconds (3));
  return clientApps;
}

ApplicationContainer
ConfigureFtpServers (NodeContainer servers, Time startTime)
{
  ApplicationContainer serverApps;
  uint16_t port = 50000;
  Address apLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", apLocalAddress);
  serverApps = packetSinkHelper.Install (servers);
  serverApps.Start (startTime);
  return serverApps;
}

ApplicationContainer
ConfigureFtpClients (NodeContainer client, Ipv4InterfaceContainer servers, Time startTime)
{
  // Randomly distribute the start times across 100ms interval
  Ptr<UniformRandomVariable> randomVariable = CreateObject<UniformRandomVariable> ();
  randomVariable->SetAttribute ("Max", DoubleValue (0.100));
  randomVariable->SetStream (streamIndex++);
  ApplicationContainer clientApps;
  uint16_t port = 50000;
  uint32_t ftpSegSize = 1448; // bytes
  uint32_t ftpFileSize = 512000; // bytes
  FileTransferHelper ftp ("ns3::UdpSocketFactory", Address ());
  ftp.SetAttribute ("SendSize", UintegerValue (ftpSegSize));
  ftp.SetAttribute ("FileSize", UintegerValue (ftpFileSize));

  ApplicationContainer pingApps;
  for (uint32_t i = 0; i < servers.GetN (); i++)
    {
      Ipv4Address ip = servers.GetAddress (i, 0);
      AddressValue remoteAddress (InetSocketAddress (ip, port));
      ftp.SetAttribute ("Remote", remoteAddress);
      clientApps.Add (ftp.Install (client));

      // Seed the ARP cache by pinging early in the simulation
      // This is a workaround until a static ARP capability is provided
      V4PingHelper ping (ip);
      pingApps.Add (ping.Install (client));
    }
  double jitter = randomVariable->GetValue ();
  clientApps.Start (startTime + Seconds (jitter));
  // Add one or two pings for ARP at the beginnning of the simulation
  pingApps.Start (Seconds (1) + Seconds (randomVariable->GetValue ()));
  pingApps.Stop (Seconds (3));
  return clientApps;
}

ApplicationContainer
ConfigureTcpServers (NodeContainer servers, Time startTime)
{
  ApplicationContainer serverApps;
  uint16_t port = 50000;
  Address apLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", apLocalAddress);
  serverApps = packetSinkHelper.Install (servers);
  serverApps.Start (startTime);
  return serverApps;
}

ApplicationContainer
ConfigureTcpClients (NodeContainer client, Ipv4InterfaceContainer servers, Time startTime)
{
  // Randomly distribute the start times across 100ms interval
  Ptr<UniformRandomVariable> randomVariable = CreateObject<UniformRandomVariable> ();
  randomVariable->SetAttribute ("Max", DoubleValue (0.100));
  randomVariable->SetStream (streamIndex++);
  ApplicationContainer clientApps;
  uint16_t port = 50000;
  uint32_t ftpSegSize = 1448; // bytes
  uint32_t ftpFileSize = 512000; // bytes
  FileTransferHelper ftp ("ns3::TcpSocketFactory", Address ());
  ftp.SetAttribute ("SendSize", UintegerValue (ftpSegSize));
  ftp.SetAttribute ("FileSize", UintegerValue (ftpFileSize));

  ApplicationContainer pingApps;
  for (uint32_t i = 0; i < servers.GetN (); i++)
    {
      Ipv4Address ip = servers.GetAddress (i, 0);
      AddressValue remoteAddress (InetSocketAddress (ip, port));
      ftp.SetAttribute ("Remote", remoteAddress);
      clientApps.Add (ftp.Install (client));

      // Seed the ARP cache by pinging early in the simulation
      // This is a workaround until a static ARP capability is provided
      V4PingHelper ping (ip);
      pingApps.Add (ping.Install (client));
    }
  double jitter = randomVariable->GetValue ();
  clientApps.Start (startTime + Seconds (jitter));
  // Add one or two pings for ARP at the beginnning of the simulation
  pingApps.Start (Seconds (1) + Seconds (randomVariable->GetValue ()));
  pingApps.Stop (Seconds (3));
  return clientApps;
}

void
StartFileTransfer (Ptr<ExponentialRandomVariable> ftpArrivals, ApplicationContainer clients, uint32_t nextClient, Time stopTime)
{
  NS_ASSERT (nextClient >= 0 && nextClient < clients.GetN ());
  Ptr<Application> app = clients.Get (nextClient);
  NS_ASSERT (app);
  NS_LOG_DEBUG ("Starting file transfer on client " << nextClient << " node number " << app->GetNode ()->GetId () << " at time " << Simulator::Now ().GetSeconds ());
  Ptr<FileTransferApplication> fileTransfer = DynamicCast <FileTransferApplication> (app);
  NS_ASSERT (fileTransfer);
  fileTransfer->SendFile ();

  nextClient += 1;
  if (nextClient == clients.GetN ())
    {
      NS_LOG_DEBUG ("Next transfer will start a new set of file transfers across " << clients.GetN () << " clients");
      nextClient = 0;
    }
  Time nextTime = Seconds (ftpArrivals->GetValue ());
  if (Simulator::Now () + nextTime < stopTime)
    {
      Simulator::Schedule (nextTime, &StartFileTransfer, ftpArrivals, clients, nextClient, stopTime);
    }
}

void
PrintGnuplottableNodeListToFile (std::string filename, NodeContainer nodes, bool printId, std::string label, std::string howToPlot)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  outFile.setf (std::ios_base::fixed);
  for (NodeContainer::Iterator it = nodes.Begin (); it != nodes.End (); ++it)
    {
      Ptr<Node> node = *it;
      Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
      outFile << "set label \"";
      if (printId)
        {
          outFile << label << node->GetId ();
        }
      outFile << "\" at " << pos.x << "," << pos.y
              << " " << howToPlot
              << std::endl;
    }
}

void
ConfigureAndRunScenario (Config_e cellConfigA,
                         Config_e cellConfigB,
                         NodeContainer bsNodesA,
                         NodeContainer bsNodesB,
                         NodeContainer ueNodesA,
                         NodeContainer ueNodesB,
                         struct PhyParams phyParams,
                         Time durationTime,
                         Transport_e transport,
                         std::string propagationLossModel,
                         bool disableApps,
                         double lteDutyCycle,
                         bool generateRem,
                         std::string outFileName,
                         std::string simulationParams)
{
  DoubleValue doubleValue;
  BooleanValue booleanValue;
  StringValue stringValue;
  UintegerValue uValue;

  // Configure times that applications will run, based on 'duration' above
  // First, let the scenario initialize for some seconds
  GlobalValue::GetValueByName ("serverStartTimeSeconds", doubleValue);
  Time serverStartTime = Seconds (doubleValue.Get ());
  GlobalValue::GetValueByName ("clientStartTimeSeconds", doubleValue);
  Time clientStartTime = Seconds (doubleValue.Get ());
  // run servers a bit longer than clients
  GlobalValue::GetValueByName ("serverLingerTimeSeconds", doubleValue);
  Time serverLingerTime = Seconds (doubleValue.Get ());
  // run simulation a bit longer than data transfers
  GlobalValue::GetValueByName ("simulationLingerTimeSeconds", doubleValue);
  Time simulationLingerTime = Seconds (doubleValue.Get ());
  // Since lbt channel access manager may affect LTE connection procedure, it would be good to install it when connection procedure is finished. This
  // is an estimated time value, which will probably depend on the type and size of scenario setup.
  GlobalValue::GetValueByName ("lbtChannelAccessManagerInstallTime", doubleValue);
  Time lbtChannelAccessManagerInstallTime = Seconds (doubleValue.Get ());

  // I order to be sure that we have received HARQ feedback information from all users acks or nacsk, as a temporal solution,
  // we are disabling error model for ctrl signals in order to ensure that UE is aware of transmissions from eNB
  // so it will always send to eNB harq feedback. This enables us to more precisely update CW in LbtAccessManager (update is based on harq feedback).
  Config::SetDefault ("ns3::LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (true));

  Config::SetDefault ("ns3::LteEnbPhy::ChannelAccessManagerStartTime", TimeValue (lbtChannelAccessManagerInstallTime));
  // defines a time until which mibs and sibs will be generated and transmitted, after that time no mibs/sibs ctrl messages will be generated
  GlobalValue::GetValueByName ("disableMibAndSibStartupTime", doubleValue);
  Time disableMibAndSibStartupTime = Seconds (doubleValue.Get ());
  Config::SetDefault ("ns3::LteEnbPhy::DisableMibAndSibStartupTime", TimeValue (disableMibAndSibStartupTime));
  GlobalValue::GetValueByName ("mibPeriod", uValue);
  Config::SetDefault ("ns3::LteEnbPhy::MibPeriod", UintegerValue (uValue.Get ()));
  GlobalValue::GetValueByName ("sibPeriod", uValue);
  Config::SetDefault ("ns3::LteEnbPhy::SibPeriod", UintegerValue (uValue.Get ()));
  GlobalValue::GetValueByName ("drsPeriod", uValue);
  Config::SetDefault ("ns3::LteEnbPhy::DrsPeriod", UintegerValue (uValue.Get ()));
  GlobalValue::GetValueByName ("drsEnabled", booleanValue);
  Config::SetDefault ("ns3::LteEnbPhy::DrsMessagesEnabled", booleanValue);
  GlobalValue::GetValueByName ("lteChannelAccessImpl", booleanValue);
  Config::SetDefault ("ns3::LteEnbPhy::ImplWith2msDelay", booleanValue);
  // in situation when packets are ready, but no channel access, whether to drop packets, used only with impl2 (lteChannelAccessImpl = false)
  GlobalValue::GetValueByName ("dropPackets", booleanValue);
  Config::SetDefault ("ns3::LteEnbPhy::DropPackets", booleanValue);


  GlobalValue::GetValueByName("rlcAmRbsTimer", uValue);
  Config::SetDefault ("ns3::LteRlcAm::ReportBufferStatusTimer", TimeValue (MilliSeconds(uValue.Get ())));

  // We need to do more tests with different value for LteRlcAm buffer status timer.
  // We have noticed that in general can imrove performance.
  //Config::SetDefault ("ns3::LteRlcAm::ReportBufferStatusTimer", TimeValue (MilliSeconds(1)));

  // VoiceApplication defaults
  Config::SetDefault ("ns3::VoiceApplication::Protocol", TypeIdValue (UdpSocketFactory::GetTypeId ()));
  Config::SetDefault ("ns3::VoiceApplication::Interval", TimeValue (MilliSeconds (10)));

  // For FTP model 1, we want to dump a lot of packets into the device,
  // but the CSMA or point-to-point drop-tail queue default size of 100 packets
  // will be too constraining, so relax this.
  Config::SetDefault ("ns3::QueueBase::MaxSize", QueueSizeValue (QueueSize ("10000p")));

  bool laaNodeEnabled = false;
  // this variable is not used in all scenarios thus might not be defined
  if (GlobalValue::GetValueByNameFailSafe ("laaNodeEnabled",booleanValue))
    {
      laaNodeEnabled = booleanValue.Get ();
    }

  // Now set start and stop times derived from the above
  Time serverStopTime = serverStartTime + durationTime + serverLingerTime;
  Time clientStopTime = clientStartTime + durationTime;
  // Overall simulation stop time is below
  Time stopTime = serverStopTime + simulationLingerTime;

  // The max data rate of UDP applications is controlled by the
  // below udpInterval variable, which is derived from the GlobalValues
  // 'udpRate' and 'udpPacketSize'.
  //
  // If the data rate equals or exceeds UDP_SATURATION_RATE, and there
  // are multiple UEs per cell, we reduce the unnecessary computational
  // load of generating more packets than necessary by spreading the load
  // across the average number of UEs per cell.  The resulting aggregate
  // rate is expected to saturate the channel.  Note that even though
  // the number of actual UEs in a cell might be lower than the average,
  // adaptive modulation and coding/rate adaptation will typically cause
  // many UEs to have a lower rate than the maximum.
  // This optimization shouldn't have much of an effect if any.
  //
  DataRateValue dataRateValue;
  GlobalValue::GetValueByName ("udpRate", dataRateValue);
  UintegerValue uintegerValue;
  GlobalValue::GetValueByName ("udpPacketSize", uintegerValue);
  uint64_t bitRate = dataRateValue.Get().GetBitRate ();
  uint32_t packetSize = uintegerValue.Get (); // bytes
  double interval = static_cast<double> (packetSize * 8) / bitRate;
  Time udpInterval;
  // if bitRate < UDP_SATURATION_RATE, use the calculated interval
  // if bitRate >= UDP_SATURATION_RATE, and the spreadUdpLoad optimization is
  // enabled,  spread the offered load across BS/UE in the cell
  GlobalValue::GetValueByName ("spreadUdpLoad", booleanValue);
  if (bitRate < UDP_SATURATION_RATE || booleanValue.Get () == false)
    {
      udpInterval = Seconds (interval);
    }
  else
    {
      NS_LOG_DEBUG ("Applying spreadUdpLoad optimization for factor " << ueNodesA.GetN () / static_cast<double> (bsNodesA.GetN ()));
      udpInterval = Seconds ((interval * ueNodesA.GetN ()) / bsNodesA.GetN ());
    }
  NS_LOG_DEBUG ("UDP will use application interval " << udpInterval.GetSeconds () << " sec");

  std::cout << "Running simulation for " << durationTime.GetSeconds () << " sec of data transfer; "
            << stopTime.GetSeconds () << " sec overall" << std::endl;

  std::cout << "Operator A: " << CellConfigToString (cellConfigA)
            << "; number of cells " << bsNodesA.GetN () << "; number of UEs "
            << ueNodesA.GetN () << std::endl;
  std::cout << "Operator B: " << CellConfigToString (cellConfigB)
            << "; number of cells " << bsNodesB.GetN () << "; number of UEs "
            << ueNodesB.GetN () << std::endl;

  // All application data will be sourced from the client node so that
  // it shows up on the downlink
  NodeContainer clientNodesA;  // for the backhaul application client
  clientNodesA.Create (1); // create one remote host for sourcing traffic
  NodeContainer clientNodesB;  // for the backhaul application client
  clientNodesB.Create (1); // create one remote host for sourcing traffic

  // For Wi-Fi, the client node needs to be connected to the bsNodes via a single
  // CSMA link
  //
  //
  //                 client
  //                   |
  //               =========
  //              /   |    |
  //           bs0   bs1... bsN
  //
  //  First we create a Csma helper that will instantiate the
  //  client <-> bsN link for each network

  CsmaHelper csmaHelper;

  //
  // Wireless setup phase
  //

  // Device containers hold the WiFi or LTE NetDevice objects created
  NetDeviceContainer bsDevicesA;
  NetDeviceContainer ueDevicesA;
  NetDeviceContainer bsDevicesB;
  NetDeviceContainer ueDevicesB;
  NetDeviceContainer laaDevice;

  // Start to create the wireless devices by first creating the shared channel
  // Note:  the design of LTE requires that we use an LteHelper to
  // create the channel, if we are potentially using LTE in one of the networks
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  lteHelper->SetAttribute ("PathlossModel", StringValue (propagationLossModel));
  // since LAA is using CA, RRC messages will be excanged in the
  // licensed bands, hence we model it using the ideal RRC
  lteHelper->SetAttribute ("UseIdealRrc", BooleanValue (true));
  lteHelper->SetAttribute ("UsePdschForCqiGeneration", BooleanValue (true));

  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->Initialize ();
  std::vector<LteSpectrumValueCatcher> lteDlSinrCatcherVectorA;
  std::vector<LteSpectrumValueCatcher> lteDlSinrCatcherVectorB;

  // set channel MaxLossDb to discard all transmissions below -15dB SNR.
  // The calculations assume -174 dBm/Hz noise PSD and 20 MHz bandwidth (73 dB)
  Ptr<SpectrumChannel> dlSpectrumChannel = lteHelper->GetDownlinkSpectrumChannel ();
  //           loss  = txpower -noisepower            -snr    ;
  double dlMaxLossDb = (phyParams.m_bsTxPower + phyParams.m_bsTxGain) -
    (-174.0 + 73.0 + phyParams.m_ueNoiseFigure) -
    (-15.0);
  dlSpectrumChannel->SetAttribute ("MaxLossDb", DoubleValue (dlMaxLossDb));
  Ptr<SpectrumChannel> ulSpectrumChannel = lteHelper->GetUplinkSpectrumChannel ();
  //           loss  = txpower -noisepower            -snr    ;
  double ulMaxLossDb = (phyParams.m_ueTxPower + phyParams.m_ueTxGain)  -
    (-174.0 + 73.0 + phyParams.m_bsNoiseFigure) -
    (-15.0);
  ulSpectrumChannel->SetAttribute ("MaxLossDb", DoubleValue (ulMaxLossDb));

  // determine the LTE Almost Blank Subframe (ABS) pattern that will implement the desired duty cycle
  NS_ASSERT_MSG (lteDutyCycle >= 0 && lteDutyCycle <= 1, "lteDutyCycle must be between 1 and 0");
  std::bitset<40> absPattern;
  // need at least two regular subframes for MIB and SIB1
  absPattern[0] = 0;
  absPattern[35] = 0;
  uint32_t regularSubframes = 2;
  int32_t subframe = 39;
  while ((regularSubframes < 40) && (regularSubframes / 40.0 < lteDutyCycle))
    {
      if (subframe != 0 && subframe != 35)
        {
          absPattern[subframe] = 0;
          ++regularSubframes;
        }
      --subframe;
    }
  while (subframe >= 0)
    {
      if (subframe != 0 && subframe != 35)
        {
          absPattern[subframe] = 1;
        }
      --subframe;
    }
  double actualLteDutyCycle = regularSubframes / 40.0;
  if (cellConfigA == LTE || cellConfigB == LTE)
    {
      std::cout << "LTE duty cycle: requested " << lteDutyCycle << ", actual " << actualLteDutyCycle << ", ABS pattern " << absPattern << std::endl;
    }



  // LTE requires some Internet stack configuration prior to device installation
  // Wifi configures it after device installation
  InternetStackHelper internetStackHelper;
  // Install an internet stack to all the nodes with IP
  internetStackHelper.Install (clientNodesA);
  internetStackHelper.Install (clientNodesB);
  internetStackHelper.Install (ueNodesA);
  internetStackHelper.Install (ueNodesB);


  // Laa node configuration and installation.

  if (laaNodeEnabled)
    {
      Vector3DValue vectorValue;
      GlobalValue::GetValueByName ("laaNodePosition", vectorValue);
      Vector3D laaPosition = vectorValue.Get ();

      NodeContainer laaNode;
      laaNode.Create (1);

      laaNode.Get (0)->AggregateObject (CreateObject<ConstantPositionMobilityModel> ());
      laaNode.Get (0)->GetObject<MobilityModel> ()->SetPosition (laaPosition);
      //  MobilityHelper mobility;
      //  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      //  mobility.SetPositionAllocator (positionAlloc);
      //  mobility.Install ();

      lteHelper->SetEnbDeviceAttribute ("CsgIndication", BooleanValue (true));
      lteHelper->SetEnbDeviceAttribute ("CsgId", UintegerValue (3));
      lteHelper->SetUeDeviceAttribute ("CsgId", UintegerValue (3));
      Ipv4AddressHelper internetIpv4Helper;
      internetIpv4Helper.SetBase ("3.0.0.0", "255.0.0.0");
      NodeContainer laaUeNodes;
      NodeContainer laaClientNodes;
      NetDeviceContainer laaUeDevices;
      ConfigureLaa (lteHelper, epcHelper, internetIpv4Helper, laaNode, laaUeNodes, laaClientNodes, laaDevice, laaUeDevices, phyParams, lteDlSinrCatcherVectorA, absPattern, transport, lbtChannelAccessManagerInstallTime);
    }

  //
  // Configure the right technology for each operator A and B
  //

  //
  // Configure operator A
  //

  Ipv4Address ipBackhaulA;
  if (cellConfigA == WIFI)
    {
      internetStackHelper.Install (bsNodesA);
      Ptr<SpectrumChannel> spectrumChannel = lteHelper->GetDownlinkSpectrumChannel ();
      bsDevicesA.Add (ConfigureWifiAp (bsNodesA, phyParams, spectrumChannel, Ssid ("ns380211n-A")));
      ueDevicesA.Add (ConfigureWifiSta (ueNodesA, phyParams, spectrumChannel, Ssid ("ns380211n-A")));
      Ipv4AddressHelper ipv4h;
      ipv4h.SetBase ("11.0.0.0", "255.255.0.0");
      // Add backhaul CSMA link from client to each BS
      NodeContainer csmaNodes;
      csmaNodes.Add (clientNodesA.Get (0));
      csmaNodes.Add (bsNodesA);
      NetDeviceContainer csmaNewDevices = csmaHelper.Install (csmaNodes);
      // Add IP addresses to backhaul links
      ipv4h.Assign (csmaNewDevices);
      //ipv4h.NewNetwork ();
      // The IP address for the backhaul traffic source will be 11.0.0.1
      ipBackhaulA = Ipv4Address ("11.0.0.1");
    }
  else if (cellConfigA == LTE)
    {
      //NS_ASSERT (cellConfigA == LTE);
      lteHelper->SetEnbDeviceAttribute ("CsgIndication", BooleanValue (true));
      lteHelper->SetEnbDeviceAttribute ("CsgId", UintegerValue (1));
      lteHelper->SetUeDeviceAttribute ("CsgId", UintegerValue (1));
      Ipv4AddressHelper internetIpv4Helper;
      internetIpv4Helper.SetBase ("1.0.0.0", "255.0.0.0");
      ConfigureLte (lteHelper, epcHelper, internetIpv4Helper, bsNodesA, ueNodesA, clientNodesA, bsDevicesA, ueDevicesA, phyParams, lteDlSinrCatcherVectorA, absPattern, transport);
      // source address for IP-based packets will

      //Ptr<SpectrumChannel> spectrumChannel = lteHelper->GetDownlinkSpectrumChannel ();
      //ConfigureMonitor(bsNodesA, phyParams, spectrumChannel);
      // The IP address for the backhaul traffic source will be 1.0.0.2
      ipBackhaulA = Ipv4Address ("1.0.0.2");
    }
  else
    {
      NS_ASSERT (cellConfigA == LAA);
      lteHelper->SetEnbDeviceAttribute ("CsgIndication", BooleanValue (true));
      lteHelper->SetEnbDeviceAttribute ("CsgId", UintegerValue (1));
      lteHelper->SetUeDeviceAttribute ("CsgId", UintegerValue (1));
      Ipv4AddressHelper internetIpv4Helper;
      internetIpv4Helper.SetBase ("1.0.0.0", "255.0.0.0");
      ConfigureLaa (lteHelper, epcHelper, internetIpv4Helper, bsNodesA, ueNodesA, clientNodesA, bsDevicesA, ueDevicesA, phyParams, lteDlSinrCatcherVectorA, absPattern, transport, lbtChannelAccessManagerInstallTime);

      GlobalValue::GetValueByName ("logCwChanges", booleanValue);
      if (booleanValue.Get () == true)
        {
          NS_ASSERT_MSG (bsDevicesA.GetN () > 0, "CW statistics cannot be configured, no device.");
          for (uint32_t i = 0; i < bsDevicesA.GetN (); i++)
            {
              Ptr<NetDevice> netDevice = bsDevicesA.Get (i);
              Simulator::Schedule (clientStartTime, &CwChangeConnect, netDevice);
              Simulator::Schedule (clientStopTime, &CwChangeDisconnect, netDevice);
            }
        }
      GlobalValue::GetValueByName ("logBackoffChanges", booleanValue);
      if (booleanValue.Get () == true)
        {
          NS_ASSERT_MSG (bsDevicesA.GetN () > 0, "Backoff statistics cannot be configured, no device.");
          for (uint32_t i = 0; i < bsDevicesA.GetN (); i++)
            {
              Ptr<NetDevice> netDevice = bsDevicesA.Get (i);
              Simulator::Schedule (clientStartTime, &BackoffChangeConnect, netDevice);
              Simulator::Schedule (clientStopTime, &BackoffChangeDisconnect, netDevice);
            }
        }

      GlobalValue::GetValueByName ("logHarqFeedback", booleanValue);
      if (booleanValue.Get () == true)
        {
          NS_ASSERT_MSG (bsDevicesA.GetN () > 0, "Harq feedback statistics cannot be configured, no device.");
          for (uint32_t i = 0; i < bsDevicesA.GetN (); i++)
            {
              Ptr<NetDevice> netDevice = bsDevicesA.Get (i);
              Simulator::Schedule (clientStartTime, &HarqFeedbackConnect, netDevice);
              //Simulator::Schedule (clientStopTime, &HarqFeedbackDisconnect, netDevice);
            }
       }

      GlobalValue::GetValueByName ("logReservationSignals", booleanValue);
      if (booleanValue.Get () == true)
        {
          NS_ASSERT_MSG (bsDevicesA.GetN () > 0, "Reservation signals statistics cannot be configured, no device.");
          for (uint32_t i = 0; i < bsDevicesA.GetN (); i++)
            {
              Ptr<NetDevice> netDevice = bsDevicesA.Get (i);
              Simulator::Schedule (clientStartTime, &ReservationSignalTraceConnect, netDevice);
              Simulator::Schedule (clientStopTime, &ReservationSignalTraceDisconnect, netDevice);
            }
        }

    }

  //
  // Configure operator B
  //

  Ipv4Address ipBackhaulB;
  if (cellConfigB == WIFI)
    {
      internetStackHelper.Install (bsNodesB);
      Ptr<SpectrumChannel> spectrumChannel = lteHelper->GetDownlinkSpectrumChannel ();
      bsDevicesB.Add (ConfigureWifiAp (bsNodesB, phyParams, spectrumChannel, Ssid ("ns380211n-B")));
      ueDevicesB.Add (ConfigureWifiSta (ueNodesB, phyParams, spectrumChannel, Ssid ("ns380211n-B")));

      Ipv4AddressHelper ipv4h;
      ipv4h.SetBase ("12.0.0.0", "255.255.0.0");
      // Add backhaul CSMA link from client to each BS
      NodeContainer csmaNodes;
      csmaNodes.Add (clientNodesB.Get (0));
      csmaNodes.Add (bsNodesB);
      NetDeviceContainer csmaNewDevices = csmaHelper.Install (csmaNodes);
      // Add IP addresses to backhaul links
      ipv4h.Assign (csmaNewDevices);
      //ipv4h.NewNetwork ();
      // The IP address for the backhaul traffic source will be 12.0.0.1
      ipBackhaulB = Ipv4Address ("12.0.0.1");
    }
  else if (cellConfigB == LTE)
    {
      //NS_ASSERT (cellConfigB == LTE);
      lteHelper->SetEnbDeviceAttribute ("CsgIndication", BooleanValue (true));
      lteHelper->SetEnbDeviceAttribute ("CsgId", UintegerValue (2));
      lteHelper->SetUeDeviceAttribute ("CsgId", UintegerValue (2));
      Ipv4AddressHelper internetIpv4Helper;
      internetIpv4Helper.SetBase ("2.0.0.0", "255.0.0.0");
      ConfigureLte (lteHelper, epcHelper,  internetIpv4Helper, bsNodesB, ueNodesB, clientNodesB, bsDevicesB, ueDevicesB, phyParams, lteDlSinrCatcherVectorB, absPattern, transport);
      // The IP address for the backhaul traffic source will be 2.0.0.2
      ipBackhaulB = Ipv4Address ("2.0.0.2");
    }
  else
    {
      NS_ASSERT (cellConfigB == LAA);
      lteHelper->SetEnbDeviceAttribute ("CsgIndication", BooleanValue (true));
      lteHelper->SetEnbDeviceAttribute ("CsgId", UintegerValue (2));
      lteHelper->SetUeDeviceAttribute ("CsgId", UintegerValue (2));
      Ipv4AddressHelper internetIpv4Helper;
      internetIpv4Helper.SetBase ("2.0.0.0", "255.0.0.0");
      ConfigureLaa (lteHelper, epcHelper,  internetIpv4Helper, bsNodesB, ueNodesB, clientNodesB, bsDevicesB, ueDevicesB, phyParams, lteDlSinrCatcherVectorB, absPattern, transport, lbtChannelAccessManagerInstallTime);
      // The IP address for the backhaul traffic source will be 2.0.0.2
      ipBackhaulB = Ipv4Address ("2.0.0.2");

      GlobalValue::GetValueByName ("logCwChanges", booleanValue);
      if (booleanValue.Get () == true)
        {
          NS_ASSERT_MSG (bsDevicesB.GetN () > 0, "CW statistics cannot be configured, no device.");
          for (uint32_t i = 0; i < bsDevicesB.GetN (); i++)
            {
              Ptr<NetDevice> netDevice = bsDevicesB.Get (i);
              Simulator::Schedule (clientStartTime, &CwChangeConnect, netDevice);
              Simulator::Schedule (clientStopTime, &CwChangeDisconnect, netDevice);
            }
        }
      GlobalValue::GetValueByName ("logBackoffChanges", booleanValue);
      if (booleanValue.Get () == true)
        {
          NS_ASSERT_MSG (bsDevicesB.GetN () > 0, "Backoff statistics cannot be configured, no device.");
          for (uint32_t i = 0; i < bsDevicesB.GetN (); i++)
            {
              Ptr<NetDevice> netDevice = bsDevicesB.Get (i);
              Simulator::Schedule (clientStartTime, &BackoffChangeConnect, netDevice);
              Simulator::Schedule (clientStopTime, &BackoffChangeDisconnect, netDevice);
            }
        }
      GlobalValue::GetValueByName ("logHarqFeedback", booleanValue);
      if (booleanValue.Get () == true)
        {
          NS_ASSERT_MSG (bsDevicesB.GetN () > 0, "Harq feedback statistics cannot be configured, no device.");
          for (uint32_t i = 0; i < bsDevicesB.GetN (); i++)
            {
              Ptr<NetDevice> netDevice = bsDevicesB.Get (i);
              Simulator::Schedule (clientStartTime, &HarqFeedbackConnect, netDevice);
              Simulator::Schedule (clientStopTime, &HarqFeedbackDisconnect, netDevice);
            }
        }
      GlobalValue::GetValueByName ("logReservationSignals", booleanValue);
      if (booleanValue.Get () == true)
        {
          NS_ASSERT_MSG (bsDevicesB.GetN () > 0, "Reservation signals statistics cannot be configured, no device.");
          for (uint32_t i = 0; i < bsDevicesB.GetN (); i++)
            {
              Ptr<NetDevice> netDevice = bsDevicesB.Get (i);
              Simulator::Schedule (clientStartTime, &ReservationSignalTraceConnect, netDevice);
              Simulator::Schedule (clientStopTime, &ReservationSignalTraceDisconnect, netDevice);
            }
        }


    }


  //
  // IP addressing setup phase
  //

  Ipv4InterfaceContainer ipBsA;
  Ipv4InterfaceContainer ipUeA;
  Ipv4InterfaceContainer ipBsB;
  Ipv4InterfaceContainer ipUeB;

  Ipv4AddressHelper ueAddress;
  ueAddress.SetBase ("17.0.0.0", "255.255.0.0");
  ipBsA = ueAddress.Assign (bsDevicesA);
  ipUeA = ueAddress.Assign (ueDevicesA);

  ueAddress.SetBase ("18.0.0.0", "255.255.0.0");
  ipBsB = ueAddress.Assign (bsDevicesB);
  ipUeB = ueAddress.Assign (ueDevicesB);

  // Routing
  // WiFi nodes will trigger an association callback, which can invoke
  // a method to configure the appropriate routes on client and STA
  Config::Connect ("/NodeList/*/DeviceList/*/Mac/Assoc", MakeCallback (&ConfigureRouteForStation));
  // Deassociation logging
  Config::Connect ("/NodeList/*/DeviceList/*/Mac/DeAssoc", MakeCallback (&DeassociationLogging));

  //
  // Application setup phase
  //

  Ptr<ExponentialRandomVariable> ftpArrivalsA = CreateObject<ExponentialRandomVariable> ();
  ftpArrivalsA->SetStream (streamIndex++);
  Ptr<ExponentialRandomVariable> ftpArrivalsB = CreateObject<ExponentialRandomVariable> ();
  ftpArrivalsB->SetStream (streamIndex++);
  double ftpLambda;
  bool found = GlobalValue::GetValueByNameFailSafe ("ftpLambda", doubleValue);
  if (found)
    {
      ftpLambda = doubleValue.Get ();
      ftpArrivalsA->SetAttribute ("Mean", DoubleValue (1 / ftpLambda));
      ftpArrivalsB->SetAttribute ("Mean", DoubleValue (1 / ftpLambda));
    }
  uint32_t nextClient = 0;

  if (disableApps == false)
    {
      // If voice is enabled, we will try to enable voice on the first one
      // or two UEs. If we are in a large 3GPP scenario, we need to suppress
      // the addition also of FTP traffic on those UEs.  So, if voice is
      // enabled and we have 4 or more UEs in network B, we will create
      // modified containers for operatorB network that exclude the first two
      // UEs
      NodeContainer nonVoiceUeNodesB;
      Ipv4InterfaceContainer nonVoiceIpUeB;
      found = GlobalValue::GetValueByNameFailSafe ("voiceEnabled", booleanValue);
      if (found && (booleanValue.Get () == true) && (ueNodesB.GetN () >= 4))
        {
          // Skip the 0th and 1st indices; they will be used for voice
          for (uint32_t index = 2; index < ueNodesB.GetN (); index++)
            {
              nonVoiceUeNodesB.Add (ueNodesB.Get (index));
              nonVoiceIpUeB.Add (ipUeB.Get (index));
            }
        }
      else
        {
          // NodeContainer has no copy constructor or assignment operator
          // so we do it manually here
          for (uint32_t index = 0; index < ueNodesB.GetN (); index++)
            {
              nonVoiceUeNodesB.Add (ueNodesB.Get (index));
              nonVoiceIpUeB.Add (ipUeB.Get (index));
            }
        }

      if (transport == UDP)
        {
          ApplicationContainer serverApps, clientApps;
          serverApps.Add (ConfigureUdpServers (ueNodesA, serverStartTime, serverStopTime));
          clientApps.Add (ConfigureUdpClients (clientNodesA, ipUeA, clientStartTime, clientStopTime, udpInterval));
          serverApps.Add (ConfigureUdpServers (nonVoiceUeNodesB, serverStartTime, serverStopTime));
          clientApps.Add (ConfigureUdpClients (clientNodesB, nonVoiceIpUeB, clientStartTime, clientStopTime, udpInterval));
        }
      else if (transport == FTP)
        {
          ApplicationContainer ftpServerApps, ftpClientApps;
          ftpServerApps.Add (ConfigureFtpServers (ueNodesA, serverStartTime));
          ftpClientApps.Add (ConfigureFtpClients (clientNodesA, ipUeA, clientStartTime));
          // Start file transfer arrival process in both networks
          double firstArrivalA = ftpArrivalsA->GetValue ();
          NS_LOG_DEBUG ("First FTP arrival for operator A at time " << clientStartTime.GetSeconds () + firstArrivalA);
          Simulator::Schedule (clientStartTime + Seconds (firstArrivalA), &StartFileTransfer, ftpArrivalsA, ftpClientApps, nextClient, clientStopTime);
          ApplicationContainer ftpServerAppsB, ftpClientAppsB;
          ftpServerAppsB.Add (ConfigureFtpServers (nonVoiceUeNodesB, serverStartTime));
          ftpClientAppsB.Add (ConfigureFtpClients (clientNodesB, nonVoiceIpUeB, clientStartTime));
          double firstArrivalB = ftpArrivalsB->GetValue ();
          NS_LOG_DEBUG ("First FTP arrival for operator B at time " << clientStartTime.GetSeconds () + firstArrivalB);
          Simulator::Schedule (clientStartTime + Seconds (firstArrivalB), &StartFileTransfer, ftpArrivalsB, ftpClientAppsB, nextClient, clientStopTime);
        }
      else
        {
          ApplicationContainer tcpServerApps, tcpClientApps;
          tcpServerApps.Add (ConfigureTcpServers (ueNodesA, serverStartTime));
          tcpClientApps.Add (ConfigureTcpClients (clientNodesA, ipUeA, clientStartTime));
          // Start file transfer arrival process
          double firstArrivalA = ftpArrivalsA->GetValue ();
          NS_LOG_DEBUG ("First FTP arrival for operator A at time " << clientStartTime.GetSeconds () + firstArrivalA);
          Simulator::Schedule (clientStartTime + Seconds (firstArrivalA), &StartFileTransfer, ftpArrivalsA, tcpClientApps, nextClient, clientStopTime);
          ApplicationContainer tcpServerAppsB, tcpClientAppsB;
          tcpServerAppsB.Add (ConfigureTcpServers (nonVoiceUeNodesB, serverStartTime));
          tcpClientAppsB.Add (ConfigureTcpClients (clientNodesB, nonVoiceIpUeB, clientStartTime));
          double firstArrivalB = ftpArrivalsB->GetValue ();
          NS_LOG_DEBUG ("First FTP arrival for operator B at time " << clientStartTime.GetSeconds () + firstArrivalB);
          Simulator::Schedule (clientStartTime + Seconds (firstArrivalB), &StartFileTransfer, ftpArrivalsB, tcpClientAppsB, nextClient, clientStopTime);
        }
      found = GlobalValue::GetValueByNameFailSafe ("voiceEnabled", booleanValue);
      if (found && (booleanValue.Get () == true))
        {
          // Add voice flows to up to two UEs in the operatorB network only
          Ptr<Node> voiceSender;
          Ptr<Node> voiceReceiver0;
          Ptr<Node> voiceReceiver1;
          Ptr<VoiceApplication> voiceAppSender0;
          Ptr<VoiceApplication> voiceAppReceiver0;
          Ptr<VoiceApplication> voiceAppSender1;
          Ptr<VoiceApplication> voiceAppReceiver1;
          Ipv4Address remoteIp0;
          Ipv4Address remoteIp1;
          Ipv4Address remoteIp2;
          std::string context;
          bool success;

          // Network B
          voiceSender = clientNodesB.Get (0);
          voiceReceiver0 = ueNodesB.Get (0);
          if (ueNodesB.GetN () > 1)
            {
              voiceReceiver1 = ueNodesB.Get (1);
            }

          voiceAppSender0 = CreateObject<VoiceApplication> ();
          voiceAppSender0->SetStartTime (clientStartTime);
          voiceAppSender0->SetStopTime (clientStopTime);
          voiceSender->AddApplication (voiceAppSender0);
          voiceAppReceiver0 = CreateObject<VoiceApplication> ();
          voiceAppReceiver0->SetStartTime (serverStartTime);
          voiceAppReceiver0->SetStopTime (serverStopTime);
          // send in downlink direction only
          voiceAppReceiver0->SetAttribute ("SendEnabled", BooleanValue (false));
          voiceReceiver0->AddApplication (voiceAppReceiver0);
          context = NodeIdToContext (voiceReceiver0->GetId ());
          success = voiceAppReceiver0->TraceConnect("Rx", context, MakeCallback(&VoiceRxCb));
          NS_ASSERT (success);

          if (ueNodesB.GetN () > 1)
            {
              voiceAppSender1 = CreateObject<VoiceApplication> ();
              voiceAppSender1->SetStartTime (clientStartTime);
              voiceAppSender1->SetStopTime (clientStopTime);
              voiceSender->AddApplication (voiceAppSender1);
              voiceAppReceiver1 = CreateObject<VoiceApplication> ();
              voiceAppReceiver1->SetStartTime (serverStartTime);
              voiceAppReceiver1->SetStopTime (serverStopTime);
              // send in downlink direction only
              voiceAppReceiver1->SetAttribute ("SendEnabled", BooleanValue (false));
              voiceReceiver1->AddApplication (voiceAppReceiver1);
              context = NodeIdToContext (voiceReceiver1->GetId ());
              success = voiceAppReceiver1->TraceConnect("Rx", context, MakeCallback(&VoiceRxCb));
              NS_ASSERT (success);
            }
          else
            {
              voiceAppSender1 = 0;
              voiceAppReceiver1 = 0;
            }

          // Configure the local address for each receiver (i.e. bind() to
          // the voice port) via the 'Local' attribute
          voiceAppReceiver0->SetAttribute ("Local", AddressValue (InetSocketAddress (Ipv4Address::GetAny (), VOICE_PORT)));
          if (voiceAppReceiver1)
            {
              voiceAppReceiver1->SetAttribute ("Local", AddressValue (InetSocketAddress (Ipv4Address::GetAny (), VOICE_PORT)));
            }

          // Configure the peer address for sender via the 'Remote' attribute
          remoteIp0 = ipUeB.GetAddress (0, 0);
          NS_LOG_DEBUG ("IP 0 " << remoteIp0);
          voiceAppSender0->SetAttribute ("Remote", AddressValue (InetSocketAddress (remoteIp0, VOICE_PORT)));
          if (voiceAppSender1)
            {
              remoteIp1 = ipUeB.GetAddress (1, 0);
              NS_LOG_DEBUG ("IP 1 " << remoteIp1);
              voiceAppSender1->SetAttribute ("Remote", AddressValue (InetSocketAddress (remoteIp1, VOICE_PORT)));
            }
        }
    }

  // All of the client apps are on node clientNodes
  // Server apps are distributed among the ueNodesA and ueNodesB.
  // Data primarily flows from client to server (i.e. on the downlink)

  //
  // Statistics setup phase
  //

  // Add flow monitors
  // *** NOTE ****
  // we'd like two FlowMonitors so that we can split operator A and B
  // stats. However if we just use two FlowMonitor instances but a
  // single FlowMonitorHelper it won't work - have a look at
  // FlowMonitorHelper::GetMonitor (). If you look at the docs, they
  // say that you should only use one FlowMonitorHelper instance
  // https://www.nsnam.org/docs/models/html/flow-monitor.html#helpers
  // but well I tried with two helpers and it seems to work :-)
  // this email might also be relevant
  // https://groups.google.com/d/msg/ns-3-users/rl77tJCcw8c/egMn__QyVNQJ

  FlowMonitorHelper flowmonHelperA;
  NodeContainer endpointNodesA;
  endpointNodesA.Add (clientNodesA);
  endpointNodesA.Add (ueNodesA);
  FlowMonitorHelper flowmonHelperB;
  NodeContainer endpointNodesB;
  endpointNodesB.Add (clientNodesB);
  endpointNodesB.Add (ueNodesB);


  Ptr<FlowMonitor> monitorA = flowmonHelperA.Install (endpointNodesA);
  monitorA->SetAttribute ("DelayBinWidth", DoubleValue (0.001));
  monitorA->SetAttribute ("JitterBinWidth", DoubleValue (0.001));
  monitorA->SetAttribute ("PacketSizeBinWidth", DoubleValue (20));

  Ptr<FlowMonitor> monitorB = flowmonHelperB.Install (endpointNodesB);
  monitorB->SetAttribute ("DelayBinWidth", DoubleValue (0.001));
  monitorB->SetAttribute ("JitterBinWidth", DoubleValue (0.001));
  monitorB->SetAttribute ("PacketSizeBinWidth", DoubleValue (20));


  // these slow down simulations, only enable them if you need them
  //lteHelper->EnableTraces();

  Ptr<RadioEnvironmentMapHelper> remHelper;
  if (generateRem)
    {
      StringValue stringValue;
      GlobalValue::GetValueByName ("remDir", stringValue);
      std::string remDir = stringValue.Get ();

      PrintGnuplottableNodeListToFile (remDir + "/bs_A_labels.gnuplot", bsNodesA, true, "A_BS_", " center textcolor rgb \"cyan\" front point pt 5 lc rgb \"cyan\" offset 0,0.5");
      PrintGnuplottableNodeListToFile (remDir + "/ue_A_labels.gnuplot", ueNodesA, true, "A_UE_", " center textcolor rgb \"cyan\" front point pt 4 lc rgb \"cyan\" offset 0,0.5");
      PrintGnuplottableNodeListToFile (remDir + "/bs_B_labels.gnuplot", bsNodesB, true, "B_BS_", " center textcolor rgb \"chartreuse\" front point pt 7 lc rgb \"chartreuse\" offset 0,0.5");
      PrintGnuplottableNodeListToFile (remDir + "/ue_B_labels.gnuplot", ueNodesB, true, "B_UE_", " center textcolor rgb \"chartreuse\" front point pt 6 lc rgb \"chartreuse\" offset 0,0.5");

      PrintGnuplottableNodeListToFile (remDir + "/bs_A.gnuplot", bsNodesA, false, "", " point pt 5 lc rgb \"cyan\" front ");
      PrintGnuplottableNodeListToFile (remDir + "/ue_A.gnuplot", ueNodesA, false, "", " point pt 4 lc rgb \"cyan\" front ");
      PrintGnuplottableNodeListToFile (remDir + "/bs_B.gnuplot", bsNodesB, false, "", " point pt 7 lc rgb \"chartreuse\" front ");
      PrintGnuplottableNodeListToFile (remDir + "/ue_B.gnuplot", ueNodesB, false, "", " point pt 6 lc rgb \"chartreuse\" front ");

      remHelper = CreateObject<RadioEnvironmentMapHelper> ();
      remHelper->SetAttribute ("ChannelPath", StringValue ("/ChannelList/0"));
      remHelper->SetAttribute ("Earfcn", UintegerValue (255444));

      remHelper->Install ();
      // simulation will stop right after the REM has been generated
    }
  else
    {
      Simulator::Stop (stopTime);
    }

  GlobalValue::GetValueByName ("logWifiFailRetries", booleanValue);
  if (booleanValue.Get () == true)
    {
      Simulator::Schedule (clientStartTime, &ScheduleWifiFailRetriesLogConnect);
      Simulator::Schedule (clientStopTime, &ScheduleWifiFailRetriesLogDisconnect);
    }

  GlobalValue::GetValueByName ("logWifiRetries", booleanValue);
  if (booleanValue.Get () == true)
    {
      Simulator::Schedule (clientStartTime, &ScheduleWifiRetriesLogConnect);
      Simulator::Schedule (clientStopTime, &ScheduleWifiRetriesLogDisconnect);
    }

  GlobalValue::GetValueByName ("logPhyArrivals", booleanValue);
  if (booleanValue.Get () == true)
    {
      Simulator::Schedule (clientStartTime, &SchedulePhyLogConnect);
      Simulator::Schedule (clientStopTime, &SchedulePhyLogDisconnect);
    }

  GlobalValue::GetValueByName ("logTxops", booleanValue);
  if (booleanValue.Get () == true)
    {
      Simulator::Schedule (clientStartTime, &ScheduleTxopLogConnect);
      //Simulator::Schedule (clientStopTime, &ScheduleTxopLogDisconnect);
    }

  GlobalValue::GetValueByName ("logDataTx", booleanValue);
  if (booleanValue.Get () == true)
    {
      Simulator::Schedule (clientStartTime, &ScheduleDataTxConnect);
     // Simulator::Schedule (clientStopTime, &ScheduleDataTxDisconnect);
    }

  GlobalValue::GetValueByName ("logBeaconArrivals", booleanValue);
  if (booleanValue.Get () == true)
    {
      Simulator::Schedule (clientStartTime, &ScheduleBeaconLogConnect);
      Simulator::Schedule (clientStopTime, &ScheduleBeaconLogDisconnect);
    }
  GlobalValue::GetValueByName ("logBackoffChanges", booleanValue);
  if (booleanValue.Get () == true)
    {
      Simulator::Schedule (clientStartTime, &ScheduleWifiBackoffLogConnect);
      Simulator::Schedule (clientStopTime, &ScheduleWifiBackoffLogDisconnect);
    }
  GlobalValue::GetValueByName ("logCwChanges", booleanValue);
  if (booleanValue.Get () == true)
    {
      Simulator::Schedule (clientStartTime, &ScheduleCwChangesLogConnect);
      Simulator::Schedule (clientStopTime, &ScheduleCwChangesLogDisconnect);
    }
  GlobalValue::GetValueByName ("logCtrlSignals", booleanValue);
  if (booleanValue.Get () == true)
    {
      Simulator::Schedule (clientStartTime, &ScheduleCtrlSignalLogConnect);
      //Simulator::Schedule (clientStopTime,  &ScheduleCtrlSignalLogDisconnect);
    }

  //lteHelper->EnableDlMacTraces();

  //
  // Running the simulation
  //
  Simulator::Run ();

  //
  // Post-processing phase
  //

  std::cout<<"Total txop duration: "<<g_txopDurationCounter<<" seconds." << std::endl;
  std::cout<<"Total phy arrivals duration: "<<g_arrivalsDurationCounter<<" seconds." << std::endl;

  std::cout << "--------monitorA----------" << std::endl;
  PrintFlowMonitorStats (monitorA, flowmonHelperA, durationTime.GetSeconds ());
  std::cout << "--------monitorB----------" << std::endl;
  PrintFlowMonitorStats (monitorB, flowmonHelperB, durationTime.GetSeconds ());

  if (transport == TCP)
    {
      SaveTcpFlowMonitorStats (outFileName + "_operatorA", simulationParams, monitorA, flowmonHelperA, durationTime.GetSeconds ());
      SaveTcpFlowMonitorStats (outFileName + "_operatorB", simulationParams, monitorB, flowmonHelperB, durationTime.GetSeconds ());
    }
  else if (transport == UDP || transport == FTP)
    {
      SaveUdpFlowMonitorStats (outFileName + "_operatorA", simulationParams, monitorA, flowmonHelperA, durationTime.GetSeconds ());
      SaveUdpFlowMonitorStats (outFileName + "_operatorB", simulationParams, monitorB, flowmonHelperB, durationTime.GetSeconds ());
    }
  else
    {
      NS_FATAL_ERROR ("transport parameter invalid: " << transport);
    }

  NodeContainer bsNodesForLogs;
  NodeContainer ueNodesForLogs;

  if(cellConfigA == LAA || cellConfigA == LTE)
    {
      bsNodesForLogs.Add(bsNodesA);
      ueNodesForLogs.Add(ueNodesA);
    }
  if (cellConfigB == LAA || cellConfigA == LTE)
    {
      bsNodesForLogs.Add(bsNodesB);
      ueNodesForLogs.Add(ueNodesB);
    }

  GlobalValue::GetValueByName ("logPhyArrivals", booleanValue);
  if (booleanValue.Get () == true)
    {
      SaveSpectrumPhyStats (outFileName + "_phy_log", g_arrivals);
    }
  GlobalValue::GetValueByName ("logTxops", booleanValue);
  if (booleanValue.Get () == true)
    {
      SaveTxopStats (outFileName + "_txop_log", g_txopLogs);
    }

  GlobalValue::GetValueByName ("logDataTx", booleanValue);
  if (booleanValue.Get () == true)
    {
      SaveDataTxStats (outFileName + "_dataTx_log", g_dataTxLogs);
    }

  GlobalValue::GetValueByName ("logBeaconArrivals", booleanValue);
  if (booleanValue.Get () == true)
    {
      SaveBeaconStats (outFileName + "_beacon_log", g_beaconArrivals);
    }
  GlobalValue::GetValueByName ("logCwChanges", booleanValue);
  if (booleanValue.Get () == true)
    {
      SaveCwStats (outFileName + "_cw_log", g_cwChanges);
    }
  GlobalValue::GetValueByName ("logBackoffChanges", booleanValue);
  if (booleanValue.Get () == true)
    {
      SaveBackoffStats (outFileName + "_backoff_log", g_backoffChanges);
    }
  GlobalValue::GetValueByName ("logHarqFeedback", booleanValue);
  if (booleanValue.Get () == true)
    {
      SaveHarqFeedbacksStats (outFileName + "_harq_feedback_log", g_harqFeedbacks);
    }
  GlobalValue::GetValueByName ("logReservationSignals", booleanValue);
  if (booleanValue.Get () == true)
    {
      SaveReservationSingalStats (outFileName + "_reservation_signal_log", g_reservationSingals);
    }
  GlobalValue::GetValueByName ("logWifiFailRetries", booleanValue);
  if (booleanValue.Get () == true)
    {
      SaveFailRetriesStats (outFileName + "_fail_retries_log", g_wifiFailRetries);
    }
  GlobalValue::GetValueByName ("logWifiRetries", booleanValue);
  if (booleanValue.Get () == true)
    {
      SaveRetriesStats (outFileName + "_retries_log", g_wifiRetries);
    }
  GlobalValue::GetValueByNameFailSafe ("voiceEnabled", booleanValue);
  if (booleanValue.Get () == true)
    {
      SaveVoiceSummaryStats (outFileName + "_operatorB_voice_summary_log", endpointNodesB);
      SaveVoiceStats (outFileName + "_operatorB_voice_log", endpointNodesB, g_voiceRxLog);
    }
  GlobalValue::GetValueByName ("logCtrlSignals", booleanValue);
  if (booleanValue.Get () == true)
    {
        SaveCtrlSigStats (outFileName + "_ctrlSignal_log", g_ctrlSignalLog);
    }
  GlobalValue::GetValueByName ("logAssociationStats", booleanValue);
  if (booleanValue.Get () == true)
    {
      SaveAssociationStats (outFileName + "_association_log", g_associations, bsNodesForLogs, ueNodesForLogs);
    }

  Simulator::Destroy ();

}

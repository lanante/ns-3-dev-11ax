/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Washington
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
 */

//
//  This example program can be used to experiment with basic spatial
//  reuse mechanisms of 802.11ax.
//
//  The geometry is as follows:
//
//                STA2          STA3
//                 |              |
//                 | d2           |d4
//          d1     |       d3     |
//  STA1 -------- AP1 -----------AP2
//
//  STA1, STA2, and AP1 are in one BSS, while AP2 and STA3 are in another BSS.
//  The distances are configurable (d1 through d4).
//
//  Each AP and STA have configurable traffic loads (traffic generators).
//  A simple Friis path loss model is used.
//
//  The basic data to be collected is:
//
//    - Average RSSI (signal strength of beacons)
//    - CCAT (CS/CCA threshold) in dBm
//    - raw signal strength data on all STAs/APs
//    - throughput and fairness
//    - NAV timer traces (and in general, the carrier sense state machine)
//
//  The attributes to be controlled are
//    - OBSS_PD threshold (dB)
//    - Tx power (W or dBm)
//    - DSC Margin (dBs)
//    - DSC Upper Limit (dBs)
//    - DSC Implemented (boolean true or false)
//    - Beacon Count Limit (limit of consecutive missed beacons)
//    - RSSI Decrement (dB) (if Beacon Count Limit exceeded)
//    - use of RTS/CTS
//
//  Some key findings to investigate:
//    - Tanguy Ropitault reports that margin highly affects performance,
//      that DSC margin of 25 dB gives best results, and DSC can increase
//      aggregate throughput compared to Legacy by 80%
//      Ref:  11-16-0604-02-00ax (0604r1), May 2016
//
//  In general, the program can be configured at run-time by passing
//  command-line arguments.  The command
//  ./waf --run "basic-spatial-reuse --help"
//  will display all of the available run-time help options.

#include <iostream>
#include <iomanip>
#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/internet-module.h>
#include <ns3/wifi-module.h>
#include <ns3/config-store-module.h>
#include <ns3/spectrum-module.h>
#include <ns3/applications-module.h>
#include <ns3/propagation-module.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("BasicSpatialReuse");

struct SignalArrival
{
  Time m_time;
  Time m_duration;
  bool m_wifi;
  uint32_t m_nodeId;
  uint32_t m_senderNodeId;
  double m_power;
};

std::vector<SignalArrival> g_arrivals;
double g_arrivalsDurationCounter = 0;

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
  g_arrivalsDurationCounter += rxDuration.GetSeconds();

  NS_LOG_DEBUG (context << " " << wifi << " " << senderNodeId << " " << rxPowerDbm << " " << rxDuration.GetSeconds ()/1000.0);
}

void
SaveSpectrumPhyStats (std::string filename, const std::vector<SignalArrival> &arrivals)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::app);
  outFile.setf (std::ios_base::fixed);

  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  outFile << "#time(s) nodeId type sender endTime(s) duration(ms)     powerDbm" << std::endl;
  for (std::vector<SignalArrival>::size_type i = 0; i != arrivals.size (); i++)
    {
      outFile << std::setprecision (9) << std::fixed << arrivals[i].m_time.GetSeconds () <<  " ";
      outFile << arrivals[i].m_nodeId << " ";
      outFile << ((arrivals[i].m_wifi == true) ? "wifi " : " lte ");
      outFile << arrivals[i].m_senderNodeId << " ";
      outFile <<  arrivals[i].m_time.GetSeconds () + arrivals[i].m_duration.GetSeconds () << " ";
      outFile << arrivals[i].m_duration.GetSeconds () * 1000.0 << " " << arrivals[i].m_power << std::endl;
    }
  outFile.close ();
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

int
main (int argc, char *argv[])
{  
  //bool disableApps = false;
  bool enableTracing = true;
  double duration = 60.0; // seconds
  double d1 = 20.0; // meters
  double d2 = 20.0; // meters
  double d3 = 20.0; // meters
  double d4 = 20.0; // meters
  uint32_t mcs = 0; // MCS value

  CommandLine cmd;
  cmd.Parse (argc, argv);

  // When logging, use prefixes
  LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);

  PacketMetadata::Enable ();

  // Create nodes and containers
  Ptr<Node> ap1 = CreateObject<Node> ();
  Ptr<Node> ap2 = CreateObject<Node> ();
  Ptr<Node> sta1 = CreateObject<Node> ();
  Ptr<Node> sta2 = CreateObject<Node> ();
  Ptr<Node> sta3 = CreateObject<Node> ();
  NodeContainer stasA, nodesA, nodesB, allNodes;
  nodesA.Add (sta1);
  nodesA.Add (sta2);
  stasA = nodesA;
  nodesA.Add (ap1);
  nodesB.Add (sta3);
  nodesB.Add (ap2);

  allNodes = NodeContainer (nodesA, nodesB);

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));     // STA1 at origin
  positionAlloc->Add (Vector (d1, d2, 0.0));       // STA2
  positionAlloc->Add (Vector (d1, 0, 0.0));        // AP1
  positionAlloc->Add (Vector (d1 + d3, d4, 0.0));  // STA3
  positionAlloc->Add (Vector (d1 + d3, 0, 0.0));   // AP2
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (allNodes);

  SpectrumWifiPhyHelper spectrumPhy = SpectrumWifiPhyHelper::Default ();
  Ptr<MultiModelSpectrumChannel> spectrumChannel
    = CreateObject<MultiModelSpectrumChannel> ();
  Ptr<FriisPropagationLossModel> lossModel
    = CreateObject<FriisPropagationLossModel> ();
  spectrumChannel->AddPropagationLossModel (lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel
    = CreateObject<ConstantSpeedPropagationDelayModel> ();
  spectrumChannel->SetPropagationDelayModel (delayModel);

  spectrumPhy.SetChannel (spectrumChannel);
  spectrumPhy.SetErrorRateModel ("ns3::YansErrorRateModel");
  spectrumPhy.Set ("Frequency", UintegerValue (5180)); // channel 36 at 20 MHz
  spectrumPhy.Set ("TxPowerStart", DoubleValue (1));
  spectrumPhy.Set ("TxPowerEnd", DoubleValue (1));
 
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211ax_5GHZ);
  WifiMacHelper mac;

  std::ostringstream oss;
  oss << "HeMcs" << mcs;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue (oss.str ()),
                                "ControlMode", StringValue (oss.str ()));

  Ssid ssidA = Ssid ("A");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssidA));
  NetDeviceContainer staDevicesA;
  staDevicesA = wifi.Install (spectrumPhy, mac, stasA);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssidA));

  NetDeviceContainer apDeviceA;
  apDeviceA = wifi.Install (spectrumPhy, mac, ap1);

  Ssid ssidB = Ssid ("B");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssidB));
  NetDeviceContainer staDevicesB;
  staDevicesB = wifi.Install (spectrumPhy, mac, sta3);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssidB));

  NetDeviceContainer apDeviceB;
  apDeviceB = wifi.Install (spectrumPhy, mac, ap2);

  /* Internet stack*/
  InternetStackHelper stack;
  stack.Install (allNodes);

  Ipv4AddressHelper address;
  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer staInterfacesA;
  Ipv4InterfaceContainer apInterfaceA;

  staInterfacesA = address.Assign (staDevicesA);
  apInterfaceA = address.Assign (apDeviceA);

  address.SetBase ("192.168.2.0", "255.255.255.0");
  Ipv4InterfaceContainer staInterfacesB;
  Ipv4InterfaceContainer apInterfaceB;

  staInterfacesB = address.Assign (staDevicesB);
  apInterfaceB = address.Assign (apDeviceB);

  // Add application data here

  // Create an OnOff application to send UDP datagrams with payload size
  // 1000 bytes at a rate of 1 pps
  NS_LOG_INFO ("Create Applications.");
  uint16_t port = 9;
  OnOffHelper onoff ("ns3::UdpSocketFactory",
                     InetSocketAddress (Ipv4Address ("192.168.1.3"), port));
  onoff.SetConstantRate (DataRate ("8kbps"));
  onoff.SetAttribute ("PacketSize", UintegerValue (1000));

  ApplicationContainer app = onoff.Install (sta1);
  app.Start (Seconds (0.5));
  app.Stop (Seconds (duration - 0.5));

  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
  ApplicationContainer sinkApp = sink.Install (ap1);
  sinkApp.Start (Seconds (0.5));
  sinkApp.Stop (Seconds (duration - 0.5));

  if (enableTracing)
    {
      AsciiTraceHelper ascii;
      spectrumPhy.EnableAsciiAll (ascii.CreateFileStream ("basic-spatial-reuse.tr"));
    }
  SchedulePhyLogConnect ();

  Time durationTime = Seconds (duration);
  Simulator::Stop (durationTime);
  Simulator::Run ();

  SchedulePhyLogDisconnect ();
  SaveSpectrumPhyStats ("basic-spatial-reuse-phy-log.dat", g_arrivals);

  Simulator::Destroy ();

  return 0;
}

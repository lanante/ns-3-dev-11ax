/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 University of Washington
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
//  This example program can be used to test the Dual NAV operation
//
//  For wifi standard 11ax, the geometry is as follows:
//
//          d1             d2     
//  STA1 -------- AP1 --------- STA2 
//                 |
//                 | d3
//                 | 
//                 |       d4
//                AP2 --------- STA3
//
//  STA1, STA2, and AP1 are in one BSS, while AP2 and STA3 are in another BSS.
//  The distances are configurable (d1 through d4).
//
//  Use of RTS/CTS can be enabled or disabled
//
//  ./waf --run "test-dual-nav-operation --help"
//  will display all of the available run-time help options.

#include <iostream>
#include <iomanip>
#include <string>
#include <ns3/core-module.h>
#include <ns3/config-store-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/internet-module.h>
#include <ns3/wifi-module.h>
#include <ns3/spectrum-module.h>
#include <ns3/applications-module.h>
#include <ns3/propagation-module.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TestNavDuration");

enum NavTestCase {
  CASE_UNKNOWN,
  CASE_11ax,
};
enum NavTestCase navTestCase;

struct SignalArrival
{
  Time m_time;
  Time m_duration;
  bool m_wifi;
  uint32_t m_nodeId;
  uint32_t m_senderNodeId;
  double m_power;
};

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

std::vector<SignalArrival> g_arrivals;
double g_arrivalsDurationCounter = 0;
std::ofstream g_phyStateFile;
std::ofstream g_navStateFile;

std::string
StateToString (WifiPhyState state)
{
  std::string stateString;
  switch (state)
    {
    case WifiPhyState::IDLE:
      stateString = "IDLE";
      break;
    case WifiPhyState::CCA_BUSY:
      stateString = "CCA_BUSY";
      break;
    case WifiPhyState::TX:
      stateString = "TX";
      break;
    case WifiPhyState::RX:
      stateString = "RX";
      break;
    case WifiPhyState::SWITCHING:
      stateString = "SWITCHING";
      break;
    case WifiPhyState::SLEEP:
      stateString = "SLEEP";
      break;
    default:
      NS_FATAL_ERROR ("Unknown state");
    }
  return stateString;
}

void
StateCb (std::string context, Time start, Time duration, WifiPhyState state)
{
  g_phyStateFile << ContextToNodeId (context) << " "
              << std::setprecision (6) << start.GetSeconds () << " " 
              << (start + duration).GetSeconds () << " " 
              << std::setprecision (1) << duration.As (Time::US) << " " 
              << StateToString (state) << std::endl;
}

void
SignalCb (std::string context, bool wifi, uint32_t senderNodeId, double rxPowerDbm, Time rxDuration)
{
  SignalArrival arr;
  arr.m_time = Simulator::Now ();
  arr.m_duration = rxDuration;
  arr.m_nodeId = ContextToNodeId (context);
  arr.m_senderNodeId = senderNodeId;
  arr.m_wifi = wifi;
  arr.m_power = rxPowerDbm;
  g_arrivals.push_back (arr);
  g_arrivalsDurationCounter += rxDuration.GetSeconds ();

  NS_LOG_DEBUG (context << " " << wifi << " " << senderNodeId << " " << rxPowerDbm << " " << rxDuration.GetSeconds () / 1000.0);
}

void
SaveSpectrumPhyStats (std::string filename, const std::vector<SignalArrival> &arrivals)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::trunc);
  outFile.setf (std::ios_base::fixed);
  outFile.flush ();

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

void
ScheduleStateLogConnect (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/State/State", MakeCallback (&StateCb));
}

void
ScheduleStateLogDisconnect (void)
{
  Config::Disconnect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/State/State", MakeCallback (&StateCb));
}

void
NavStateTrace (std::string context, Time start, Time duration)
{
  g_navStateFile << context << " " << std::setprecision (6) 
                  << start.GetSeconds () << " " << (start + duration).GetSeconds () 
                  << " " << std::setprecision (1) << duration.As (Time::US) << std::endl; 
}

int
main (int argc, char *argv[])
{
  bool enableTracing = true;
  bool verbose = false;
  double duration = 10.0; // seconds
  double d1 = 30.0; // meters
  double d2 = 30.0; // meters
  double d3 = 100.0; // meters
  double d4 = 30.0; // meters
  double powSta1 = 10.0; // dBm
  double powSta2 = 10.0; // dBm
  double powSta3 = 10.0; // dBm
  double powAp1 = 21.0; // dBm
  double powAp2 = 21.0; // dBm
  double ccaTrSta1 = -62; // dBm
  double ccaTrSta2 = -62; // dBm
  double ccaTrSta3 = -62; // dBm
  double ccaTrAp1 = -62; // dBm
  double ccaTrAp2 = -62; // dBm
  uint32_t payloadSize = 1500; // bytes
  Time interval = MicroSeconds (20);
  bool enableObssPd = true;
  uint32_t maxPackets = 20;
  std::string testCaseString = "11ax";
  RngSeedManager::SetSeed (3);
  RngSeedManager::SetRun (7);

  CommandLine cmd;
  cmd.AddValue ("interval", "Per-packet interval (s)", interval);
  cmd.AddValue ("enableObssPd", "Enable OBSS_PD", enableObssPd);
  cmd.AddValue ("d1", "Distance of STA1 to AP1", d1);
  cmd.AddValue ("d2", "Distance of STA2 to AP1", d2);
  cmd.AddValue ("d3", "Distance of AP1 to AP2", d3);
  cmd.AddValue ("d4", "Distance of STA3 to AP2", d4);
  cmd.AddValue ("powSta1", "Power of STA1", powSta1);
  cmd.AddValue ("powSta2", "Power of STA2", powSta2);
  cmd.AddValue ("powSta3", "Power of STA3", powSta3);
  cmd.AddValue ("powAp1", "Power of AP1", powAp1);
  cmd.AddValue ("powAp2", "Power of AP2", powAp2);
  cmd.AddValue ("ccaTrSta1", "CCA Threshold of STA1", ccaTrSta1);
  cmd.AddValue ("ccaTrSta2", "CCA Threshold of STA2", ccaTrSta2);
  cmd.AddValue ("ccaTrSta3", "CCA Threshold of STA3", ccaTrSta3);
  cmd.AddValue ("ccaTrAp1", "CCA Threshold of AP1", ccaTrAp1);
  cmd.AddValue ("ccaTrAp2", "CCA Threshold of AP2", ccaTrAp2);
  cmd.AddValue ("enableTracing", "Enable tracing", enableTracing);
  cmd.AddValue ("verbose", "Verbose", verbose);
  cmd.AddValue ("testCase", "Test case", testCaseString);
  cmd.Parse (argc, argv);

  // Map strings to enum so we can switch on the value
  if (testCaseString == "11ax") navTestCase = CASE_11ax;

  WifiHelper wifi;
  bool sta1Client = true;
  Time startTimeSta1 = Seconds (1);
  bool sta2Client = true;
  Time startTimeSta2 = Seconds (1);
  bool sta3Client = true;
  Time startTimeSta3 = Seconds (1);

  // Handle variable assignments for each test case
  switch (navTestCase)
    {
    case CASE_11ax:
      wifi.SetStandard (WIFI_PHY_STANDARD_80211ax_5GHZ);
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue ("HeMcs2"),
                                    "ControlMode", StringValue ("HeMcs0"));
      startTimeSta1 = MicroSeconds (300000);
      startTimeSta2 = MicroSeconds (300100);
      startTimeSta3 = MicroSeconds (300100);
      duration = 0.5;
      if (verbose)
        {
          LogComponentEnable ("ChannelAccessManager", LOG_LEVEL_ALL);
        }
      break;
    default:
      NS_FATAL_ERROR ("Unknown test case");
      break;
    }

  // When logging, use prefixes
  LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);

  PacketMetadata::Enable ();

  // Create nodes and containers
  Ptr<Node> sta1 = CreateObject<Node> ();
  Ptr<Node> sta2 = CreateObject<Node> ();
  Ptr<Node> ap1 = CreateObject<Node> ();
  Ptr<Node> sta3 = CreateObject<Node> ();
  Ptr<Node> ap2 = CreateObject<Node> ();

  NodeContainer stasA, nodesA, nodesB, allNodes;
  nodesA.Add (sta1);
  nodesA.Add (sta2);
  stasA = nodesA;
  nodesA.Add (ap1);
  nodesB.Add (sta3);
  nodesB.Add (ap2);

  allNodes = NodeContainer (nodesA, nodesB);

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
  spectrumPhy.Set ("TxPowerStart", DoubleValue (powSta1));
  spectrumPhy.Set ("TxPowerEnd", DoubleValue (powSta1));
  spectrumPhy.Set ("CcaEdThreshold", DoubleValue (ccaTrSta1));
  spectrumPhy.Set ("RxSensitivity", DoubleValue (-82.0));

  WifiMacHelper mac;
  Ssid ssidA = Ssid ("A");
  mac.SetType ("ns3::StaWifiMac","Ssid", SsidValue (ssidA));
  NetDeviceContainer staDevicesA;
  staDevicesA = wifi.Install (spectrumPhy, mac, stasA);

  spectrumPhy.Set ("TxPowerStart", DoubleValue (powAp1));
  spectrumPhy.Set ("TxPowerEnd", DoubleValue (powAp1));
  spectrumPhy.Set ("CcaEdThreshold", DoubleValue (ccaTrAp1));
  spectrumPhy.Set ("RxSensitivity", DoubleValue (-82.0));
  mac.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssidA));

  NetDeviceContainer apDeviceA;
  apDeviceA = wifi.Install (spectrumPhy, mac, ap1);
  Ptr<WifiNetDevice> apDevice = apDeviceA.Get (0)->GetObject<WifiNetDevice> ();
  Ptr<ApWifiMac> apWifiMac = apDevice->GetMac ()->GetObject<ApWifiMac> ();
#ifdef NOTYET
  if (enableObssPd)
    {
      apWifiMac->SetBssColor (1);
    }
#endif

  Ptr<WifiNetDevice> nd0 = staDevicesA.Get (0)->GetObject<WifiNetDevice> ();
  Ptr<WifiNetDevice> nd1 = staDevicesA.Get (1)->GetObject<WifiNetDevice> ();
  Ptr<RegularWifiMac> mac0 = nd0->GetMac ()->GetObject<RegularWifiMac> ();
  Ptr<RegularWifiMac> mac1 = nd1->GetMac ()->GetObject<RegularWifiMac> ();
#ifdef NOTYET
  Ptr<DcfManager> dcf0 = mac0->GetDcfManager ();
  dcf0->TraceConnect ("NavState", std::to_string (nd0->GetNode ()->GetId ()), MakeCallback (&NavStateTrace));
  Ptr<DcfManager> dcf1 = mac1->GetDcfManager ();
  dcf1->TraceConnect ("NavState", std::to_string (nd1->GetNode ()->GetId ()), MakeCallback (&NavStateTrace));
#endif
  
  NetDeviceContainer staDevicesB;
  NetDeviceContainer apDeviceB;
  spectrumPhy.Set ("TxPowerStart", DoubleValue (powSta3));
  spectrumPhy.Set ("TxPowerEnd", DoubleValue (powSta3));
  spectrumPhy.Set ("CcaEdThreshold", DoubleValue (ccaTrSta3));
  spectrumPhy.Set ("RxSensitivity", DoubleValue (-92.0));
  Ssid ssidB = Ssid ("B");
  mac.SetType ("ns3::StaWifiMac","Ssid", SsidValue (ssidB));
  staDevicesB = wifi.Install (spectrumPhy, mac, sta3);

  spectrumPhy.Set ("TxPowerStart", DoubleValue (powAp2));
  spectrumPhy.Set ("TxPowerEnd", DoubleValue (powAp2));
  spectrumPhy.Set ("CcaEdThreshold", DoubleValue (ccaTrAp2));
  spectrumPhy.Set ("RxSensitivity", DoubleValue (-92.0));
  mac.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssidB));

  apDeviceB = wifi.Install (spectrumPhy, mac, ap2);
  Ptr<WifiNetDevice> ap2Device = apDeviceB.Get (0)->GetObject<WifiNetDevice> ();
  apWifiMac = ap2Device->GetMac ()->GetObject<ApWifiMac> ();
#ifdef NOTYET
  if (enableObssPd)
    {
      apWifiMac->SetBssColor (2);
    }
#endif

  Ptr<WifiNetDevice> nd3 = staDevicesB.Get (0)->GetObject<WifiNetDevice> ();
  Ptr<RegularWifiMac> mac3 = nd3->GetMac ()->GetObject<RegularWifiMac> ();
#ifdef NOTYET
  Ptr<DcfManager> dcf3 = mac3->GetDcfManager ();
  dcf3->TraceConnect ("NavState", std::to_string (nd3->GetNode ()->GetId ()), MakeCallback (&NavStateTrace));
#endif

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));     // STA1 at origin
  positionAlloc->Add (Vector (d1, d2, 0.0));       // STA2
  positionAlloc->Add (Vector (d1, 0, 0.0));        // AP1
  positionAlloc->Add (Vector (d1 + d3, d4, 0.0));  // STA3
  positionAlloc->Add (Vector (d1 + d3, 0, 0.0));   // AP2


  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (allNodes);

  // Pakcet Socket for lower layer
  PacketSocketHelper packetSocket;
  packetSocket.Install (allNodes);

  //uint32_t nNodes = allNodes.GetN ();
  ApplicationContainer apps;
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

  // Add STA1 client
  if (sta1Client)
    {
      PacketSocketAddress socketAddr;
      socketAddr.SetSingleDevice (staDevicesA.Get (0)->GetIfIndex ());
      socketAddr.SetPhysicalAddress (apDeviceA.Get (0)->GetAddress ());
      socketAddr.SetProtocol (1);
      Ptr<PacketSocketClient> client = CreateObject<PacketSocketClient> ();
      client->SetRemote (socketAddr);
      allNodes.Get (0)->AddApplication (client);
      client->SetAttribute ("PacketSize", UintegerValue (payloadSize));
      client->SetAttribute ("MaxPackets", UintegerValue (maxPackets));
      client->SetAttribute ("Interval", TimeValue (interval));
      client->SetStartTime (startTimeSta1);
      Ptr<PacketSocketServer> server = CreateObject<PacketSocketServer> ();
      server->SetLocal (socketAddr);
      allNodes.Get (2)->AddApplication (server);
    }

  // Add STA2 client
  if (sta2Client)
    {
      PacketSocketAddress socketAddr;
      socketAddr.SetSingleDevice (staDevicesA.Get (1)->GetIfIndex ());
      socketAddr.SetPhysicalAddress (apDeviceA.Get (0)->GetAddress ());
      socketAddr.SetProtocol (1);
      Ptr<PacketSocketClient> client = CreateObject<PacketSocketClient> ();
      client->SetRemote (socketAddr);
      allNodes.Get (1)->AddApplication (client);
      client->SetAttribute ("PacketSize", UintegerValue (payloadSize));
      client->SetAttribute ("MaxPackets", UintegerValue (maxPackets));
      client->SetAttribute ("Interval", TimeValue (interval));
      client->SetStartTime (startTimeSta2);
      Ptr<PacketSocketServer> server = CreateObject<PacketSocketServer> ();
      server->SetLocal (socketAddr);
      allNodes.Get (2)->AddApplication (server);
    }

  // Add STA3 client
  if (sta3Client)
    {
      PacketSocketAddress socketAddr;
      socketAddr.SetSingleDevice (staDevicesB.Get (0)->GetIfIndex ());
      socketAddr.SetPhysicalAddress (apDeviceB.Get (0)->GetAddress ());
      socketAddr.SetProtocol (1);
      Ptr<PacketSocketClient> client = CreateObject<PacketSocketClient> ();
      client->SetRemote (socketAddr);
      allNodes.Get (3)->AddApplication (client);
      client->SetAttribute ("PacketSize", UintegerValue (payloadSize));
      client->SetAttribute ("MaxPackets", UintegerValue (maxPackets));
      client->SetAttribute ("Interval", TimeValue (interval));
      client->SetStartTime (startTimeSta2);
      Ptr<PacketSocketServer> server = CreateObject<PacketSocketServer> ();
      server->SetLocal (socketAddr);
      allNodes.Get (4)->AddApplication (server);
    }

  if (enableTracing)
    {
      AsciiTraceHelper ascii;
      spectrumPhy.EnableAsciiAll (ascii.CreateFileStream ("test-nav-operation.tr"));
    }

  // This enabling function could be scheduled later in simulation if desired
  SchedulePhyLogConnect ();
  g_phyStateFile.open ("test-nav-operation.phy.log", std::ofstream::out | std::ofstream::trunc);
  g_phyStateFile.setf (std::ios_base::fixed);
  g_navStateFile.open ("test-nav-operation.nav.log", std::ofstream::out | std::ofstream::trunc);
  g_navStateFile.setf (std::ios_base::fixed);
  ScheduleStateLogConnect ();

  // Save attribute configuration
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("test-nav-operation.config"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureAttributes ();

  if (enableTracing)
    {
      spectrumPhy.EnablePcap ("test-nav-operationA", staDevicesA);
      spectrumPhy.EnablePcap ("test-nav-operationB", staDevicesB);
    }

  Time durationTime = Seconds (duration);
  Simulator::Stop (durationTime);
  Simulator::Run ();

  SchedulePhyLogDisconnect ();
  ScheduleStateLogDisconnect ();
  g_phyStateFile.flush ();
  g_phyStateFile.close ();
  g_navStateFile.flush ();
  g_navStateFile.close ();
  SaveSpectrumPhyStats ("test-nav-operation.spectrum.log", g_arrivals);

  Simulator::Destroy ();

  return 0;
}

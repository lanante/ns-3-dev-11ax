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
//  This example program can be used to experiment with spatial
//  reuse mechanisms of 802.11ax.
//
//  The geometry is as follows:
//
//  There are two APs (AP1 and AP2) separated by distance d.
//  Around each AP, there are n STAs dropped uniformly in a circle of
//  radious r (i.e., using hte UnitDiscPositionAllocator)..
//  Parameters d, n, and r are configurable command line arguments
//
//  Each AP and STA have configurable traffic loads (traffic generators).
//  A simple Friis path loss model is used.
//
//  Key confirmation parameters available through command line options, include:
//  --duration             Duration of simulation, in seconds
//  --powSta               Power of STA (dBm)
//  --powAp                Power of AP (dBm)
//  --ccaTrSta             CCA threshold of STA (dBm)
//  --ccaTrAp              CCA threshold of AP (dBm)
//  --d                    Distance between AP1 and AP2, in meters
//  --n                    Number of STAs to scatter around each AP
//  --r                    Radius of circle around each AP in which to scatter STAS, in meters
//  --uplink               Total aggregate uplink load, STAs->AP (Mbps).  Allocated pro rata to each link.
//  --downlink             Total aggregate downlink load,  AP->STAs (Mbps).  Allocated pro rata to each link.
//  --standard             802.11 standard.  E.g., "11ax_5GHZ"
//  --bw                   Bandwidth (consistent with standard), in MHz
//  --enableObssPd         Enable OBSS_PD.  Default is True for 11ax only, false for others
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
//  ./waf --run "spatial-reuse --help"
//  will display all of the available run-time help options.

#include <iostream>
#include <iomanip>
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

NS_LOG_COMPONENT_DEFINE ("SpatialReuse");

struct SignalArrival
{
  Time m_time;
  Time m_duration;
  bool m_wifi;
  uint32_t m_nodeId;
  uint32_t m_senderNodeId;
  double m_power;
};

// for tracking packets and bytes received. will be reallocated once we finalize number of nodes
std::vector<uint32_t> packetsReceived (0);
std::vector<uint32_t> bytesReceived (0);

std::vector<std::vector<uint32_t>> packetsReceivedPerNode;
std::vector<std::vector<double>> rssiPerNode;

// Parse context strings of the form "/NodeList/3/DeviceList/1/Mac/Assoc"
// to extract the NodeId
uint32_t
ContextToNodeId (std::string context)
{
  //std::cout << "Context=" << context << std::endl;
  std::string sub = context.substr (10);  // skip "/NodeList/"
  uint32_t pos = sub.find ("/Device");
  uint32_t nodeId = atoi (sub.substr (0, pos).c_str ());
  NS_LOG_DEBUG ("Found NodeId " << nodeId);
  //std::cout << "and nodeId=" << nodeId << std::endl;
  return nodeId;
}

void
SocketRecvStats (std::string context, Ptr<const Packet> p, const Address &addr)
{
  uint32_t nodeId = ContextToNodeId (context);
  uint32_t pktSize = p->GetSize ();
  //  std::cout << "Node ID: " << nodeId << " RX addr=" << addr << " size=" << pktSize << std::endl;
  bytesReceived[nodeId] += pktSize;
  packetsReceived[nodeId]++;
}

std::vector<SignalArrival> g_arrivals;
double g_arrivalsDurationCounter = 0;
std::ofstream g_stateFile;

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
  g_stateFile << ContextToNodeId (context) << " " << start.GetSeconds () << " " << duration.GetSeconds () << " " << StateToString (state) << std::endl;

  //uint32_t nodeId = ContextToNodeId (context);
  //std::string stateStr = StateToString (state);
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
  uint32_t nodeId = ContextToNodeId (context);
  packetsReceivedPerNode[nodeId][senderNodeId] += 1;
  rssiPerNode[nodeId][senderNodeId] += rxPowerDbm;
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
AddClient (Ptr<NetDevice> rxDevice, Ptr<NetDevice> txDevice, Ptr<Node> txNode, uint32_t payloadSize, uint32_t maxPackets, Time interval)
{
  PacketSocketAddress socketAddr;
  socketAddr.SetSingleDevice (txDevice->GetIfIndex ());
  socketAddr.SetPhysicalAddress (rxDevice->GetAddress ());
  socketAddr.SetProtocol (1);
  Ptr<PacketSocketClient> client = CreateObject<PacketSocketClient> ();
  client->SetRemote (socketAddr);
  txNode->AddApplication (client);
  client->SetAttribute ("PacketSize", UintegerValue (payloadSize));
  client->SetAttribute ("MaxPackets", UintegerValue (maxPackets));
  client->SetAttribute ("Interval", TimeValue (interval));
}

void
AddServer (Ptr<NetDevice> rxDevice, Ptr<NetDevice> txDevice, Ptr<Node> rxNode)
{
  PacketSocketAddress socketAddr;
  socketAddr.SetSingleDevice (txDevice->GetIfIndex ());
  socketAddr.SetPhysicalAddress (rxDevice->GetAddress ());
  socketAddr.SetProtocol (1);
  Ptr<PacketSocketClient> client = CreateObject<PacketSocketClient> ();
  Ptr<PacketSocketServer> server = CreateObject<PacketSocketServer> ();
  server->SetLocal (socketAddr);
  rxNode->AddApplication (server);
}

void
SaveSpatialReuseStats (std::string filename, 
  const std::vector<uint32_t> &packetsReceived, 
  const std::vector<uint32_t> &bytesReceived, 
  const double duration,
  const double d,
  const double r,
  int freqHz)
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

  uint32_t numNodes = packetsReceived.size();
  uint32_t n = (numNodes / 2) - 1;

  outFile << "Spatial Reuse Statistics" << std::endl;
  outFile << "APs: " << "2" << std::endl;
  outFile << "Nodes per AP: " << n << std::endl;
  outFile << "Distance between APs [m]: " << d << std::endl;
  outFile << "Radius [m]: " << r << std::endl;

  uint32_t bytesReceivedAp1Uplink = 0.0;
  uint32_t bytesReceivedAp1Downlink = 0.0;
  uint32_t bytesReceivedAp2Uplink = 0.0;
  uint32_t bytesReceivedAp2Downlink = 0.0;

  double rxThroughputPerNode[numNodes];
  for (uint32_t k = 0; k < numNodes; k++)
    {
      if (k == 0)
        {
          bytesReceivedAp1Uplink += bytesReceived[k];
        }
      else if (k == 1)
        {
          bytesReceivedAp2Uplink += bytesReceived[k];
        }
      else if (k < n)
        {
          bytesReceivedAp1Downlink += bytesReceived[k];
        }
      else
        {
          bytesReceivedAp2Downlink += bytesReceived[k];
        }
      double bitsReceived = bytesReceived[k] * 8;
      // rxThroughputPerNode[k] = static_cast<double> (packetsReceived[k] * payloadSize * 8) / 1e6 / duration; 
      rxThroughputPerNode[k] = static_cast<double> (bitsReceived) / 1e6 / duration; 
      outFile << "Node " << k << ", pkts " << packetsReceived[k] << ", bytes " << bytesReceived[k] << ", throughput [MMb/s] " << rxThroughputPerNode[k] << std::endl;
    }

  double tputAp1Uplink = bytesReceivedAp1Uplink * 8 / 1e6 / duration;
  double tputAp1Downlink = bytesReceivedAp1Downlink * 8 / 1e6 / duration;
  double tputAp2Uplink = bytesReceivedAp2Uplink * 8 / 1e6 / duration;
  double tputAp2Downlink = bytesReceivedAp2Downlink * 8 /1e6 / duration;

  // TODO: debug to print out t-put, can remove
  std::cout << "Throughput,  AP1 Uplink   [Mbps] : " << tputAp1Uplink << std::endl;
  std::cout << "Throughput,  AP1 Downlink [Mbps] : " << tputAp1Downlink << std::endl;
  std::cout << "Throughput,  AP2 Uplink   [Mbps] : " << tputAp2Uplink << std::endl;
  std::cout << "Throughput,  AP1 Downlink [Mbps] : " << tputAp2Downlink << std::endl;

  outFile << "Throughput,  AP1 Uplink   [Mbps] : " << tputAp1Uplink << std::endl;
  outFile << "Throughput,  AP1 Downlink [Mbps] : " << tputAp1Downlink << std::endl;
  outFile << "Throughput,  AP2 Uplink   [Mbps] : " << tputAp2Uplink << std::endl;
  outFile << "Throughput,  AP1 Downlink [Mbps] : " << tputAp2Downlink << std::endl;

  double area = M_PI * r * r;
  outFile << "Area Capacity, AP1 Uplink   [Mbps/m^2] : " << tputAp1Uplink / area << std::endl;
  outFile << "Area Capacity, AP1 Downlink [Mbps/m^2] : " << tputAp1Downlink / area << std::endl;
  outFile << "Area Capacity, AP2 Uplink   [Mbps/m^2] : " << tputAp2Uplink / area << std::endl;
  outFile << "Area Capacity, AP2 Downlink [Mbps/m^2] : " << tputAp2Downlink / area << std::endl;

  outFile << "Spectrum Efficiency, AP1 Uplink   [Mbps/Hz] : " << tputAp1Uplink / freqHz << std::endl;
  outFile << "Spectrum Efficiency, AP1 Downlink [Mbps/Hz] : " << tputAp1Downlink / freqHz << std::endl;
  outFile << "Spectrum Efficiency, AP2 Uplink   [Mbps/Hz] : " << tputAp2Uplink / freqHz << std::endl;
  outFile << "Spectrum Efficiency, AP2 Downlink [Mbps/Hz] : " << tputAp2Downlink / freqHz << std::endl;

  outFile << "Avg. RSSI:" << std::endl;
  for (uint32_t rxNodeId = 0; rxNodeId < numNodes; rxNodeId++)
    {
      for (uint32_t txNodeId = 0; txNodeId < numNodes; txNodeId++)
        {
          uint32_t pkts = packetsReceivedPerNode[rxNodeId][txNodeId];
          double rssi = rssiPerNode[rxNodeId][txNodeId];
          double avgRssi = 0.0;
          if (pkts > 0)
            {
              avgRssi = rssi / pkts;
            }
          outFile << avgRssi << "  ";
        }
      outFile << std::endl;
    }

  outFile.close ();

  std::cout << "Spatial Reuse Stats written to: " << filename << std::endl;
}

// main script
int
main (int argc, char *argv[])
{
  Config::SetDefault ("ns3::RegularWifiMac::VO_BlockAckThreshold", UintegerValue (0));
  Config::SetDefault ("ns3::RegularWifiMac::VI_BlockAckThreshold", UintegerValue (0));
  Config::SetDefault ("ns3::RegularWifiMac::BE_BlockAckThreshold", UintegerValue (0));
  Config::SetDefault ("ns3::RegularWifiMac::BK_BlockAckThreshold", UintegerValue (0));
  Config::SetDefault ("ns3::HeConfiguration::ObssPdThreshold", DoubleValue (-82.0));

  // command line configurable parameters
  bool tracing = true;
  bool enableTracing = true;
  double duration = 20.0; // seconds
  double powSta = 10.0; // dBm
  double powAp = 21.0; // dBm
  double ccaTrSta = -102; // dBm
  double ccaTrAp = -82; // dBm
  double d = 100.0; // distance between AP1 and AP2, m
  uint32_t n = 1; // number of STAs to scatter around each AP;
  double r = 50.0; // radius of circle around each AP in which to scatter the STAs
  double aggregateUplinkMbps = 1.0;
  double aggregateDownlinkMbps = 1.0;
  int bw = 20;
  std::string standard ("11ax_5GHZ");

  // local variables
  std::string outputFilePrefix = "spatial-reuse";
  uint32_t payloadSize = 1500; // bytes
  uint32_t mcs = 7; // MCS value
  Time interval = MicroSeconds (1000);
  bool enableObssPd = false;

  CommandLine cmd;
  cmd.AddValue ("duration", "Duration of simulation (s)", duration);
  cmd.AddValue ("powSta", "Power of STA (dBm)", powSta);
  cmd.AddValue ("powAp", "Power of AP (dBm)", powAp);
  cmd.AddValue ("ccaTrSta", "CCA Threshold of STA (dBm)", ccaTrSta);
  cmd.AddValue ("ccaTrAp", "CCA Threshold of AP (dBm)", ccaTrAp);
  cmd.AddValue ("d", "Distance between AP1 and AP2 (m)", d);
  cmd.AddValue ("n", "Number of STAs to scatter around each AP", n);
  cmd.AddValue ("r", "Radius of circle around each AP in which to scatter STAs (m)", r);
  cmd.AddValue ("uplink", "Aggregate uplink load, STAs-AP (Mbps)", aggregateUplinkMbps);
  cmd.AddValue ("downlink", "Aggregate downlink load, AP-STAs (Mbps)", aggregateDownlinkMbps);
  cmd.AddValue ("standard", "Set standard (802.11a, 802.11b, 802.11g, 802.11n-5GHz, 802.11n-2.4GHz, 802.11ac, 802.11-holland, 802.11-10MHz, 802.11-5MHz, 802.11ax-5GHz, 802.11ax-2.4GHz)", standard);
  cmd.AddValue ("bw", "Bandwidth (consistent with standard, in MHz)", bw);
  cmd.AddValue ("enableObssPd", "Enable OBSS_PD", enableObssPd);
  cmd.Parse (argc, argv);

  WifiHelper wifi;
  std::string dataRate;
  int freq;
  Time dataStartTime = MicroSeconds (800); // leaving enough time for beacon and association procedure
  Time dataDuration = MicroSeconds (300); // leaving enough time for data transfer (+ acknowledgment)
  if (standard == "11a")
    {
      wifi.SetStandard (WIFI_PHY_STANDARD_80211a);
      // ssid = Ssid ("ns380211a");
      dataRate = "OfdmRate6Mbps";
      freq = 5180;
      if (bw != 20)
        {
          std::cout << "Bandwidth is not compatible with standard" << std::endl;
          return 1;
        }
    }
  else if (standard == "11_10MHZ")
    {
      wifi.SetStandard (WIFI_PHY_STANDARD_80211_10MHZ);
      // ssid = Ssid ("ns380211_10MHZ");
      dataRate = "OfdmRate3MbpsBW10MHz";
      freq = 5860;
      dataStartTime = MicroSeconds (1400);
      dataDuration = MicroSeconds (600);
      if (bw != 10)
        {
          std::cout << "Bandwidth is not compatible with standard" << std::endl;
          return 1;
        }
    }
  else if (standard == "11_5MHZ")
    {
      wifi.SetStandard (WIFI_PHY_STANDARD_80211_5MHZ);
      // ssid = Ssid ("ns380211_5MHZ");
      dataRate = "OfdmRate1_5MbpsBW5MHz";
      freq = 5860;
      dataStartTime = MicroSeconds (2500);
      dataDuration = MicroSeconds (1200);
      if (bw != 5)
        {
          std::cout << "Bandwidth is not compatible with standard" << std::endl;
          return 1;
        }
    }
  else if (standard == "11n_2_4GHZ")
    {
      wifi.SetStandard (WIFI_PHY_STANDARD_80211n_2_4GHZ);
      // ssid = Ssid ("ns380211n_2_4GHZ");
      dataRate = "HtMcs0";
      freq = 2402 + (bw / 2); //so as to have 2412/2422 for 20/40
      dataStartTime = MicroSeconds (4700);
      dataDuration = MicroSeconds (400);
      if (bw != 20 && bw != 40)
        {
          std::cout << "Bandwidth is not compatible with standard" << std::endl;
          return 1;
        }
    }
  else if (standard == "11n_5GHZ")
    {
      wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
      // ssid = Ssid ("ns380211n_5GHZ");
      dataRate = "HtMcs0";
      freq = 5170 + (bw / 2); //so as to have 5180/5190 for 20/40
      dataStartTime = MicroSeconds (1000);
      if (bw != 20 && bw != 40)
        {
          std::cout << "Bandwidth is not compatible with standard" << std::endl;
          return 1;
        }
    }
  else if (standard == "11ac")
    {
      wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);
      // ssid = Ssid ("ns380211ac");
      dataRate = "VhtMcs0";
      freq = 5170 + (bw / 2); //so as to have 5180/5190/5210/5250 for 20/40/80/160
      dataStartTime = MicroSeconds (1100);
      dataDuration += MicroSeconds (400); //account for ADDBA procedure
      if (bw != 20 && bw != 40 && bw != 80 && bw != 160)
        {
          std::cout << "Bandwidth is not compatible with standard" << std::endl;
          return 1;
        }
    }
  else if (standard == "11ax_2_4GHZ")
    {
      wifi.SetStandard (WIFI_PHY_STANDARD_80211ax_2_4GHZ);
      // ssid = Ssid ("ns380211ax_2_4GHZ");
      dataRate = "HeMcs0";
      freq = 2402 + (bw / 2); //so as to have 2412/2422/2442 for 20/40/80
      dataStartTime = MicroSeconds (5500);
      dataDuration += MicroSeconds (2000); //account for ADDBA procedure
      if (bw != 20 && bw != 40 && bw != 80)
        {
          std::cout << "Bandwidth is not compatible with standard" << std::endl;
          return 1;
        }
    }
  else if (standard == "11ax_5GHZ")
    {
      wifi.SetStandard (WIFI_PHY_STANDARD_80211ax_5GHZ);
      // ssid = Ssid ("ns380211ax_5GHZ");
      dataRate = "HeMcs0";  // TODO MCS 0 or 7 ???
      freq = 5170 + (bw / 2); //so as to have 5180/5190/5210/5250 for 20/40/80/160
      dataStartTime = MicroSeconds (1200);
      dataDuration += MicroSeconds (500); //account for ADDBA procedure
      if (bw != 20 && bw != 40 && bw != 80 && bw != 160)
        {
          std::cout << "Bandwidth is not compatible with standard" << std::endl;
          return 1;
        }

      // enable OSBB_PD
      enableObssPd = true;
    }
  else
    {
      std::cout << "Unknown OFDM standard (please refer to the listed possible values)" << std::endl;
      return 1;
    }


  // total expected nodes.  n STAs for each AP
  uint32_t numNodes = 2 * (n + 1);
  packetsReceived = std::vector<uint32_t> (numNodes);
  bytesReceived = std::vector<uint32_t> (numNodes);

  packetsReceivedPerNode.resize (numNodes, std::vector<uint32_t> (numNodes, 0));
  rssiPerNode.resize (numNodes, std::vector<double> (numNodes, 0.0));

  for (uint32_t nodeId = 0; nodeId < numNodes; nodeId++)
    {
      packetsReceived[nodeId] = 0;
      bytesReceived[nodeId] = 0;
    }

  // When logging, use prefixes
  LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);

  PacketMetadata::Enable ();

  // Create nodes and containers
  Ptr<Node> ap1 = CreateObject<Node> ();
  Ptr<Node> ap2 = CreateObject<Node> ();
  // node containers for two APs and their STAs
  NodeContainer stasA, stasB,nodesA, nodesB, allNodes;

  // network "A"
  for (uint32_t i = 0; i < n; i++)
    {
      Ptr<Node> sta = CreateObject<Node> ();
      stasA.Add (sta);
    }

  // AP at front of node container, then STAs
  nodesA.Add (ap1);
  nodesA.Add (stasA);

  // network "B"
  for (uint32_t i = 0; i < n; i++)
    {
      Ptr<Node> sta = CreateObject<Node> ();
      stasB.Add (sta);
    }

  // AP at front of node container, then STAs
  nodesB.Add (ap2);
  nodesB.Add (stasB);

  // the container for all nodes (from Network "A" and Network "B")
  allNodes = NodeContainer (nodesA, nodesB);

  // PHY setup
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
  spectrumPhy.Set ("Frequency", UintegerValue (freq)); // channel 36 at 20 MHz
  spectrumPhy.Set ("ChannelWidth", UintegerValue (bw));

  // WiFi setup / helpers
  WifiMacHelper mac;

  std::ostringstream oss;
  oss << "HeMcs" << mcs;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue (dataRate),
                                "ControlMode", StringValue (dataRate));

  // Set PHY power and CCA threshold for STAs
  spectrumPhy.Set ("TxPowerStart", DoubleValue (powSta));
  spectrumPhy.Set ("TxPowerEnd", DoubleValue (powSta));
  spectrumPhy.Set ("CcaMode1Threshold", DoubleValue (ccaTrSta));
  //PHY energy threshold -92 dBm
  spectrumPhy.Set ("EnergyDetectionThreshold", DoubleValue (-92.0));

  // Network "A"
  Ssid ssidA = Ssid ("A");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssidA));
  NetDeviceContainer staDevicesA;
  staDevicesA = wifi.Install (spectrumPhy, mac, stasA);

  // Set  PHY power and CCA threshold for APs
  spectrumPhy.Set ("TxPowerStart", DoubleValue (powAp));
  spectrumPhy.Set ("TxPowerEnd", DoubleValue (powAp));
  spectrumPhy.Set ("CcaMode1Threshold", DoubleValue (ccaTrAp));
  spectrumPhy.Set ("EnergyDetectionThreshold", DoubleValue (-92.0));
  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssidA));

  // AP1
  NetDeviceContainer apDeviceA;
  apDeviceA = wifi.Install (spectrumPhy, mac, ap1);
  Ptr<WifiNetDevice> apDevice = apDeviceA.Get (0)->GetObject<WifiNetDevice> ();
  Ptr<ApWifiMac> apWifiMac = apDevice->GetMac ()->GetObject<ApWifiMac> ();
  // The below statements may be simplified in a future HeConfigurationHelper
  Ptr<HeConfiguration> heConfiguration = CreateObject<HeConfiguration> ();
  if (enableObssPd)
    {
      heConfiguration->SetAttribute ("BssColor", UintegerValue (1));
    }
  apWifiMac->SetHeConfiguration (heConfiguration);

  // Set PHY power and CCA threshold for STAs
  spectrumPhy.Set ("TxPowerStart", DoubleValue (powSta));
  spectrumPhy.Set ("TxPowerEnd", DoubleValue (powSta));
  spectrumPhy.Set ("CcaMode1Threshold", DoubleValue (ccaTrSta));
  spectrumPhy.Set ("EnergyDetectionThreshold", DoubleValue (-92.0));

  // Network "B"
  Ssid ssidB = Ssid ("B");
  mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssidB));
  NetDeviceContainer staDevicesB;
  staDevicesB = wifi.Install (spectrumPhy, mac, stasB);

  // Set PHY power and CCA threshold for APs
  spectrumPhy.Set ("TxPowerStart", DoubleValue (powAp));
  spectrumPhy.Set ("TxPowerEnd", DoubleValue (powAp));
  spectrumPhy.Set ("CcaMode1Threshold", DoubleValue (ccaTrAp));
  spectrumPhy.Set ("EnergyDetectionThreshold", DoubleValue (-92.0));
  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssidB));

  // AP2
  NetDeviceContainer apDeviceB;
  apDeviceB = wifi.Install (spectrumPhy, mac, ap2);
  Ptr<WifiNetDevice> ap2Device = apDeviceB.Get (0)->GetObject<WifiNetDevice> ();
  apWifiMac = ap2Device->GetMac ()->GetObject<ApWifiMac> ();
  heConfiguration = CreateObject<HeConfiguration> ();
  if (enableObssPd)
    {
      heConfiguration->SetAttribute ("BssColor", UintegerValue (2));
    }
  apWifiMac->SetHeConfiguration (heConfiguration);

  // Assign positions to all nodes using position allocator
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  // allocate in the order of AP_A, STAs_A, AP_B, STAs_B

  std::string filename = outputFilePrefix + "-positions.csv";
  std::ofstream positionOutFile;
  positionOutFile.open (filename.c_str (), std::ofstream::out | std::ofstream::trunc);
  positionOutFile.setf (std::ios_base::fixed);
  positionOutFile.flush ();

  if (!positionOutFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return 1;
    }


  // bounding box
  positionOutFile << -r <<      ", " << -r << std::endl;
  positionOutFile << (d + r) << ", " << -r << std::endl;
  positionOutFile << (d + r) << ", " <<  r << std::endl;
  positionOutFile << -r <<      ", " <<  r << std::endl;
  positionOutFile << std::endl;
  positionOutFile << std::endl;

  // "A" - APs
  positionOutFile << 0.0 << ", " << 0.0 << ", " << r << std::endl;
  positionOutFile << std::endl;
  positionOutFile << std::endl;

  // "B" - APs
  positionOutFile << d << ", " << 0.0 << ", " << r << std::endl;
  positionOutFile << std::endl;
  positionOutFile << std::endl;

  // consistent stream to that positional layout is not perturbed by other configuration choices
  int64_t streamNumber = 100;

  // Network "A"
  // AP1
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));        // AP1
  // STAs for AP1
  // STAs for each AP are allocated uwing a different instance of a UnitDiscPositionAllocation.  To
  // ensure unique randomness of positions,  each allocator must be allocated a different stream number.
  Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator1 = CreateObject<UniformDiscPositionAllocator> ();
  unitDiscPositionAllocator1->AssignStreams (streamNumber);
  // AP1 is at origin (x=0, y=0), with radius Rho=r
  unitDiscPositionAllocator1->SetX (0);
  unitDiscPositionAllocator1->SetY (0);
  unitDiscPositionAllocator1->SetRho (r);
  for (uint32_t i = 0; i < n; i++)
    {
      Vector v = unitDiscPositionAllocator1->GetNext ();
      positionAlloc->Add (v);
      positionOutFile << v.x << ", " << v.y << std::endl;
    }
  positionOutFile << std::endl;
  positionOutFile << std::endl;

  // Network "B"
  // AP2
  positionAlloc->Add (Vector (d, 0.0, 0.0));        // AP2
  // STAs for AP2
  Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator2 = CreateObject<UniformDiscPositionAllocator> ();
  // see comments above - each allocator must have unique stream number.
  unitDiscPositionAllocator2->AssignStreams (streamNumber + 1);
  // AP2 is at (x=d, y=0), with radius Rho=r
  unitDiscPositionAllocator2->SetX (d);
  unitDiscPositionAllocator2->SetY (0);
  unitDiscPositionAllocator2->SetRho (r);
  for (uint32_t i = 0; i < n; i++)
    {
      Vector v = unitDiscPositionAllocator2->GetNext ();
      positionAlloc->Add (v);
      positionOutFile << v.x << ", " << v.y << std::endl;
    }
  positionOutFile << std::endl;

  positionOutFile.close ();

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (allNodes);

  // Packet Socket for lower layer
  PacketSocketHelper packetSocket;
  packetSocket.Install (allNodes);

  //uint32_t nNodes = allNodes.GetN ();
  // ApplicationContainer apps;
  //  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

  double perNodeUplinkMbps = aggregateUplinkMbps / n;
  double perNodeDownlinkMbps = aggregateDownlinkMbps / n;
  Time intervalUplink = MicroSeconds (payloadSize * 8 / perNodeUplinkMbps);
  Time intervalDownlink = MicroSeconds (payloadSize * 8 / perNodeDownlinkMbps);
  std::cout << "Uplink interval:" << intervalUplink << " Downlink interval:" << intervalDownlink << std::endl;

  Ptr<UniformRandomVariable> urv = CreateObject<UniformRandomVariable> ();
  urv->SetAttribute ("Min", DoubleValue (-5.0));
  urv->SetAttribute ("Max", DoubleValue (5.0));
  double next_rng = 0;

  if (payloadSize > 0)
    {
      //BSS 1
      // if there is uplilnk traffic from STA to AP, then create server to receive pkts, at the AP
      if (aggregateUplinkMbps)
      {
        Ptr<Node> rxNode = allNodes.Get (0);
        AddServer (apDeviceA.Get (0), staDevicesA.Get (0), rxNode);
      }
      for (uint32_t i = 0; i < n; i++)
        {
          // uplink traffic - each STA to AP
          if (aggregateUplinkMbps > 0)
            {
              // each STA needs a client to generate traffic
              // random offset so that all transmissions do not occur at the same time
              next_rng = urv->GetValue();
              AddClient (apDeviceA.Get (0), staDevicesA.Get (i), allNodes.Get (i + 1), payloadSize, 0, intervalUplink + NanoSeconds(next_rng));
            }
          // downlink traffic - AP to each STA
          if (aggregateDownlinkMbps > 0)
            {
              // for downlink, need to create client at AP to generate traffic to the server at the STA
              // random offset so that all transmissions do not occur at the same time
              next_rng = urv->GetValue();
              AddClient (staDevicesA.Get (i), apDeviceA.Get (0), allNodes.Get (0), payloadSize, 0, intervalDownlink + NanoSeconds(next_rng));
              AddServer (staDevicesA.Get (i), apDeviceA.Get (0), allNodes.Get (i + 1));
            }
        }
    }

  if (payloadSize > 0)
    {
      // BSS 2
      // if there is uplilnk traffic from STA to AP, then create server to receive pkts, at the AP
      if (aggregateUplinkMbps)
      {
        Ptr<Node> rxNode = allNodes.Get (n + 1);
        AddServer (apDeviceB.Get (0), staDevicesB.Get (0), rxNode);
      }
      for (uint32_t i = 0; i < n; i++)
        {
          // uplink traffic - each STA to AP
          if (aggregateUplinkMbps > 0)
            {
              // each STA needs a client to generate traffic
              // random offset so that all transmissions do not occur at the same time
              next_rng = urv->GetValue();
              AddClient (apDeviceB.Get (0), staDevicesB.Get (i), allNodes.Get (n + 1 + i + 1), payloadSize, 0, intervalUplink + NanoSeconds(next_rng));
            }
          // downlink traffic - AP to each STA
          if (aggregateDownlinkMbps > 0)
            {
              next_rng = urv->GetValue();
              // for downlink, need to create client at AP to generate traffic to the server at the STA
              // random offset so that all transmissions do not occur at the same time
              AddClient (staDevicesB.Get (i), apDeviceB.Get (0), allNodes.Get (n + 1), payloadSize, 0, intervalDownlink + NanoSeconds(next_rng));
              AddServer (staDevicesB.Get (i), apDeviceB.Get (0), allNodes.Get (n + 1 + i + 1));
            }
        }
    }

  // Log packet receptions
  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::PacketSocketServer/Rx", MakeCallback (&SocketRecvStats));

  if (enableTracing)
    {
      AsciiTraceHelper ascii;
      spectrumPhy.EnableAsciiAll (ascii.CreateFileStream (outputFilePrefix + ".tr"));
      //spectrumPhy.EnablePcapAll("trycap");
    }

  // This enabling function could be scheduled later in simulation if desired
  SchedulePhyLogConnect ();
  g_stateFile.open (outputFilePrefix + "-state.dat", std::ofstream::out | std::ofstream::trunc);
  g_stateFile.setf (std::ios_base::fixed);
  ScheduleStateLogConnect ();

  // Save attribute configuration
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (outputFilePrefix + ".config"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureAttributes ();

  if (tracing == true)
    {
      spectrumPhy.EnablePcap ("pcapforPackets", staDevicesA);
    }

  Time durationTime = Seconds (duration);
  Simulator::Stop (durationTime);
  Simulator::Run ();

  SchedulePhyLogDisconnect ();
  ScheduleStateLogDisconnect ();
  g_stateFile.flush ();
  g_stateFile.close ();
  SaveSpectrumPhyStats (outputFilePrefix + "-phy-log.dat", g_arrivals);

  Simulator::Destroy ();

  // Save spatial reuse statistics to an output file
  SaveSpatialReuseStats (outputFilePrefix + "-SR-stats.dat", packetsReceived, bytesReceived, duration, d,  r, freq);

  return 0;
}


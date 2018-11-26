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
#include <ns3/ieee-80211ax-indoor-propagation-loss-model.h>
#include <ns3/itu-umi-propagation-loss-model.h>

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

std::vector<std::vector<uint32_t> > packetsReceivedPerNode;
std::vector<std::vector<double> > rssiPerNode;

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
  // std::cout << "Node ID: " << nodeId << " RX addr=" << addr << " size=" << pktSize << std::endl;
  bytesReceived[nodeId] += pktSize;
  packetsReceived[nodeId]++;
}

std::vector<SignalArrival> g_arrivals;
double g_arrivalsDurationCounter = 0;
std::ofstream g_stateFile;
std::ofstream g_TGaxCalibrationTimingsFile;

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

Time lastTxAmpduEnd = Seconds (0);
Time lastRxBlockAckEnd = Seconds (0);
Time lastAmpduDuration = Seconds (0);
Time lastBlockAckDuration = Seconds (-1);
Time lastDeferAndBackoffDuration = Seconds (-1);
Time lastSifsDuration = Seconds (-1);

void WriteCalibrationResult (std::string context, std::string type, Time duration,  Ptr<const Packet> p, const WifiMacHeader &hdr)
{
  Time t_now = Simulator::Now ();
  g_TGaxCalibrationTimingsFile << "---------" << std::endl;
  g_TGaxCalibrationTimingsFile << *p << " " << hdr << std::endl;
  g_TGaxCalibrationTimingsFile << t_now << " " << context << " " << type << " " << duration << std::endl;
}

void TxAmpduCallback (std::string context, Ptr<const Packet> p, const WifiMacHeader &hdr)
{
  Time t_now = Simulator::Now ();
  Time t_duration = hdr.GetDuration ();

  // TGax calibration checkpoint calculations
  Time t_cp1 = t_now;
  Time t_cp2 = t_now + t_duration;

  Time ampduDuration = t_cp2 - t_cp1;  // same as t_duration

  // if (ampduDuration != lastAmpduDuration)
  {
    WriteCalibrationResult (context, "A-MPDU-duration", ampduDuration, p, hdr);
    lastAmpduDuration = ampduDuration;
  }

  lastTxAmpduEnd = t_cp2;

  if (lastRxBlockAckEnd > Seconds (0))
    {
      Time t_cp5 = t_now;
      Time t_cp4 = lastRxBlockAckEnd;

      Time deferAndBackoffDuration = t_cp5 - t_cp4;

      // if (deferAndBackoffDuration != lastDeferAndBackoffDuration)
      {
        WriteCalibrationResult (context, "Defer-and-backoff-duration", deferAndBackoffDuration, p, hdr);
        lastDeferAndBackoffDuration = deferAndBackoffDuration;
      }
    }
}

void RxBlockAckCallback (std::string context, Ptr<const Packet> p, const WifiMacHeader &hdr)
{
  Time t_now = Simulator::Now ();
  Time t_duration = hdr.GetDuration ();

  // TGax calibration checkpoint calculations
  Time t_cp3 = t_now;
  Time t_cp4 = t_now + t_duration;

  Time sifsDuration = t_cp3 - lastTxAmpduEnd;
  // if (sifsDuration != lastSifsDuration)
  {
    // std::cout << "t_cp3 " << t_cp3 << " lastTxAmpduEnd " << lastTxAmpduEnd << std::endl;
    WriteCalibrationResult (context, "Sifs-duration", sifsDuration, p, hdr);
    lastSifsDuration = sifsDuration;
  }

  Time blockAckDuration = t_cp4 - t_cp3;  // same as t_duration

  // if (blockAckDuration != lastBlockAckDuration)
  {
    WriteCalibrationResult (context, "Block-ACK-duration", blockAckDuration, p, hdr);
    lastBlockAckDuration = blockAckDuration;
  }

  lastRxBlockAckEnd = t_cp4;
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

void AddClient (ApplicationContainer &clientAppA, Ipv4Address address, Ptr<Node> node, uint16_t port, Time interval, uint32_t payloadSize)
{
  UdpClientHelper clientA (address, port);
  clientA.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
  clientA.SetAttribute ("Interval", TimeValue (interval)); // s/packet
  clientA.SetAttribute ("PacketSize", UintegerValue (payloadSize));

  clientAppA.Add (clientA.Install (node));
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

void AddServer (UdpServerHelper &serverA, ApplicationContainer &serverAppA, Ptr<Node> node)
{
  serverAppA.Add (serverA.Install (node));
}

std::vector<uint32_t> signals (100);
std::vector<uint32_t> noises (100);

void
SaveSpatialReuseStats (const std::string filename,
                       const std::vector<uint32_t> &packetsReceived,
                       const std::vector<uint32_t> &bytesReceived,
                       const uint32_t nBss,
                       const double duration,
                       const double d,
                       const double r,
                       const int freqHz,
                       const double csr,
                       const std::string scenario)
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

  uint32_t numNodes = packetsReceived.size ();
  uint32_t n = (numNodes / nBss) - 1;

  outFile << "Spatial Reuse Statistics" << std::endl;
  outFile << "Scenario: " << scenario << std::endl;
  outFile << "APs: " << nBss << std::endl;
  outFile << "Nodes per AP: " << n << std::endl;
  outFile << "Distance between APs [m]: " << d << std::endl;
  outFile << "Radius [m]: " << r << std::endl;

  // TODO: debug to print out t-put, can remove
  std::cout << "Scenario: " << scenario << std::endl;

  for (uint32_t bss = 1; bss <= nBss; bss++)
    {

      uint32_t bytesReceivedApUplink = 0.0;
      uint32_t bytesReceivedApDownlink = 0.0;

      uint32_t bssFirstStaIdx = 0;
      bssFirstStaIdx = (n * (bss - 1)) + (nBss);
      uint32_t bssLastStaIdx = 0;
      bssLastStaIdx = bssFirstStaIdx + n;

      // uplink is the received bytes at the AP
      bytesReceivedApUplink += bytesReceived[bss - 1];

      // downlink is the sum of the received bytes for the STAs associated with the AP
      for (uint32_t k = bssFirstStaIdx; k < bssLastStaIdx; k++)
        {
          bytesReceivedApDownlink += bytesReceived[k];
        }

      double tputApUplink = bytesReceivedApUplink * 8 / 1e6 / duration;
      double tputApDownlink = bytesReceivedApDownlink * 8 / 1e6 / duration;

      // TODO: debug to print out t-put, can remove
      std::cout << "Throughput,  AP" << bss << " Uplink   [Mbps] : " << tputApUplink << std::endl;
      std::cout << "Throughput,  AP" << bss << " Downlink [Mbps] : " << tputApDownlink << std::endl;

      outFile << "Throughput,  AP" << bss << " Uplink   [Mbps] : " << tputApUplink << std::endl;
      outFile << "Throughput,  AP" << bss << " Downlink [Mbps] : " << tputApDownlink << std::endl;

      double area = M_PI * r * r;
      outFile << "Area Capacity, AP" << bss << " Uplink   [Mbps/m^2] : " << tputApUplink / area << std::endl;
      outFile << "Area Capacity, AP" << bss << " Downlink [Mbps/m^2] : " << tputApDownlink / area << std::endl;

      outFile << "Spectrum Efficiency, AP" << bss << " Uplink   [Mbps/Hz] : " << tputApUplink / freqHz << std::endl;
      outFile << "Spectrum Efficiency, AP" << bss << " Downlink [Mbps/Hz] : " << tputApDownlink / freqHz << std::endl;
    }

    double rxThroughputPerNode[numNodes];
    // output for all nodes
    for (uint32_t k = 0; k < numNodes; k++)
      {
        double bitsReceived = bytesReceived[k] * 8;
        rxThroughputPerNode[k] = static_cast<double> (bitsReceived) / 1e6 / duration;
        outFile << "Node " << k << ", pkts " << packetsReceived[k] << ", bytes " << bytesReceived[k] << ", throughput [MMb/s] " << rxThroughputPerNode[k] << std::endl;
      }

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

  outFile << "CDF (dBm, signal, noise)" << std::endl;
  uint32_t signals_total = 0;
  uint32_t noises_total = 0;
  for (uint32_t idx = 0; idx < 100; idx++)
    {
      signals_total += signals[idx];
      noises_total += noises[idx];
    }

  uint32_t sum_signal = 0;
  uint32_t sum_noise = 0;
  // for dBm from -100 to -30
  for (uint32_t idx = 0; idx < 71; idx++)
    {
      sum_signal += signals[idx];
      sum_noise += noises[idx];
      outFile << ((double) idx - 100.0) << " " << (double) sum_signal / (double) signals_total << " " << (double) sum_noise / (double) noises_total << std::endl;
    }

  outFile.close ();

  std::cout << "Spatial Reuse Stats written to: " << filename << std::endl;

}

NodeContainer allNodes;

// Find nodeId given a MacAddress
int
MacAddressToNodeId (Mac48Address macAddress)
{
  int nodeId = -1;
  Mac48Address inAddress = macAddress;
  uint32_t nNodes = allNodes.GetN ();
  for (uint32_t n = 0; n < nNodes; n++)
    {
      Mac48Address nodeAddress = Mac48Address::ConvertFrom (allNodes.Get (n)->GetDevice (0)->GetAddress ());
      if (inAddress == nodeAddress)
        {
          nodeId = n;
          break;
        }
    }
  if (nodeId == -1)
    {
      // not found
      //std::cout << "No node with addr " << macAddress << std::endl;
    }
  return nodeId;
}

// Global variables for use in callbacks.
double g_signalDbmAvg;
double g_noiseDbmAvg;
uint32_t g_samples;
std::ofstream g_rxSniffFile;

double g_min_signal = 1000.0;
double g_max_signal = -1000.0;
double g_min_noise = 1000.0;
double g_max_noise = -1000.0;

void ProcessPacket (std::string context,
                    Ptr<const Packet> packet,
                    uint16_t channelFreqMhz,
                    WifiTxVector txVector,
                    MpduInfo aMpdu,
                    SignalNoiseDbm signalNoise)
{
  uint32_t rxNodeId = ContextToNodeId (context);
  int dstNodeId = -1;
  int srcNodeId = -1;
  Mac48Address addr1;
  Mac48Address addr2;
  if (packet)
    {
      WifiMacHeader hdr;
      packet->PeekHeader (hdr);
      addr1 = hdr.GetAddr1 ();
      addr2 = hdr.GetAddr2 ();
      Mac48Address whatsThis = Mac48Address ("00:00:00:00:00:00");
      if (!addr1.IsBroadcast () && (addr2 != whatsThis))
        {
          dstNodeId = MacAddressToNodeId (addr1);
          srcNodeId = MacAddressToNodeId (addr2);
          // std::cout << "RX " << rxNodeId << " dst " << dstNodeId << " context " << context << " addr1 " << addr1 << " addr2 " << addr2 << std::endl;

          g_samples++;
          g_signalDbmAvg += ((signalNoise.signal - g_signalDbmAvg) / g_samples);
          g_noiseDbmAvg += ((signalNoise.noise - g_noiseDbmAvg) / g_samples);
          Address rxNodeAddress = allNodes.Get (rxNodeId)->GetDevice (0)->GetAddress ();
          uint32_t pktSize = packet->GetSize ();
          if (pktSize >= 1500)
            {
              // std::cout << context << " " << rxNodeId << ", " << dstNodeId << ", " << srcNodeId << ", " << rxNodeAddress << ", " << addr1 << ", " << addr2 << ", " << signalNoise.noise << ", " << signalNoise.signal << ", " << pktSize << std::endl;
            }
          g_rxSniffFile << rxNodeId << ", " << dstNodeId << ", " << srcNodeId << ", " << rxNodeAddress << ", " << addr1 << ", " << addr2 << ", " << signalNoise.noise << ", " << signalNoise.signal << ", " << pktSize << std::endl;
          if (signalNoise.signal < g_min_signal)
            {
              g_min_signal = signalNoise.signal;
            }
          if (signalNoise.signal > g_max_signal)
            {
              g_max_signal = signalNoise.signal;
            }
          if (signalNoise.noise < g_min_noise)
            {
              g_min_noise = signalNoise.noise;
            }
          if (signalNoise.noise > g_max_noise)
            {
              g_max_noise = signalNoise.noise;
            }
          // std::cout << "sigbal min " << g_min_signal << " max " << g_max_signal << " Noise min " << g_min_noise << " max " << g_max_noise << std::endl;
          uint32_t idx = floor (signalNoise.signal) + 100;
          signals[idx]++;
          idx = floor (signalNoise.noise) + 100;
          noises[idx]++;
        }
    }
}

void MonitorSniffRx (std::string context,
                     Ptr<const Packet> packet,
                     uint16_t channelFreqMhz,
                     WifiTxVector txVector,
                     MpduInfo aMpdu,
                     SignalNoiseDbm signalNoise)
{
  if (packet)
    {
      Ptr <Packet> packetCopy = new Packet (*packet);
      AmpduTag ampdu;
      if (packetCopy->RemovePacketTag (ampdu))
        {
          // A-MPDU frame
          MpduAggregator::DeaggregatedMpdus packets = MpduAggregator::Deaggregate (packetCopy);
          for (MpduAggregator::DeaggregatedMpdusCI n = packets.begin (); n != packets.end (); ++n)
            {
              std::pair<Ptr<Packet>, AmpduSubframeHeader> deAggPair = (std::pair<Ptr<Packet>, AmpduSubframeHeader>) * n;
              Ptr<Packet> aggregatedPacket = deAggPair.first;
              ProcessPacket (context,
                             aggregatedPacket,
                             channelFreqMhz,
                             txVector,
                             aMpdu,
                             signalNoise);
            }
        }
      else
        {
          ProcessPacket (context,
                         packet,
                         channelFreqMhz,
                         txVector,
                         aMpdu,
                         signalNoise);
        }
    }
}

// main script
int
main (int argc, char *argv[])
{
  Config::SetDefault ("ns3::RegularWifiMac::VO_BlockAckThreshold", UintegerValue (0));
  Config::SetDefault ("ns3::RegularWifiMac::VI_BlockAckThreshold", UintegerValue (0));
  Config::SetDefault ("ns3::RegularWifiMac::BE_BlockAckThreshold", UintegerValue (0));
  Config::SetDefault ("ns3::RegularWifiMac::BK_BlockAckThreshold", UintegerValue (0));

  int maxSlrc = 7;

  // command line configurable parameters
  bool enablePcap = false;
  bool enableAscii = false;
  double duration = 20.0; // seconds
  double powSta = 10.0; // dBm
  double powAp = 21.0; // dBm
  double ccaTrSta = -102; // dBm
  double ccaTrAp = -82; // dBm
  uint32_t nBss = 2; // number of BSSs.  Can be 1 or 2 (default)
  double d = 100.0; // distance between AP1 and AP2, m
  uint32_t n = 1; // number of STAs to scatter around each AP;
  double r = 50.0; // radius of circle around each AP in which to scatter the STAs
  double aggregateUplinkMbps = 1.0;
  double aggregateDownlinkMbps = 1.0;
  bool enableRts = 0;
  double txRange = 54.0; // [m]
  int bw = 20;
  std::string standard ("11ax_5GHZ");
  double csr = 1000.0; // carrier sense range
  double txStartOffset = 5.0; // [ns]
  double obssPdThreshold = -99.0;
  double obssPdThresholdMin = -82.0;
  double obssPdThresholdMax = -62.0;
  double txGain = 0.0; // dBi
  double rxGain = 0.0; // dBi
  uint32_t antennas = 1;
  uint32_t maxSupportedTxSpatialStreams = 1;
  uint32_t maxSupportedRxSpatialStreams = 1;
  uint32_t performTgaxTimingChecks = 0;
  // the scenario - should be one of: residential, enterprise, indoor, or outdoor
  std::string scenario ("residential");

  // local variables
  std::string outputFilePrefix = "spatial-reuse";
  uint32_t payloadSizeUplink = 1500; // bytes
  uint32_t payloadSizeDownlink = 300; // bytes
  uint32_t mcs = 0; // MCS value
  Time interval = MicroSeconds (1000);
  bool enableObssPd = false;
  uint32_t maxAmpduSize = 65535;
  // uint32_t maxAmsduSize = 7935;
  std::string nodePositionsFile ("");
  // brief delay (s) for the network to settle into a steady state before applications start sending packets
  double applicationTxStart = 1.0;

  CommandLine cmd;
  cmd.AddValue ("duration", "Duration of simulation (s)", duration);
  cmd.AddValue ("powSta", "Power of STA (dBm)", powSta);
  cmd.AddValue ("powAp", "Power of AP (dBm)", powAp);
  cmd.AddValue ("txGain", "Transmission gain (dB)", txGain);
  cmd.AddValue ("rxGain", "Reception gain (dB)", rxGain);
  cmd.AddValue ("antennas", "The number of antennas on each device.", antennas);
  cmd.AddValue ("maxSupportedTxSpatialStreams", "The maximum number of supported Tx spatial streams.", maxSupportedTxSpatialStreams);
  cmd.AddValue ("maxSupportedRxSpatialStreams", "The maximum number of supported Rx spatial streams.", maxSupportedRxSpatialStreams);
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
  cmd.AddValue ("csr", "Carrier Sense Range (CSR) [m]", csr);
  cmd.AddValue ("enableRts", "Enable or disable RTS/CTS", enableRts);
  cmd.AddValue ("maxSlrc", "MaxSlrc", maxSlrc);
  cmd.AddValue ("txRange", "Max TX range [m]", txRange);
  cmd.AddValue ("payloadSizeUplink", "Payload size of 1 uplink packet [bytes]", payloadSizeUplink);
  cmd.AddValue ("payloadSizeDownlink", "Payload size of 1 downlink packet [bytes]", payloadSizeDownlink);
  cmd.AddValue ("MCS", "Modulation and Coding Scheme (MCS) index (default=0)", mcs);
  cmd.AddValue ("txStartOffset", "N(0, mu) offset for each node's start of packet transmission.  Default mu=5 [ns]", txStartOffset);
  cmd.AddValue ("obssPdThreshold", "Energy threshold (dBm) of received signal below which the PHY layer can avoid declaring CCA BUSY for inter-BSS frames.", obssPdThreshold);
  cmd.AddValue ("obssPdThresholdMin", "Minimum value (dBm) of OBSS_PD threshold.", obssPdThresholdMin);
  cmd.AddValue ("checkTimings", "Perform TGax timings checks (for MAC simulation calibrations).", performTgaxTimingChecks);
  cmd.AddValue ("scenario", "The spatial-reuse scenario (residential, enterprise, indoor, outdoor, study1).", scenario);
  cmd.AddValue ("nBss", "The number of BSSs.  Can be either 1 or 2 (default).", nBss);
  cmd.AddValue ("maxAmpduSize", "The maximum A-MPDU size (bytes).", maxAmpduSize);
  cmd.AddValue ("nodePositionsFile", "Node positions file, ns-2 format for Ns2MobilityHelper.", nodePositionsFile);
  cmd.AddValue ("enablePcap", "Enable PCAP trace file generation.", enablePcap);
  cmd.AddValue ("enableAscii", "Enable ASCII trace file generation.", enableAscii);
  cmd.Parse (argc, argv);

  if (scenario == "study1")
    {
      nBss = 7;
    }

  if ((nBss < 1) || (nBss > 7))
    {
      std::cout << "Invalid nBss parameter: " << nBss << ".  Can only be 1, 2, 3 or 4." << std::endl;
    }

  if (enableRts)
    {
      Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("0"));
    }

  if (maxSlrc < 0)
    {
      maxSlrc = std::numeric_limits<uint32_t>::max ();
    }
  Config::SetDefault ("ns3::WifiRemoteStationManager::MaxSlrc", UintegerValue (maxSlrc));

  std::ostringstream ossMcs;
  ossMcs << mcs;

  // carrier sense range (csr) is a calculated value that is used for displaying the
  // estimated range in which an AP can successfully receive from STAs.
  // first, let us calculate the max distance, txRange, that a transmitting STA's signal
  // can be received above the CcaMode1Threshold.
  // for simple calculation, we assume Friis propagation loss, CcaMode1Threshold = -102dBm,
  // freq = 5.9GHz, no antenna gains, txPower = 10 dBm. the resulting value is:
  // calculation of Carrier Sense Range (CSR).
  // see: https://onlinelibrary.wiley.com/doi/pdf/10.1002/wcm.264
  // In (8), the optimum CSR = D x S0 ^ (1 / gamma)
  // S0 (the  min. SNR value for decoding a particular rate MCS) is calculated for several MCS values.
  // For reference, please see the  802.11ax Evaluation Methodology document (Appendix 3).
  // https://mentor.ieee.org/802.11/dcn/14/11-14-0571-12-00ax-evaluation-methodology.do
  //For the following conditions:
  // AWGN Channel
  // BCC with 1482 bytes Packet Length
  // PER=0.1
  // the minimum SINR values (S0) are
  //
  // [MCS S0 ]
  // [0 0.7dB]
  // [1 3.7dB]
  // [2  6.2dB]
  // [3 9.3dB]
  // [4 12.6dB]
  // [5 16.8dB]
  // [6 18.2dB]
  // [7 19.4dB]
  // [8 23.5dB]
  // [9 25.1dB]
  // Caclculating CSR for MCS0, assuming gamma = 3, we get
  double s0 = 0.7;
  double gamma = 3.0;
  csr = txRange * pow (s0, 1.0 / gamma);
  // std::cout << "S0 " << s0 << " gamma " << gamma << " txRange " << txRange << " csr " << csr << std::endl;

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
      dataRate = "HtMcs" + ossMcs.str ();
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
      dataRate = "HtMcs" + ossMcs.str ();
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
      dataRate = "VhtMcs" + ossMcs.str ();
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
      dataRate = "HeMcs" + ossMcs.str ();
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
      dataRate = "HeMcs" + ossMcs.str ();
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

  // disable ObssPd if not 11ax
  if ((standard != "11ax_2_4GHZ") && (standard != "11ax_5GHZ"))
    {
      enableObssPd = false;
    }

  // total expected nodes.  n STAs for each AP
  uint32_t numNodes = nBss * (n + 1);
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
  Ptr<Node> ap2 = 0;
  Ptr<Node> ap3 = 0;
  Ptr<Node> ap4 = 0;
  Ptr<Node> ap5 = 0;
  Ptr<Node> ap6 = 0;
  Ptr<Node> ap7 = 0;
  // node containers for two APs and their STAs
  NodeContainer stasA, nodesA;
  NodeContainer stasB, nodesB;
  NodeContainer stasC, nodesC;
  NodeContainer stasD, nodesD;
  NodeContainer stasE, nodesE;
  NodeContainer stasF, nodesF;
  NodeContainer stasG, nodesG;

  // network "A"
  for (uint32_t i = 0; i < n; i++)
    {
      Ptr<Node> sta = CreateObject<Node> ();
      stasA.Add (sta);
    }

  // AP at front of node container, then STAs
  nodesA.Add (ap1);
  nodesA.Add (stasA);

  if ((nBss >= 2) || (scenario == "study1"))
    {
      ap2 = CreateObject<Node> ();
      // network "B"
      for (uint32_t i = 0; i < n; i++)
        {
          Ptr<Node> sta = CreateObject<Node> ();
          stasB.Add (sta);
        }

      // AP at front of node container, then STAs
      nodesB.Add (ap2);
      nodesB.Add (stasB);
    }

  if ((nBss >= 3) || (scenario == "study1"))
    {
      ap3 = CreateObject<Node> ();
      // network "C"
      for (uint32_t i = 0; i < n; i++)
        {
          Ptr<Node> sta = CreateObject<Node> ();
          stasC.Add (sta);
        }

      // AP at front of node container, then STAs
      nodesC.Add (ap3);
      nodesC.Add (stasC);
    }

  if ((nBss >= 4) || (scenario == "study1"))
    {
      ap4 = CreateObject<Node> ();
      // network "D"
      for (uint32_t i = 0; i < n; i++)
        {
          Ptr<Node> sta = CreateObject<Node> ();
          stasD.Add (sta);
        }

      // AP at front of node container, then STAs
      nodesD.Add (ap4);
      nodesD.Add (stasD);
    }

  if (scenario == "study1")
    {
      ap5 = CreateObject<Node> ();
      // network "E"
      for (uint32_t i = 0; i < n; i++)
        {
          Ptr<Node> sta = CreateObject<Node> ();
          stasE.Add (sta);
        }

      // AP at front of node container, then STAs
      nodesE.Add (ap5);
      nodesE.Add (stasE);

      ap6 = CreateObject<Node> ();
      // network "F"
      for (uint32_t i = 0; i < n; i++)
        {
          Ptr<Node> sta = CreateObject<Node> ();
          stasF.Add (sta);
        }

      // AP at front of node container, then STAs
      nodesF.Add (ap6);
      nodesF.Add (stasF);

      ap7 = CreateObject<Node> ();
      // network "G"
      for (uint32_t i = 0; i < n; i++)
        {
          Ptr<Node> sta = CreateObject<Node> ();
          stasG.Add (sta);
        }

      // AP at front of node container, then STAs
      nodesG.Add (ap7);
      nodesG.Add (stasG);
    }

  // the container for all nodes (from Network "A" and Network "B")
  allNodes = NodeContainer (nodesA, nodesB, nodesC, nodesD);
  allNodes.Add(nodesE);
  allNodes.Add(nodesF);
  allNodes.Add(nodesG);

  // PHY setup
  SpectrumWifiPhyHelper spectrumPhy = SpectrumWifiPhyHelper::Default ();
  Ptr<MultiModelSpectrumChannel> spectrumChannel
    = CreateObject<MultiModelSpectrumChannel> ();
  // path loss model uses one of the 802.11ax path loss models
  // described in the TGax simulations scenarios.
  // currently using just the IndoorPropagationLossModel, which
  // appears suitable for Test2 - Enterprise.
  // additional code tweaks needed for Test 1 and Test 3,
  // handling of 'W=1 wall' and using the ItuUmitPropagationLossModel
  // for Test 4.
  if (scenario == "residential")
    {
      Config::SetDefault ("ns3::Ieee80211axIndoorPropagationLossModel::DistanceBreakpoint", DoubleValue (5.0));
      Config::SetDefault ("ns3::Ieee80211axIndoorPropagationLossModel::Walls", DoubleValue (1.0));
      Config::SetDefault ("ns3::Ieee80211axIndoorPropagationLossModel::WallsFactor", DoubleValue (5.0));

      Ptr<Ieee80211axIndoorPropagationLossModel> lossModel = CreateObject<Ieee80211axIndoorPropagationLossModel> ();
      spectrumChannel->AddPropagationLossModel (lossModel);
    }
  else if (scenario == "enterprise")
    {
      Config::SetDefault ("ns3::Ieee80211axIndoorPropagationLossModel::DistanceBreakpoint", DoubleValue (10.0));
      Config::SetDefault ("ns3::Ieee80211axIndoorPropagationLossModel::Walls", DoubleValue (1.0));
      Config::SetDefault ("ns3::Ieee80211axIndoorPropagationLossModel::WallsFactor", DoubleValue (7.0));

      Ptr<Ieee80211axIndoorPropagationLossModel> lossModel = CreateObject<Ieee80211axIndoorPropagationLossModel> ();
      spectrumChannel->AddPropagationLossModel (lossModel);
    }
  else if ((scenario == "indoor") || (scenario == "study1"))
    {
      Config::SetDefault ("ns3::Ieee80211axIndoorPropagationLossModel::DistanceBreakpoint", DoubleValue (10.0));
      Config::SetDefault ("ns3::Ieee80211axIndoorPropagationLossModel::Walls", DoubleValue (0.0));
      Config::SetDefault ("ns3::Ieee80211axIndoorPropagationLossModel::WallsFactor", DoubleValue (0.0));

      Ptr<Ieee80211axIndoorPropagationLossModel> lossModel = CreateObject<Ieee80211axIndoorPropagationLossModel> ();
      spectrumChannel->AddPropagationLossModel (lossModel);
    }
  else if (scenario == "outdoor")
    {
      Ptr<ItuUmiPropagationLossModel> lossModel = CreateObject<ItuUmiPropagationLossModel> ();
      spectrumChannel->AddPropagationLossModel (lossModel);
    }
  else
    {
      std::cout << "Unknown scenario: " << scenario << ". Must be one of:  residential, enterprise, indoor, outdoor." << std::endl;
      return 1;
    }

  Ptr<ConstantSpeedPropagationDelayModel> delayModel
    = CreateObject<ConstantSpeedPropagationDelayModel> ();
  spectrumChannel->SetPropagationDelayModel (delayModel);

  spectrumPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

  spectrumPhy.SetChannel (spectrumChannel);
  spectrumPhy.SetErrorRateModel ("ns3::YansErrorRateModel");
  spectrumPhy.Set ("Frequency", UintegerValue (freq)); // channel 36 at 20 MHz
  spectrumPhy.Set ("ChannelWidth", UintegerValue (bw));

  // WiFi setup / helpers
  WifiMacHelper mac;

  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue (dataRate),
                                "ControlMode", StringValue (dataRate));

  // Set PHY power and CCA threshold for STAs
  spectrumPhy.Set ("TxPowerStart", DoubleValue (powSta));
  spectrumPhy.Set ("TxPowerEnd", DoubleValue (powSta));
  spectrumPhy.Set ("TxGain", DoubleValue (txGain));
  spectrumPhy.Set ("RxGain", DoubleValue (rxGain));
  spectrumPhy.Set ("Antennas", UintegerValue (antennas));
  spectrumPhy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (maxSupportedTxSpatialStreams));
  spectrumPhy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (maxSupportedRxSpatialStreams));
  spectrumPhy.Set ("CcaMode1Threshold", DoubleValue (ccaTrSta));
  //PHY energy threshold -92 dBm
  spectrumPhy.Set ("EnergyDetectionThreshold", DoubleValue (-92.0));

  // Network "A"
  Ssid ssidA = Ssid ("A");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssidA),
               "BE_MaxAmpduSize", UintegerValue (maxAmpduSize));
  // Do we also want to allow Amsdu Size to be modified?
  //              "BE_MaxAmsduSize", UintegerValue(maxAmsduSize));
  NetDeviceContainer staDevicesA;
  staDevicesA = wifi.Install (spectrumPhy, mac, stasA);

  // Set  PHY power and CCA threshold for APs
  spectrumPhy.Set ("TxPowerStart", DoubleValue (powAp));
  spectrumPhy.Set ("TxPowerEnd", DoubleValue (powAp));
  spectrumPhy.Set ("TxGain", DoubleValue (txGain));
  spectrumPhy.Set ("RxGain", DoubleValue (rxGain));
  spectrumPhy.Set ("Antennas", UintegerValue (antennas));
  spectrumPhy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (maxSupportedTxSpatialStreams));
  spectrumPhy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (maxSupportedRxSpatialStreams));
  spectrumPhy.Set ("CcaMode1Threshold", DoubleValue (ccaTrAp));
  spectrumPhy.Set ("EnergyDetectionThreshold", DoubleValue (-92.0));
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssidA),
               "BE_MaxAmpduSize", UintegerValue (maxAmpduSize));
  // Do we also want to allow Amsdu Size to be modified?
  //              "BE_MaxAmsduSize", UintegerValue(maxAmsduSize));

  // AP1
  NetDeviceContainer apDeviceA;
  apDeviceA = wifi.Install (spectrumPhy, mac, ap1);
  Ptr<WifiNetDevice> apDevice = apDeviceA.Get (0)->GetObject<WifiNetDevice> ();
  Ptr<ApWifiMac> apWifiMac = apDevice->GetMac ()->GetObject<ApWifiMac> ();
  // The below statements may be simplified in a future HeConfigurationHelper
  if ((enableObssPd))
    {
      Ptr<HeConfiguration> heConfiguration = apDevice->GetHeConfiguration ();
      heConfiguration->SetAttribute ("BssColor", UintegerValue (1));
      heConfiguration->SetAttribute ("ObssPdThreshold", DoubleValue (obssPdThreshold));
      heConfiguration->SetAttribute ("ObssPdThresholdMin", DoubleValue (obssPdThresholdMin));
      heConfiguration->SetAttribute ("ObssPdThresholdMax", DoubleValue (obssPdThresholdMax));
    }

  NetDeviceContainer apDeviceB;
  NetDeviceContainer staDevicesB;
  if ((nBss >= 2) || (scenario == "study1"))
    {
      // Set PHY power and CCA threshold for STAs
      spectrumPhy.Set ("TxPowerStart", DoubleValue (powSta));
      spectrumPhy.Set ("TxPowerEnd", DoubleValue (powSta));
      spectrumPhy.Set ("TxGain", DoubleValue (txGain));
      spectrumPhy.Set ("RxGain", DoubleValue (rxGain));
      spectrumPhy.Set ("Antennas", UintegerValue (antennas));
      spectrumPhy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (maxSupportedTxSpatialStreams));
      spectrumPhy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (maxSupportedRxSpatialStreams));
      spectrumPhy.Set ("CcaMode1Threshold", DoubleValue (ccaTrSta));
      spectrumPhy.Set ("EnergyDetectionThreshold", DoubleValue (-92.0));

      // Network "B"
      Ssid ssidB = Ssid ("B");
      mac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssidB),
                   "BE_MaxAmpduSize", UintegerValue (maxAmpduSize));
      // Do we also want to allow Amsdu Size to be modified?
      //              "BE_MaxAmsduSize", UintegerValue(maxAmsduSize));
      staDevicesB = wifi.Install (spectrumPhy, mac, stasB);

      // Set PHY power and CCA threshold for APs
      spectrumPhy.Set ("TxPowerStart", DoubleValue (powAp));
      spectrumPhy.Set ("TxPowerEnd", DoubleValue (powAp));
      spectrumPhy.Set ("TxGain", DoubleValue (txGain));
      spectrumPhy.Set ("RxGain", DoubleValue (rxGain));
      spectrumPhy.Set ("Antennas", UintegerValue (antennas));
      spectrumPhy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (maxSupportedTxSpatialStreams));
      spectrumPhy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (maxSupportedRxSpatialStreams));
      spectrumPhy.Set ("CcaMode1Threshold", DoubleValue (ccaTrAp));
      spectrumPhy.Set ("EnergyDetectionThreshold", DoubleValue (-92.0));
      mac.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssidB),
                   "BE_MaxAmpduSize", UintegerValue (maxAmpduSize));
      // Do we also want to allow Amsdu Size to be modified?
      //              "BE_MaxAmsduSize", UintegerValue(maxAmsduSize));

      // AP2
      apDeviceB = wifi.Install (spectrumPhy, mac, ap2);
      Ptr<WifiNetDevice> ap2Device = apDeviceB.Get (0)->GetObject<WifiNetDevice> ();
      apWifiMac = ap2Device->GetMac ()->GetObject<ApWifiMac> ();
      if (enableObssPd)
        {
          Ptr <HeConfiguration> heConfiguration = ap2Device->GetHeConfiguration ();
          heConfiguration->SetAttribute ("BssColor", UintegerValue (2));
          heConfiguration->SetAttribute ("ObssPdThreshold", DoubleValue (obssPdThreshold));
          heConfiguration->SetAttribute ("ObssPdThresholdMin", DoubleValue (obssPdThresholdMin));
          heConfiguration->SetAttribute ("ObssPdThresholdMax", DoubleValue (obssPdThresholdMax));
        }
    }

  NetDeviceContainer apDeviceC;
  NetDeviceContainer staDevicesC;
  if ((nBss >= 3) || (scenario == "study1"))
    {
      // Set PHY power and CCA threshold for STAs
      spectrumPhy.Set ("TxPowerStart", DoubleValue (powSta));
      spectrumPhy.Set ("TxPowerEnd", DoubleValue (powSta));
      spectrumPhy.Set ("TxGain", DoubleValue (txGain));
      spectrumPhy.Set ("RxGain", DoubleValue (rxGain));
      spectrumPhy.Set ("Antennas", UintegerValue (antennas));
      spectrumPhy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (maxSupportedTxSpatialStreams));
      spectrumPhy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (maxSupportedRxSpatialStreams));
      spectrumPhy.Set ("CcaMode1Threshold", DoubleValue (ccaTrSta));
      spectrumPhy.Set ("EnergyDetectionThreshold", DoubleValue (-92.0));

      // Network "C"
      Ssid ssidC = Ssid ("C");
      mac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssidC),
                   "BE_MaxAmpduSize", UintegerValue(maxAmpduSize));
      // Do we also want to allow Amsdu Size to be modified?
      //              "BE_MaxAmsduSize", UintegerValue(maxAmsduSize));
      staDevicesC = wifi.Install (spectrumPhy, mac, stasC);

      // Set PHY power and CCA threshold for APs
      spectrumPhy.Set ("TxPowerStart", DoubleValue (powAp));
      spectrumPhy.Set ("TxPowerEnd", DoubleValue (powAp));
      spectrumPhy.Set ("TxGain", DoubleValue (txGain));
      spectrumPhy.Set ("RxGain", DoubleValue (rxGain));
      spectrumPhy.Set ("Antennas", UintegerValue (antennas));
      spectrumPhy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (maxSupportedTxSpatialStreams));
      spectrumPhy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (maxSupportedRxSpatialStreams));
      spectrumPhy.Set ("CcaMode1Threshold", DoubleValue (ccaTrAp));
      spectrumPhy.Set ("EnergyDetectionThreshold", DoubleValue (-92.0));
      mac.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssidC),
                   "BE_MaxAmpduSize", UintegerValue(maxAmpduSize));
      // Do we also want to allow Amsdu Size to be modified?
      //              "BE_MaxAmsduSize", UintegerValue(maxAmsduSize));

      // AP3
      apDeviceC = wifi.Install (spectrumPhy, mac, ap3);
      Ptr<WifiNetDevice> ap3Device = apDeviceC.Get (0)->GetObject<WifiNetDevice> ();
      apWifiMac = ap3Device->GetMac ()->GetObject<ApWifiMac> ();
      if (enableObssPd)
        {
          Ptr <HeConfiguration> heConfiguration = ap3Device->GetHeConfiguration ();
          heConfiguration->SetAttribute ("BssColor", UintegerValue (3));
          heConfiguration->SetAttribute ("ObssPdThreshold", DoubleValue(obssPdThreshold));
          heConfiguration->SetAttribute ("ObssPdThresholdMin", DoubleValue(obssPdThresholdMin));
          heConfiguration->SetAttribute ("ObssPdThresholdMax", DoubleValue(obssPdThresholdMax));
        }
    }

  NetDeviceContainer apDeviceD;
  NetDeviceContainer staDevicesD;
  if ((nBss >= 4) || (scenario == "study1"))
    {
      // Set PHY power and CCA threshold for STAs
      spectrumPhy.Set ("TxPowerStart", DoubleValue (powSta));
      spectrumPhy.Set ("TxPowerEnd", DoubleValue (powSta));
      spectrumPhy.Set ("TxGain", DoubleValue (txGain));
      spectrumPhy.Set ("RxGain", DoubleValue (rxGain));
      spectrumPhy.Set ("Antennas", UintegerValue (antennas));
      spectrumPhy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (maxSupportedTxSpatialStreams));
      spectrumPhy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (maxSupportedRxSpatialStreams));
      spectrumPhy.Set ("CcaMode1Threshold", DoubleValue (ccaTrSta));
      spectrumPhy.Set ("EnergyDetectionThreshold", DoubleValue (-92.0));

      // Network "D"
      Ssid ssidD = Ssid ("D");
      mac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssidD),
                   "BE_MaxAmpduSize", UintegerValue(maxAmpduSize));
      // Do we also want to allow Amsdu Size to be modified?
      //              "BE_MaxAmsduSize", UintegerValue(maxAmsduSize));
      staDevicesD = wifi.Install (spectrumPhy, mac, stasD);

      // Set PHY power and CCA threshold for APs
      spectrumPhy.Set ("TxPowerStart", DoubleValue (powAp));
      spectrumPhy.Set ("TxPowerEnd", DoubleValue (powAp));
      spectrumPhy.Set ("TxGain", DoubleValue (txGain));
      spectrumPhy.Set ("RxGain", DoubleValue (rxGain));
      spectrumPhy.Set ("Antennas", UintegerValue (antennas));
      spectrumPhy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (maxSupportedTxSpatialStreams));
      spectrumPhy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (maxSupportedRxSpatialStreams));
      spectrumPhy.Set ("CcaMode1Threshold", DoubleValue (ccaTrAp));
      spectrumPhy.Set ("EnergyDetectionThreshold", DoubleValue (-92.0));
      mac.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssidD),
                   "BE_MaxAmpduSize", UintegerValue(maxAmpduSize));
      // Do we also want to allow Amsdu Size to be modified?
      //              "BE_MaxAmsduSize", UintegerValue(maxAmsduSize));

      // AP4
      apDeviceD = wifi.Install (spectrumPhy, mac, ap4);
      Ptr<WifiNetDevice> ap4Device = apDeviceD.Get (0)->GetObject<WifiNetDevice> ();
      apWifiMac = ap4Device->GetMac ()->GetObject<ApWifiMac> ();
      if (enableObssPd)
        {
          Ptr <HeConfiguration> heConfiguration = ap4Device->GetHeConfiguration ();
          heConfiguration->SetAttribute ("BssColor", UintegerValue (4));
          heConfiguration->SetAttribute ("ObssPdThreshold", DoubleValue(obssPdThreshold));
          heConfiguration->SetAttribute ("ObssPdThresholdMin", DoubleValue(obssPdThresholdMin));
          heConfiguration->SetAttribute ("ObssPdThresholdMax", DoubleValue(obssPdThresholdMax));
        }
    }

  NetDeviceContainer apDeviceE;
  NetDeviceContainer staDevicesE;
  NetDeviceContainer apDeviceF;
  NetDeviceContainer staDevicesF;
  NetDeviceContainer apDeviceG;
  NetDeviceContainer staDevicesG;
  if (scenario == "study1")
    {
      // Set PHY power and CCA threshold for STAs
      spectrumPhy.Set ("TxPowerStart", DoubleValue (powSta));
      spectrumPhy.Set ("TxPowerEnd", DoubleValue (powSta));
      spectrumPhy.Set ("TxGain", DoubleValue (txGain));
      spectrumPhy.Set ("RxGain", DoubleValue (rxGain));
      spectrumPhy.Set ("Antennas", UintegerValue (antennas));
      spectrumPhy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (maxSupportedTxSpatialStreams));
      spectrumPhy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (maxSupportedRxSpatialStreams));
      spectrumPhy.Set ("CcaMode1Threshold", DoubleValue (ccaTrSta));
      spectrumPhy.Set ("EnergyDetectionThreshold", DoubleValue (-92.0));

      // Network "E"
      Ssid ssidE = Ssid ("E");
      mac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssidE),
                   "BE_MaxAmpduSize", UintegerValue(maxAmpduSize));
      // Do we also want to allow Amsdu Size to be modified?
      //              "BE_MaxAmsduSize", UintegerValue(maxAmsduSize));
      staDevicesE = wifi.Install (spectrumPhy, mac, stasE);

      // Set PHY power and CCA threshold for APs
      spectrumPhy.Set ("TxPowerStart", DoubleValue (powAp));
      spectrumPhy.Set ("TxPowerEnd", DoubleValue (powAp));
      spectrumPhy.Set ("TxGain", DoubleValue (txGain));
      spectrumPhy.Set ("RxGain", DoubleValue (rxGain));
      spectrumPhy.Set ("Antennas", UintegerValue (antennas));
      spectrumPhy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (maxSupportedTxSpatialStreams));
      spectrumPhy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (maxSupportedRxSpatialStreams));
      spectrumPhy.Set ("CcaMode1Threshold", DoubleValue (ccaTrAp));
      spectrumPhy.Set ("EnergyDetectionThreshold", DoubleValue (-92.0));
      mac.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssidE),
                   "BE_MaxAmpduSize", UintegerValue(maxAmpduSize));
      // Do we also want to allow Amsdu Size to be modified?
      //              "BE_MaxAmsduSize", UintegerValue(maxAmsduSize));

      // AP5
      apDeviceE = wifi.Install (spectrumPhy, mac, ap5);
      Ptr<WifiNetDevice> ap5Device = apDeviceE.Get (0)->GetObject<WifiNetDevice> ();
      apWifiMac = ap5Device->GetMac ()->GetObject<ApWifiMac> ();
      if (enableObssPd)
        {
          Ptr <HeConfiguration> heConfiguration = ap5Device->GetHeConfiguration ();
          heConfiguration->SetAttribute ("BssColor", UintegerValue (5));
          heConfiguration->SetAttribute ("ObssPdThreshold", DoubleValue(obssPdThreshold));
          heConfiguration->SetAttribute ("ObssPdThresholdMin", DoubleValue(obssPdThresholdMin));
          heConfiguration->SetAttribute ("ObssPdThresholdMax", DoubleValue(obssPdThresholdMax));
        }

      // Set PHY power and CCA threshold for STAs
      spectrumPhy.Set ("TxPowerStart", DoubleValue (powSta));
      spectrumPhy.Set ("TxPowerEnd", DoubleValue (powSta));
      spectrumPhy.Set ("TxGain", DoubleValue (txGain));
      spectrumPhy.Set ("RxGain", DoubleValue (rxGain));
      spectrumPhy.Set ("Antennas", UintegerValue (antennas));
      spectrumPhy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (maxSupportedTxSpatialStreams));
      spectrumPhy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (maxSupportedRxSpatialStreams));
      spectrumPhy.Set ("CcaMode1Threshold", DoubleValue (ccaTrSta));
      spectrumPhy.Set ("EnergyDetectionThreshold", DoubleValue (-92.0));

      // Network "F"
      Ssid ssidF = Ssid ("F");
      mac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssidF),
                   "BE_MaxAmpduSize", UintegerValue(maxAmpduSize));
      // Do we also want to allow Amsdu Size to be modified?
      //              "BE_MaxAmsduSize", UintegerValue(maxAmsduSize));
      staDevicesF = wifi.Install (spectrumPhy, mac, stasF);

      // Set PHY power and CCA threshold for APs
      spectrumPhy.Set ("TxPowerStart", DoubleValue (powAp));
      spectrumPhy.Set ("TxPowerEnd", DoubleValue (powAp));
      spectrumPhy.Set ("TxGain", DoubleValue (txGain));
      spectrumPhy.Set ("RxGain", DoubleValue (rxGain));
      spectrumPhy.Set ("Antennas", UintegerValue (antennas));
      spectrumPhy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (maxSupportedTxSpatialStreams));
      spectrumPhy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (maxSupportedRxSpatialStreams));
      spectrumPhy.Set ("CcaMode1Threshold", DoubleValue (ccaTrAp));
      spectrumPhy.Set ("EnergyDetectionThreshold", DoubleValue (-92.0));
      mac.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssidF),
                   "BE_MaxAmpduSize", UintegerValue(maxAmpduSize));
      // Do we also want to allow Amsdu Size to be modified?
      //              "BE_MaxAmsduSize", UintegerValue(maxAmsduSize));

      // AP6
      apDeviceF = wifi.Install (spectrumPhy, mac, ap6);
      Ptr<WifiNetDevice> ap6Device = apDeviceF.Get (0)->GetObject<WifiNetDevice> ();
      apWifiMac = ap6Device->GetMac ()->GetObject<ApWifiMac> ();
      if (enableObssPd)
        {
          Ptr <HeConfiguration> heConfiguration = ap6Device->GetHeConfiguration ();
          heConfiguration->SetAttribute ("BssColor", UintegerValue (6));
          heConfiguration->SetAttribute ("ObssPdThreshold", DoubleValue(obssPdThreshold));
          heConfiguration->SetAttribute ("ObssPdThresholdMin", DoubleValue(obssPdThresholdMin));
          heConfiguration->SetAttribute ("ObssPdThresholdMax", DoubleValue(obssPdThresholdMax));
        }

      // Network "G"
      Ssid ssidG = Ssid ("G");
      mac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssidG),
                   "BE_MaxAmpduSize", UintegerValue(maxAmpduSize));
      // Do we also want to allow Amsdu Size to be modified?
      //              "BE_MaxAmsduSize", UintegerValue(maxAmsduSize));
      staDevicesG = wifi.Install (spectrumPhy, mac, stasG);

      // Set PHY power and CCA threshold for APs
      spectrumPhy.Set ("TxPowerStart", DoubleValue (powAp));
      spectrumPhy.Set ("TxPowerEnd", DoubleValue (powAp));
      spectrumPhy.Set ("TxGain", DoubleValue (txGain));
      spectrumPhy.Set ("RxGain", DoubleValue (rxGain));
      spectrumPhy.Set ("Antennas", UintegerValue (antennas));
      spectrumPhy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (maxSupportedTxSpatialStreams));
      spectrumPhy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (maxSupportedRxSpatialStreams));
      spectrumPhy.Set ("CcaMode1Threshold", DoubleValue (ccaTrAp));
      spectrumPhy.Set ("EnergyDetectionThreshold", DoubleValue (-92.0));
      mac.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssidG),
                   "BE_MaxAmpduSize", UintegerValue(maxAmpduSize));
      // Do we also want to allow Amsdu Size to be modified?
      //              "BE_MaxAmsduSize", UintegerValue(maxAmsduSize));

      // AP7
      apDeviceG = wifi.Install (spectrumPhy, mac, ap7);
      Ptr<WifiNetDevice> ap7Device = apDeviceG.Get (0)->GetObject<WifiNetDevice> ();
      apWifiMac = ap7Device->GetMac ()->GetObject<ApWifiMac> ();
      if (enableObssPd)
        {
          Ptr <HeConfiguration> heConfiguration = ap7Device->GetHeConfiguration ();
          heConfiguration->SetAttribute ("BssColor", UintegerValue (7));
          heConfiguration->SetAttribute ("ObssPdThreshold", DoubleValue(obssPdThreshold));
          heConfiguration->SetAttribute ("ObssPdThresholdMin", DoubleValue(obssPdThresholdMin));
          heConfiguration->SetAttribute ("ObssPdThresholdMax", DoubleValue(obssPdThresholdMax));
        }
    }

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
  if (scenario != "study1")
    {
      double boundingBoxExtension = 10.0;
      positionOutFile << -(r + boundingBoxExtension) <<      ", " << -d + -r - boundingBoxExtension << std::endl;
      positionOutFile << (d + r + boundingBoxExtension) << ", " << -d + -r - boundingBoxExtension << std::endl;
      positionOutFile << (d + r + boundingBoxExtension) << ", " <<  r + boundingBoxExtension << std::endl;
      positionOutFile << -(r + boundingBoxExtension) <<      ", " <<  r + boundingBoxExtension << std::endl;
    }
  else
    {
      // study1 scenario bounding box
      positionOutFile << -40.0 <<  ", " << -40.0 << std::endl;
      positionOutFile <<  40.0 <<  ", " << -40.0 << std::endl;
      positionOutFile <<  40.0 <<  ", " <<  40.0 << std::endl;
      positionOutFile <<  40.0 <<  ", " << -40.0 << std::endl;
    }
  positionOutFile << std::endl;
  positionOutFile << std::endl;

  // consistent stream to that positional layout is not perturbed by other configuration choices
  int64_t streamNumber = 100;

  if ((nodePositionsFile != "") && (nodePositionsFile != "NONE"))
    {
      std::cout << "Loading node positions from file: " << nodePositionsFile << std::endl;
      // Create Ns2MobilityHelper with the specified trace log file as parameter
      Ns2MobilityHelper ns2 = Ns2MobilityHelper (nodePositionsFile);
      ns2.Install (); // configure movements for each node, while reading trace file

      uint32_t numNodes = nodesA.GetN ();
      for (uint32_t nodeIdx = 0; nodeIdx < numNodes; nodeIdx++)
        {
          Ptr<Node> node = nodesA.Get (nodeIdx);
          Ptr<MobilityModel> mob = node->GetObject<MobilityModel> ();
          Vector position = mob->GetPosition ();
          double x = position.x;
          double y = position.y;

          // nodeIdx == 0 is AP1
          if (nodeIdx == 0)
            {
              // "A" - APs
              positionOutFile << x << ", " << y << ", " << r << ", " << csr << std::endl;
              positionOutFile << std::endl;
              positionOutFile << std::endl;

              // "B" - APs
              // Position is not provided in Ns2Mobility file, but we have to output something
              // here so that the plotting script works.
              positionOutFile << 0.0 << ", " << 0.0 << ", " << r << ", " << csr << std::endl;
              positionOutFile << std::endl;
              positionOutFile << std::endl;
            }
          else
            {
              positionOutFile << x << ", " << y << std::endl;
            }
        }
      // End of nodes for BSS1.  Output blank lines as section separator.
      positionOutFile << std::endl;
      positionOutFile << std::endl;

      // need to output something here to represent the positions section for STAs B
      // since the post-processnig script expects there to be something here.
      positionOutFile << d << ", " << 0 << std::endl;
    }
  else if (scenario == "study1")
    {
      double x1 = 0.0;
      double y1 = 0.0;
      double theta = 0.0;  // radians
      double x2 = d * cos(theta);
      double y2 = d * sin(theta);
      theta += 60.0 / 360.0 * 2.0 * M_PI;
      double x3 = d * cos(theta);
      double y3 = d * sin(theta);
      theta += 60.0 / 360.0 * 2.0 * M_PI;
      double x4 = d * cos(theta);
      double y4 = d * sin(theta);
      theta += 60.0 / 360.0 * 2.0 * M_PI;
      double x5 = d * cos(theta);
      double y5 = d * sin(theta);
      theta += 60.0 / 360.0 * 2.0 * M_PI;
      double x6 = d * cos(theta);
      double y6 = d * sin(theta);
      theta += 60.0 / 360.0 * 2.0 * M_PI;
      double x7 = d * cos(theta);
      double y7 = d * sin(theta);

      // "A" - APs
      positionOutFile << x1 << ", " << y1 << ", " << r << ", " << csr << std::endl;
      positionOutFile << std::endl;
      positionOutFile << std::endl;

      // "B" - APs
      positionOutFile << x2 << ", " << y2 << ", " << r << ", " << csr << std::endl;
      positionOutFile << std::endl;
      positionOutFile << std::endl;

      // "C" - APs
      positionOutFile << x3 << ", " << y3 << ", " << r << ", " << csr << std::endl;
      positionOutFile << std::endl;
      positionOutFile << std::endl;

      // "D" - APs
      positionOutFile << x4 << ", " << y4 << ", " << r << ", " << csr << std::endl;
      positionOutFile << std::endl;
      positionOutFile << std::endl;

      // "E" - APs
      positionOutFile << x5 << ", " << y5 << ", " << r << ", " << csr << std::endl;
      positionOutFile << std::endl;
      positionOutFile << std::endl;

      // "F" - APs
      positionOutFile << x6 << ", " << y6 << ", " << r << ", " << csr << std::endl;
      positionOutFile << std::endl;
      positionOutFile << std::endl;

      // "G" - APs
      positionOutFile << x7 << ", " << y7 << ", " << r << ", " << csr << std::endl;
      positionOutFile << std::endl;
      positionOutFile << std::endl;

      // Network "A"
      // AP1
      positionAlloc->Add (Vector (x1, y1, 0.0));        // AP1
      // STAs for AP1
      // STAs for each AP are allocated uwing a different instance of a UnitDiscPositionAllocation.  To
      // ensure unique randomness of positions,  each allocator must be allocated a different stream number.
      Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator1 = CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator1->AssignStreams (streamNumber);
      // AP1 is at origin (x=x1, y=y1), with radius Rho=r
      unitDiscPositionAllocator1->SetX (x1);
      unitDiscPositionAllocator1->SetY (y1);
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
      positionAlloc->Add (Vector (x2, y2, 0.0));        // AP2
      // STAs for AP2
      // STAs for each AP are allocated uwing a different instance of a UnitDiscPositionAllocation.  To
      // ensure unique randomness of positions,  each allocator must be allocated a different stream number.
      Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator2 = CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator2->AssignStreams (streamNumber + 1);
      // AP1 is at origin (x=x2, y=y2), with radius Rho=r
      unitDiscPositionAllocator2->SetX (x2);
      unitDiscPositionAllocator2->SetY (y2);
      unitDiscPositionAllocator2->SetRho (r);
      for (uint32_t i = 0; i < n; i++)
        {
          Vector v = unitDiscPositionAllocator2->GetNext ();
          positionAlloc->Add (v);
          positionOutFile << v.x << ", " << v.y << std::endl;
        }
      positionOutFile << std::endl;
      positionOutFile << std::endl;

      // Network "C"
      // AP3
      positionAlloc->Add (Vector (x3, y3, 0.0));        // AP3
      // STAs for AP3
      // STAs for each AP are allocated uwing a different instance of a UnitDiscPositionAllocation.  To
      // ensure unique randomness of positions,  each allocator must be allocated a different stream number.
      Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator3 = CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator3->AssignStreams (streamNumber + 2);
      // AP1 is at origin (x=x3, y=y3), with radius Rho=r
      unitDiscPositionAllocator3->SetX (x3);
      unitDiscPositionAllocator3->SetY (y3);
      unitDiscPositionAllocator3->SetRho (r);
      for (uint32_t i = 0; i < n; i++)
        {
          Vector v = unitDiscPositionAllocator3->GetNext ();
          positionAlloc->Add (v);
          positionOutFile << v.x << ", " << v.y << std::endl;
        }
      positionOutFile << std::endl;
      positionOutFile << std::endl;

      // Network "D"
      // AP4
      positionAlloc->Add (Vector (x4, y4, 0.0));        // AP4
      // STAs for AP4
      // STAs for each AP are allocated uwing a different instance of a UnitDiscPositionAllocation.  To
      // ensure unique randomness of positions,  each allocator must be allocated a different stream number.
      Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator4 = CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator4->AssignStreams (streamNumber + 3);
      // AP1 is at origin (x=x4, y=y4), with radius Rho=r
      unitDiscPositionAllocator4->SetX (x4);
      unitDiscPositionAllocator4->SetY (y4);
      unitDiscPositionAllocator4->SetRho (r);
      for (uint32_t i = 0; i < n; i++)
        {
          Vector v = unitDiscPositionAllocator4->GetNext ();
          positionAlloc->Add (v);
          positionOutFile << v.x << ", " << v.y << std::endl;
        }
      positionOutFile << std::endl;
      positionOutFile << std::endl;

      // Network "E"
      // AP5
      positionAlloc->Add (Vector (x5, y5, 0.0));        // AP5
      // STAs for AP5
      // STAs for each AP are allocated uwing a different instance of a UnitDiscPositionAllocation.  To
      // ensure unique randomness of positions,  each allocator must be allocated a different stream number.
      Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator5 = CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator5->AssignStreams (streamNumber + 4);
      // AP1 is at origin (x=x5, y=y5), with radius Rho=r
      unitDiscPositionAllocator5->SetX (x5);
      unitDiscPositionAllocator5->SetY (y5);
      unitDiscPositionAllocator5->SetRho (r);
      for (uint32_t i = 0; i < n; i++)
        {
          Vector v = unitDiscPositionAllocator5->GetNext ();
          positionAlloc->Add (v);
          positionOutFile << v.x << ", " << v.y << std::endl;
        }
      positionOutFile << std::endl;
      positionOutFile << std::endl;

      // Network "F"
      // AP6
      positionAlloc->Add (Vector (x6, y6, 0.0));        // AP6
      // STAs for AP6
      // STAs for each AP are allocated uwing a different instance of a UnitDiscPositionAllocation.  To
      // ensure unique randomness of positions,  each allocator must be allocated a different stream number.
      Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator6 = CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator6->AssignStreams (streamNumber + 5);
      // AP1 is at origin (x=x6, y=y6), with radius Rho=r
      unitDiscPositionAllocator6->SetX (x6);
      unitDiscPositionAllocator6->SetY (y6);
      unitDiscPositionAllocator6->SetRho (r);
      for (uint32_t i = 0; i < n; i++)
        {
          Vector v = unitDiscPositionAllocator6->GetNext ();
          positionAlloc->Add (v);
          positionOutFile << v.x << ", " << v.y << std::endl;
        }
      positionOutFile << std::endl;
      positionOutFile << std::endl;

      // Network "G"
      // AP7
      positionAlloc->Add (Vector (x7, y7, 0.0));        // AP7
      // STAs for AP7
      // STAs for each AP are allocated uwing a different instance of a UnitDiscPositionAllocation.  To
      // ensure unique randomness of positions,  each allocator must be allocated a different stream number.
      Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator7 = CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator7->AssignStreams (streamNumber + 6);
      // AP1 is at origin (x=x7, y=y7), with radius Rho=r
      unitDiscPositionAllocator7->SetX (x7);
      unitDiscPositionAllocator7->SetY (y7);
      unitDiscPositionAllocator7->SetRho (r);
      for (uint32_t i = 0; i < n; i++)
        {
          Vector v = unitDiscPositionAllocator7->GetNext ();
          positionAlloc->Add (v);
          positionOutFile << v.x << ", " << v.y << std::endl;
        }
      positionOutFile << std::endl;
      positionOutFile << std::endl;

      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.SetPositionAllocator (positionAlloc);
      mobility.Install (allNodes);
    }
  else
    {
      // "A" - APs
      positionOutFile << 0.0 << ", " << 0.0 << ", " << r << ", " << csr << std::endl;
      positionOutFile << std::endl;
      positionOutFile << std::endl;

      // "B" - APs
      positionOutFile << d << ", " << 0.0 << ", " << r << ", " << csr << std::endl;
      positionOutFile << std::endl;
      positionOutFile << std::endl;

      // "C" - APs
      positionOutFile << 0.0 << ", " << -d << ", " << r << ", " << csr << std::endl;
      positionOutFile << std::endl;
      positionOutFile << std::endl;

      // "D" - APs
      positionOutFile << d << ", " << -d << ", " << r << ", " << csr << std::endl;
      positionOutFile << std::endl;
      positionOutFile << std::endl;

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

      if (nBss >= 2)
        {
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
        }
      else
        {
          // need to output something here to represent the positions section for STAs B
          // since the post-processnig script expects there to be something here.
          positionOutFile << d << ", " << 0 << std::endl;
        }

      positionOutFile << std::endl;
      positionOutFile << std::endl;

      if (nBss >= 3)
        {
          // Network "C"
          // AP3
          positionAlloc->Add (Vector (0.0, -d, 0.0));        // AP3
          // STAs for AP3
          Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator3 = CreateObject<UniformDiscPositionAllocator> ();
          // see comments above - each allocator must have unique stream number.
          unitDiscPositionAllocator3->AssignStreams (streamNumber + 2);
          // AP3 is at (x=0, y=-d), with radius Rho=r
          unitDiscPositionAllocator3->SetX (0);
          unitDiscPositionAllocator3->SetY (-d);
          unitDiscPositionAllocator3->SetRho (r);
          for (uint32_t i = 0; i < n; i++)
            {
              Vector v = unitDiscPositionAllocator3->GetNext ();
              positionAlloc->Add (v);
              positionOutFile << v.x << ", " << v.y << std::endl;
            }
        }
      else
        {
          // need to output something here to represent the positions section for STAs C
          // since the post-processnig script expects there to be something here.
          positionOutFile << 0 << ", " << -d << std::endl;
        }

      positionOutFile << std::endl;
      positionOutFile << std::endl;

      if (nBss >= 4)
        {
          // Network "D"
          // AP4
          positionAlloc->Add (Vector (d, -d, 0.0));        // AP4
          // STAs for AP4
          Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator4 = CreateObject<UniformDiscPositionAllocator> ();
          // see comments above - each allocator must have unique stream number.
          unitDiscPositionAllocator4->AssignStreams (streamNumber + 3);
          // AP3 is at (x=0, y=-d), with radius Rho=r
          unitDiscPositionAllocator4->SetX (d);
          unitDiscPositionAllocator4->SetY (-d);
          unitDiscPositionAllocator4->SetRho (r);
          for (uint32_t i = 0; i < n; i++)
            {
              Vector v = unitDiscPositionAllocator4->GetNext ();
              positionAlloc->Add (v);
              positionOutFile << v.x << ", " << v.y << std::endl;
            }
        }
      else
        {
          // need to output something here to represent the positions section for STAs B
          // since the post-processnig script expects there to be something here.
          positionOutFile << d << ", " << -d << std::endl;
        }

      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.SetPositionAllocator (positionAlloc);
      mobility.Install (allNodes);
    }

  positionOutFile << std::endl;

  positionOutFile.close ();

  //uint32_t nNodes = allNodes.GetN ();
  // ApplicationContainer apps;
  //  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

  double perNodeUplinkMbps = aggregateUplinkMbps / n;
  double perNodeDownlinkMbps = aggregateDownlinkMbps / n;
  Time intervalUplink = MicroSeconds (payloadSizeUplink * 8 / perNodeUplinkMbps);
  Time intervalDownlink = MicroSeconds (payloadSizeDownlink * 8 / perNodeDownlinkMbps);
  std::cout << "Uplink interval:" << intervalUplink << " Downlink interval:" << intervalDownlink << std::endl;

  Ptr<UniformRandomVariable> urv = CreateObject<UniformRandomVariable> ();
  urv->SetAttribute ("Min", DoubleValue (-txStartOffset));
  urv->SetAttribute ("Max", DoubleValue (txStartOffset));
  double next_rng = 0;

  /* Internet stack */
  InternetStackHelper stack;
  stack.Install (nodesA);
  stack.Install (nodesB);
  stack.Install (nodesC);
  stack.Install (nodesD);
  stack.Install (nodesE);
  stack.Install (nodesF);
  stack.Install (nodesG);

  Ipv4AddressHelper address;
  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer StaInterfaceA;
  StaInterfaceA = address.Assign (staDevicesA);
  Ipv4InterfaceContainer ApInterfaceA;
  ApInterfaceA = address.Assign (apDeviceA);
  Ipv4InterfaceContainer StaInterfaceB;
  StaInterfaceB = address.Assign (staDevicesB);
  Ipv4InterfaceContainer ApInterfaceB;
  ApInterfaceB = address.Assign (apDeviceB);
  Ipv4InterfaceContainer StaInterfaceC;
  StaInterfaceC = address.Assign (staDevicesC);
  Ipv4InterfaceContainer ApInterfaceC;
  ApInterfaceC = address.Assign (apDeviceC);
  Ipv4InterfaceContainer StaInterfaceD;
  StaInterfaceD = address.Assign (staDevicesD);
  Ipv4InterfaceContainer ApInterfaceD;
  ApInterfaceD = address.Assign (apDeviceD);
  // an additional address help to prevent address overflow
  Ipv4AddressHelper address2;
  address2.SetBase ("192.168.2.0", "255.255.255.0");
  Ipv4InterfaceContainer StaInterfaceE;
  StaInterfaceE = address2.Assign (staDevicesE);
  Ipv4InterfaceContainer ApInterfaceE;
  ApInterfaceE = address2.Assign (apDeviceE);
  Ipv4InterfaceContainer StaInterfaceF;
  StaInterfaceF = address2.Assign (staDevicesF);
  Ipv4InterfaceContainer ApInterfaceF;
  ApInterfaceF = address2.Assign (apDeviceF);
  Ipv4InterfaceContainer StaInterfaceG;
  StaInterfaceG = address2.Assign (staDevicesG);
  Ipv4InterfaceContainer ApInterfaceG;
  ApInterfaceG = address2.Assign (apDeviceG);

  /* Setting applications */
  uint16_t uplinkPortA = 9;
  uint16_t downlinkPortA = 10;
  UdpServerHelper uplinkServerA (uplinkPortA);
  UdpServerHelper downlinkServerA (downlinkPortA);
  uint16_t uplinkPortB = 11;
  uint16_t downlinkPortB = 12;
  UdpServerHelper uplinkServerB (uplinkPortB);
  UdpServerHelper downlinkServerB (downlinkPortB);
  uint16_t uplinkPortC = 13;
  uint16_t downlinkPortC = 14;
  UdpServerHelper uplinkServerC (uplinkPortC);
  UdpServerHelper downlinkServerC (downlinkPortC);
  uint16_t uplinkPortD = 15;
  uint16_t downlinkPortD = 16;
  UdpServerHelper uplinkServerD (uplinkPortD);
  UdpServerHelper downlinkServerD (downlinkPortD);
  uint16_t uplinkPortE = 17;
  uint16_t downlinkPortE = 18;
  UdpServerHelper uplinkServerE (uplinkPortE);
  UdpServerHelper downlinkServerE (downlinkPortE);
  uint16_t uplinkPortF = 19;
  uint16_t downlinkPortF = 20;
  UdpServerHelper uplinkServerF (uplinkPortF);
  UdpServerHelper downlinkServerF (downlinkPortF);
  uint16_t uplinkPortG = 21;
  uint16_t downlinkPortG = 22;
  UdpServerHelper uplinkServerG (uplinkPortG);
  UdpServerHelper downlinkServerG (downlinkPortG);

  ApplicationContainer uplinkServerAppA;
  ApplicationContainer downlinkServerAppA;
  ApplicationContainer uplinkClientAppA;
  ApplicationContainer downlinkClientAppA;

  ApplicationContainer uplinkServerAppB;
  ApplicationContainer downlinkServerAppB;
  ApplicationContainer uplinkClientAppB;
  ApplicationContainer downlinkClientAppB;

  ApplicationContainer uplinkServerAppC;
  ApplicationContainer downlinkServerAppC;
  ApplicationContainer uplinkClientAppC;
  ApplicationContainer downlinkClientAppC;

  ApplicationContainer uplinkServerAppD;
  ApplicationContainer downlinkServerAppD;
  ApplicationContainer uplinkClientAppD;
  ApplicationContainer downlinkClientAppD;

  ApplicationContainer uplinkServerAppE;
  ApplicationContainer downlinkServerAppE;
  ApplicationContainer uplinkClientAppE;
  ApplicationContainer downlinkClientAppE;

  ApplicationContainer uplinkServerAppF;
  ApplicationContainer downlinkServerAppF;
  ApplicationContainer uplinkClientAppF;
  ApplicationContainer downlinkClientAppF;

  ApplicationContainer uplinkServerAppG;
  ApplicationContainer downlinkServerAppG;
  ApplicationContainer uplinkClientAppG;
  ApplicationContainer downlinkClientAppG;

  if ((payloadSizeUplink > 0) || (payloadSizeDownlink > 0))
    {
      //BSS 1

      // create one server (receiver) for uplink traffic
      AddServer (uplinkServerA, uplinkServerAppA, ap1);

      for (uint32_t i = 0; i < n; i++)
        {
          if (aggregateUplinkMbps > 0)
            {
              // for downlink, need to create client at AP to generate traffic to the server at the STA
              // random offset so that all transmissions do not occur at the same time
              if (txStartOffset > 0)
                {
                  next_rng = urv->GetValue ();
                }
              AddClient (uplinkClientAppA, ApInterfaceA.GetAddress (0), stasA.Get (i), uplinkPortA, Time (intervalUplink + NanoSeconds (next_rng)), payloadSizeUplink);
            }
          // one server (receiver) for each AP-STA pair
          AddServer (downlinkServerA, downlinkServerAppA, stasA.Get (i));
          if (aggregateDownlinkMbps > 0)
            {
              // each STA needs a client to generate traffic
              // random offset so that all transmissions do not occur at the same time
              if (txStartOffset > 0)
                {
                  next_rng = urv->GetValue ();
                }
              AddClient (downlinkClientAppA, StaInterfaceA.GetAddress (i), ap1, downlinkPortA, Time (intervalDownlink + NanoSeconds (next_rng)), payloadSizeDownlink);
            }
        }
    }

  if (((payloadSizeUplink > 0) || (payloadSizeDownlink > 0)) && ((nBss >= 2) || (scenario == "study1")))
    {
      // BSS 2

      // create one server (receiver) for uplink traffic
      AddServer (uplinkServerB, uplinkServerAppB, ap2);

      for (uint32_t i = 0; i < n; i++)
        {
          if (aggregateUplinkMbps > 0)
            {
              // for downlink, need to create client at AP to generate traffic to the server at the STA
              // random offset so that all transmissions do not occur at the same time
              if (txStartOffset > 0)
                {
                  next_rng = urv->GetValue ();
                }
              AddClient (uplinkClientAppB, ApInterfaceB.GetAddress (0), stasB.Get (i), uplinkPortB, Time (intervalUplink + NanoSeconds (next_rng)), payloadSizeUplink);
            }
          // one server (receiver) for each AP-STA pair
          AddServer (downlinkServerB, downlinkServerAppB, stasB.Get (i));
          if (aggregateDownlinkMbps > 0)
            {
              // each STA needs a client to generate traffic
              // random offset so that all transmissions do not occur at the same time
              if (txStartOffset > 0)
                {
                  next_rng = urv->GetValue ();
                }
              AddClient (downlinkClientAppB, StaInterfaceB.GetAddress (i), ap2, downlinkPortB, Time (intervalDownlink + NanoSeconds (next_rng)), payloadSizeDownlink);
            }
        }
    }

  if (((payloadSizeUplink > 0) || (payloadSizeDownlink > 0)) && ((nBss >= 3) || (scenario == "study1")))
    {
      // BSS 3

      // create one server (receiver) for uplink traffic
      AddServer (uplinkServerC, uplinkServerAppC, ap3);

      for (uint32_t i = 0; i < n; i++)
        {
          if (aggregateUplinkMbps > 0)
            {
              // for downlink, need to create client at AP to generate traffic to the server at the STA
              // random offset so that all transmissions do not occur at the same time
              if (txStartOffset > 0)
                {
                  next_rng = urv->GetValue();
                }
              AddClient (uplinkClientAppC, ApInterfaceC.GetAddress (0), stasC.Get (i), uplinkPortC, Time (intervalUplink + NanoSeconds (next_rng)), payloadSizeUplink);
            }
          // one server (receiver) for each AP-STA pair
          AddServer (downlinkServerC, downlinkServerAppC, stasC.Get (i));
          if (aggregateDownlinkMbps > 0)
            {
              // each STA needs a client to generate traffic
              // random offset so that all transmissions do not occur at the same time
              if (txStartOffset > 0)
                {
                  next_rng = urv->GetValue();
                }
              AddClient (downlinkClientAppC, StaInterfaceC.GetAddress (i), ap3, downlinkPortC, Time (intervalDownlink + NanoSeconds (next_rng)), payloadSizeDownlink);
            }
        }
    }

  if (((payloadSizeUplink > 0) || (payloadSizeDownlink > 0)) && ((nBss >= 4) || (scenario == "study1")))
    {
      // BSS 4

      // create one server (receiver) for uplink traffic
      AddServer (uplinkServerD, uplinkServerAppD, ap4);

      for (uint32_t i = 0; i < n; i++)
        {
          if (aggregateUplinkMbps > 0)
            {
              // for downlink, need to create client at AP to generate traffic to the server at the STA
              // random offset so that all transmissions do not occur at the same time
              if (txStartOffset > 0)
                {
                  next_rng = urv->GetValue();
                }
              AddClient (uplinkClientAppD, ApInterfaceD.GetAddress (0), stasD.Get (i), uplinkPortD, Time (intervalUplink + NanoSeconds (next_rng)), payloadSizeUplink);
            }
          // one server (receiver) for each AP-STA pair
          AddServer (downlinkServerD, downlinkServerAppD, stasD.Get (i));
          if (aggregateDownlinkMbps > 0)
            {
              // each STA needs a client to generate traffic
              // random offset so that all transmissions do not occur at the same time
              if (txStartOffset > 0)
                {
                  next_rng = urv->GetValue();
                }
              AddClient (downlinkClientAppD, StaInterfaceD.GetAddress (i), ap4, downlinkPortD, Time (intervalDownlink + NanoSeconds (next_rng)), payloadSizeDownlink);
            }
        }
    }

  if (scenario == "study1")
    {
      // BSS 5

      // create one server (receiver) for uplink traffic
      AddServer (uplinkServerE, uplinkServerAppE, ap5);

      for (uint32_t i = 0; i < n; i++)
        {
          if (aggregateUplinkMbps > 0)
            {
              // for downlink, need to create client at AP to generate traffic to the server at the STA
              // random offset so that all transmissions do not occur at the same time
              if (txStartOffset > 0)
                {
                  next_rng = urv->GetValue();
                }
              AddClient (uplinkClientAppE, ApInterfaceE.GetAddress (0), stasE.Get (i), uplinkPortE, Time (intervalUplink + NanoSeconds (next_rng)), payloadSizeUplink);
            }
          // one server (receiver) for each AP-STA pair
          AddServer (downlinkServerE, downlinkServerAppE, stasE.Get (i));
          if (aggregateDownlinkMbps > 0)
            {
              // each STA needs a client to generate traffic
              // random offset so that all transmissions do not occur at the same time
              if (txStartOffset > 0)
                {
                  next_rng = urv->GetValue();
                }
              AddClient (downlinkClientAppE, StaInterfaceE.GetAddress (i), ap5, downlinkPortE, Time (intervalDownlink + NanoSeconds (next_rng)), payloadSizeDownlink);
            }
        }

      // BSS 6

      // create one server (receiver) for uplink traffic
      AddServer (uplinkServerF, uplinkServerAppF, ap6);

      for (uint32_t i = 0; i < n; i++)
        {
          if (aggregateUplinkMbps > 0)
            {
              // for downlink, need to create client at AP to generate traffic to the server at the STA
              // random offset so that all transmissions do not occur at the same time
              if (txStartOffset > 0)
                {
                  next_rng = urv->GetValue();
                }
              AddClient (uplinkClientAppF, ApInterfaceF.GetAddress (0), stasF.Get (i), uplinkPortF, Time (intervalUplink + NanoSeconds (next_rng)), payloadSizeUplink);
            }
          // one server (receiver) for each AP-STA pair
          AddServer (downlinkServerF, downlinkServerAppF, stasF.Get (i));
          if (aggregateDownlinkMbps > 0)
            {
              // each STA needs a client to generate traffic
              // random offset so that all transmissions do not occur at the same time
              if (txStartOffset > 0)
                {
                  next_rng = urv->GetValue();
                }
              AddClient (downlinkClientAppF, StaInterfaceF.GetAddress (i), ap6, downlinkPortF, Time (intervalDownlink + NanoSeconds (next_rng)), payloadSizeDownlink);
            }
        }

      // BSS 7

      // create one server (receiver) for uplink traffic
      AddServer (uplinkServerG, uplinkServerAppG, ap7);

      for (uint32_t i = 0; i < n; i++)
        {
          if (aggregateUplinkMbps > 0)
            {
              // for downlink, need to create client at AP to generate traffic to the server at the STA
              // random offset so that all transmissions do not occur at the same time
              if (txStartOffset > 0)
                {
                  next_rng = urv->GetValue();
                }
              AddClient (uplinkClientAppG, ApInterfaceG.GetAddress (0), stasG.Get (i), uplinkPortG, Time (intervalUplink + NanoSeconds (next_rng)), payloadSizeUplink);
            }
          // one server (receiver) for each AP-STA pair
          AddServer (downlinkServerG, downlinkServerAppG, stasG.Get (i));
          if (aggregateDownlinkMbps > 0)
            {
              // each STA needs a client to generate traffic
              // random offset so that all transmissions do not occur at the same time
              if (txStartOffset > 0)
                {
                  next_rng = urv->GetValue();
                }
              AddClient (downlinkClientAppG, StaInterfaceG.GetAddress (i), ap7, downlinkPortG, Time (intervalDownlink + NanoSeconds (next_rng)), payloadSizeDownlink);
            }
        }
    }

  uplinkServerAppA.Start (Seconds (0.0));
  uplinkServerAppA.Stop (Seconds (duration + applicationTxStart));
  uplinkClientAppA.Start (Seconds (applicationTxStart));
  uplinkClientAppA.Stop (Seconds (duration + applicationTxStart));

  uplinkServerAppB.Start (Seconds (0.0));
  uplinkServerAppB.Stop (Seconds (duration + applicationTxStart));
  uplinkClientAppB.Start (Seconds (applicationTxStart));
  uplinkClientAppB.Stop (Seconds (duration + applicationTxStart));

  uplinkServerAppC.Start (Seconds (0.0));
  uplinkServerAppC.Stop (Seconds (duration + applicationTxStart));
  uplinkClientAppC.Start (Seconds (applicationTxStart));
  uplinkClientAppC.Stop (Seconds (duration + applicationTxStart));

  uplinkServerAppD.Start (Seconds (0.0));
  uplinkServerAppD.Stop (Seconds (duration + applicationTxStart));
  uplinkClientAppD.Start (Seconds (applicationTxStart));
  uplinkClientAppD.Stop (Seconds (duration + applicationTxStart));

  uplinkServerAppE.Start (Seconds (0.0));
  uplinkServerAppE.Stop (Seconds (duration + applicationTxStart));
  uplinkClientAppE.Start (Seconds (applicationTxStart));
  uplinkClientAppE.Stop (Seconds (duration + applicationTxStart));

  uplinkServerAppF.Start (Seconds (0.0));
  uplinkServerAppF.Stop (Seconds (duration + applicationTxStart));
  uplinkClientAppF.Start (Seconds (applicationTxStart));
  uplinkClientAppF.Stop (Seconds (duration + applicationTxStart));

  uplinkServerAppG.Start (Seconds (0.0));
  uplinkServerAppG.Stop (Seconds (duration + applicationTxStart));
  uplinkClientAppG.Start (Seconds (applicationTxStart));
  uplinkClientAppG.Stop (Seconds (duration + applicationTxStart));

  downlinkServerAppA.Start (Seconds (0.0));
  downlinkServerAppA.Stop (Seconds (duration + applicationTxStart));
  downlinkClientAppA.Start (Seconds (applicationTxStart));
  downlinkClientAppA.Stop (Seconds (duration + applicationTxStart));

  downlinkServerAppB.Start (Seconds (0.0));
  downlinkServerAppB.Stop (Seconds (duration + applicationTxStart));
  downlinkClientAppB.Start (Seconds (applicationTxStart));
  downlinkClientAppB.Stop (Seconds (duration + applicationTxStart));

  downlinkServerAppC.Start (Seconds (0.0));
  downlinkServerAppC.Stop (Seconds (duration + applicationTxStart));
  downlinkClientAppC.Start (Seconds (applicationTxStart));
  downlinkClientAppC.Stop (Seconds (duration + applicationTxStart));

  downlinkServerAppD.Start (Seconds (0.0));
  downlinkServerAppD.Stop (Seconds (duration + applicationTxStart));
  downlinkClientAppD.Start (Seconds (applicationTxStart));
  downlinkClientAppD.Stop (Seconds (duration + applicationTxStart));

  downlinkServerAppE.Start (Seconds (0.0));
  downlinkServerAppE.Stop (Seconds (duration + applicationTxStart));
  downlinkClientAppE.Start (Seconds (applicationTxStart));
  downlinkClientAppE.Stop (Seconds (duration + applicationTxStart));

  downlinkServerAppF.Start (Seconds (0.0));
  downlinkServerAppF.Stop (Seconds (duration + applicationTxStart));
  downlinkClientAppF.Start (Seconds (applicationTxStart));
  downlinkClientAppF.Stop (Seconds (duration + applicationTxStart));

  downlinkServerAppG.Start (Seconds (0.0));
  downlinkServerAppG.Stop (Seconds (duration + applicationTxStart));
  downlinkClientAppG.Start (Seconds (applicationTxStart));
  downlinkClientAppG.Stop (Seconds (duration + applicationTxStart));

  Config::Connect ("/NodeList/*/DeviceList/*/Phy/MonitorSnifferRx", MakeCallback (&MonitorSniffRx));

  if (performTgaxTimingChecks)
    {
      Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/MacLow/TxAmpdu", MakeCallback (&TxAmpduCallback));
      Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/MacLow/RxBlockAck", MakeCallback (&RxBlockAckCallback));
    }


  if (enableAscii)
    {
      AsciiTraceHelper ascii;
      spectrumPhy.EnableAsciiAll (ascii.CreateFileStream (outputFilePrefix + ".tr"));
    }

  // This enabling function could be scheduled later in simulation if desired
  SchedulePhyLogConnect ();
  g_stateFile.open (outputFilePrefix + "-state.dat", std::ofstream::out | std::ofstream::trunc);
  g_stateFile.setf (std::ios_base::fixed);
  ScheduleStateLogConnect ();

  g_rxSniffFile.open (outputFilePrefix + "-rx-sniff.dat", std::ofstream::out | std::ofstream::trunc);
  g_rxSniffFile.setf (std::ios_base::fixed);
  g_rxSniffFile << "RxNodeId, DstNodeId, SrcNodeId, RxNodeAddr, DA, SA, Noise, Signal " << std::endl;

  g_TGaxCalibrationTimingsFile.open (outputFilePrefix + "-tgax-calibration-timings.dat", std::ofstream::out | std::ofstream::trunc);
  g_TGaxCalibrationTimingsFile.setf (std::ios_base::fixed);

  // Save attribute configuration
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (outputFilePrefix + ".config"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureAttributes ();

  if (enablePcap == true)
    {
      spectrumPhy.EnablePcap ("pcapforPackets", staDevicesA);
    }

  // uint32_t nNodes = allNodes.GetN ();
  // std::cout << "Node Address" << std::endl;
  // for (uint32_t n = 0; n < nNodes; n++)
  //   {
  //
  //     Address address = allNodes.Get (n)->GetDevice(0)->GetAddress();
  //     Mac48Address mac48address = Mac48Address::ConvertFrom(address);
  //     Ptr<NetDevice> dev = allNodes.Get (n)->GetDevice(0);
  //     Ptr<WifiNetDevice> wifi_dev = DynamicCast<WifiNetDevice>(dev);
  //     Mac48Address addrm = wifi_dev->GetMac ()->GetAddress ();
  //     std::cout << n << " " << address << " Mac48Address " << mac48address << " addrm " << addrm << std::endl;
  //   }

  Time durationTime = Seconds (duration + applicationTxStart);
  Simulator::Stop (durationTime);
  Simulator::Run ();

  /* Show results */
  uint32_t nodeIdx = 0;
  uint32_t nUplinkAppsA = uplinkServerAppA.GetN ();
  uint64_t totalUplinkPacketsThroughA = 0;
  for (uint32_t i = 0; i < nUplinkAppsA; i++)
    {
      totalUplinkPacketsThroughA += DynamicCast<UdpServer> (uplinkServerAppA.Get (i))->GetReceived ();
    }
  bytesReceived[nodeIdx] = payloadSizeUplink * totalUplinkPacketsThroughA;
  packetsReceived[nodeIdx] = totalUplinkPacketsThroughA;

  if ((nBss >= 2) || (scenario == "study1"))
    {
      nodeIdx++;

      uint32_t nUplinkAppsB = uplinkServerAppB.GetN ();
      uint64_t totalUplinkPacketsThroughB = 0;
      for (uint32_t i = 0; i < nUplinkAppsB; i++)
        {
          totalUplinkPacketsThroughB += DynamicCast<UdpServer> (uplinkServerAppB.Get (i))->GetReceived ();
        }
      bytesReceived[nodeIdx] = payloadSizeUplink * totalUplinkPacketsThroughB;
      packetsReceived[nodeIdx] = totalUplinkPacketsThroughB;
    }

  if ((nBss >= 3) || (scenario == "study1"))
    {
      nodeIdx++;

      uint32_t nUplinkAppsC = uplinkServerAppC.GetN ();
      uint64_t totalUplinkPacketsThroughC = 0;
      for (uint32_t i = 0; i < nUplinkAppsC; i++)
        {
          totalUplinkPacketsThroughC += DynamicCast<UdpServer> (uplinkServerAppC.Get (i))->GetReceived ();
        }
      bytesReceived[nodeIdx] = payloadSizeUplink * totalUplinkPacketsThroughC;
      packetsReceived[nodeIdx] = totalUplinkPacketsThroughC;
    }

  if ((nBss >= 4) || (scenario == "study1"))
    {
      nodeIdx++;

      uint32_t nUplinkAppsD = uplinkServerAppD.GetN ();
      uint64_t totalUplinkPacketsThroughD = 0;
      for (uint32_t i = 0; i < nUplinkAppsD; i++)
        {
          totalUplinkPacketsThroughD += DynamicCast<UdpServer> (uplinkServerAppD.Get (i))->GetReceived ();
        }
      bytesReceived[nodeIdx] = payloadSizeUplink * totalUplinkPacketsThroughD;
      packetsReceived[nodeIdx] = totalUplinkPacketsThroughD;
    }

  if (scenario == "study1")
    {
      nodeIdx++;

      uint32_t nUplinkAppsE = uplinkServerAppE.GetN ();
      uint64_t totalUplinkPacketsThroughE = 0;
      for (uint32_t i = 0; i < nUplinkAppsE; i++)
        {
          totalUplinkPacketsThroughE += DynamicCast<UdpServer> (uplinkServerAppE.Get (i))->GetReceived ();
        }
      bytesReceived[nodeIdx] = payloadSizeUplink * totalUplinkPacketsThroughE;
      packetsReceived[nodeIdx] = totalUplinkPacketsThroughE;

      nodeIdx++;

      uint32_t nUplinkAppsF = uplinkServerAppF.GetN ();
      uint64_t totalUplinkPacketsThroughF = 0;
      for (uint32_t i = 0; i < nUplinkAppsF; i++)
        {
          totalUplinkPacketsThroughF += DynamicCast<UdpServer> (uplinkServerAppF.Get (i))->GetReceived ();
        }
      bytesReceived[nodeIdx] = payloadSizeUplink * totalUplinkPacketsThroughF;
      packetsReceived[nodeIdx] = totalUplinkPacketsThroughF;

      nodeIdx++;

      uint32_t nUplinkAppsG = uplinkServerAppG.GetN ();
      uint64_t totalUplinkPacketsThroughG = 0;
      for (uint32_t i = 0; i < nUplinkAppsG; i++)
        {
          totalUplinkPacketsThroughG += DynamicCast<UdpServer> (uplinkServerAppG.Get (i))->GetReceived ();
        }
      bytesReceived[nodeIdx] = payloadSizeUplink * totalUplinkPacketsThroughG;
      packetsReceived[nodeIdx] = totalUplinkPacketsThroughG;
    }

  uint32_t nDownlinkAppsA = downlinkServerAppA.GetN ();
  for (uint32_t i = 0; i < nDownlinkAppsA; i++)
    {
      nodeIdx++;
      uint64_t downlinkPacketsThroughA = DynamicCast<UdpServer> (downlinkServerAppA.Get (i))->GetReceived ();
      bytesReceived[nodeIdx] = payloadSizeUplink * downlinkPacketsThroughA;
      packetsReceived[nodeIdx] = downlinkPacketsThroughA;
    }

  if ((nBss >= 2) || (scenario == "study1"))
    {
      uint32_t nDownlinkAppsB = downlinkServerAppB.GetN ();
      for (uint32_t i = 0; i < nDownlinkAppsB; i++)
        {
          nodeIdx++;
          uint64_t downlinkPacketsThroughB = DynamicCast<UdpServer> (downlinkServerAppB.Get (i))->GetReceived ();
          bytesReceived[nodeIdx] = payloadSizeUplink * downlinkPacketsThroughB;
          packetsReceived[nodeIdx] = downlinkPacketsThroughB;
        }
    }

  if ((nBss >= 3) || (scenario == "study1"))
    {
      uint32_t nDownlinkAppsC = downlinkServerAppC.GetN ();
      for (uint32_t i = 0; i < nDownlinkAppsC; i++)
        {
          nodeIdx++;
          uint64_t downlinkPacketsThroughC = DynamicCast<UdpServer> (downlinkServerAppC.Get (i))->GetReceived ();
          bytesReceived[nodeIdx] = payloadSizeUplink * downlinkPacketsThroughC;
          packetsReceived[nodeIdx] = downlinkPacketsThroughC;
        }
    }

  if ((nBss >= 4) || (scenario == "study1"))
    {
      uint32_t nDownlinkAppsD = downlinkServerAppD.GetN ();
      for (uint32_t i = 0; i < nDownlinkAppsD; i++)
        {
          nodeIdx++;
          uint64_t downlinkPacketsThroughD = DynamicCast<UdpServer> (downlinkServerAppD.Get (i))->GetReceived ();
          bytesReceived[nodeIdx] = payloadSizeUplink * downlinkPacketsThroughD;
          packetsReceived[nodeIdx] = downlinkPacketsThroughD;
        }
    }

  if (scenario == "study1")
    {
      uint32_t nDownlinkAppsE = downlinkServerAppE.GetN ();
      for (uint32_t i = 0; i < nDownlinkAppsE; i++)
        {
          nodeIdx++;
          uint64_t downlinkPacketsThroughE = DynamicCast<UdpServer> (downlinkServerAppE.Get (i))->GetReceived ();
          bytesReceived[nodeIdx] = payloadSizeUplink * downlinkPacketsThroughE;
          packetsReceived[nodeIdx] = downlinkPacketsThroughE;
        }

      uint32_t nDownlinkAppsF = downlinkServerAppF.GetN ();
      for (uint32_t i = 0; i < nDownlinkAppsF; i++)
        {
          nodeIdx++;
          uint64_t downlinkPacketsThroughF = DynamicCast<UdpServer> (downlinkServerAppF.Get (i))->GetReceived ();
          bytesReceived[nodeIdx] = payloadSizeUplink * downlinkPacketsThroughF;
          packetsReceived[nodeIdx] = downlinkPacketsThroughF;
        }

      uint32_t nDownlinkAppsG = downlinkServerAppG.GetN ();
      for (uint32_t i = 0; i < nDownlinkAppsG; i++)
        {
          nodeIdx++;
          uint64_t downlinkPacketsThroughG = DynamicCast<UdpServer> (downlinkServerAppG.Get (i))->GetReceived ();
          bytesReceived[nodeIdx] = payloadSizeUplink * downlinkPacketsThroughG;
          packetsReceived[nodeIdx] = downlinkPacketsThroughG;
        }
    }

  SchedulePhyLogDisconnect ();
  ScheduleStateLogDisconnect ();
  g_stateFile.flush ();
  g_stateFile.close ();

  g_TGaxCalibrationTimingsFile.close ();

  SaveSpectrumPhyStats (outputFilePrefix + "-phy-log.dat", g_arrivals);

  Simulator::Destroy ();

  // Save spatial reuse statistics to an output file
  SaveSpatialReuseStats (outputFilePrefix + "-SR-stats.dat", packetsReceived, bytesReceived, nBss, duration, d,  r, freq, csr, scenario);

  return 0;
}


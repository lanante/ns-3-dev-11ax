/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 University of Washington
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
 * Author: Sébastien Deronne <sebastien.deronne@gmail.com>
 */

#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/string.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/ssid.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/wifi-net-device.h"

// for tracking packets and bytes received. will be reallocated once we finalize number of nodes
std::vector<uint64_t> packetsReceived (0);
std::vector<uint64_t> bytesReceived (0);

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiChannelBonding");
// Parse context strings of the form "/NodeList/3/DeviceList/1/Mac/Assoc"
// to extract the NodeId
uint32_t
ContextToNodeId (std::string context)
{
  std::string sub = context.substr (10);  // skip "/NodeList/"
  uint32_t pos = sub.find ("/Device");
  uint32_t nodeId = atoi (sub.substr (0, pos).c_str ());
  NS_LOG_DEBUG ("Found NodeId " << nodeId);
  return nodeId;
}
void AddClient (ApplicationContainer &clientApps, Ipv4Address address, Ptr<Node> node, uint16_t port, Time interval, uint32_t payloadSize)
{
  UdpClientHelper client (address, port);
  client.SetAttribute ("Interval", TimeValue (interval ));
  client.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
  client.SetAttribute ("PacketSize", UintegerValue (payloadSize));
  clientApps.Add (client.Install (node));
}
void AddServer (ApplicationContainer &serverApps, UdpServerHelper &server, Ptr<Node> node)
{
  serverApps.Add (server.Install (node));
}

void
PacketRx (std::string context, const Ptr<const Packet> p, const Address &srcAddress, const Address &destAddress)
{
  uint32_t nodeId = ContextToNodeId (context);
  uint32_t pktSize = p->GetSize ();
      bytesReceived[nodeId] += pktSize;
      packetsReceived[nodeId]++;

}


int main (int argc, char *argv[])
{
  uint32_t payloadSize = 1472; //bytes
  double simulationTime = 20; //seconds
  double distance = 10; //meters
  double interBssDistance = 50; //meters
  double txMaskInnerBandMinimumRejection = -40.0; //dBr
  double txMaskOuterBandMinimumRejection = -56.0; //dBr
  double txMaskOuterBandMaximumRejection = -80.0; //dBr

  bool useDynamicChannelBonding = true;
  uint16_t channelBssA = 36;
  uint16_t channelBssB = 40;
  uint16_t channelBssC = 36;
  uint16_t primaryChannelBssA = 36;
  uint16_t primaryChannelBssB = 36;
  uint16_t primaryChannelBssC = 38;

  std::string mcs = "";
  double ccaEdThresholdPrimaryBssA = -62.0;
  double ccaEdThresholdSecondaryBssA = -62.0;
  double ccaEdThresholdPrimaryBssB = -62.0;
  double ccaEdThresholdSecondaryBssB = -62.0;
  double ccaEdThresholdPrimaryBssC = -62.0;
  double ccaEdThresholdSecondaryBssC = -62.0;

double aggregateDownlinkAMbps=0;
double aggregateDownlinkBMbps=0;
double aggregateDownlinkCMbps=0;
double aggregateUplinkAMbps=0;
double aggregateUplinkBMbps=0;
double aggregateUplinkCMbps=0;
  bool verifyResults = 0; //used for regression
  double minExpectedThroughputBssA = 0; //Mbit/s
  double maxExpectedThroughputBssA = 0; //Mbit/s
  double minExpectedThroughputBssB = 0; //Mbit/s
  double maxExpectedThroughputBssB = 0; //Mbit/s
uint16_t n=2;
  uint32_t maxMissedBeacons = 4294967295;
  CommandLine cmd;


  cmd.AddValue ("payloadSize", "Payload size in bytes", payloadSize);
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("distance", "Distance in meters between the station and the access point", distance);
  cmd.AddValue ("interBssDistance", "Distance in meters between BSS A and BSS B", interBssDistance);
  cmd.AddValue ("txMaskInnerBandMinimumRejection", "Minimum rejection in dBr for the inner band of the transmit spectrum masks", txMaskInnerBandMinimumRejection);
  cmd.AddValue ("txMaskOuterBandMinimumRejection", "Minimum rejection in dBr for the outer band of the transmit spectrum mask", txMaskOuterBandMinimumRejection);
  cmd.AddValue ("txMaskOuterBandMaximumRejection", "Maximum rejection in dBr for the outer band of the transmit spectrum mask", txMaskOuterBandMaximumRejection);
  cmd.AddValue ("channelBssA", "The selected channel for BSS A", channelBssA);
  cmd.AddValue ("channelBssB", "The selected channel for BSS B", channelBssB);
  cmd.AddValue ("channelBssC", "The selected channel for BSS C", channelBssC);
 cmd.AddValue ("primaryChannelBssA", "The primary 20 MHz channel for BSS A", primaryChannelBssA);
  cmd.AddValue ("primaryChannelBssB", "The primary 20 MHz channel for BSS B", primaryChannelBssB);
  cmd.AddValue ("primaryChannelBssC", "The primary 20 MHz channel for BSS C", primaryChannelBssC);

  cmd.AddValue ("useDynamicChannelBonding", "Enable/disable use of dynamic channel bonding", useDynamicChannelBonding);
  cmd.AddValue ("ccaEdThresholdPrimaryBssA", "The energy detection threshold on the primary channel for BSS A", ccaEdThresholdPrimaryBssA);
  cmd.AddValue ("ccaEdThresholdSecondaryBssA", "The energy detection threshold on the secondary channel for BSS A", ccaEdThresholdSecondaryBssA);
  cmd.AddValue ("ccaEdThresholdPrimaryBssB", "The energy detection threshold on the primary channel for BSS B", ccaEdThresholdPrimaryBssB);
  cmd.AddValue ("ccaEdThresholdSecondaryBssB", "The energy detection threshold on the secondary channel for BSS B", ccaEdThresholdSecondaryBssB);
  cmd.AddValue ("ccaEdThresholdPrimaryBssC", "The energy detection threshold on the primary channel for BSS C", ccaEdThresholdPrimaryBssC);
  cmd.AddValue ("ccaEdThresholdSecondaryBssC", "The energy detection threshold on the secondary channel for BSS C", ccaEdThresholdSecondaryBssC);
  cmd.AddValue ("uplinkA", "Aggregate uplink load, BSS-A(Mbps)", aggregateUplinkAMbps);
  cmd.AddValue ("downlinkA", "Aggregate downlink load, BSS-A (Mbps)", aggregateDownlinkAMbps);
 cmd.AddValue ("uplinkB", "Aggregate uplink load, BSS-B(Mbps)", aggregateUplinkBMbps);
  cmd.AddValue ("downlinkB", "Aggregate downlink load, BSS-B (Mbps)", aggregateDownlinkBMbps); 
cmd.AddValue ("uplinkC", "Aggregate uplink load, BSS-C(Mbps)", aggregateUplinkCMbps);
  cmd.AddValue ("downlinkC", "Aggregate downlink load, BSS-C (Mbps)", aggregateDownlinkCMbps);

  cmd.AddValue ("n", "The number of STAs per BSS", n);
  cmd.AddValue ("mcs", "MCS", mcs);
  cmd.AddValue ("verifyResults", "Enable/disable results verification at the end of the simulation", verifyResults);
  cmd.AddValue ("minExpectedThroughputBssA", "Minimum expected throughput for BSS A", minExpectedThroughputBssA);
  cmd.AddValue ("maxExpectedThroughputBssA", "Maximum expected throughput for BSS A", maxExpectedThroughputBssA);
  cmd.AddValue ("minExpectedThroughputBssB", "Minimum expected throughput for BSS B", minExpectedThroughputBssB);
  cmd.AddValue ("maxExpectedThroughputBssB", "Maximum expected throughput for BSS B", maxExpectedThroughputBssB);
  cmd.Parse (argc, argv);

  /*LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  LogComponentEnable ("MacLow", LOG_LEVEL_ALL);
  LogComponentEnable ("WifiPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("SpectrumWifiPhy", LOG_LEVEL_ALL);*/

  Config::SetDefault ("ns3::SpectrumWifiPhy::TxMaskInnerBandMinimumRejection", DoubleValue (txMaskInnerBandMinimumRejection));
  Config::SetDefault ("ns3::SpectrumWifiPhy::TxMaskOuterBandMinimumRejection", DoubleValue (txMaskOuterBandMinimumRejection));
  Config::SetDefault ("ns3::SpectrumWifiPhy::TxMaskOuterBandMaximumRejection", DoubleValue (txMaskOuterBandMaximumRejection));

  NodeContainer wifiStaNodesA,wifiStaNodesB,wifiStaNodesC;
  NodeContainer wifiApNodes;
  wifiApNodes.Create (3);
  wifiStaNodesA.Create (n);
  wifiStaNodesB.Create (n);
  wifiStaNodesC.Create (n);




  uint32_t numNodes = 3 * (n + 1);
  packetsReceived = std::vector<uint64_t> (numNodes);
  bytesReceived = std::vector<uint64_t> (numNodes);

  double perNodeUplinkAMbps = aggregateUplinkAMbps / n;
  double perNodeDownlinkAMbps = aggregateDownlinkAMbps / n;
  double perNodeUplinkBMbps = aggregateUplinkAMbps / n;
  double perNodeDownlinkBMbps = aggregateDownlinkBMbps / n;
  double perNodeUplinkCMbps = aggregateUplinkAMbps / n;
  double perNodeDownlinkCMbps = aggregateDownlinkCMbps / n;

  Time intervalUplinkA = MicroSeconds (payloadSize * 8 / perNodeUplinkAMbps);
  Time intervalDownlinkA = MicroSeconds (payloadSize * 8 / perNodeDownlinkAMbps);
  Time intervalUplinkB = MicroSeconds (payloadSize * 8 / perNodeUplinkBMbps);
  Time intervalDownlinkB = MicroSeconds (payloadSize * 8 / perNodeDownlinkBMbps);
  Time intervalUplinkC = MicroSeconds (payloadSize * 8 / perNodeUplinkCMbps);
  Time intervalDownlinkC = MicroSeconds (payloadSize * 8 / perNodeDownlinkCMbps);



  SpectrumWifiPhyHelper phy = SpectrumWifiPhyHelper::Default ();
  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();
	Ptr<LogDistancePropagationLossModel> lossModel = CreateObject<LogDistancePropagationLossModel> ();
  lossModel ->SetAttribute ("ReferenceDistance", DoubleValue (1));
  lossModel ->SetAttribute ("Exponent", DoubleValue (3.5));
 lossModel ->SetAttribute ("ReferenceLoss", DoubleValue (50));
      channel->AddPropagationLossModel (lossModel); 
phy.SetChannel (channel);


  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);
if (mcs == "")
 { wifi.SetRemoteStationManager ("ns3::IdealWifiManager", "DataMode", StringValue (mcs), "ControlMode", StringValue ("VhtMcs0"));
}
else
 { wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (mcs), "ControlMode", StringValue ("VhtMcs0"));
}

  if (useDynamicChannelBonding)
    {
      wifi.SetChannelBondingManager ("ns3::ConstantThresholdChannelBondingManager");
    }

  NetDeviceContainer staDeviceA, staDeviceB,staDeviceC, apDeviceA, apDeviceB,apDeviceC;
  WifiMacHelper mac;
  Ssid ssid;

  // network A
  ssid = Ssid ("network-A");

  mac.SetType ("ns3::StaWifiMac",
               "MaxMissedBeacons", UintegerValue (maxMissedBeacons),
               "Ssid", SsidValue (ssid));
  staDeviceA = wifi.Install (phy, mac, wifiStaNodesA);

Ptr<NetDevice> staDeviceAPtr;
for (uint16_t i=0;i<n;i++) {

   staDeviceAPtr= staDeviceA.Get (i);
  Ptr<WifiNetDevice> wifiStaDeviceAPtr = staDeviceAPtr->GetObject <WifiNetDevice> ();
  wifiStaDeviceAPtr->GetPhy ()->SetChannelNumber (channelBssA);
  wifiStaDeviceAPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssA);
  wifiStaDeviceAPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssA);
  wifiStaDeviceAPtr->GetPhy ()->SetCcaEdThresholdSecondary (ccaEdThresholdSecondaryBssA);

}


  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "EnableBeaconJitter", BooleanValue (true));
  apDeviceA = wifi.Install (phy, mac, wifiApNodes.Get (0));

  Ptr<NetDevice> apDeviceAPtr = apDeviceA.Get (0);
  Ptr<WifiNetDevice> wifiApDeviceAPtr = apDeviceAPtr->GetObject <WifiNetDevice> ();
  wifiApDeviceAPtr->GetPhy ()->SetChannelNumber (channelBssA);
  wifiApDeviceAPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssA);
  wifiApDeviceAPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssA);
  wifiApDeviceAPtr->GetPhy ()->SetCcaEdThresholdSecondary (ccaEdThresholdSecondaryBssA);

  // network B
  ssid = Ssid ("network-B");

   mac.SetType ("ns3::StaWifiMac",
               "MaxMissedBeacons", UintegerValue (maxMissedBeacons),
               "Ssid", SsidValue (ssid));
  staDeviceB = wifi.Install (phy, mac, wifiStaNodesB);


Ptr<NetDevice> staDeviceBPtr;
for (uint16_t i=0;i<n;i++) {

   staDeviceBPtr= staDeviceB.Get (i);
  Ptr<WifiNetDevice> wifiStaDeviceBPtr = staDeviceBPtr->GetObject <WifiNetDevice> ();
  wifiStaDeviceBPtr->GetPhy ()->SetChannelNumber (channelBssB);
  wifiStaDeviceBPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssB);
  wifiStaDeviceBPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssB);
  wifiStaDeviceBPtr->GetPhy ()->SetCcaEdThresholdSecondary (ccaEdThresholdSecondaryBssB);
}

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "EnableBeaconJitter", BooleanValue (true));
  apDeviceB = wifi.Install (phy, mac, wifiApNodes.Get (1));

  Ptr<NetDevice> apDeviceBPtr = apDeviceB.Get (0);
  Ptr<WifiNetDevice> wifiApDeviceBPtr = apDeviceBPtr->GetObject <WifiNetDevice> ();
  wifiApDeviceBPtr->GetPhy ()->SetChannelNumber (channelBssB);
  wifiApDeviceBPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssB);
  wifiApDeviceBPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssB);
  wifiApDeviceBPtr->GetPhy ()->SetCcaEdThresholdSecondary (ccaEdThresholdSecondaryBssB);

  // network C
  ssid = Ssid ("network-C");
  mac.SetType ("ns3::StaWifiMac",
               "MaxMissedBeacons", UintegerValue (maxMissedBeacons),
               "Ssid", SsidValue (ssid));
  staDeviceC = wifi.Install (phy, mac, wifiStaNodesC);

Ptr<NetDevice> staDeviceCPtr;
for (uint16_t i=0;i<n;i++) {

   staDeviceCPtr= staDeviceC.Get (i);
  Ptr<WifiNetDevice> wifiStaDeviceCPtr = staDeviceCPtr->GetObject <WifiNetDevice> ();
  wifiStaDeviceCPtr->GetPhy ()->SetChannelNumber (channelBssC);
  wifiStaDeviceCPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssC);
  wifiStaDeviceCPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssC);
  wifiStaDeviceCPtr->GetPhy ()->SetCcaEdThresholdSecondary (ccaEdThresholdSecondaryBssC);
}

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "EnableBeaconJitter", BooleanValue (true));
  apDeviceC = wifi.Install (phy, mac, wifiApNodes.Get (2));

  Ptr<NetDevice> apDeviceCPtr = apDeviceC.Get (0);
  Ptr<WifiNetDevice> wifiApDeviceCPtr = apDeviceCPtr->GetObject <WifiNetDevice> ();
  wifiApDeviceCPtr->GetPhy ()->SetChannelNumber (channelBssC);
  wifiApDeviceCPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssC);
  wifiApDeviceCPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssC);
  wifiApDeviceCPtr->GetPhy ()->SetCcaEdThresholdSecondary (ccaEdThresholdSecondaryBssC);






  // Setting mobility model
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // Set position for APs
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (interBssDistance, 0.0, 0.0));
  positionAlloc->Add (Vector (interBssDistance*2, 0.0, 0.0));



  // Set position for STAs
  int64_t streamNumber = 100;
Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator1 = CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator1->AssignStreams (streamNumber);
      // AP1 is at origin (x=x1, y=y1), with radius Rho=r
      unitDiscPositionAllocator1->SetX (0);
      unitDiscPositionAllocator1->SetY (0);
      unitDiscPositionAllocator1->SetRho (distance);
      for (uint32_t i = 0; i < n; i++)
        {
          Vector v = unitDiscPositionAllocator1->GetNext ();
          positionAlloc->Add (v);
        }
Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator2= CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator2->AssignStreams (streamNumber+1);
      // AP2 is at origin (x=x2, y=y2), with radius Rho=r
      unitDiscPositionAllocator2->SetX (interBssDistance);
      unitDiscPositionAllocator2->SetY (0);
      unitDiscPositionAllocator2->SetRho (distance);
      for (uint32_t i = 0; i < n; i++)
        {
          Vector v = unitDiscPositionAllocator2->GetNext ();
          positionAlloc->Add (v);
        }
Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator3 = CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator3->AssignStreams (streamNumber+2);
      // AP3 is at origin (x=x3, y=y3), with radius Rho=r
      unitDiscPositionAllocator3->SetX (interBssDistance*2);
      unitDiscPositionAllocator3->SetY (0);
      unitDiscPositionAllocator3->SetRho (distance);
      for (uint32_t i = 0; i < n; i++)
        {
          Vector v = unitDiscPositionAllocator3->GetNext ();
          positionAlloc->Add (v);
        }



  mobility.SetPositionAllocator (positionAlloc);
NodeContainer allNodes = NodeContainer (wifiApNodes, wifiStaNodesA, wifiStaNodesB, wifiStaNodesC);
  mobility.Install (allNodes);

  // Internet stack
  InternetStackHelper stack;
  stack.Install (allNodes);


  Ipv4AddressHelper address;
  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer StaInterfaceA;
  StaInterfaceA = address.Assign (staDeviceA);
  Ipv4InterfaceContainer ApInterfaceA;
  ApInterfaceA = address.Assign (apDeviceA);

  address.SetBase ("192.168.2.0", "255.255.255.0");
  Ipv4InterfaceContainer StaInterfaceB;
  StaInterfaceB = address.Assign (staDeviceB);
  Ipv4InterfaceContainer ApInterfaceB;
  ApInterfaceB = address.Assign (apDeviceB);

  address.SetBase ("192.168.3.0", "255.255.255.0");
  Ipv4InterfaceContainer StaInterfaceC;
  StaInterfaceC = address.Assign (staDeviceC);
  Ipv4InterfaceContainer ApInterfaceC;
  ApInterfaceC = address.Assign (apDeviceC);


  // Setting applications

ApplicationContainer uplinkServerApps;
ApplicationContainer downlinkServerApps;
ApplicationContainer uplinkClientApps;
ApplicationContainer downlinkClientApps;

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

 for (uint32_t i = 0; i < n; i++)
        {
          if (aggregateUplinkAMbps > 0)
            {
              AddClient (uplinkClientApps, ApInterfaceA.GetAddress (0), wifiStaNodesA.Get (i), uplinkPortA, intervalUplinkA, payloadSize) ;
            }
          if (aggregateDownlinkAMbps > 0)
            {
              AddClient (downlinkClientApps, StaInterfaceA.GetAddress (i), wifiApNodes.Get(0), downlinkPortA, intervalDownlinkA, payloadSize);
              AddServer (downlinkServerApps, downlinkServerA, wifiStaNodesA.Get (i));
            }
        }
      if (aggregateUplinkAMbps > 0)
      {
        AddServer (uplinkServerApps, uplinkServerA, wifiApNodes.Get(0));
      }

   for (uint32_t i = 0; i < n; i++)
        {
          if (aggregateUplinkBMbps > 0)
            {
              AddClient (uplinkClientApps, ApInterfaceB.GetAddress (0), wifiStaNodesB.Get (i), uplinkPortB, intervalUplinkB, payloadSize);
            }
          if (aggregateDownlinkBMbps > 0)
            {
              AddClient (downlinkClientApps, StaInterfaceB.GetAddress (i), wifiApNodes.Get(1), downlinkPortB, intervalDownlinkB, payloadSize);
              AddServer (downlinkServerApps, downlinkServerB, wifiStaNodesB.Get (i));

            }
        }
      if (aggregateUplinkBMbps > 0)
      {
        AddServer (uplinkServerApps, uplinkServerB, wifiApNodes.Get(1));
      }

  // BSS 3

      for (uint32_t i = 0; i < n; i++)
        {
          if (aggregateUplinkCMbps > 0)
            {
              AddClient (uplinkClientApps, ApInterfaceC.GetAddress (0), wifiStaNodesC.Get (i), uplinkPortC, intervalUplinkC, payloadSize);
            }
          if (aggregateDownlinkCMbps > 0)
            {
              AddClient (downlinkClientApps, StaInterfaceC.GetAddress (i), wifiApNodes.Get(2), downlinkPortC, intervalDownlinkC, payloadSize);
              AddServer (downlinkServerApps, downlinkServerC,wifiStaNodesC.Get (i));
            }
        }
      if (aggregateUplinkCMbps > 0)
      {
        AddServer (uplinkServerApps, uplinkServerC, wifiApNodes.Get(2));
      }



  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::UdpServer/RxWithAddresses", MakeCallback (&PacketRx));
/*
phy.EnablePcap ("staA_pcap", staDeviceA);
phy.EnablePcap ("apA_pcap", apDeviceA);
phy.EnablePcap ("staB_pcap", staDeviceB);
phy.EnablePcap ("apB_pcap", apDeviceB);
phy.EnablePcap ("staC_pcap", staDeviceC);
phy.EnablePcap ("apC_pcap", apDeviceC);
*/



  Simulator::Stop (Seconds (simulationTime + 1));
  Simulator::Run ();





  Simulator::Destroy ();
  // allocate in the order of AP_A, STAs_A, AP_B, STAs_B
std::string filename;
if (mcs=="")
{
  filename =  "Tput_.csv";
}
else
{
  filename =  "Tput_"+mcs+".csv";
} 
 std::ofstream TputFile;
  TputFile.open (filename.c_str (), std::ofstream::out | std::ofstream::app);
  TputFile.setf (std::ios_base::fixed);
  TputFile.flush ();
  if (!TputFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return 1;
    }

      TputFile << ccaEdThresholdSecondaryBssC<<std::endl;
  double rxThroughputPerNode[numNodes];
  // output for all nodes
  for (uint32_t k = 0; k < numNodes; k++)
    {
      double bitsReceived = bytesReceived[k] * 8;
      rxThroughputPerNode[k] = static_cast<double> (bitsReceived) / 1e6 / simulationTime;
      std::cout << "Node " << k << ", pkts " << packetsReceived[k] << ", bytes " << bytesReceived[k] << ", throughput [MMb/s] " << rxThroughputPerNode[k] << std::endl;
if (k<3)
{
      TputFile << rxThroughputPerNode[k] << std::endl;
}
    }

TputFile << std::endl;
TputFile.close ();

 
  return 0;
}


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

// This is an example to evaluate 802.11n channel bonding performance.
// It defines 2 independent Wi-Fi networks, made each of one access
// point (AP) and one station (STA). Each station continuously transmits
// data packets to its respective AP.
//
// The parameters that can be configured are:
//   - the channel number the network is operating on;
//   - the maximum supported channel width per network;
//   - whether the secondary channel is upper or lower than the primary channel;
//   - transmit mask points used for the simulation;
//   - whether dynamic channel bonding is used;
//   - the distance between AP and STA and the distance between the two networks;
//   - CCA-ED per network for both primary and secondary channels;
//   - the packet size and the number of packets generated per second;
//   - the time the simulation will run.
//
// One can run a scenario where each network occupies its 20 MHz band, i.e. network A
// uses channel 36 whereas network B is configured to use channel 40:
//     ./waf --run "wifi-channel-bonding --channelBssA=36 --channelBssB=40 --useDynamicChannelBonding=false"
// The output gives:
//     Throughput for BSS A: 59.5465 Mbit/s
//     Throughput for BSS B: 59.4782 Mbit/s
// The throughput per network is maximum and not affected by the presence of the other network,
// since they are operating on different channels.
//
// One can run a scenario where a 40 MHz channel is used for network A, while keeping network B as previously:
//     ./waf --run "wifi-channel-bonding --channelBssA=38 --channelBssB=40 --useDynamicChannelBonding=false"
// The output gives:
//     Throughput for BSS A: 80.0815 Mbit/s
//     Throughput for BSS B: 17.0681 Mbit/s
// Since dynamic channel bonding is disabled, network A will always transmit on 40 MHz, regardless of
// CCA on the secondary channel. As a result, the two networks will suffer from collisions from each others.
//
// One can run the previous scenario with dynamic channel bonding enabled:
//     ./waf --run "wifi-channel-bonding --channelBssA=38 --channelBssB=40 --useDynamicChannelBonding=true"
// The output gives:
//     Throughput for BSS A: 59.6266 Mbit/s
//     Throughput for BSS B: 59.3746 Mbit/s
// We can see the benefit of using a dynamic channel bonding. Since activity is detected on the secondary channel,
// network A limits its channel width to 20 MHz and this gives a better share of the spectrum.
//
// One can run a scenario where both networks make use of channel bonding:
//     ./waf --run "wifi-channel-bonding --channelBssA=38 --channelBssB=38 --useDynamicChannelBonding=false"
//     Throughput for BSS A: 60.3073 Mbit/s
//     Throughput for BSS B: 62.288 Mbit/s
// The channel is shared with the two networks as they operate on the same channel, but since they can use both
// a 40 Mhz channel, the maximum throughput is almost doubled.

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiChannelBonding");

void AddClient (ApplicationContainer &clientApps, Ipv4Address address, Ptr<Node> node, uint16_t port, Time interval, uint32_t payloadSize)
{
  UdpClientHelper client (address, port);
  client.SetAttribute ("Interval", TimeValue (interval ));
  client.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
  client.SetAttribute ("PacketSize", UintegerValue (payloadSize));
  clientApps.Add (client.Install (node));
}


int main (int argc, char *argv[])
{
  uint32_t payloadSize = 1472; //bytes
  double simulationTime = 5; //seconds
  double distance = 10; //meters
  double interBssDistance = 50; //meters
  double txMaskInnerBandMinimumRejection = -40.0; //dBr
  double txMaskOuterBandMinimumRejection = -56.0; //dBr
  double txMaskOuterBandMaximumRejection = -80.0; //dBr
  double loadBssA = 0.00002; //packets/s
  double loadBssB = 0.00002; //packets/s
 double loadBssC = 0.00002; //packets/s
  bool useDynamicChannelBonding = true;
  uint16_t channelBssA = 36;
  uint16_t channelBssB = 40;
  uint16_t channelBssC = 38;
  std::string secondaryChannelBssA = "";
  std::string secondaryChannelBssB = "";
  std::string secondaryChannelBssC = "UPPER";
  std::string mcs = "HtMcs0";
  double ccaEdThresholdPrimaryBssA = -62.0;
  double ccaEdThresholdSecondaryBssA = -62.0;
  double ccaEdThresholdPrimaryBssB = -62.0;
  double ccaEdThresholdSecondaryBssB = -62.0;
  double ccaEdThresholdPrimaryBssC = -62.0;
  double ccaEdThresholdSecondaryBssC = -62.0;

  bool verifyResults = 0; //used for regression
  double minExpectedThroughputBssA = 0; //Mbit/s
  double maxExpectedThroughputBssA = 0; //Mbit/s
  double minExpectedThroughputBssB = 0; //Mbit/s
  double maxExpectedThroughputBssB = 0; //Mbit/s
uint16_t n=2;
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
  cmd.AddValue ("channelBssC", "The selected channel for BSS B", channelBssC);
  cmd.AddValue ("secondaryChannelBssA", "The secondary channel position for BSS A: UPPER or LOWER", secondaryChannelBssA);
  cmd.AddValue ("secondaryChannelBssB", "The secondary channel position for BSS B: UPPER or LOWER", secondaryChannelBssB);
  cmd.AddValue ("secondaryChannelBssC", "The secondary channel position for BSS C: UPPER or LOWER", secondaryChannelBssC);
  cmd.AddValue ("useDynamicChannelBonding", "Enable/disable use of dynamic channel bonding", useDynamicChannelBonding);
  cmd.AddValue ("ccaEdThresholdPrimaryBssA", "The energy detection threshold on the primary channel for BSS A", ccaEdThresholdPrimaryBssA);
  cmd.AddValue ("ccaEdThresholdSecondaryBssA", "The energy detection threshold on the secondary channel for BSS A", ccaEdThresholdSecondaryBssA);
  cmd.AddValue ("ccaEdThresholdPrimaryBssB", "The energy detection threshold on the primary channel for BSS B", ccaEdThresholdPrimaryBssB);
  cmd.AddValue ("ccaEdThresholdSecondaryBssB", "The energy detection threshold on the secondary channel for BSS B", ccaEdThresholdSecondaryBssB);
  cmd.AddValue ("ccaEdThresholdPrimaryBssC", "The energy detection threshold on the primary channel for BSS C", ccaEdThresholdPrimaryBssC);
  cmd.AddValue ("ccaEdThresholdSecondaryBssC", "The energy detection threshold on the secondary channel for BSS C", ccaEdThresholdSecondaryBssB);
  cmd.AddValue ("loadBssA", "The number of packets per second for BSS A", loadBssA);
  cmd.AddValue ("loadBssB", "The number of packets per second for BSS B", loadBssB);
  cmd.AddValue ("loadBssC", "The number of packets per second for BSS C", loadBssC);
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
  wifiStaNodesA.Create (n);
  wifiStaNodesB.Create (n);
  wifiStaNodesC.Create (n);
  NodeContainer wifiApNodes;
  wifiApNodes.Create (3);

  /*SpectrumWifiPhyHelper phy = SpectrumWifiPhyHelper::Default ();
  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();
  Ptr<FriisPropagationLossModel> lossModel = CreateObject<FriisPropagationLossModel> ();
  lossModel->SetFrequency (5.180e9);
  channel->AddPropagationLossModel (lossModel);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  channel->SetPropagationDelayModel (delayModel);
  phy.SetChannel (channel);*/

  SpectrumWifiPhyHelper phy = SpectrumWifiPhyHelper::Default ();
  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();
	Ptr<LogDistancePropagationLossModel> lossModel = CreateObject<LogDistancePropagationLossModel> ();
  lossModel ->SetAttribute ("ReferenceDistance", DoubleValue (1));
  lossModel ->SetAttribute ("Exponent", DoubleValue (3.5));
 lossModel ->SetAttribute ("ReferenceLoss", DoubleValue (50));
      channel->AddPropagationLossModel (lossModel); 
phy.SetChannel (channel);


  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (mcs), "ControlMode", StringValue ("HtMcs0"));
  if (useDynamicChannelBonding)
    {
      wifi.SetChannelBondingManager ("ns3::ConstantThresholdChannelBondingManager");
    }

  NetDeviceContainer staDeviceA, staDeviceB,staDeviceC, apDeviceA, apDeviceB,apDeviceC;
  WifiMacHelper mac;
  Ssid ssid;

  // network A
  ssid = Ssid ("network-A");

  phy.Set ("CcaEdThreshold", DoubleValue (ccaEdThresholdPrimaryBssA));
  phy.Set ("CcaEdThresholdSecondary", DoubleValue (ccaEdThresholdSecondaryBssA));
  if (secondaryChannelBssA == "LOWER")
    {
      phy.Set ("SecondaryChannelOffset", EnumValue (LOWER));
    }
  else
    {
      phy.Set ("SecondaryChannelOffset", EnumValue (UPPER));
    }

  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid));
  staDeviceA = wifi.Install (phy, mac, wifiStaNodesA);

Ptr<NetDevice> staDeviceAPtr;
for (uint16_t i=0;i<n;i++) {

   staDeviceAPtr= staDeviceA.Get (i);
  Ptr<WifiNetDevice> wifiStaDeviceAPtr = staDeviceAPtr->GetObject <WifiNetDevice> ();
  wifiStaDeviceAPtr->GetPhy ()->SetChannelNumber (channelBssA);
}


  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "EnableBeaconJitter", BooleanValue (true));
  apDeviceA = wifi.Install (phy, mac, wifiApNodes.Get (0));

  Ptr<NetDevice> apDeviceAPtr = apDeviceA.Get (0);
  Ptr<WifiNetDevice> wifiApDeviceAPtr = apDeviceAPtr->GetObject <WifiNetDevice> ();
  wifiApDeviceAPtr->GetPhy ()->SetChannelNumber (channelBssA);

  // network B
  ssid = Ssid ("network-B");

  phy.Set ("CcaEdThreshold", DoubleValue (ccaEdThresholdPrimaryBssB));
  phy.Set ("CcaEdThresholdSecondary", DoubleValue (ccaEdThresholdSecondaryBssB));
  if (secondaryChannelBssB == "LOWER")
    {
      phy.Set ("SecondaryChannelOffset", EnumValue (LOWER));
    }
  else
    {
      phy.Set ("SecondaryChannelOffset", EnumValue (UPPER));
    }

  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid));
  staDeviceB = wifi.Install (phy, mac, wifiStaNodesB);


Ptr<NetDevice> staDeviceBPtr;
for (uint16_t i=0;i<n;i++) {

   staDeviceBPtr= staDeviceB.Get (i);
  Ptr<WifiNetDevice> wifiStaDeviceBPtr = staDeviceBPtr->GetObject <WifiNetDevice> ();
  wifiStaDeviceBPtr->GetPhy ()->SetChannelNumber (channelBssB);
}

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "EnableBeaconJitter", BooleanValue (true));
  apDeviceB = wifi.Install (phy, mac, wifiApNodes.Get (1));

  Ptr<NetDevice> apDeviceBPtr = apDeviceB.Get (0);
  Ptr<WifiNetDevice> wifiApDeviceBPtr = apDeviceBPtr->GetObject <WifiNetDevice> ();
  wifiApDeviceBPtr->GetPhy ()->SetChannelNumber (channelBssB);

  // network C
  ssid = Ssid ("network-C");

  phy.Set ("CcaEdThreshold", DoubleValue (ccaEdThresholdPrimaryBssC));
  phy.Set ("CcaEdThresholdSecondary", DoubleValue (ccaEdThresholdSecondaryBssC));
  if (secondaryChannelBssC == "LOWER")
    {
      phy.Set ("SecondaryChannelOffset", EnumValue (LOWER));
    }
  else
    {
      phy.Set ("SecondaryChannelOffset", EnumValue (UPPER));
    }

  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid));
  staDeviceC = wifi.Install (phy, mac, wifiStaNodesC);

Ptr<NetDevice> staDeviceCPtr;
for (uint16_t i=0;i<n;i++) {

   staDeviceCPtr= staDeviceC.Get (i);
  Ptr<WifiNetDevice> wifiStaDeviceCPtr = staDeviceCPtr->GetObject <WifiNetDevice> ();
  wifiStaDeviceCPtr->GetPhy ()->SetChannelNumber (channelBssC);
}

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "EnableBeaconJitter", BooleanValue (true));
  apDeviceC = wifi.Install (phy, mac, wifiApNodes.Get (2));

  Ptr<NetDevice> apDeviceCPtr = apDeviceC.Get (0);
  Ptr<WifiNetDevice> wifiApDeviceCPtr = apDeviceCPtr->GetObject <WifiNetDevice> ();
  wifiApDeviceCPtr->GetPhy ()->SetChannelNumber (channelBssC);







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
  mobility.Install (wifiApNodes);
  mobility.Install (wifiStaNodesA);
  mobility.Install (wifiStaNodesB);
  mobility.Install (wifiStaNodesC);

  // Internet stack
  InternetStackHelper stack;
  stack.Install (wifiApNodes);
  stack.Install (wifiStaNodesA);
  stack.Install (wifiStaNodesB);
  stack.Install (wifiStaNodesC);

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
  uint16_t port = 9;

  UdpServerHelper serverA (port);
ApplicationContainer serverAppA;
ApplicationContainer clientAppA ;

 
for (uint16_t i=0;i<n;i++){
   serverAppA= serverA.Install (wifiStaNodesA.Get (i));
  serverAppA.Start (Seconds (0.0));
  serverAppA.Stop (Seconds (simulationTime + 1));
AddClient (clientAppA, StaInterfaceA.GetAddress (i), wifiApNodes.Get (0), port, Seconds(loadBssA), payloadSize);
}
  clientAppA.Start (Seconds (1.0));
  clientAppA.Stop (Seconds (simulationTime + 1));




  UdpServerHelper serverB (port);
ApplicationContainer serverAppB;
ApplicationContainer clientAppB ;

 
for (uint16_t i=0;i<n;i++){
   serverAppB= serverA.Install (wifiStaNodesB.Get (i));
  serverAppB.Start (Seconds (0.0));
  serverAppB.Stop (Seconds (simulationTime + 1));
AddClient (clientAppB, StaInterfaceB.GetAddress (i), wifiApNodes.Get (1), port, Seconds(loadBssB), payloadSize);
}
  clientAppB.Start (Seconds (1.0));
  clientAppB.Stop (Seconds (simulationTime + 1));




  UdpServerHelper serverC (port);
ApplicationContainer serverAppC;
ApplicationContainer clientAppC ;

 
for (uint16_t i=0;i<n;i++){
   serverAppC= serverC.Install (wifiStaNodesC.Get (i));
  serverAppC.Start (Seconds (0.0));
  serverAppC.Stop (Seconds (simulationTime + 1));
AddClient (clientAppC, StaInterfaceC.GetAddress (i), wifiApNodes.Get (2), port, Seconds(loadBssC), payloadSize);
}
  clientAppC.Start (Seconds (1.0));
  clientAppC.Stop (Seconds (simulationTime + 1));

phy.EnablePcap ("staA_pcap", staDeviceA);
phy.EnablePcap ("apA_pcap", apDeviceA);
phy.EnablePcap ("staB_pcap", staDeviceB);
phy.EnablePcap ("apB_pcap", apDeviceB);
phy.EnablePcap ("staC_pcap", staDeviceC);
phy.EnablePcap ("apC_pcap", apDeviceC);


  Simulator::Stop (Seconds (simulationTime + 1));
  Simulator::Run ();

  // Show results
  uint64_t totalPacketsThroughA=DynamicCast<UdpServer> (serverAppA.Get (0))->GetReceived ();
  uint64_t totalPacketsThroughB=DynamicCast<UdpServer> (serverAppB.Get (0))->GetReceived ();
  uint64_t totalPacketsThroughC=DynamicCast<UdpServer> (serverAppC.Get (0))->GetReceived ();



  Simulator::Destroy ();

  double throughput = totalPacketsThroughA * payloadSize * 8 / (simulationTime * 1000000.0);
  std::cout << "Throughput for BSS A: " << throughput << " Mbit/s" << '\n';

  throughput = totalPacketsThroughB * payloadSize * 8 / (simulationTime * 1000000.0);
  std::cout << "Throughput for BSS B: " << throughput << " Mbit/s" << '\n';

  throughput = totalPacketsThroughC * payloadSize * 8 / (simulationTime * 1000000.0);
  std::cout << "Throughput for BSS C: " << throughput << " Mbit/s" << '\n';


  return 0;
}

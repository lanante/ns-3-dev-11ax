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
//     Throughput for BSS A: 59.4829 Mbit/s
//     Throughput for BSS B: 59.417 Mbit/s
// The throughput per network is maximum and not affected by the presence of the other network,
// since they are operating on different channels.
//
// One can run the same scenario but with a less strict transmit mask:
//     ./waf --run "wifi-channel-bonding --channelBssA=36 --channelBssB=40 --useDynamicChannelBonding=false
//     --txMaskInnerBandMinimumRejection=-20 --txMaskOuterBandMinimumRejection=-28 --txMaskOuterBandMaximumRejection=-40"
// The output gives:
//     Throughput for BSS A: 59.351 Mbit/s
//     Throughput for BSS B: 59.351 Mbit/s
// FIXME: throughput is doubled since rebase!
// The throughput per network is lower since a part of the signal is leaking in the other band, causing one network to
// declare CCA_BUSY when the other network is transmitting. As a result, the throughput is shared by the two networks.
//
// One can run a scenario where a 40 MHz channel is used for network A, while keeping network B as previously:
//     ./waf --run "wifi-channel-bonding --channelBssA=38 --channelBssB=40 --useDynamicChannelBonding=false"
// The output gives:
//     Throughput for BSS A: 21.974 Mbit/s
//     Throughput for BSS B: 0.249651 Mbit/s
// Since dynamic channel bonding is disabled, network A will always transmit on 40 MHz, regardless of
// CCA on the secondary channel. As a result, network B suffers from a lot of collisions and has a very low
// throughput. On the other hand, throughput for network A is higher since it uses channel bonding, but it is
// also impacted by transmissions on the secondary channel.
//
// One can run the previous scenario with dynamic channel bonding enabled:
//     ./waf --run "wifi-channel-bonding --channelBssA=38 --channelBssB=40 --useDynamicChannelBonding=true"
// The output gives:
//     Throughput for BSS A: 59.6172 Mbit/s
//     Throughput for BSS B: 59.2851 Mbit/s
// We can see the benefit of using a dynamic channel bonding. Since activity is detected on the secondary channel,
// network A limits its channel width to 20 MHz and this gives similar results as in the first scenario presented above.
//
// One can run a scenario where both networks make use of channel bonding:
//     ./waf --run "wifi-channel-bonding --channelBssA=38 --channelBssB=38 --useDynamicChannelBonding=false"
//     Throughput for BSS A: 53.1498 Mbit/s
//     Throughput for BSS B: 50.2906 Mbit/s
// The channel is shared with the two networks as they operate on the same channel, but since they can use both
// a 40 Mhz channel, the maximum throughput is almost doubled.

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiChannelBonding");

int main (int argc, char *argv[])
{
  uint32_t payloadSize = 1472; //bytes
  double simulationTime = 5; //seconds
  double distance = 10; //meters
  double interBssDistance = 50; //meters
  double txMaskInnerBandMinimumRejection = -40.0; //dBr
  double txMaskOuterBandMinimumRejection = -56.0; //dBr
  double txMaskOuterBandMaximumRejection = -80.0; //dBr
  double loadBssA = 6; //Mbps
  double loadBssB = 6; //Mbps
  double loadBssC = 6; //Mbps

  bool useDynamicChannelBonding = true;
  uint16_t channelBssA = 36;
  uint16_t channelBssB = 36;
  uint16_t channelBssC = 40;

  std::string secondaryChannelBssA = "";
  std::string secondaryChannelBssB = "";
 std::string secondaryChannelBssC = "";
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
  double minExpectedThroughputBssC = 0; //Mbit/s
  double maxExpectedThroughputBssC = 0; //Mbit/s

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
  cmd.AddValue ("secondaryChannelBssA", "The secondary channel position for BSS A: UPPER or LOWER", secondaryChannelBssA);
  cmd.AddValue ("secondaryChannelBssB", "The secondary channel position for BSS B: UPPER or LOWER", secondaryChannelBssB);
  cmd.AddValue ("secondaryChannelBssC", "The secondary channel position for BSS C: UPPER or LOWER", secondaryChannelBssC);
  cmd.AddValue ("useDynamicChannelBonding", "Enable/disable use of dynamic channel bonding", useDynamicChannelBonding);
  cmd.AddValue ("ccaEdThresholdPrimaryBssA", "The energy detection threshold on the primary channel for BSS A", ccaEdThresholdPrimaryBssA);
  cmd.AddValue ("ccaEdThresholdSecondaryBssA", "The energy detection threshold on the secondary channel for BSS A", ccaEdThresholdSecondaryBssA);
  cmd.AddValue ("ccaEdThresholdPrimaryBssB", "The energy detection threshold on the primary channel for BSS B", ccaEdThresholdPrimaryBssB);
  cmd.AddValue ("ccaEdThresholdSecondaryBssB", "The energy detection threshold on the secondary channel for BSS B", ccaEdThresholdSecondaryBssB);
  cmd.AddValue ("ccaEdThresholdPrimaryBssC", "The energy detection threshold on the primary channel for BSS C", ccaEdThresholdPrimaryBssC);
  cmd.AddValue ("ccaEdThresholdSecondaryBssC", "The energy detection threshold on the secondary channel for BSS C", ccaEdThresholdSecondaryBssC);

  cmd.AddValue ("loadBssA", "The number of packets per second for BSS A", loadBssA);
  cmd.AddValue ("loadBssB", "The number of packets per second for BSS B", loadBssB);
  cmd.AddValue ("loadBssC", "The number of packets per second for BSS C", loadBssC);

  cmd.AddValue ("verifyResults", "Enable/disable results verification at the end of the simulation", verifyResults);
  cmd.AddValue ("minExpectedThroughputBssA", "Minimum expected throughput for BSS A", minExpectedThroughputBssA);
  cmd.AddValue ("maxExpectedThroughputBssA", "Maximum expected throughput for BSS A", maxExpectedThroughputBssA);
  cmd.AddValue ("minExpectedThroughputBssB", "Minimum expected throughput for BSS B", minExpectedThroughputBssB);
  cmd.AddValue ("maxExpectedThroughputBssB", "Maximum expected throughput for BSS B", maxExpectedThroughputBssB);
  cmd.AddValue ("minExpectedThroughputBssC", "Minimum expected throughput for BSS C", minExpectedThroughputBssC);
  cmd.AddValue ("maxExpectedThroughputBssC", "Maximum expected throughput for BSS C", maxExpectedThroughputBssC);
  cmd.Parse (argc, argv);

  /*LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnableAll (LOG_PREFIX_NODE);

  LogComponentEnable ("MacLow", LOG_LEVEL_ALL);
  LogComponentEnable ("WifiPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("SpectrumWifiPhy", LOG_LEVEL_ALL);*/

  Config::SetDefault ("ns3::SpectrumWifiPhy::TxMaskInnerBandMinimumRejection", DoubleValue (txMaskInnerBandMinimumRejection));
  Config::SetDefault ("ns3::SpectrumWifiPhy::TxMaskOuterBandMinimumRejection", DoubleValue (txMaskOuterBandMinimumRejection));
  Config::SetDefault ("ns3::SpectrumWifiPhy::TxMaskOuterBandMaximumRejection", DoubleValue (txMaskOuterBandMaximumRejection));

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (3);
  NodeContainer wifiApNodes;
  wifiApNodes.Create (3);


  SpectrumWifiPhyHelper phy = SpectrumWifiPhyHelper::Default ();
  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();
    		Ptr<LogDistancePropagationLossModel> lossModel = CreateObject<LogDistancePropagationLossModel> ();
     // more prominent example values:
  lossModel ->SetAttribute ("ReferenceDistance", DoubleValue (1));
  lossModel ->SetAttribute ("Exponent", DoubleValue (3.5));
 lossModel ->SetAttribute ("ReferenceLoss", DoubleValue (50));
  channel->AddPropagationLossModel (lossModel);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  channel->SetPropagationDelayModel (delayModel);
  phy.SetChannel (channel);

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("HtMcs0"), "ControlMode", StringValue ("HtMcs0"));
  if (useDynamicChannelBonding)
    {
      wifi.SetChannelBondingManager ("ns3::ConstantThresholdChannelBondingManager");
    }

  NetDeviceContainer staDeviceA, staDeviceB, staDeviceC,apDeviceA, apDeviceB, apDeviceC;
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
  staDeviceA = wifi.Install (phy, mac, wifiStaNodes.Get (0));

  Ptr<NetDevice> staDeviceAPtr = staDeviceA.Get (0);
  Ptr<WifiNetDevice> wifiStaDeviceAPtr = staDeviceAPtr->GetObject <WifiNetDevice> ();
  wifiStaDeviceAPtr->GetPhy ()->SetChannelNumber (channelBssA);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "EnableBeaconJitter", BooleanValue (false));
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

  staDeviceB = wifi.Install (phy, mac, wifiStaNodes.Get (1));

  Ptr<NetDevice> staDeviceBPtr = staDeviceB.Get (0);
  Ptr<WifiNetDevice> wifiStaDeviceBPtr = staDeviceBPtr->GetObject <WifiNetDevice> ();
  wifiStaDeviceBPtr->GetPhy ()->SetChannelNumber (channelBssB);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "EnableBeaconJitter", BooleanValue (false));
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

  staDeviceC = wifi.Install (phy, mac, wifiStaNodes.Get (2));

  Ptr<NetDevice> staDeviceCPtr = staDeviceC.Get (0);
  Ptr<WifiNetDevice> wifiStaDeviceCPtr = staDeviceCPtr->GetObject <WifiNetDevice> ();
  wifiStaDeviceCPtr->GetPhy ()->SetChannelNumber (channelBssC);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "EnableBeaconJitter", BooleanValue (false));
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
  positionAlloc->Add (Vector (interBssDistance/2, -interBssDistance/2*sqrt(3), 0.0));
  // Set position for STAs
  positionAlloc->Add (Vector (0.0, distance, 0.0));
  positionAlloc->Add (Vector (interBssDistance, distance, 0.0));
  positionAlloc->Add (Vector (interBssDistance/2, -interBssDistance/2*sqrt(3)+distance, 0.0));

  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (wifiApNodes);
  mobility.Install (wifiStaNodes);



  // Internet stack
  InternetStackHelper stack;
  stack.Install (wifiApNodes);
  stack.Install (wifiStaNodes);

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
double loadBssAPs=1/(loadBssA/payloadSize/8*1000000);
double loadBssBPs=1/(loadBssB/payloadSize/8*1000000);
double loadBssCPs=1/(loadBssC/payloadSize/8*1000000);

  UdpServerHelper serverA (port);
  ApplicationContainer serverAppA = serverA.Install (wifiStaNodes.Get (0));
  serverAppA.Start (Seconds (0.0));
  serverAppA.Stop (Seconds (simulationTime + 1));

  UdpClientHelper clientA (StaInterfaceA.GetAddress (0), port);
  clientA.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
  clientA.SetAttribute ("Interval", TimeValue (Time (Seconds (loadBssAPs)))); //packets/s
  clientA.SetAttribute ("PacketSize", UintegerValue (payloadSize));

  ApplicationContainer clientAppA = clientA.Install (wifiApNodes.Get (0));
  clientAppA.Start (Seconds (1.0));
  clientAppA.Stop (Seconds (simulationTime + 1));

  UdpServerHelper serverB (port);
  ApplicationContainer serverAppB = serverB.Install (wifiStaNodes.Get (1));
  serverAppB.Start (Seconds (0.0));
  serverAppB.Stop (Seconds (simulationTime + 1));

  UdpClientHelper clientB (StaInterfaceB.GetAddress (0), port);
  clientB.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
  clientB.SetAttribute ("Interval", TimeValue (Time (Seconds (loadBssBPs)))); //packets/s
  clientB.SetAttribute ("PacketSize", UintegerValue (payloadSize));

 // ApplicationContainer clientAppB = clientB.Install (wifiApNodes.Get (1));
  //clientAppB.Start (Seconds (1.0));
  //clientAppB.Stop (Seconds (simulationTime + 1));

  UdpServerHelper serverC (port);
  ApplicationContainer serverAppC = serverC.Install (wifiStaNodes.Get (2));
  serverAppC.Start (Seconds (0.0));
  serverAppC.Stop (Seconds (simulationTime + 1));

  UdpClientHelper clientC (StaInterfaceC.GetAddress (0), port);
  clientC.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
  clientC.SetAttribute ("Interval", TimeValue (Time (Seconds (loadBssCPs)))); //packets/s
  clientC.SetAttribute ("PacketSize", UintegerValue (payloadSize));

  //ApplicationContainer clientAppC = clientC.Install (wifiApNodes.Get (2));
 // clientAppC.Start (Seconds (1.0));
//  clientAppC.Stop (Seconds (simulationTime + 1));


  Simulator::Stop (Seconds (simulationTime + 1));
  Simulator::Run ();

  // Show results
  uint64_t totalPacketsThroughA = DynamicCast<UdpServer> (serverAppA.Get (0))->GetReceived ();
  uint64_t totalPacketsThroughB = DynamicCast<UdpServer> (serverAppB.Get (0))->GetReceived ();
  uint64_t totalPacketsThroughC = DynamicCast<UdpServer> (serverAppC.Get (0))->GetReceived ();

  Simulator::Destroy ();

  double throughput = totalPacketsThroughA * payloadSize * 8 / (simulationTime * 1000000.0);
  std::cout << "Throughput for BSS A: " << throughput << " Mbit/s" << '\n';
  if (verifyResults && (throughput < minExpectedThroughputBssA || throughput > maxExpectedThroughputBssA))
    {
      NS_LOG_ERROR ("Obtained throughput for BSS A is not in the expected boundaries!");
      exit (1);
    }

  throughput = totalPacketsThroughB * payloadSize * 8 / (simulationTime * 1000000.0);
  std::cout << "Throughput for BSS B: " << throughput << " Mbit/s" << '\n';
  if (verifyResults && (throughput < minExpectedThroughputBssB || throughput > maxExpectedThroughputBssB))
    {
      NS_LOG_ERROR ("Obtained throughput for BSS B is not in the expected boundaries!");
      exit (1);
    }

  throughput = totalPacketsThroughC * payloadSize * 8 / (simulationTime * 1000000.0);
  std::cout << "Throughput for BSS C: " << throughput << " Mbit/s" << '\n';
  if (verifyResults && (throughput < minExpectedThroughputBssC || throughput > maxExpectedThroughputBssC))
    {
      NS_LOG_ERROR ("Obtained throughput for BSS C is not in the expected boundaries!");
      exit (1);
    }



  return 0;
}


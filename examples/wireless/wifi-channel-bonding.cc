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
//   - the MCS used to transmit the data packets
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
//     ./waf --run "wifi-channel-bonding --channelBssA=36 --channelBssB=40"
// The output gives:
//     Throughput for BSS A: 59.4747 Mbit/s
//     Throughput for BSS B: 59.5359 Mbit/s
// The throughput per network is maximum and not affected by the presence of the other network,
// since they are operating on different channels.
//
// One can run a scenario where a static 40 MHz channel is used for network A, while keeping network B as previously:
//     ./waf --run "wifi-channel-bonding --channelBssA=38 --channelBssB=40 --channelBondingType=Static"
// The output gives:
//     Throughput for BSS A: 67.4129 Mbit/s
//     Throughput for BSS B: 26.8057 Mbit/s
// Since this makes use of static channel bonding, network A will have to share channel 40 together with network B.
// But as network A makes use of 40 MHz channel when it transmits, it gets a higher throughput than network B that is not using channel bonding.
//
// One can run the previous scenario with constant threshold dynamic channel bonding:
//     ./waf --run "wifi-channel-bonding --channelBssA=38 --channelBssB=40 --channelBondingType=ConstantThreshold"
// The output gives:
//     Throughput for BSS A: 59.702 Mbit/s
//     Throughput for BSS B: 59.298 Mbit/s
// We can see the benefit of using a dynamic channel bonding. Since activity is detected on the secondary channel,
// network A limits its channel width to 20 MHz and this gives a better share of the spectrum.
//
// One can compare performance when using constant threshold versus dynamic threshold.
// In this case it makes use of MCS 0 and a distance of 10 meters between the two networks.
// BSS A is using dynamic channel bonding (36 + 40) and BSS A is using channel 40:
//     ./waf --run "wifi-channel-bonding --interBssDistance=10  --mcs=0 --channelBssA=38 --channelBssB=40 --primaryChannelBssA=36 --primaryChannelBssB=40 --channelBondingType=ConstantThreshold"
// The output gives:
//     Throughput for BSS A: 5.82441 Mbit/s
//     Throughput for BSS B: 5.80675 Mbit/s
//     ./waf --run "wifi-channel-bonding --interBssDistance=10  --mcs=0 --channelBssA=38 --channelBssB=40 --primaryChannelBssA=36 --primaryChannelBssB=40 --channelBondingType=DynamicThreshold"
// The output gives:
//     Throughput for BSS A: 12.0516 Mbit/s
//     Throughput for BSS B: 5.75022 Mbit/s
// Since we are using MCS 0, the SNR needed to decode the signal is quite low, so channel bonding can be used even if channel 40 is used
// by another transmission (also using MCS 0).
//
// One can run a scenario where both networks make use of static channel bonding:
//     ./waf --run "wifi-channel-bonding --channelBssA=38 --channelBssB=38 --channelBondingType=Static"
//     Throughput for BSS A: 64.3346 Mbit/s
//     Throughput for BSS B: 65.1437 Mbit/s
// The channel is shared with the two networks as they operate on the same channel, but since they can use both
// a 40 MHz channel, the maximum throughput is almost doubled.

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiChannelBonding");

int main (int argc, char *argv[])
{
  uint32_t payloadSize = 1472; //bytes
  double simulationTime = 10; //seconds
  double distance = 1; //meters
  double interBssDistance = 5; //meters
  int mcs = 7;
  double txMaskInnerBandMinimumRejection = -80.0; //dBr
  double txMaskOuterBandMinimumRejection = -112.0; //dBr
  double txMaskOuterBandMaximumRejection = -160.0; //dBr
  double loadBssA = 0.00002; //packets/s
  double loadBssB = 0.00002; //packets/s
  std::string channelBondingType = "ConstantThreshold";
  uint16_t channelBssA = 36;
  uint16_t channelBssB = 40;
  uint16_t primaryChannelBssA = 36;
  uint16_t primaryChannelBssB = 40;
  double ccaEdThresholdPrimaryBssA = -62.0;
  double constantCcaEdThresholdSecondaryBssA = -72.0;
  double ccaEdThresholdPrimaryBssB = -62.0;
  double constantCcaEdThresholdSecondaryBssB = -72.0;
  bool verifyResults = 0; //used for regression
  double minExpectedThroughputBssA = 0; //Mbit/s
  double maxExpectedThroughputBssA = 0; //Mbit/s
  double minExpectedThroughputBssB = 0; //Mbit/s
  double maxExpectedThroughputBssB = 0; //Mbit/s

  CommandLine cmd;
  cmd.AddValue ("payloadSize", "Payload size in bytes", payloadSize);
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("distance", "Distance in meters between the station and the access point", distance);
  cmd.AddValue ("interBssDistance", "Distance in meters between BSS A and BSS B", interBssDistance);
  cmd.AddValue ("mcs", "The MCS to use to transmit the data packets", mcs);
  cmd.AddValue ("txMaskInnerBandMinimumRejection", "Minimum rejection in dBr for the inner band of the transmit spectrum masks", txMaskInnerBandMinimumRejection);
  cmd.AddValue ("txMaskOuterBandMinimumRejection", "Minimum rejection in dBr for the outer band of the transmit spectrum mask", txMaskOuterBandMinimumRejection);
  cmd.AddValue ("txMaskOuterBandMaximumRejection", "Maximum rejection in dBr for the outer band of the transmit spectrum mask", txMaskOuterBandMaximumRejection);
  cmd.AddValue ("channelBssA", "The selected channel for BSS A", channelBssA);
  cmd.AddValue ("channelBssB", "The selected channel for BSS B", channelBssB);
  cmd.AddValue ("primaryChannelBssA", "The primary 20 MHz channel for BSS A", primaryChannelBssA);
  cmd.AddValue ("primaryChannelBssB", "The primary 20 MHz channel for BSS B", primaryChannelBssB);
  cmd.AddValue ("channelBondingType", "The channel bonding type: Static, ConstantThreshold or DynamicThreshold", channelBondingType);
  cmd.AddValue ("ccaEdThresholdPrimaryBssA", "The energy detection threshold on the primary channel for BSS A", ccaEdThresholdPrimaryBssA);
  cmd.AddValue ("constantCcaEdThresholdSecondaryBssA", "The energy detection threshold on the secondary channel for BSS A (only used by CT-DCB)", constantCcaEdThresholdSecondaryBssA);
  cmd.AddValue ("ccaEdThresholdPrimaryBssB", "The energy detection threshold on the primary channel for BSS B", ccaEdThresholdPrimaryBssB);
  cmd.AddValue ("constantCcaEdThresholdSecondaryBssB", "The energy detection threshold on the secondary channel for BSS B (only used by CT-DCB)", constantCcaEdThresholdSecondaryBssB);
  cmd.AddValue ("loadBssA", "The number of packets per second for BSS A", loadBssA);
  cmd.AddValue ("loadBssB", "The number of packets per second for BSS B", loadBssB);
  cmd.AddValue ("verifyResults", "Enable/disable results verification at the end of the simulation", verifyResults);
  cmd.AddValue ("minExpectedThroughputBssA", "Minimum expected throughput for BSS A", minExpectedThroughputBssA);
  cmd.AddValue ("maxExpectedThroughputBssA", "Maximum expected throughput for BSS A", maxExpectedThroughputBssA);
  cmd.AddValue ("minExpectedThroughputBssB", "Minimum expected throughput for BSS B", minExpectedThroughputBssB);
  cmd.AddValue ("maxExpectedThroughputBssB", "Maximum expected throughput for BSS B", maxExpectedThroughputBssB);
  cmd.Parse (argc, argv);

  Config::SetDefault ("ns3::SpectrumWifiPhy::TxMaskInnerBandMinimumRejection", DoubleValue (txMaskInnerBandMinimumRejection));
  Config::SetDefault ("ns3::SpectrumWifiPhy::TxMaskOuterBandMinimumRejection", DoubleValue (txMaskOuterBandMinimumRejection));
  Config::SetDefault ("ns3::SpectrumWifiPhy::TxMaskOuterBandMaximumRejection", DoubleValue (txMaskOuterBandMaximumRejection));

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (2);
  NodeContainer wifiApNodes;
  wifiApNodes.Create (2);

  SpectrumWifiPhyHelper phy = SpectrumWifiPhyHelper::Default ();
  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();
  Ptr<LogDistancePropagationLossModel> lossModel = CreateObject<LogDistancePropagationLossModel> ();
  lossModel ->SetAttribute ("ReferenceDistance", DoubleValue (1));
  lossModel ->SetAttribute ("Exponent", DoubleValue (3.5));
  lossModel ->SetAttribute ("ReferenceLoss", DoubleValue (50));
  channel->AddPropagationLossModel (lossModel);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  channel->SetPropagationDelayModel (delayModel);
  phy.SetChannel (channel);

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);

  std::ostringstream oss;
  oss << "VhtMcs" << mcs;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (oss.str ()), "ControlMode", StringValue (oss.str ()));
  wifi.SetChannelBondingManager ("ns3::" + channelBondingType + "ChannelBondingManager");

  NetDeviceContainer staDeviceA, staDeviceB, apDeviceA, apDeviceB;
  WifiMacHelper mac;
  Ssid ssid;

  // network A
  ssid = Ssid ("network-A");

  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid));
  staDeviceA = wifi.Install (phy, mac, wifiStaNodes.Get (0));

  Ptr<NetDevice> staDeviceAPtr = staDeviceA.Get (0);
  Ptr<WifiNetDevice> wifiStaDeviceAPtr = staDeviceAPtr->GetObject <WifiNetDevice> ();
  wifiStaDeviceAPtr->GetPhy ()->SetChannelNumber (channelBssA);
  wifiStaDeviceAPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssA);
  wifiStaDeviceAPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssA);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "EnableBeaconJitter", BooleanValue (false));
  apDeviceA = wifi.Install (phy, mac, wifiApNodes.Get (0));

  Ptr<NetDevice> apDeviceAPtr = apDeviceA.Get (0);
  Ptr<WifiNetDevice> wifiApDeviceAPtr = apDeviceAPtr->GetObject <WifiNetDevice> ();
  wifiApDeviceAPtr->GetPhy ()->SetChannelNumber (channelBssA);
  wifiApDeviceAPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssA);
  wifiApDeviceAPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssA);

  // network B
  ssid = Ssid ("network-B");

  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid));

  staDeviceB = wifi.Install (phy, mac, wifiStaNodes.Get (1));

  Ptr<NetDevice> staDeviceBPtr = staDeviceB.Get (0);
  Ptr<WifiNetDevice> wifiStaDeviceBPtr = staDeviceBPtr->GetObject <WifiNetDevice> ();
  wifiStaDeviceBPtr->GetPhy ()->SetChannelNumber (channelBssB);
  wifiStaDeviceBPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssB);
  wifiStaDeviceBPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssB);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "EnableBeaconJitter", BooleanValue (false));
  apDeviceB = wifi.Install (phy, mac, wifiApNodes.Get (1));

  Config::Set ("/NodeList/0/DeviceList/*/Phy/ChannelBondingManager/$ns3::ConstantThresholdChannelBondingManager/CcaEdThresholdSecondary", DoubleValue (constantCcaEdThresholdSecondaryBssA));
  Config::Set ("/NodeList/2/DeviceList/*/Phy/ChannelBondingManager/$ns3::ConstantThresholdChannelBondingManager/CcaEdThresholdSecondary", DoubleValue (constantCcaEdThresholdSecondaryBssA));
  Config::Set ("/NodeList/1/DeviceList/*/Phy/ChannelBondingManager/$ns3::ConstantThresholdChannelBondingManager/CcaEdThresholdSecondary", DoubleValue (constantCcaEdThresholdSecondaryBssB));
  Config::Set ("/NodeList/3/DeviceList/*/Phy/ChannelBondingManager/$ns3::ConstantThresholdChannelBondingManager/CcaEdThresholdSecondary", DoubleValue (constantCcaEdThresholdSecondaryBssB));

  Ptr<NetDevice> apDeviceBPtr = apDeviceB.Get (0);
  Ptr<WifiNetDevice> wifiApDeviceBPtr = apDeviceBPtr->GetObject <WifiNetDevice> ();
  wifiApDeviceBPtr->GetPhy ()->SetChannelNumber (channelBssB);
  wifiApDeviceBPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssB);
  wifiApDeviceBPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssB);

  // Setting mobility model
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // Set position for APs
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (interBssDistance, 0.0, 0.0));
  // Set position for STAs
  positionAlloc->Add (Vector (0.0, distance, 0.0));
  positionAlloc->Add (Vector (interBssDistance, distance, 0.0));

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

  // Setting applications
  uint16_t port = 9;
  UdpServerHelper serverA (port);
  ApplicationContainer serverAppA = serverA.Install (wifiApNodes.Get (0));
  serverAppA.Start (Seconds (0.0));
  serverAppA.Stop (Seconds (simulationTime + 1));

  UdpClientHelper clientA (ApInterfaceA.GetAddress (0), port);
  clientA.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
  clientA.SetAttribute ("Interval", TimeValue (Time (Seconds (loadBssA)))); //packets/s
  clientA.SetAttribute ("PacketSize", UintegerValue (payloadSize));

  ApplicationContainer clientAppA = clientA.Install (wifiStaNodes.Get (0));
  clientAppA.Start (Seconds (1.0));
  clientAppA.Stop (Seconds (simulationTime + 1));

  UdpServerHelper serverB (port);
  ApplicationContainer serverAppB = serverB.Install (wifiApNodes.Get (1));
  serverAppB.Start (Seconds (0.0));
  serverAppB.Stop (Seconds (simulationTime + 1));

  UdpClientHelper clientB (ApInterfaceB.GetAddress (0), port);
  clientB.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
  clientB.SetAttribute ("Interval", TimeValue (Time (Seconds (loadBssB)))); //packets/s
  clientB.SetAttribute ("PacketSize", UintegerValue (payloadSize));

  ApplicationContainer clientAppB = clientB.Install (wifiStaNodes.Get (1));
  clientAppB.Start (Seconds (1.0));
  clientAppB.Stop (Seconds (simulationTime + 1));

  Simulator::Stop (Seconds (simulationTime + 1));
  Simulator::Run ();

  // Show results
  uint64_t totalPacketsThroughA = DynamicCast<UdpServer> (serverAppA.Get (0))->GetReceived ();
  uint64_t totalPacketsThroughB = DynamicCast<UdpServer> (serverAppB.Get (0))->GetReceived ();

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

  return 0;
}

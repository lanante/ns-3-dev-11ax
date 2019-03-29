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

// TODO: add doc here, and comment on expected results
// TODO: add some cases to regression

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ChannelBonding");

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
  bool useDynamicChannelBonding = true;
  uint16_t maxSupportedChannelWidthBssA = 40;
  uint16_t maxSupportedChannelWidthBssB = 20;
  uint16_t channelBssA = 38;
  uint16_t channelBssB = 40;
  std::string secondaryChannelBssA = "UPPER";
  std::string secondaryChannelBssB = "";
  double ccaEdThresholdPrimaryBssA = -62.0;
  double ccaEdThresholdSecondaryBssA = -62.0;
  double ccaEdThresholdPrimaryBssB = -62.0;
  double ccaEdThresholdSecondaryBssB = -62.0;
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
  cmd.AddValue ("txMaskInnerBandMinimumRejection", "Minimum rejection in dBr for the inner band of the transmit spectrum masks", txMaskInnerBandMinimumRejection);
  cmd.AddValue ("txMaskOuterBandMinimumRejection", "Minimum rejection in dBr for the outer band of the transmit spectrum mask", txMaskOuterBandMinimumRejection);
  cmd.AddValue ("txMaskOuterBandMaximumRejection", "Maximum rejection in dBr for the outer band of the transmit spectrum mask", txMaskOuterBandMaximumRejection);
  cmd.AddValue ("channelBssA", "The selected channel for BSS A", channelBssA);
  cmd.AddValue ("channelBssB", "The selected channel for BSS B", channelBssB);
  cmd.AddValue ("secondaryChannelBssA", "The secondary channel position for BSS A: UPPER or LOWER", secondaryChannelBssA);
  cmd.AddValue ("secondaryChannelBssB", "The secondary channel position for BSS B: UPPER or LOWER", secondaryChannelBssB);
  cmd.AddValue ("useDynamicChannelBonding", "Enable/disable use of dynamic channel bonding", useDynamicChannelBonding);
  cmd.AddValue ("maxSupportedChannelWidthBssA", "The maximum support channel width in MHz for BSS A", maxSupportedChannelWidthBssA);
  cmd.AddValue ("maxSupportedChannelWidthBssB", "The maximum support channel width in MHz for BSS B", maxSupportedChannelWidthBssB);
  cmd.AddValue ("ccaEdThresholdPrimaryBssA", "The energy detection threshold on the primary channel for BSS A", ccaEdThresholdPrimaryBssA);
  cmd.AddValue ("ccaEdThresholdSecondaryBssA", "The energy detection threshold on the secondary channel for BSS A", ccaEdThresholdSecondaryBssA);
  cmd.AddValue ("ccaEdThresholdPrimaryBssB", "The energy detection threshold on the primary channel for BSS B", ccaEdThresholdPrimaryBssB);
  cmd.AddValue ("ccaEdThresholdSecondaryBssB", "The energy detection threshold on the secondary channel for BSS B", ccaEdThresholdSecondaryBssB);
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
  Ptr<FriisPropagationLossModel> lossModel = CreateObject<FriisPropagationLossModel> ();
  lossModel->SetFrequency (5.180e9);
  channel->AddPropagationLossModel (lossModel);
  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  channel->SetPropagationDelayModel (delayModel);
  phy.SetChannel (channel);

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("HtMcs7"), "ControlMode", StringValue ("HtMcs0"));
  if (useDynamicChannelBonding)
    {
      wifi.SetChannelBondingManager ("ns3::ConstantThresholdChannelBondingManager");
    }

  NetDeviceContainer staDeviceA, staDeviceB, apDeviceA, apDeviceB;
  WifiMacHelper mac;
  Ssid ssid;

  // network A
  ssid = Ssid ("network-A");

  phy.Set ("ChannelNumber", UintegerValue (channelBssA));
  phy.Set ("ChannelWidth", UintegerValue (maxSupportedChannelWidthBssA));
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

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "EnableBeaconJitter", BooleanValue (false));
  apDeviceA = wifi.Install (phy, mac, wifiApNodes.Get (0));

  // network B
  ssid = Ssid ("network-B");

  phy.Set ("ChannelNumber", UintegerValue (channelBssB));
  phy.Set ("ChannelWidth", UintegerValue (maxSupportedChannelWidthBssB));
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

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "EnableBeaconJitter", BooleanValue (false));
  apDeviceB = wifi.Install (phy, mac, wifiApNodes.Get (1));

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
  ApplicationContainer serverAppA = serverA.Install (wifiStaNodes.Get (0));
  serverAppA.Start (Seconds (0.0));
  serverAppA.Stop (Seconds (simulationTime + 1));

  UdpClientHelper clientA (StaInterfaceA.GetAddress (0), port);
  clientA.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
  clientA.SetAttribute ("Interval", TimeValue (Time (Seconds (loadBssA)))); //packets/s
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
  clientB.SetAttribute ("Interval", TimeValue (Time (Seconds (loadBssB)))); //packets/s
  clientB.SetAttribute ("PacketSize", UintegerValue (payloadSize));

  ApplicationContainer clientAppB = clientB.Install (wifiApNodes.Get (1));
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
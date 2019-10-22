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
  std::string sub = context.substr (10); // skip "/NodeList/"
  uint32_t pos = sub.find ("/Device");
  uint32_t nodeId = atoi (sub.substr (0, pos).c_str ());
  NS_LOG_DEBUG ("Found NodeId " << nodeId);
  return nodeId;
}
void
AddClient (ApplicationContainer &clientApps, Ipv4Address address, Ptr<Node> node, uint16_t port,
           Time interval, uint32_t payloadSize)
{
  UdpClientHelper client (address, port);
  client.SetAttribute ("Interval", TimeValue (interval));
  client.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
  client.SetAttribute ("PacketSize", UintegerValue (payloadSize));
  clientApps.Add (client.Install (node));
}
void
AddServer (ApplicationContainer &serverApps, UdpServerHelper &server, Ptr<Node> node)
{
  serverApps.Add (server.Install (node));
}

void
PacketRx (std::string context, const Ptr<const Packet> p, const Address &srcAddress,
          const Address &destAddress)
{
  uint32_t nodeId = ContextToNodeId (context);
  uint32_t pktSize = p->GetSize ();
  bytesReceived[nodeId] += pktSize;
  packetsReceived[nodeId]++;
}

int
main (int argc, char *argv[])
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
  uint16_t channelBssB = 36;
  uint16_t channelBssC = 36;
  uint16_t channelBssD = 36;
  uint16_t channelBssE = 36;
  uint16_t channelBssF = 36;
  uint16_t channelBssG = 36;

  uint16_t primaryChannelBssA = 36;
  uint16_t primaryChannelBssB = 36;
  uint16_t primaryChannelBssC = 36;
  uint16_t primaryChannelBssD = 36;
  uint16_t primaryChannelBssE = 36;
  uint16_t primaryChannelBssF = 36;
  uint16_t primaryChannelBssG = 36;

  std::string mcs = "";
  double ccaEdThresholdPrimaryBssA = -62.0;
  double ccaEdThresholdSecondaryBssA = -62.0;
  double ccaEdThresholdPrimaryBssB = -62.0;
  double ccaEdThresholdSecondaryBssB = -62.0;
  double ccaEdThresholdPrimaryBssC = -62.0;
  double ccaEdThresholdSecondaryBssC = -62.0;
  double ccaEdThresholdPrimaryBssD = -62.0;
  double ccaEdThresholdSecondaryBssD = -62.0;
  double ccaEdThresholdPrimaryBssE = -62.0;
  double ccaEdThresholdSecondaryBssE = -62.0;
  double ccaEdThresholdPrimaryBssF = -62.0;
  double ccaEdThresholdSecondaryBssF = -62.0;
  double ccaEdThresholdPrimaryBssG = -62.0;
  double ccaEdThresholdSecondaryBssG = -62.0;

  double aggregateDownlinkAMbps = 0;
  double aggregateDownlinkBMbps = 0;
  double aggregateDownlinkCMbps = 0;
  double aggregateDownlinkDMbps = 0;
  double aggregateDownlinkEMbps = 0;
  double aggregateDownlinkFMbps = 0;
  double aggregateDownlinkGMbps = 0;

  double aggregateUplinkAMbps = 0;
  double aggregateUplinkBMbps = 0;
  double aggregateUplinkCMbps = 0;
  double aggregateUplinkDMbps = 0;
  double aggregateUplinkEMbps = 0;
  double aggregateUplinkFMbps = 0;
  double aggregateUplinkGMbps = 0;

  uint16_t n = 1;
  uint16_t nBss = 1;
  uint32_t maxMissedBeacons = 4294967295;
  CommandLine cmd;

  cmd.AddValue ("payloadSize", "Payload size in bytes", payloadSize);
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("distance", "Distance in meters between the station and the access point",
                distance);
  cmd.AddValue ("interBssDistance", "Distance in meters between BSS A and BSS B", interBssDistance);
  cmd.AddValue ("txMaskInnerBandMinimumRejection",
                "Minimum rejection in dBr for the inner band of the transmit spectrum masks",
                txMaskInnerBandMinimumRejection);
  cmd.AddValue ("txMaskOuterBandMinimumRejection",
                "Minimum rejection in dBr for the outer band of the transmit spectrum mask",
                txMaskOuterBandMinimumRejection);
  cmd.AddValue ("txMaskOuterBandMaximumRejection",
                "Maximum rejection in dBr for the outer band of the transmit spectrum mask",
                txMaskOuterBandMaximumRejection);
  cmd.AddValue ("channelBssA", "The selected channel for BSS A", channelBssA);
  cmd.AddValue ("channelBssB", "The selected channel for BSS B", channelBssB);
  cmd.AddValue ("channelBssC", "The selected channel for BSS C", channelBssC);
  cmd.AddValue ("channelBssD", "The selected channel for BSS D", channelBssD);
  cmd.AddValue ("channelBssE", "The selected channel for BSS E", channelBssE);
  cmd.AddValue ("channelBssF", "The selected channel for BSS F", channelBssF);
  cmd.AddValue ("channelBssG", "The selected channel for BSS G", channelBssG);

  cmd.AddValue ("primaryChannelBssA", "The primary 20 MHz channel for BSS A", primaryChannelBssA);
  cmd.AddValue ("primaryChannelBssB", "The primary 20 MHz channel for BSS B", primaryChannelBssB);
  cmd.AddValue ("primaryChannelBssC", "The primary 20 MHz channel for BSS C", primaryChannelBssC);
  cmd.AddValue ("primaryChannelBssD", "The primary 20 MHz channel for BSS D", primaryChannelBssD);
  cmd.AddValue ("primaryChannelBssE", "The primary 20 MHz channel for BSS E", primaryChannelBssE);
  cmd.AddValue ("primaryChannelBssF", "The primary 20 MHz channel for BSS F", primaryChannelBssF);
  cmd.AddValue ("primaryChannelBssG", "The primary 20 MHz channel for BSS G", primaryChannelBssG);

  cmd.AddValue ("useDynamicChannelBonding", "Enable/disable use of dynamic channel bonding",
                useDynamicChannelBonding);
  cmd.AddValue ("ccaEdThresholdPrimaryBssA",
                "The energy detection threshold on the primary channel for BSS A",
                ccaEdThresholdPrimaryBssA);
  cmd.AddValue ("ccaEdThresholdSecondaryBssA",
                "The energy detection threshold on the secondary channel for BSS A",
                ccaEdThresholdSecondaryBssA);
  cmd.AddValue ("ccaEdThresholdPrimaryBssB",
                "The energy detection threshold on the primary channel for BSS B",
                ccaEdThresholdPrimaryBssB);
  cmd.AddValue ("ccaEdThresholdSecondaryBssB",
                "The energy detection threshold on the secondary channel for BSS B",
                ccaEdThresholdSecondaryBssB);
  cmd.AddValue ("ccaEdThresholdPrimaryBssC",
                "The energy detection threshold on the primary channel for BSS C",
                ccaEdThresholdPrimaryBssC);
  cmd.AddValue ("ccaEdThresholdSecondaryBssC",
                "The energy detection threshold on the secondary channel for BSS C",
                ccaEdThresholdSecondaryBssC);
  cmd.AddValue ("ccaEdThresholdPrimaryBssD",
                "The energy detection threshold on the primary channel for BSS D",
                ccaEdThresholdPrimaryBssD);
  cmd.AddValue ("ccaEdThresholdSecondaryBssD",
                "The energy detection threshold on the secondary channel for BSS D",
                ccaEdThresholdSecondaryBssD);
  cmd.AddValue ("ccaEdThresholdPrimaryBssE",
                "The energy detection threshold on the primary channel for BSS E",
                ccaEdThresholdPrimaryBssE);
  cmd.AddValue ("ccaEdThresholdSecondaryBssE",
                "The energy detection threshold on the secondary channel for BSS E",
                ccaEdThresholdSecondaryBssE);
  cmd.AddValue ("ccaEdThresholdPrimaryBssF",
                "The energy detection threshold on the primary channel for BSS F",
                ccaEdThresholdPrimaryBssF);
  cmd.AddValue ("ccaEdThresholdSecondaryBssF",
                "The energy detection threshold on the secondary channel for BSS F",
                ccaEdThresholdSecondaryBssF);
  cmd.AddValue ("ccaEdThresholdPrimaryBssG",
                "The energy detection threshold on the primary channel for BSS G",
                ccaEdThresholdPrimaryBssG);
  cmd.AddValue ("ccaEdThresholdSecondaryBssG",
                "The energy detection threshold on the secondary channel for BSS G",
                ccaEdThresholdSecondaryBssG);

  cmd.AddValue ("uplinkA", "Aggregate uplink load, BSS-A(Mbps)", aggregateUplinkAMbps);
  cmd.AddValue ("downlinkA", "Aggregate downlink load, BSS-A (Mbps)", aggregateDownlinkAMbps);
  cmd.AddValue ("uplinkB", "Aggregate uplink load, BSS-B(Mbps)", aggregateUplinkBMbps);
  cmd.AddValue ("downlinkB", "Aggregate downlink load, BSS-B (Mbps)", aggregateDownlinkBMbps);
  cmd.AddValue ("uplinkC", "Aggregate uplink load, BSS-C(Mbps)", aggregateUplinkCMbps);
  cmd.AddValue ("downlinkC", "Aggregate downlink load, BSS-C (Mbps)", aggregateDownlinkCMbps);
  cmd.AddValue ("uplinkD", "Aggregate uplink load, BSS-D(Mbps)", aggregateUplinkDMbps);
  cmd.AddValue ("downlinkD", "Aggregate downlink load, BSS-D (Mbps)", aggregateDownlinkDMbps);
  cmd.AddValue ("uplinkE", "Aggregate uplink load, BSS-E(Mbps)", aggregateUplinkEMbps);
  cmd.AddValue ("downlinkE", "Aggregate downlink load, BSS-E (Mbps)", aggregateDownlinkEMbps);
  cmd.AddValue ("uplinkF", "Aggregate uplink load, BSS-F(Mbps)", aggregateUplinkFMbps);
  cmd.AddValue ("downlinkF", "Aggregate downlink load, BSS-F (Mbps)", aggregateDownlinkFMbps);
  cmd.AddValue ("uplinkG", "Aggregate uplink load, BSS-G(Mbps)", aggregateUplinkGMbps);
  cmd.AddValue ("downlinkG", "Aggregate downlink load, BSS-G (Mbps)", aggregateDownlinkGMbps);

  cmd.AddValue ("nBss", "The number of BSS", nBss);
  cmd.AddValue ("n", "The number of STAs per BSS", n);
  cmd.AddValue ("mcs", "MCS", mcs);

  cmd.Parse (argc, argv);

  /*LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  LogComponentEnable ("MacLow", LOG_LEVEL_ALL);
  LogComponentEnable ("WifiPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("SpectrumWifiPhy", LOG_LEVEL_ALL);*/

  Config::SetDefault ("ns3::SpectrumWifiPhy::TxMaskInnerBandMinimumRejection",
                      DoubleValue (txMaskInnerBandMinimumRejection));
  Config::SetDefault ("ns3::SpectrumWifiPhy::TxMaskOuterBandMinimumRejection",
                      DoubleValue (txMaskOuterBandMinimumRejection));
  Config::SetDefault ("ns3::SpectrumWifiPhy::TxMaskOuterBandMaximumRejection",
                      DoubleValue (txMaskOuterBandMaximumRejection));

  uint32_t numNodes = nBss * (n + 1);
  packetsReceived = std::vector<uint64_t> (numNodes);
  bytesReceived = std::vector<uint64_t> (numNodes);
  NodeContainer wifiApNodes;
  wifiApNodes.Create (nBss);

  NodeContainer wifiStaNodesA;
  NodeContainer wifiStaNodesB;
  NodeContainer wifiStaNodesC;
  NodeContainer wifiStaNodesD;
  NodeContainer wifiStaNodesE;
  NodeContainer wifiStaNodesF;
  NodeContainer wifiStaNodesG;
  wifiStaNodesA.Create (n);
  double perNodeUplinkAMbps = aggregateUplinkAMbps / n;
  double perNodeDownlinkAMbps = aggregateDownlinkAMbps / n;
  Time intervalUplinkA = MicroSeconds (payloadSize * 8 / perNodeUplinkAMbps);
  Time intervalDownlinkA = MicroSeconds (payloadSize * 8 / perNodeDownlinkAMbps);

  double perNodeUplinkBMbps = aggregateUplinkBMbps / n;
  double perNodeDownlinkBMbps = aggregateDownlinkBMbps / n;
  Time intervalUplinkB = MicroSeconds (payloadSize * 8 / perNodeUplinkBMbps);
  Time intervalDownlinkB = MicroSeconds (payloadSize * 8 / perNodeDownlinkBMbps);

  double perNodeUplinkCMbps = aggregateUplinkCMbps / n;
  double perNodeDownlinkCMbps = aggregateDownlinkCMbps / n;
  Time intervalUplinkC = MicroSeconds (payloadSize * 8 / perNodeUplinkCMbps);
  Time intervalDownlinkC = MicroSeconds (payloadSize * 8 / perNodeDownlinkCMbps);

  double perNodeUplinkDMbps = aggregateUplinkDMbps / n;
  double perNodeDownlinkDMbps = aggregateDownlinkDMbps / n;
  Time intervalUplinkD = MicroSeconds (payloadSize * 8 / perNodeUplinkDMbps);
  Time intervalDownlinkD = MicroSeconds (payloadSize * 8 / perNodeDownlinkDMbps);

  double perNodeUplinkEMbps = aggregateUplinkEMbps / n;
  double perNodeDownlinkEMbps = aggregateDownlinkEMbps / n;
  Time intervalUplinkE = MicroSeconds (payloadSize * 8 / perNodeUplinkEMbps);
  Time intervalDownlinkE = MicroSeconds (payloadSize * 8 / perNodeDownlinkEMbps);

  double perNodeUplinkFMbps = aggregateUplinkFMbps / n;
  double perNodeDownlinkFMbps = aggregateDownlinkFMbps / n;
  Time intervalUplinkF = MicroSeconds (payloadSize * 8 / perNodeUplinkFMbps);
  Time intervalDownlinkF = MicroSeconds (payloadSize * 8 / perNodeDownlinkFMbps);

  double perNodeUplinkGMbps = aggregateUplinkGMbps / n;
  double perNodeDownlinkGMbps = aggregateDownlinkGMbps / n;
  Time intervalUplinkG = MicroSeconds (payloadSize * 8 / perNodeUplinkGMbps);
  Time intervalDownlinkG = MicroSeconds (payloadSize * 8 / perNodeDownlinkGMbps);
  if (nBss > 1)
    {

      wifiStaNodesB.Create (n);
    }
  if (nBss > 2)
    {

      wifiStaNodesC.Create (n);
    }
  if (nBss > 3)
    {

      wifiStaNodesD.Create (n);
    }
  if (nBss > 4)
    {

      wifiStaNodesE.Create (n);
    }
  if (nBss > 5)
    {

      wifiStaNodesF.Create (n);
    }
  if (nBss > 6)
    {

      wifiStaNodesG.Create (n);
    }

  SpectrumWifiPhyHelper phy = SpectrumWifiPhyHelper::Default ();
  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();
  Ptr<LogDistancePropagationLossModel> lossModel = CreateObject<LogDistancePropagationLossModel> ();
  lossModel->SetAttribute ("ReferenceDistance", DoubleValue (1));
  lossModel->SetAttribute ("Exponent", DoubleValue (3.5));
  lossModel->SetAttribute ("ReferenceLoss", DoubleValue (50));
  channel->AddPropagationLossModel (lossModel);
  phy.SetChannel (channel);

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);
  if (mcs == "")
    {
      wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
    }
  else
    {
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (mcs),
                                    "ControlMode", StringValue ("VhtMcs0"));
    }

  if (useDynamicChannelBonding)
    {
      wifi.SetChannelBondingManager ("ns3::ConstantThresholdChannelBondingManager");
    }

  NetDeviceContainer staDeviceA, apDeviceA;
  NetDeviceContainer staDeviceB, apDeviceB;
  NetDeviceContainer staDeviceC, apDeviceC;
  NetDeviceContainer staDeviceD, apDeviceD;
  NetDeviceContainer staDeviceE, apDeviceE;
  NetDeviceContainer staDeviceF, apDeviceF;
  NetDeviceContainer staDeviceG, apDeviceG;

  WifiMacHelper mac;
  Ssid ssid;

  // network A
  ssid = Ssid ("network-A");

  mac.SetType ("ns3::StaWifiMac", "MaxMissedBeacons", UintegerValue (maxMissedBeacons), "Ssid",
               SsidValue (ssid));
  staDeviceA = wifi.Install (phy, mac, wifiStaNodesA);

  Ptr<NetDevice> staDeviceAPtr;
  for (uint16_t i = 0; i < n; i++)
    {

      staDeviceAPtr = staDeviceA.Get (i);
      Ptr<WifiNetDevice> wifiStaDeviceAPtr = staDeviceAPtr->GetObject<WifiNetDevice> ();
      wifiStaDeviceAPtr->GetPhy ()->SetChannelNumber (channelBssA);
      wifiStaDeviceAPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssA);
      wifiStaDeviceAPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssA);
      wifiStaDeviceAPtr->GetPhy ()->SetCcaEdThresholdSecondary (ccaEdThresholdSecondaryBssA);
    }

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
               BooleanValue (true));
  apDeviceA = wifi.Install (phy, mac, wifiApNodes.Get (0));

  Ptr<NetDevice> apDeviceAPtr = apDeviceA.Get (0);
  Ptr<WifiNetDevice> wifiApDeviceAPtr = apDeviceAPtr->GetObject<WifiNetDevice> ();
  wifiApDeviceAPtr->GetPhy ()->SetChannelNumber (channelBssA);
  wifiApDeviceAPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssA);
  wifiApDeviceAPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssA);
  wifiApDeviceAPtr->GetPhy ()->SetCcaEdThresholdSecondary (ccaEdThresholdSecondaryBssA);

  if (nBss > 1)
    {
      // network B
      ssid = Ssid ("network-B");

      mac.SetType ("ns3::StaWifiMac", "MaxMissedBeacons", UintegerValue (maxMissedBeacons), "Ssid",
                   SsidValue (ssid));
      staDeviceB = wifi.Install (phy, mac, wifiStaNodesB);

      Ptr<NetDevice> staDeviceBPtr;
      for (uint16_t i = 0; i < n; i++)
        {

          staDeviceBPtr = staDeviceB.Get (i);
          Ptr<WifiNetDevice> wifiStaDeviceBPtr = staDeviceBPtr->GetObject<WifiNetDevice> ();
          wifiStaDeviceBPtr->GetPhy ()->SetChannelNumber (channelBssB);
          wifiStaDeviceBPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssB);
          wifiStaDeviceBPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssB);
          wifiStaDeviceBPtr->GetPhy ()->SetCcaEdThresholdSecondary (ccaEdThresholdSecondaryBssB);
        }

      mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
                   BooleanValue (true));
      apDeviceB = wifi.Install (phy, mac, wifiApNodes.Get (1));

      Ptr<NetDevice> apDeviceBPtr = apDeviceB.Get (0);
      Ptr<WifiNetDevice> wifiApDeviceBPtr = apDeviceBPtr->GetObject<WifiNetDevice> ();
      wifiApDeviceBPtr->GetPhy ()->SetChannelNumber (channelBssB);
      wifiApDeviceBPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssB);
      wifiApDeviceBPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssB);
      wifiApDeviceBPtr->GetPhy ()->SetCcaEdThresholdSecondary (ccaEdThresholdSecondaryBssB);
    }
  if (nBss > 2)
    {
      // network C
      ssid = Ssid ("network-C");
      mac.SetType ("ns3::StaWifiMac", "MaxMissedBeacons", UintegerValue (maxMissedBeacons), "Ssid",
                   SsidValue (ssid));
      staDeviceC = wifi.Install (phy, mac, wifiStaNodesC);

      Ptr<NetDevice> staDeviceCPtr;
      for (uint16_t i = 0; i < n; i++)
        {

          staDeviceCPtr = staDeviceC.Get (i);
          Ptr<WifiNetDevice> wifiStaDeviceCPtr = staDeviceCPtr->GetObject<WifiNetDevice> ();
          wifiStaDeviceCPtr->GetPhy ()->SetChannelNumber (channelBssC);
          wifiStaDeviceCPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssC);
          wifiStaDeviceCPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssC);
          wifiStaDeviceCPtr->GetPhy ()->SetCcaEdThresholdSecondary (ccaEdThresholdSecondaryBssC);
        }

      mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
                   BooleanValue (true));
      apDeviceC = wifi.Install (phy, mac, wifiApNodes.Get (2));

      Ptr<NetDevice> apDeviceCPtr = apDeviceC.Get (0);
      Ptr<WifiNetDevice> wifiApDeviceCPtr = apDeviceCPtr->GetObject<WifiNetDevice> ();
      wifiApDeviceCPtr->GetPhy ()->SetChannelNumber (channelBssC);
      wifiApDeviceCPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssC);
      wifiApDeviceCPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssC);
      wifiApDeviceCPtr->GetPhy ()->SetCcaEdThresholdSecondary (ccaEdThresholdSecondaryBssC);
    }
  if (nBss > 3)
    {
      // network D
      ssid = Ssid ("network-D");
      mac.SetType ("ns3::StaWifiMac", "MaxMissedBeacons", UintegerValue (maxMissedBeacons), "Ssid",
                   SsidValue (ssid));
      staDeviceD = wifi.Install (phy, mac, wifiStaNodesD);

      Ptr<NetDevice> staDeviceDPtr;
      for (uint16_t i = 0; i < n; i++)
        {

          staDeviceDPtr = staDeviceD.Get (i);
          Ptr<WifiNetDevice> wifiStaDeviceDPtr = staDeviceDPtr->GetObject<WifiNetDevice> ();
          wifiStaDeviceDPtr->GetPhy ()->SetChannelNumber (channelBssD);
          wifiStaDeviceDPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssD);
          wifiStaDeviceDPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssD);
          wifiStaDeviceDPtr->GetPhy ()->SetCcaEdThresholdSecondary (ccaEdThresholdSecondaryBssD);
        }

      mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
                   BooleanValue (true));
      apDeviceD = wifi.Install (phy, mac, wifiApNodes.Get (3));

      Ptr<NetDevice> apDeviceDPtr = apDeviceD.Get (0);
      Ptr<WifiNetDevice> wifiApDeviceDPtr = apDeviceDPtr->GetObject<WifiNetDevice> ();
      wifiApDeviceDPtr->GetPhy ()->SetChannelNumber (channelBssD);
      wifiApDeviceDPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssD);
      wifiApDeviceDPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssD);
      wifiApDeviceDPtr->GetPhy ()->SetCcaEdThresholdSecondary (ccaEdThresholdSecondaryBssD);
    }
  if (nBss > 4)
    {
      // network E
      ssid = Ssid ("network-E");
      mac.SetType ("ns3::StaWifiMac", "MaxMissedBeacons", UintegerValue (maxMissedBeacons), "Ssid",
                   SsidValue (ssid));
      staDeviceE = wifi.Install (phy, mac, wifiStaNodesE);

      Ptr<NetDevice> staDeviceEPtr;
      for (uint16_t i = 0; i < n; i++)
        {

          staDeviceEPtr = staDeviceE.Get (i);
          Ptr<WifiNetDevice> wifiStaDeviceEPtr = staDeviceEPtr->GetObject<WifiNetDevice> ();
          wifiStaDeviceEPtr->GetPhy ()->SetChannelNumber (channelBssE);
          wifiStaDeviceEPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssE);
          wifiStaDeviceEPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssE);
          wifiStaDeviceEPtr->GetPhy ()->SetCcaEdThresholdSecondary (ccaEdThresholdSecondaryBssE);
        }

      mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
                   BooleanValue (true));
      apDeviceE = wifi.Install (phy, mac, wifiApNodes.Get (4));

      Ptr<NetDevice> apDeviceEPtr = apDeviceE.Get (0);
      Ptr<WifiNetDevice> wifiApDeviceEPtr = apDeviceEPtr->GetObject<WifiNetDevice> ();
      wifiApDeviceEPtr->GetPhy ()->SetChannelNumber (channelBssE);
      wifiApDeviceEPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssE);
      wifiApDeviceEPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssE);
      wifiApDeviceEPtr->GetPhy ()->SetCcaEdThresholdSecondary (ccaEdThresholdSecondaryBssE);
    }
  if (nBss > 5)
    {
      // network F
      ssid = Ssid ("network-F");
      mac.SetType ("ns3::StaWifiMac", "MaxMissedBeacons", UintegerValue (maxMissedBeacons), "Ssid",
                   SsidValue (ssid));
      staDeviceF = wifi.Install (phy, mac, wifiStaNodesF);

      Ptr<NetDevice> staDeviceFPtr;
      for (uint16_t i = 0; i < n; i++)
        {

          staDeviceFPtr = staDeviceF.Get (i);
          Ptr<WifiNetDevice> wifiStaDeviceFPtr = staDeviceFPtr->GetObject<WifiNetDevice> ();
          wifiStaDeviceFPtr->GetPhy ()->SetChannelNumber (channelBssF);
          wifiStaDeviceFPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssF);
          wifiStaDeviceFPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssF);
          wifiStaDeviceFPtr->GetPhy ()->SetCcaEdThresholdSecondary (ccaEdThresholdSecondaryBssF);
        }

      mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
                   BooleanValue (true));
      apDeviceF = wifi.Install (phy, mac, wifiApNodes.Get (5));

      Ptr<NetDevice> apDeviceFPtr = apDeviceF.Get (0);
      Ptr<WifiNetDevice> wifiApDeviceFPtr = apDeviceFPtr->GetObject<WifiNetDevice> ();
      wifiApDeviceFPtr->GetPhy ()->SetChannelNumber (channelBssF);
      wifiApDeviceFPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssF);
      wifiApDeviceFPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssF);
      wifiApDeviceFPtr->GetPhy ()->SetCcaEdThresholdSecondary (ccaEdThresholdSecondaryBssF);
    }
  if (nBss > 6)
    {
      // network G
      ssid = Ssid ("network-G");
      mac.SetType ("ns3::StaWifiMac", "MaxMissedBeacons", UintegerValue (maxMissedBeacons), "Ssid",
                   SsidValue (ssid));
      staDeviceG = wifi.Install (phy, mac, wifiStaNodesG);

      Ptr<NetDevice> staDeviceGPtr;
      for (uint16_t i = 0; i < n; i++)
        {

          staDeviceGPtr = staDeviceG.Get (i);
          Ptr<WifiNetDevice> wifiStaDeviceGPtr = staDeviceGPtr->GetObject<WifiNetDevice> ();
          wifiStaDeviceGPtr->GetPhy ()->SetChannelNumber (channelBssG);
          wifiStaDeviceGPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssG);
          wifiStaDeviceGPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssG);
          wifiStaDeviceGPtr->GetPhy ()->SetCcaEdThresholdSecondary (ccaEdThresholdSecondaryBssG);
        }

      mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
                   BooleanValue (true));
      apDeviceG = wifi.Install (phy, mac, wifiApNodes.Get (6));

      Ptr<NetDevice> apDeviceGPtr = apDeviceG.Get (0);
      Ptr<WifiNetDevice> wifiApDeviceGPtr = apDeviceGPtr->GetObject<WifiNetDevice> ();
      wifiApDeviceGPtr->GetPhy ()->SetChannelNumber (channelBssG);
      wifiApDeviceGPtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBssG);
      wifiApDeviceGPtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBssG);
      wifiApDeviceGPtr->GetPhy ()->SetCcaEdThresholdSecondary (ccaEdThresholdSecondaryBssG);
    }

  // Setting mobility model
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // Set position for APs
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  if (nBss > 1)
    {
      positionAlloc->Add (Vector (interBssDistance, 0.0, 0.0));
    }
  if (nBss > 2)
    {
      positionAlloc->Add (Vector (interBssDistance / 2, sqrt (3) / 2 * interBssDistance, 0.0));
    }
  if (nBss > 3)
    {
      positionAlloc->Add (Vector (-interBssDistance / 2, sqrt (3) / 2 * interBssDistance, 0.0));
    }
  if (nBss > 5)
    {
      positionAlloc->Add (Vector (-interBssDistance, 0.0, 0.0));
    }
  if (nBss > 6)
    {
      positionAlloc->Add (Vector (-interBssDistance / 2, -sqrt (3) / 2 * interBssDistance, 0.0));
    }
  if (nBss > 6)
    {
      positionAlloc->Add (Vector (interBssDistance / 2, -sqrt (3) / 2 * interBssDistance, 0.0));
    }

  // Set position for STAs
  int64_t streamNumber = 100;
  Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator1 =
      CreateObject<UniformDiscPositionAllocator> ();
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

  if (nBss > 1)
    {
      Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator2 =
          CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator2->AssignStreams (streamNumber + 1);
      // AP2 is at origin (x=x2, y=y2), with radius Rho=r
      unitDiscPositionAllocator2->SetX (interBssDistance);
      unitDiscPositionAllocator2->SetY (0);
      unitDiscPositionAllocator2->SetRho (distance);
      for (uint32_t i = 0; i < n; i++)
        {
          Vector v = unitDiscPositionAllocator2->GetNext ();
          positionAlloc->Add (v);
        }
    }
  if (nBss > 2)
    {
      Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator3 =
          CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator3->AssignStreams (streamNumber + 2);
      // AP3 is at origin (x=x3, y=y3), with radius Rho=r
      unitDiscPositionAllocator3->SetX (interBssDistance / 2);
      unitDiscPositionAllocator3->SetY (sqrt (3) / 2 * interBssDistance);
      unitDiscPositionAllocator3->SetRho (distance);
      for (uint32_t i = 0; i < n; i++)
        {
          Vector v = unitDiscPositionAllocator3->GetNext ();
          positionAlloc->Add (v);
        }
    }
  if (nBss > 3)
    {
      Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator4 =
          CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator4->AssignStreams (streamNumber + 3);
      // AP4 is at origin (x=x4, y=y4), with radius Rho=r
      unitDiscPositionAllocator4->SetX (-interBssDistance / 2);
      unitDiscPositionAllocator4->SetY (sqrt (3) / 2 * interBssDistance);
      unitDiscPositionAllocator4->SetRho (distance);
      for (uint32_t i = 0; i < n; i++)
        {
          Vector v = unitDiscPositionAllocator4->GetNext ();
          positionAlloc->Add (v);
        }
    }
  if (nBss > 4)
    {
      Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator5 =
          CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator5->AssignStreams (streamNumber + 4);
      // AP5 is at origin (x=x5, y=y5), with radius Rho=r
      unitDiscPositionAllocator5->SetX (-interBssDistance);
      unitDiscPositionAllocator5->SetY (0);
      unitDiscPositionAllocator5->SetRho (distance);
      for (uint32_t i = 0; i < n; i++)
        {
          Vector v = unitDiscPositionAllocator5->GetNext ();
          positionAlloc->Add (v);
        }
    }
  if (nBss > 5)
    {
      Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator6 =
          CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator6->AssignStreams (streamNumber + 5);
      // AP6 is at origin (x=x6, y=y6), with radius Rho=r
      unitDiscPositionAllocator6->SetX (-interBssDistance / 2);
      unitDiscPositionAllocator6->SetY (-sqrt (3) / 2 * interBssDistance);
      unitDiscPositionAllocator6->SetRho (distance);
      for (uint32_t i = 0; i < n; i++)
        {
          Vector v = unitDiscPositionAllocator6->GetNext ();
          positionAlloc->Add (v);
        }
    }
  if (nBss > 6)
    {
      Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator7 =
          CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator7->AssignStreams (streamNumber + 6);
      // AP7 is at origin (x=x7, y=y7), with radius Rho=r
      unitDiscPositionAllocator7->SetX (interBssDistance / 2);
      unitDiscPositionAllocator7->SetY (-sqrt (3) / 2 * interBssDistance);
      unitDiscPositionAllocator7->SetRho (distance);
      for (uint32_t i = 0; i < n; i++)
        {
          Vector v = unitDiscPositionAllocator7->GetNext ();
          positionAlloc->Add (v);
        }
    }

  mobility.SetPositionAllocator (positionAlloc);
  NodeContainer allNodes = NodeContainer (wifiApNodes, wifiStaNodesA);
  if (nBss > 1)
    {
      allNodes = NodeContainer (allNodes, wifiStaNodesB);
    }
  if (nBss > 2)
    {
      allNodes = NodeContainer (allNodes, wifiStaNodesC);
    }
  if (nBss > 3)
    {
      allNodes = NodeContainer (allNodes, wifiStaNodesD);
    }
  if (nBss > 4)
    {
      allNodes = NodeContainer (allNodes, wifiStaNodesE);
    }
  if (nBss > 5)
    {
      allNodes = NodeContainer (allNodes, wifiStaNodesF);
    }
  if (nBss > 6)
    {
      allNodes = NodeContainer (allNodes, wifiStaNodesG);
    }

  mobility.Install (allNodes);

  // Internet stack
  InternetStackHelper stack;
  stack.Install (allNodes);

  Ipv4AddressHelper address;
  Ipv4InterfaceContainer ApInterfaceA;
  Ipv4InterfaceContainer ApInterfaceB;
  Ipv4InterfaceContainer ApInterfaceC;
  Ipv4InterfaceContainer ApInterfaceD;
  Ipv4InterfaceContainer ApInterfaceE;
  Ipv4InterfaceContainer ApInterfaceF;
  Ipv4InterfaceContainer ApInterfaceG;

  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer StaInterfaceA;
  StaInterfaceA = address.Assign (staDeviceA);
  ApInterfaceA = address.Assign (apDeviceA);

  Ipv4InterfaceContainer StaInterfaceB;
  Ipv4InterfaceContainer StaInterfaceC;
  Ipv4InterfaceContainer StaInterfaceD;
  Ipv4InterfaceContainer StaInterfaceE;
  Ipv4InterfaceContainer StaInterfaceF;
  Ipv4InterfaceContainer StaInterfaceG;

  if (nBss > 2)
    {
      address.SetBase ("192.168.2.0", "255.255.255.0");

      StaInterfaceB = address.Assign (staDeviceB);

      ApInterfaceB = address.Assign (apDeviceB);
    }
  if (nBss > 3)
    {
      address.SetBase ("192.168.3.0", "255.255.255.0");

      StaInterfaceC = address.Assign (staDeviceC);

      ApInterfaceC = address.Assign (apDeviceC);
    }
  if (nBss > 4)
    {
      address.SetBase ("192.168.4.0", "255.255.255.0");

      StaInterfaceD = address.Assign (staDeviceD);

      ApInterfaceD = address.Assign (apDeviceD);
    }
  if (nBss > 5)
    {
      address.SetBase ("192.168.5.0", "255.255.255.0");

      StaInterfaceE = address.Assign (staDeviceE);

      ApInterfaceE = address.Assign (apDeviceE);
    }
  if (nBss > 6)
    {
      address.SetBase ("192.168.6.0", "255.255.255.0");

      StaInterfaceF = address.Assign (staDeviceF);

      ApInterfaceF = address.Assign (apDeviceF);
    }
  if (nBss > 7)
    {
      address.SetBase ("192.168.7.0", "255.255.255.0");

      StaInterfaceG = address.Assign (staDeviceG);

      ApInterfaceG = address.Assign (apDeviceG);
    }

  // Setting applications

  ApplicationContainer uplinkServerApps;
  ApplicationContainer downlinkServerApps;
  ApplicationContainer uplinkClientApps;
  ApplicationContainer downlinkClientApps;

  uint16_t uplinkPortA = 9;
  uint16_t downlinkPortA = 10;
  UdpServerHelper uplinkServerA (uplinkPortA);
  UdpServerHelper downlinkServerA (downlinkPortA);
  for (uint32_t i = 0; i < n; i++)
    {
      if (aggregateUplinkAMbps > 0)
        {
          AddClient (uplinkClientApps, ApInterfaceA.GetAddress (0), wifiStaNodesA.Get (i),
                     uplinkPortA, intervalUplinkA, payloadSize);
        }
      if (aggregateDownlinkAMbps > 0)
        {
          AddClient (downlinkClientApps, StaInterfaceA.GetAddress (i), wifiApNodes.Get (0),
                     downlinkPortA, intervalDownlinkA, payloadSize);
          AddServer (downlinkServerApps, downlinkServerA, wifiStaNodesA.Get (i));
        }
    }
  if (aggregateUplinkAMbps > 0)
    {
      AddServer (uplinkServerApps, uplinkServerA, wifiApNodes.Get (0));
    }
  // BSS 2
  if (nBss > 1)
    {
      uint16_t uplinkPortB = 11;
      uint16_t downlinkPortB = 12;
      UdpServerHelper uplinkServerB (uplinkPortB);
      UdpServerHelper downlinkServerB (downlinkPortB);
      for (uint32_t i = 0; i < n; i++)
        {
          if (aggregateUplinkBMbps > 0)
            {
              AddClient (uplinkClientApps, ApInterfaceB.GetAddress (0), wifiStaNodesB.Get (i),
                         uplinkPortB, intervalUplinkB, payloadSize);
            }
          if (aggregateDownlinkBMbps > 0)
            {
              AddClient (downlinkClientApps, StaInterfaceB.GetAddress (i), wifiApNodes.Get (1),
                         downlinkPortB, intervalDownlinkB, payloadSize);
              AddServer (downlinkServerApps, downlinkServerB, wifiStaNodesB.Get (i));
            }
        }
      if (aggregateUplinkBMbps > 0)
        {
          AddServer (uplinkServerApps, uplinkServerB, wifiApNodes.Get (1));
        }
    }
  // BSS 3
  if (nBss > 2)
    {
      uint16_t uplinkPortC = 13;
      uint16_t downlinkPortC = 14;
      UdpServerHelper uplinkServerC (uplinkPortC);
      UdpServerHelper downlinkServerC (downlinkPortC);
      for (uint32_t i = 0; i < n; i++)
        {
          if (aggregateUplinkCMbps > 0)
            {
              AddClient (uplinkClientApps, ApInterfaceC.GetAddress (0), wifiStaNodesC.Get (i),
                         uplinkPortC, intervalUplinkC, payloadSize);
            }
          if (aggregateDownlinkCMbps > 0)
            {
              AddClient (downlinkClientApps, StaInterfaceC.GetAddress (i), wifiApNodes.Get (2),
                         downlinkPortC, intervalDownlinkC, payloadSize);
              AddServer (downlinkServerApps, downlinkServerC, wifiStaNodesC.Get (i));
            }
        }
      if (aggregateUplinkCMbps > 0)
        {
          AddServer (uplinkServerApps, uplinkServerC, wifiApNodes.Get (2));
        }
    }
  // BSS 4
  if (nBss > 3)
    {
      uint16_t uplinkPortD = 15;
      uint16_t downlinkPortD = 16;
      UdpServerHelper uplinkServerD (uplinkPortD);
      UdpServerHelper downlinkServerD (downlinkPortD);
      for (uint32_t i = 0; i < n; i++)
        {
          if (aggregateUplinkDMbps > 0)
            {
              AddClient (uplinkClientApps, ApInterfaceD.GetAddress (0), wifiStaNodesD.Get (i),
                         uplinkPortD, intervalUplinkD, payloadSize);
            }
          if (aggregateDownlinkDMbps > 0)
            {
              AddClient (downlinkClientApps, StaInterfaceD.GetAddress (i), wifiApNodes.Get (3),
                         downlinkPortD, intervalDownlinkD, payloadSize);
              AddServer (downlinkServerApps, downlinkServerD, wifiStaNodesD.Get (i));
            }
        }
      if (aggregateUplinkDMbps > 0)
        {
          AddServer (uplinkServerApps, uplinkServerD, wifiApNodes.Get (3));
        }
    }
  // BSS 5
  if (nBss > 4)
    {
      uint16_t uplinkPortE = 17;
      uint16_t downlinkPortE = 18;
      UdpServerHelper uplinkServerE (uplinkPortE);
      UdpServerHelper downlinkServerE (downlinkPortE);
      for (uint32_t i = 0; i < n; i++)
        {
          if (aggregateUplinkEMbps > 0)
            {
              AddClient (uplinkClientApps, ApInterfaceE.GetAddress (0), wifiStaNodesE.Get (i),
                         uplinkPortE, intervalUplinkE, payloadSize);
            }
          if (aggregateDownlinkEMbps > 0)
            {
              AddClient (downlinkClientApps, StaInterfaceE.GetAddress (i), wifiApNodes.Get (4),
                         downlinkPortE, intervalDownlinkE, payloadSize);
              AddServer (downlinkServerApps, downlinkServerE, wifiStaNodesE.Get (i));
            }
        }
      if (aggregateUplinkEMbps > 0)
        {
          AddServer (uplinkServerApps, uplinkServerE, wifiApNodes.Get (4));
        }
    }
  // BSS 6
  if (nBss > 5)
    {
      uint16_t uplinkPortF = 19;
      uint16_t downlinkPortF = 20;
      UdpServerHelper uplinkServerF (uplinkPortF);
      UdpServerHelper downlinkServerF (downlinkPortF);

      for (uint32_t i = 0; i < n; i++)
        {
          if (aggregateUplinkFMbps > 0)
            {
              AddClient (uplinkClientApps, ApInterfaceF.GetAddress (0), wifiStaNodesF.Get (i),
                         uplinkPortF, intervalUplinkF, payloadSize);
            }
          if (aggregateDownlinkFMbps > 0)
            {
              AddClient (downlinkClientApps, StaInterfaceF.GetAddress (i), wifiApNodes.Get (5),
                         downlinkPortF, intervalDownlinkF, payloadSize);
              AddServer (downlinkServerApps, downlinkServerF, wifiStaNodesF.Get (i));
            }
        }
      if (aggregateUplinkFMbps > 0)
        {
          AddServer (uplinkServerApps, uplinkServerF, wifiApNodes.Get (5));
        }
    }
  // BSS 7
  if (nBss > 6)
    {
      uint16_t uplinkPortG = 21;
      uint16_t downlinkPortG = 22;
      UdpServerHelper uplinkServerG (uplinkPortG);
      UdpServerHelper downlinkServerG (downlinkPortG);
      for (uint32_t i = 0; i < n; i++)
        {
          if (aggregateUplinkGMbps > 0)
            {
              AddClient (uplinkClientApps, ApInterfaceG.GetAddress (0), wifiStaNodesG.Get (i),
                         uplinkPortG, intervalUplinkG, payloadSize);
            }
          if (aggregateDownlinkGMbps > 0)
            {
              AddClient (downlinkClientApps, StaInterfaceG.GetAddress (i), wifiApNodes.Get (6),
                         downlinkPortG, intervalDownlinkG, payloadSize);
              AddServer (downlinkServerApps, downlinkServerG, wifiStaNodesG.Get (i));
            }
        }
      if (aggregateUplinkGMbps > 0)
        {
          AddServer (uplinkServerApps, uplinkServerG, wifiApNodes.Get (6));
        }
    }

  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::UdpServer/RxWithAddresses",
                   MakeCallback (&PacketRx));
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
  if (mcs == "")
    {
      filename = "Tput_" + std::to_string (interBssDistance) + ".csv";
    }
  else
    {
      filename = "Tput_" + mcs + ".csv";
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

  double rxThroughputPerNode[numNodes];
  // output for all nodes
  for (uint32_t k = 0; k < numNodes; k++)
    {
      double bitsReceived = bytesReceived[k] * 8;
      rxThroughputPerNode[k] = static_cast<double> (bitsReceived) / 1e6 / simulationTime;
      std::cout << "Node " << k << ", pkts " << packetsReceived[k] << ", bytes " << bytesReceived[k]
                << ", throughput [MMb/s] " << rxThroughputPerNode[k] << std::endl;
      if (k < nBss)
        {
          TputFile << rxThroughputPerNode[k] << std::endl;
        }
    }

  TputFile << std::endl;
  TputFile.close ();

  return 0;
}

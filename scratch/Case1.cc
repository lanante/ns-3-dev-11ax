

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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiChannelBonding");

void AddClient (ApplicationContainer &clientApps, Ipv4Address address, Ptr<Node> node, uint16_t port, Time interval, uint32_t payloadSize)
{
  UdpClientHelper client (address, port);
  client.SetAttribute ("Interval", TimeValue (interval ));
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
uint16_t n=1;
uint16_t i=0;
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
  cmd.AddValue ("ccaEdThresholdPrimaryBssA", "The energy detection threshold on the primary channel for BSS A", ccaEdThresholdPrimaryBssA);
  cmd.AddValue ("ccaEdThresholdSecondaryBssA", "The energy detection threshold on the secondary channel for BSS A", ccaEdThresholdSecondaryBssA);
  cmd.AddValue ("ccaEdThresholdPrimaryBssB", "The energy detection threshold on the primary channel for BSS B", ccaEdThresholdPrimaryBssB);
  cmd.AddValue ("ccaEdThresholdSecondaryBssB", "The energy detection threshold on the secondary channel for BSS B", ccaEdThresholdSecondaryBssB);
  cmd.AddValue ("loadBssA", "The number of packets per second for BSS A", loadBssA);
  cmd.AddValue ("loadBssB", "The number of packets per second for BSS B", loadBssB);
  cmd.AddValue ("loadBssC", "The number of packets per second for BSS C", loadBssC);
  cmd.AddValue ("n", "The number of STAs per BSS", n);

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
  wifiStaNodesA.Create (2);
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
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("HtMcs7"), "ControlMode", StringValue ("HtMcs0"));
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
for (i=0;i<n;i++) {
   staDeviceAPtr= staDeviceA.Get (i);
  Ptr<WifiNetDevice> wifiStaDeviceAPtr = staDeviceAPtr->GetObject <WifiNetDevice> ();
  wifiStaDeviceAPtr->GetPhy ()->SetChannelNumber (channelBssA);
}
i=0;

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

  staDeviceB = wifi.Install (phy, mac, wifiStaNodesB);

Ptr<NetDevice> staDeviceBPtr;
//for (i=0;i<n;i++) {
   staDeviceBPtr= staDeviceB.Get (i);
  Ptr<WifiNetDevice> wifiStaDeviceBPtr = staDeviceBPtr->GetObject <WifiNetDevice> ();
  wifiStaDeviceBPtr->GetPhy ()->SetChannelNumber (channelBssB);
//}

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

  staDeviceC= wifi.Install (phy, mac, wifiStaNodesC);

Ptr<NetDevice> staDeviceCPtr;
//for (i=0;i<n;i++) {
   staDeviceCPtr= staDeviceC.Get (i);
  Ptr<WifiNetDevice> wifiStaDeviceCPtr = staDeviceCPtr->GetObject <WifiNetDevice> ();
  wifiStaDeviceCPtr->GetPhy ()->SetChannelNumber (channelBssC);
//}

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
  positionAlloc->Add (Vector (interBssDistance*2, 0.0, 0.0));

  // Set position for STAs
  positionAlloc->Add (Vector (0.0, distance, 0.0));
  positionAlloc->Add (Vector (0.0, distance*2, 0.0));
 // positionAlloc->Add (Vector (0.0, distance*3, 0.0));

  positionAlloc->Add (Vector (interBssDistance, distance, 0.0));
  //positionAlloc->Add (Vector (interBssDistance, distance*2, 0.0));
  //positionAlloc->Add (Vector (interBssDistance, distance*3, 0.0));


  positionAlloc->Add (Vector (interBssDistance*2, distance, 0.0));
  //positionAlloc->Add (Vector (interBssDistance*2, distance*2, 0.0));
  //positionAlloc->Add (Vector (interBssDistance*2, distance*3, 0.0));


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
ApplicationContainer serverAppA1,serverAppA2;
ApplicationContainer clientAppA ;
//UdpClientHelper clientA;
//for (i=0;i<n;i++){

   serverAppA1= serverA.Install (wifiStaNodesA.Get (0));
  serverAppA1.Start (Seconds (0.0));
  serverAppA1.Stop (Seconds (simulationTime + 1));


 
Time intervalDownlink = MicroSeconds (loadBssA*8*payloadSize/100000);
AddClient (clientAppA, StaInterfaceA.GetAddress (0), wifiStaNodesA.Get (0), port, Time(intervalDownlink), payloadSize);
AddClient (clientAppA, StaInterfaceA.GetAddress (0), wifiStaNodesA.Get (1), port, Time(intervalDownlink), payloadSize);

   serverAppA2= serverA.Install (wifiStaNodesA.Get (1));
  serverAppA2.Start (Seconds (0.0));
  serverAppA2.Stop (Seconds (simulationTime + 1));



//}



  UdpServerHelper serverB (port);
ApplicationContainer serverAppB;
ApplicationContainer clientAppB ;
//for (i=0;i<n;i++){
   serverAppB= serverB.Install (wifiStaNodesB.Get (i));
  serverAppB.Start (Seconds (0.0));
  serverAppB.Stop (Seconds (simulationTime + 1));
  UdpClientHelper clientB (StaInterfaceB.GetAddress (i), port);
  clientB.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
  clientB.SetAttribute ("Interval", TimeValue (Time (Seconds (loadBssB)))); //packets/s
  clientB.SetAttribute ("PacketSize", UintegerValue (payloadSize));
//}
  clientAppB= clientB.Install (wifiApNodes.Get (1));
  clientAppB.Start (Seconds (1.0));
  clientAppB.Stop (Seconds (simulationTime + 1));




  UdpServerHelper serverC (port);
ApplicationContainer serverAppC;
ApplicationContainer clientAppC ;

//for (i=0;i<n;i++){
   serverAppC= serverC.Install (wifiStaNodesC.Get (i));
  serverAppC.Start (Seconds (0.0));
  serverAppC.Stop (Seconds (simulationTime + 1));
  UdpClientHelper clientC (StaInterfaceC.GetAddress (i), port);
  clientC.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
  clientC.SetAttribute ("Interval", TimeValue (Time (Seconds (loadBssC)))); //packets/s
  clientC.SetAttribute ("PacketSize", UintegerValue (payloadSize));
//}
  clientAppC= clientC.Install (wifiApNodes.Get (2));
  clientAppC.Start (Seconds (1.0));
  clientAppC.Stop (Seconds (simulationTime + 1));




  Simulator::Stop (Seconds (simulationTime + 1));
  Simulator::Run ();

  // Show results
  uint64_t totalPacketsThroughA = DynamicCast<UdpServer> (serverAppA1.Get (0))->GetReceived ();
  uint64_t totalPacketsThroughB = DynamicCast<UdpServer> (serverAppB.Get (0))->GetReceived ();
  uint64_t totalPacketsThroughC = DynamicCast<UdpServer> (serverAppC.Get (0))->GetReceived ();
  Simulator::Destroy ();

  double throughput = totalPacketsThroughA * payloadSize * 8 / (simulationTime * 1000000.0);
  std::cout << "Throughput for BSS A: " << throughput << " Mbit/s" << '\n';

  throughput = totalPacketsThroughB * payloadSize * 8 / (simulationTime * 1000000.0);
  std::cout << "Throughput for BSS B: " << throughput << " Mbit/s" << '\n';

  throughput = totalPacketsThroughC * payloadSize * 8 / (simulationTime * 1000000.0);
  std::cout << "Throughput for BSS C: " << throughput << " Mbit/s" << '\n';


  return 0;
}

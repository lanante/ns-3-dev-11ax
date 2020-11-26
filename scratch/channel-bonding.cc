 /* Copyright (c) 2019 University of Washington
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

#include "ns3/boolean.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/string.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/uinteger.h"
#include "ns3/wifi-net-device.h"
#include "ns3/random-variable-stream.h"
#include "ns3/wifi-utils.h"

// for tracking packets and bytes received. will be reallocated once we finalize
// number of nodes
std::vector<uint64_t> packetsReceived (0);
std::vector<uint64_t> bytesReceived (0);
double expn = 3.5, Pref = -30, Pn = -94, TxP = 20;

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


  double reqSinrPerMcs[12] = {0.7, 3.7, 6.2, 9.3, 12.6, 16.8, 18.2, 19.4, 23.5, 30, 35, 40};
uint16_t
selectMCS (Vector v, double InterferenceVal)
{

  uint8_t i = 0;
  double SNR = Pref - expn * 10 / 2 * log10 (v.x * v.x + v.y * v.y + v.z * v.z) - WToDbm(DbmToW(InterferenceVal)+DbmToW(Pn)) - 10;
//std::cout<<WToDbm(DbmToW(InterferenceVal)+DbmToW(Pn)) <<std::endl;
//std::cout<<SNR<<std::endl;
  for (i = 0; i < 9; i++)
    {
      if (reqSinrPerMcs[i] > SNR)
        {
          break;
        }
    }
  if (i > 0)
    {
      i = i - 1;
    }
//std::cout<<+i<<std::endl;
  return i;

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


  uint32_t payloadSize = 1472; // bytes
  double simulationTime = 20; // seconds
  double distance = 10; // meters
  double interBssDistance = 50; // meters
  double txMaskInnerBandMinimumRejection = -40.0; // dBr
  double txMaskOuterBandMinimumRejection = -56.0; // dBr
  double txMaskOuterBandMaximumRejection = -80.0; // dBr

  uint16_t channelBss=36;
  uint16_t channelBssA = 36;
  uint16_t channelBssB = 36;
  uint16_t channelBssC = 36;
  uint16_t channelBssD = 36;
  uint16_t channelBssE = 36;
  uint16_t channelBssF = 36;
  uint16_t channelBssG = 36;

uint16_t primaryChannelBss=36;
  uint16_t primaryChannelBssA = 36;
  uint16_t primaryChannelBssB = 36;
  uint16_t primaryChannelBssC = 36;
  uint16_t primaryChannelBssD = 36;
  uint16_t primaryChannelBssE = 36;
  uint16_t primaryChannelBssF = 36;
  uint16_t primaryChannelBssG = 36;

  std::string mcs ="IdealWifi";
  std::string mcs1 = "IdealWifi";
  std::string mcs2 = "IdealWifi";
  std::string mcs3 = "IdealWifi";
  std::string mcs4 = "IdealWifi";
  std::string mcs5 = "IdealWifi";
  std::string mcs6 = "IdealWifi";
  std::string mcs7 = "IdealWifi";

  double ccaEdThresholdPrimaryBss = -62.0;
  double constantCcaEdThresholdSecondaryBss = -62.0;
  double ccaEdThresholdPrimaryBssA = -62.0;
  double constantCcaEdThresholdSecondaryBssA = -62.0;
  double ccaEdThresholdPrimaryBssB = -62.0;
  double constantCcaEdThresholdSecondaryBssB = -62.0;
  double ccaEdThresholdPrimaryBssC = -62.0;
  double constantCcaEdThresholdSecondaryBssC = -62.0;
  double ccaEdThresholdPrimaryBssD = -62.0;
  double constantCcaEdThresholdSecondaryBssD = -62.0;
  double ccaEdThresholdPrimaryBssE = -62.0;
  double constantCcaEdThresholdSecondaryBssE = -62.0;
  double ccaEdThresholdPrimaryBssF = -62.0;
  double constantCcaEdThresholdSecondaryBssF = -62.0;
  double ccaEdThresholdPrimaryBssG = -62.0;
  double constantCcaEdThresholdSecondaryBssG = -62.0;

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

  std::string channelBondingType = "ConstantThreshold";
  std::string Test = "";

  uint16_t n = 1;
  uint16_t nBss = 1;
  uint32_t maxMissedBeacons = 4294967295;
  CommandLine cmd;

  cmd.AddValue ("payloadSize", "Payload size in bytes", payloadSize);
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("distance", "Distance in meters between the station and the access point",
                distance);
  cmd.AddValue ("interBssDistance", "Distance in meters between BSS A and BSS B", interBssDistance);
  cmd.AddValue ("txMaskInnerBandMinimumRejection", "Minimum rejection in dBr "
                                                   "for the inner band of the "
                                                   "transmit spectrum masks",
                txMaskInnerBandMinimumRejection);
  cmd.AddValue ("txMaskOuterBandMinimumRejection", "Minimum rejection in dBr "
                                                   "for the outer band of the "
                                                   "transmit spectrum mask",
                txMaskOuterBandMinimumRejection);
  cmd.AddValue ("txMaskOuterBandMaximumRejection", "Maximum rejection in dBr "
                                                   "for the outer band of the "
                                                   "transmit spectrum mask",
                txMaskOuterBandMaximumRejection);
  cmd.AddValue ("channelBssA", "The selected channel for BSS A", channelBssA);
  cmd.AddValue ("channelBssB", "The selected channel for BSS B", channelBssB);
  cmd.AddValue ("channelBssC", "The selected channel for BSS C", channelBssC);
  cmd.AddValue ("channelBssD", "The selected channel for BSS D", channelBssD);
  cmd.AddValue ("channelBssE", "The selected channel for BSS E", channelBssE);
  cmd.AddValue ("channelBssF", "The selected channel for BSS F", channelBssF);
  cmd.AddValue ("channelBssG", "The selected channel for BSS G", channelBssG);
  cmd.AddValue ("Test", "Test name", Test);
  cmd.AddValue ("primaryChannelBssA", "The primary 20 MHz channel for BSS A", primaryChannelBssA);
  cmd.AddValue ("primaryChannelBssB", "The primary 20 MHz channel for BSS B", primaryChannelBssB);
  cmd.AddValue ("primaryChannelBssC", "The primary 20 MHz channel for BSS C", primaryChannelBssC);
  cmd.AddValue ("primaryChannelBssD", "The primary 20 MHz channel for BSS D", primaryChannelBssD);
  cmd.AddValue ("primaryChannelBssE", "The primary 20 MHz channel for BSS E", primaryChannelBssE);
  cmd.AddValue ("primaryChannelBssF", "The primary 20 MHz channel for BSS F", primaryChannelBssF);
  cmd.AddValue ("primaryChannelBssG", "The primary 20 MHz channel for BSS G", primaryChannelBssG);
  cmd.AddValue ("channelBondingType",
                "The channel bonding type: Static, ConstantThreshold or DynamicThreshold",
                channelBondingType);

  cmd.AddValue ("ccaEdThresholdPrimaryBssA",
                "The energy detection threshold on the primary channel for BSS A",
                ccaEdThresholdPrimaryBssA);
  cmd.AddValue ("ccaEdThresholdSecondaryBssA",
                "The energy detection threshold on the secondary channel for BSS A",
                constantCcaEdThresholdSecondaryBssA);
  cmd.AddValue ("ccaEdThresholdPrimaryBssB",
                "The energy detection threshold on the primary channel for BSS B",
                ccaEdThresholdPrimaryBssB);
  cmd.AddValue ("ccaEdThresholdSecondaryBssB",
                "The energy detection threshold on the secondary channel for BSS B",
                constantCcaEdThresholdSecondaryBssB);
  cmd.AddValue ("ccaEdThresholdPrimaryBssC",
                "The energy detection threshold on the primary channel for BSS C",
                ccaEdThresholdPrimaryBssC);
  cmd.AddValue ("ccaEdThresholdSecondaryBssC",
                "The energy detection threshold on the secondary channel for BSS C",
                constantCcaEdThresholdSecondaryBssC);
  cmd.AddValue ("ccaEdThresholdPrimaryBssD",
                "The energy detection threshold on the primary channel for BSS D",
                ccaEdThresholdPrimaryBssD);
  cmd.AddValue ("ccaEdThresholdSecondaryBssD",
                "The energy detection threshold on the secondary channel for BSS D",
                constantCcaEdThresholdSecondaryBssD);
  cmd.AddValue ("ccaEdThresholdPrimaryBssE",
                "The energy detection threshold on the primary channel for BSS E",
                ccaEdThresholdPrimaryBssE);
  cmd.AddValue ("ccaEdThresholdSecondaryBssE",
                "The energy detection threshold on the secondary channel for BSS E",
                constantCcaEdThresholdSecondaryBssE);
  cmd.AddValue ("ccaEdThresholdPrimaryBssF",
                "The energy detection threshold on the primary channel for BSS F",
                ccaEdThresholdPrimaryBssF);
  cmd.AddValue ("ccaEdThresholdSecondaryBssF",
                "The energy detection threshold on the secondary channel for BSS F",
                constantCcaEdThresholdSecondaryBssF);
  cmd.AddValue ("ccaEdThresholdPrimaryBssG",
                "The energy detection threshold on the primary channel for BSS G",
                ccaEdThresholdPrimaryBssG);
  cmd.AddValue ("ccaEdThresholdSecondaryBssG",
                "The energy detection threshold on the secondary channel for BSS G",
                constantCcaEdThresholdSecondaryBssG);

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
  cmd.AddValue ("mcs1", "MCS1", mcs1);
  cmd.AddValue ("mcs2", "MCS2", mcs2);
  cmd.AddValue ("mcs3", "MCS3", mcs3);
  cmd.AddValue ("mcs4", "MCS4", mcs4);
  cmd.AddValue ("mcs5", "MCS5", mcs5);
  cmd.AddValue ("mcs6", "MCS6", mcs6);
  cmd.AddValue ("mcs7", "MCS7", mcs7);


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
  uint8_t maxMcsNode[numNodes];
  NodeContainer wifiApNodes;
  wifiApNodes.Create (nBss);

NodeContainer wifiStaNodes;
  NodeContainer wifiStaNodesA;
  NodeContainer wifiStaNodesB;
  NodeContainer wifiStaNodesC;
  NodeContainer wifiStaNodesD;
  NodeContainer wifiStaNodesE;
  NodeContainer wifiStaNodesF;
  NodeContainer wifiStaNodesG;



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

  wifiStaNodesA.Create (n);

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

  // Setting mobility model
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // Set position for APs
//  double apPositionX[7] = {0,
//                           interBssDistance,
//                           interBssDistance / 2,
//                           -interBssDistance / 2,
//                           -interBssDistance,
//                           -interBssDistance / 2,
//                           interBssDistance / 2};
 // double apPositionY[7] = {0, sqrt (3) / 2 * interBssDistance,  sqrt (3) / 2 * interBssDistance,
  //                         0, -sqrt (3) / 2 * interBssDistance, -sqrt (3) / 2 * interBssDistance};
 double apPositionX[7] = {0,interBssDistance,interBssDistance*2,interBssDistance*2,interBssDistance,-interBssDistance};
 double apPositionY[7] = {0,0,0,interBssDistance,interBssDistance,interBssDistance,interBssDistance};
 for (uint8_t i = 0; i < nBss; i++)
    {
      positionAlloc->Add (Vector (apPositionX[i], apPositionY[i], 0.0));
      maxMcsNode[i] = 0;
    }


double min = 0.0;
double max = 3.0;
Ptr<UniformRandomVariable> chRand = CreateObject<UniformRandomVariable> ();
chRand->SetAttribute ("Min", DoubleValue (min));
chRand->SetAttribute ("Max", DoubleValue (max));


/*
Ptr<UniformRandomVariable> chn = CreateObject<UniformRandomVariable> ();
if (primaryChannelBssA==4)
{
primaryChannelBssA=chRand->GetInteger ()*4+36;
std::cout<<primaryChannelBssA<<std::endl;
}
*/
double distAP1[nBss],distAP2[nBss],distAP3[nBss],distAP4[nBss],distAP5[nBss],distAP6[nBss],distAP7[nBss];
for (int i=0;i<nBss;i++)
{
distAP1[i]=(apPositionX[i]-apPositionX[0])*(apPositionX[i]-apPositionX[0])+(apPositionY[i]-apPositionY[0])*(apPositionY[i]-apPositionY[0]);
distAP2[i]=(apPositionX[i]-apPositionX[1])*(apPositionX[i]-apPositionX[1])+(apPositionY[i]-apPositionY[1])*(apPositionY[i]-apPositionY[1]);
distAP3[i]=(apPositionX[i]-apPositionX[2])*(apPositionX[i]-apPositionX[2])+(apPositionY[i]-apPositionY[2])*(apPositionY[i]-apPositionY[2]);
distAP4[i]=(apPositionX[i]-apPositionX[3])*(apPositionX[i]-apPositionX[3])+(apPositionY[i]-apPositionY[3])*(apPositionY[i]-apPositionY[3]);
distAP5[i]=(apPositionX[i]-apPositionX[4])*(apPositionX[i]-apPositionX[4])+(apPositionY[i]-apPositionY[4])*(apPositionY[i]-apPositionY[4]);
distAP6[i]=(apPositionX[i]-apPositionX[5])*(apPositionX[i]-apPositionX[5])+(apPositionY[i]-apPositionY[5])*(apPositionY[i]-apPositionY[5]);
distAP7[i]=(apPositionX[i]-apPositionX[6])*(apPositionX[i]-apPositionX[6])+(apPositionY[i]-apPositionY[6])*(apPositionY[i]-apPositionY[6]);
}



double InterferenceVector1[4]={0,0,0,0};
double InterferenceVector2[4]={0,0,0,0};
double InterferenceVector3[4]={0,0,0,0};
double InterferenceVector4[4]={0,0,0,0};
double InterferenceVector5[4]={0,0,0,0};
double InterferenceVector6[4]={0,0,0,0};
double InterferenceVector7[4]={0,0,0,0};
int priChannelX, ChannelX;
int ind;
priChannelX=primaryChannelBssA;
ChannelX=channelBssA;
if (priChannelX==4)
{
priChannelX=chRand->GetInteger ()*4+36;
if (ChannelX==1)
{ChannelX=priChannelX;}
}
primaryChannelBssA=priChannelX;
channelBssA=ChannelX;
ind=1;

InterferenceVector1[(ChannelX-36)/4]=InterferenceVector1[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP1[ind]));
InterferenceVector2[(ChannelX-36)/4]=InterferenceVector2[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP2[ind]));
InterferenceVector3[(ChannelX-36)/4]=InterferenceVector3[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP3[ind]));
InterferenceVector4[(ChannelX-36)/4]=InterferenceVector4[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP4[ind]));
InterferenceVector5[(ChannelX-36)/4]=InterferenceVector5[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP5[ind]));
InterferenceVector6[(ChannelX-36)/4]=InterferenceVector6[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP6[ind]));
InterferenceVector7[(ChannelX-36)/4]=InterferenceVector7[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP7[ind]));

if (nBss>1){
priChannelX=primaryChannelBssB;
ChannelX=channelBssB;
if (priChannelX==4)
{
priChannelX=chRand->GetInteger ()*4+36;
if (ChannelX==1)
{ChannelX=priChannelX;}
}
primaryChannelBssB=priChannelX;
channelBssB=ChannelX;
ind=1;

InterferenceVector1[(ChannelX-36)/4]=InterferenceVector1[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP1[ind]));
InterferenceVector2[(ChannelX-36)/4]=InterferenceVector2[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP2[ind]));
InterferenceVector3[(ChannelX-36)/4]=InterferenceVector3[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP3[ind]));
InterferenceVector4[(ChannelX-36)/4]=InterferenceVector4[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP4[ind]));
InterferenceVector5[(ChannelX-36)/4]=InterferenceVector5[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP5[ind]));
InterferenceVector6[(ChannelX-36)/4]=InterferenceVector6[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP6[ind]));
InterferenceVector7[(ChannelX-36)/4]=InterferenceVector7[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP7[ind]));
}

if (nBss>2){
priChannelX=primaryChannelBssC;
ChannelX=channelBssC;
if (priChannelX==4)
{
priChannelX=chRand->GetInteger ()*4+36;
if (ChannelX==1)
{ChannelX=priChannelX;}
}
primaryChannelBssC=priChannelX;
channelBssC=ChannelX;
ind=2;

InterferenceVector1[(ChannelX-36)/4]=InterferenceVector1[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP1[ind]));
InterferenceVector2[(ChannelX-36)/4]=InterferenceVector2[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP2[ind]));
InterferenceVector3[(ChannelX-36)/4]=InterferenceVector3[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP3[ind]));
InterferenceVector4[(ChannelX-36)/4]=InterferenceVector4[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP4[ind]));
InterferenceVector5[(ChannelX-36)/4]=InterferenceVector5[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP5[ind]));
InterferenceVector6[(ChannelX-36)/4]=InterferenceVector6[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP6[ind]));
InterferenceVector7[(ChannelX-36)/4]=InterferenceVector7[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP7[ind]));
}

if (nBss>3){
priChannelX=primaryChannelBssD;
ChannelX=channelBssD;
if (priChannelX==4)
{
priChannelX=chRand->GetInteger ()*4+36;
if (ChannelX==1)
{ChannelX=priChannelX;}
}
primaryChannelBssD=priChannelX;
channelBssD=ChannelX;
ind=3;

InterferenceVector1[(ChannelX-36)/4]=InterferenceVector1[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP1[ind]));
InterferenceVector2[(ChannelX-36)/4]=InterferenceVector2[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP2[ind]));
InterferenceVector3[(ChannelX-36)/4]=InterferenceVector3[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP3[ind]));
InterferenceVector4[(ChannelX-36)/4]=InterferenceVector4[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP4[ind]));
InterferenceVector5[(ChannelX-36)/4]=InterferenceVector5[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP5[ind]));
InterferenceVector6[(ChannelX-36)/4]=InterferenceVector6[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP6[ind]));
InterferenceVector7[(ChannelX-36)/4]=InterferenceVector7[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP7[ind]));
}

if (nBss>4){
priChannelX=primaryChannelBssE;
ChannelX=channelBssE;
if (priChannelX==4)
{
priChannelX=chRand->GetInteger ()*4+36;
if (ChannelX==1)
{ChannelX=priChannelX;}
}
primaryChannelBssE=priChannelX;
channelBssE=ChannelX;
ind=4;

InterferenceVector1[(ChannelX-36)/4]=InterferenceVector1[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP1[ind]));
InterferenceVector2[(ChannelX-36)/4]=InterferenceVector2[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP2[ind]));
InterferenceVector3[(ChannelX-36)/4]=InterferenceVector3[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP3[ind]));
InterferenceVector4[(ChannelX-36)/4]=InterferenceVector4[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP4[ind]));
InterferenceVector5[(ChannelX-36)/4]=InterferenceVector5[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP5[ind]));
InterferenceVector6[(ChannelX-36)/4]=InterferenceVector6[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP6[ind]));
InterferenceVector7[(ChannelX-36)/4]=InterferenceVector7[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP7[ind]));
}

if (nBss>5){
priChannelX=primaryChannelBssF;
ChannelX=channelBssF;
if (priChannelX==4)
{
priChannelX=chRand->GetInteger ()*4+36;
if (ChannelX==1)
{ChannelX=priChannelX;}
}
primaryChannelBssF=priChannelX;
channelBssF=ChannelX;
ind=5;

InterferenceVector1[(ChannelX-36)/4]=InterferenceVector1[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP1[ind]));
InterferenceVector2[(ChannelX-36)/4]=InterferenceVector2[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP2[ind]));
InterferenceVector3[(ChannelX-36)/4]=InterferenceVector3[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP3[ind]));
InterferenceVector4[(ChannelX-36)/4]=InterferenceVector4[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP4[ind]));
InterferenceVector5[(ChannelX-36)/4]=InterferenceVector5[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP5[ind]));
InterferenceVector6[(ChannelX-36)/4]=InterferenceVector6[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP6[ind]));
InterferenceVector7[(ChannelX-36)/4]=InterferenceVector7[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP7[ind]));
}


if (nBss>1){
priChannelX=primaryChannelBssG;
ChannelX=channelBssG;
if (priChannelX==4)
{
priChannelX=chRand->GetInteger ()*4+36;
if (ChannelX==1)
{ChannelX=priChannelX;}
}
primaryChannelBssG=priChannelX;
channelBssG=ChannelX;
ind=6;

InterferenceVector1[(ChannelX-36)/4]=InterferenceVector1[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP1[ind]));
InterferenceVector2[(ChannelX-36)/4]=InterferenceVector2[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP2[ind]));
InterferenceVector3[(ChannelX-36)/4]=InterferenceVector3[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP3[ind]));
InterferenceVector4[(ChannelX-36)/4]=InterferenceVector4[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP4[ind]));
InterferenceVector5[(ChannelX-36)/4]=InterferenceVector5[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP5[ind]));
InterferenceVector6[(ChannelX-36)/4]=InterferenceVector6[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP6[ind]));
InterferenceVector7[(ChannelX-36)/4]=InterferenceVector7[(ChannelX-36)/4]+DbmToW(Pref-expn * 10 / 2 * log10(distAP7[ind]));
}



double maxSecInterference1=-200;
double maxSecInterference2=-200;
double maxSecInterference3=-200;
double maxSecInterference4=-200;
double maxSecInterference5=-200;
double maxSecInterference6=-200;
double maxSecInterference7=-200;
double maxSecInterferenceX;
double *InterferenceVectorX;

InterferenceVectorX=InterferenceVector1;
maxSecInterferenceX=maxSecInterference1;
for (int i=1;i<4;i++){
if (WToDbm(InterferenceVectorX[i])>=maxSecInterferenceX)
{
maxSecInterferenceX=WToDbm(InterferenceVectorX[i]);
}
}
maxSecInterference1=maxSecInterferenceX;

InterferenceVectorX=InterferenceVector2;
maxSecInterferenceX=maxSecInterference2;
for (int i=1;i<4;i++){
if (WToDbm(InterferenceVectorX[i])>=maxSecInterferenceX)
{
maxSecInterferenceX=WToDbm(InterferenceVectorX[i]);
}
}
maxSecInterference2=maxSecInterferenceX;

InterferenceVectorX=InterferenceVector3;
maxSecInterferenceX=maxSecInterference3;
for (int i=1;i<4;i++){
if (WToDbm(InterferenceVectorX[i])>=maxSecInterferenceX)
{
maxSecInterferenceX=WToDbm(InterferenceVectorX[i]);
}
}
maxSecInterference3=maxSecInterferenceX;

InterferenceVectorX=InterferenceVector4;
maxSecInterferenceX=maxSecInterference4;
for (int i=1;i<4;i++){
if (WToDbm(InterferenceVectorX[i])>=maxSecInterferenceX)
{
maxSecInterferenceX=WToDbm(InterferenceVectorX[i]);
}
}
maxSecInterference4=maxSecInterferenceX;

InterferenceVectorX=InterferenceVector5;
maxSecInterferenceX=maxSecInterference5;
for (int i=1;i<4;i++){
if (WToDbm(InterferenceVectorX[i])>=maxSecInterferenceX)
{
maxSecInterferenceX=WToDbm(InterferenceVectorX[i]);
}
}
maxSecInterference5=maxSecInterferenceX;

InterferenceVectorX=InterferenceVector6;
maxSecInterferenceX=maxSecInterference6;
for (int i=1;i<4;i++){
if (WToDbm(InterferenceVectorX[i])>=maxSecInterferenceX)
{
maxSecInterferenceX=WToDbm(InterferenceVectorX[i]);
}
}
maxSecInterference6=maxSecInterferenceX;

InterferenceVectorX=InterferenceVector7;
maxSecInterferenceX=maxSecInterference7;
for (int i=1;i<4;i++){
if (WToDbm(InterferenceVectorX[i])>=maxSecInterferenceX)
{
maxSecInterferenceX=WToDbm(InterferenceVectorX[i]);
}
}
maxSecInterference7=maxSecInterferenceX;



//std::cout<<channelBssA<<" "<<channelBssB<<" "<<channelBssC<<" "<<channelBssD<<" "<<channelBssE<<" "<<channelBssF<<std::endl;
//std::cout<<primaryChannelBssA<<" "<<primaryChannelBssB<<" "<<primaryChannelBssC<<" "<<primaryChannelBssD<<" "<<primaryChannelBssE<<" "<<primaryChannelBssF<<std::endl;
//std::cout<<WToDbm(InterferenceVector[0])<<" "<<WToDbm(InterferenceVector[1])<<" "<<WToDbm(InterferenceVector[2])<<" "<<WToDbm(InterferenceVector[3])<<std::endl;
//std::cout<<maxSecInterference1<<" max secInterference"<<std::endl;
//std::cout<<maxInterference1<<" max Interference"<<std::endl;
//std::cout<<priInterference<<" "<<"0"<<std::endl;

double InterferenceVal1=WToDbm(0);
double InterferenceVal2=WToDbm(0); 
double InterferenceVal3=WToDbm(0); 
double InterferenceVal4=WToDbm(0); 
double InterferenceVal5=WToDbm(0);
double InterferenceVal6=WToDbm(0);
double InterferenceVal7=WToDbm(0);
if (mcs1=="proposedMcs")
{
InterferenceVal1=maxSecInterference1;
}
if (mcs2=="proposedMcs")
{
InterferenceVal2=maxSecInterference2;
}
if (mcs3=="proposedMcs")
{
InterferenceVal3=maxSecInterference3;
}
if (mcs4=="proposedMcs")
{
InterferenceVal4=maxSecInterference4;
}
if (mcs5=="proposedMcs")
{
InterferenceVal5=maxSecInterference5;
}
if (mcs6=="proposedMcs")
{
InterferenceVal6=maxSecInterference6;
}
if (mcs7=="proposedMcs")
{
InterferenceVal7=maxSecInterference7;
}

  // Set position for STAs
  int64_t streamNumber = 100;
  Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator1 =
      CreateObject<UniformDiscPositionAllocator> ();
  unitDiscPositionAllocator1->AssignStreams (streamNumber);
  // AP1 is at origin (x=x1, y=y1), with radius Rho=r
  unitDiscPositionAllocator1->SetX (apPositionX[0]);
  unitDiscPositionAllocator1->SetY (apPositionY[0]);
  unitDiscPositionAllocator1->SetRho (distance);
  for (uint32_t i = 0; i < n; i++)
    {
      Vector v = unitDiscPositionAllocator1->GetNext ();
      positionAlloc->Add (v);
      maxMcsNode[i + nBss] = selectMCS (Vector (v.x - apPositionX[0], v.y - apPositionY[0], v.z),InterferenceVal1);
    }

  if (nBss > 1)
    {
      Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator2 =
          CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator2->AssignStreams (streamNumber + 1);
      // AP2 is at origin (x=x2, y=y2), with radius Rho=r
      unitDiscPositionAllocator2->SetX (apPositionX[1]);
      unitDiscPositionAllocator2->SetY (apPositionY[1]);
      unitDiscPositionAllocator2->SetRho (distance);
      for (uint32_t i = 0; i < n; i++)
        {
          Vector v = unitDiscPositionAllocator2->GetNext ();
          positionAlloc->Add (v);
          maxMcsNode[i + nBss + n * 1] =
              selectMCS (Vector (v.x - apPositionX[1], v.y - apPositionY[1], v.z),InterferenceVal2);
        }
    }

  if (nBss > 2)
    {
      Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator3 =
          CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator3->AssignStreams (streamNumber + 2);
      // AP3 is at origin (x=x3, y=y3), with radius Rho=r
      unitDiscPositionAllocator3->SetX (apPositionX[2]);
      unitDiscPositionAllocator3->SetY (apPositionY[2]);
      unitDiscPositionAllocator3->SetRho (distance);
      for (uint32_t i = 0; i < n; i++)
        {
          Vector v = unitDiscPositionAllocator3->GetNext ();
          positionAlloc->Add (v);
          maxMcsNode[i + nBss + n * 2] =
              selectMCS (Vector (v.x - apPositionX[2], v.y - apPositionY[2], v.z),InterferenceVal3);
        }
    }

  if (nBss > 3)
    {
      Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator4 =
          CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator4->AssignStreams (streamNumber + 3);
      // AP4 is at origin (x=x4, y=y4), with radius Rho=r
      unitDiscPositionAllocator4->SetX (apPositionX[3]);
      unitDiscPositionAllocator4->SetY (apPositionY[3]);
      unitDiscPositionAllocator4->SetRho (distance);
      for (uint32_t i = 0; i < n; i++)
        {
          Vector v = unitDiscPositionAllocator4->GetNext ();
          positionAlloc->Add (v);
          maxMcsNode[i + nBss + n * 3] =
              selectMCS (Vector (v.x - apPositionX[3], v.y - apPositionY[3], v.z),InterferenceVal4);
        }
    }
  if (nBss > 4)
    {
      Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator5 =
          CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator5->AssignStreams (streamNumber + 4);
      // AP5 is at origin (x=x5, y=y5), with radius Rho=r
      unitDiscPositionAllocator5->SetX (apPositionX[4]);
      unitDiscPositionAllocator5->SetY (apPositionY[4]);
      unitDiscPositionAllocator5->SetRho (distance);
      for (uint32_t i = 0; i < n; i++)
        {
          Vector v = unitDiscPositionAllocator5->GetNext ();
          positionAlloc->Add (v);
          maxMcsNode[i + nBss + n * 4] =
              selectMCS (Vector (v.x - apPositionX[4], v.y - apPositionY[4], v.z),InterferenceVal5);
        }
    }
  if (nBss > 5)
    {
      Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator6 =
          CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator6->AssignStreams (streamNumber + 5);
      // AP6 is at origin (x=x6, y=y6), with radius Rho=r
      unitDiscPositionAllocator6->SetX (apPositionX[5]);
      unitDiscPositionAllocator6->SetY (apPositionY[5]);
      unitDiscPositionAllocator6->SetRho (distance);
      for (uint32_t i = 0; i < n; i++)
        {
          Vector v = unitDiscPositionAllocator6->GetNext ();
          positionAlloc->Add (v);
          maxMcsNode[i + nBss + n * 5] =
              selectMCS (Vector (v.x - apPositionX[5], v.y - apPositionY[5], v.z),InterferenceVal6);
        }
    }
  if (nBss > 6)
    {
      Ptr<UniformDiscPositionAllocator> unitDiscPositionAllocator7 =
          CreateObject<UniformDiscPositionAllocator> ();
      unitDiscPositionAllocator7->AssignStreams (streamNumber + 6);
      // AP7 is at origin (x=x7, y=y7), with radius Rho=r
      unitDiscPositionAllocator7->SetX (apPositionX[6]);
      unitDiscPositionAllocator7->SetY (apPositionY[6]);
      unitDiscPositionAllocator7->SetRho (distance);
      for (uint32_t i = 0; i < n; i++)
        {
          Vector v = unitDiscPositionAllocator7->GetNext ();
          positionAlloc->Add (v);
          maxMcsNode[i + nBss + n * 6] =
              selectMCS (Vector (v.x - apPositionX[6], v.y - apPositionY[6], v.z),InterferenceVal7);
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

  SpectrumWifiPhyHelper phy = SpectrumWifiPhyHelper::Default ();
  phy.Set ("TxPowerStart", DoubleValue (TxP));
  phy.Set ("TxPowerEnd", DoubleValue (TxP));

  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();
  Ptr<LogDistancePropagationLossModel> lossModel = CreateObject<LogDistancePropagationLossModel> ();
  lossModel->SetAttribute ("ReferenceDistance", DoubleValue (1));
  lossModel->SetAttribute ("Exponent", DoubleValue (expn));
  lossModel->SetAttribute ("ReferenceLoss", DoubleValue (TxP - Pref));
  channel->AddPropagationLossModel (lossModel);
  phy.SetChannel (channel);

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);
  std::ostringstream ossMcs;
  // ossMcs << 0;
  std::string dataRate;
  // dataRate = "VhtMcs" + ossMcs.str ();
/*
  if (mcs == "IdealWifi")
    {
      wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
    }
  else if (mcs == "MaxMcs")
    {
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                    StringValue ("VhtMcs0"), "ControlMode",
                                    StringValue ("VhtMcs0"));
    }
  else
    {
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (mcs1),
                                    "ControlMode", StringValue ("VhtMcs0"));
    }
*/
double offset=12;
if (constantCcaEdThresholdSecondaryBssA==0)
{
std::cout<<"Changed CCAED threshold of A from "<<constantCcaEdThresholdSecondaryBssA<<" to "<<maxSecInterference1-reqSinrPerMcs[(int)mcs1[6] - 48]+offset<<std::endl;
constantCcaEdThresholdSecondaryBssA=maxSecInterference1-reqSinrPerMcs[(int)mcs1[6] - 48]+offset;
}
if (constantCcaEdThresholdSecondaryBssB==0)
{
std::cout<<"Changed CCAED threshold of B from "<<constantCcaEdThresholdSecondaryBssB<<" to "<<maxSecInterference1-reqSinrPerMcs[(int)mcs2[6] - 48]+offset<<std::endl;
constantCcaEdThresholdSecondaryBssB=maxSecInterference1-reqSinrPerMcs[(int)mcs2[6] - 48]+offset;
}
if (constantCcaEdThresholdSecondaryBssC==0)
{
std::cout<<"Changed CCAED threshold of C from "<<constantCcaEdThresholdSecondaryBssA<<" to "<<maxSecInterference1-reqSinrPerMcs[(int)mcs3[6] - 48]+offset<<std::endl;
constantCcaEdThresholdSecondaryBssC=maxSecInterference1-reqSinrPerMcs[(int)mcs3[6] - 48]+offset;
}
if (constantCcaEdThresholdSecondaryBssD==0)
{
std::cout<<"Changed CCAED threshold of D from "<<constantCcaEdThresholdSecondaryBssA<<" to "<<maxSecInterference1-reqSinrPerMcs[(int)mcs4[6] - 48]+offset<<std::endl;
constantCcaEdThresholdSecondaryBssD=maxSecInterference1-reqSinrPerMcs[(int)mcs4[6] - 48]+offset;
}
if (constantCcaEdThresholdSecondaryBssE==0)
{
std::cout<<"Changed CCAED threshold of E from "<<constantCcaEdThresholdSecondaryBssA<<" to "<<maxSecInterference1-reqSinrPerMcs[(int)mcs5[6] - 48]+offset<<std::endl;
constantCcaEdThresholdSecondaryBssE=maxSecInterference1-reqSinrPerMcs[(int)mcs5[6] - 48]+offset;
}
if (constantCcaEdThresholdSecondaryBssF==0)
{
std::cout<<"Changed CCAED threshold of F from "<<constantCcaEdThresholdSecondaryBssA<<" to "<<maxSecInterference1-reqSinrPerMcs[(int)mcs6[6] - 48]+offset<<std::endl;
constantCcaEdThresholdSecondaryBssF=maxSecInterference1-reqSinrPerMcs[(int)mcs6[6] - 48]+offset;
}
if (constantCcaEdThresholdSecondaryBssG==0)
{
std::cout<<"Changed CCAED threshold of G from "<<constantCcaEdThresholdSecondaryBssA<<" to "<<maxSecInterference1-reqSinrPerMcs[(int)mcs7[6] - 48]+offset<<std::endl;
constantCcaEdThresholdSecondaryBssG=maxSecInterference1-reqSinrPerMcs[(int)mcs7[6] - 48]+offset;
}

  wifi.SetChannelBondingManager ("ns3::" + channelBondingType + "ChannelBondingManager");

  NetDeviceContainer staDeviceA, apDeviceA;
  NetDeviceContainer staDeviceB, apDeviceB;
  NetDeviceContainer staDeviceC, apDeviceC;
  NetDeviceContainer staDeviceD, apDeviceD;
  NetDeviceContainer staDeviceE, apDeviceE;
  NetDeviceContainer staDeviceF, apDeviceF;
  NetDeviceContainer staDeviceG, apDeviceG;

  WifiMacHelper mac;
  Ssid ssid;
  uint16_t bss_i;
  uint16_t start_i;
  uint16_t end_i;
  std::stringstream stmp;
  // network A
  ssid = Ssid ("network-A");
  bss_i = 1;
mcs=mcs1;

wifiStaNodes=wifiStaNodesA;
primaryChannelBss=primaryChannelBssA;
channelBss=channelBssA;
ccaEdThresholdPrimaryBss=ccaEdThresholdPrimaryBssA;
constantCcaEdThresholdSecondaryBss = constantCcaEdThresholdSecondaryBssA;


  mac.SetType ("ns3::StaWifiMac", "MaxMissedBeacons", UintegerValue (maxMissedBeacons), "Ssid",
               SsidValue (ssid));
  if (mcs == "IdealWifi")
    {
      wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
          staDeviceA = wifi.Install (phy, mac, wifiStaNodes);
    }

else if (mcs == "MaxMcs"||mcs=="proposedMcs")
    {

      for (uint8_t i = 0; i < n; i++)
        {
          ossMcs.str (std::to_string (maxMcsNode[i + nBss]));
          dataRate = "VhtMcs" + ossMcs.str ();

          wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                        StringValue (dataRate), "ControlMode",
                                        StringValue ("VhtMcs0"));
          staDeviceA.Add (wifi.Install (phy, mac, wifiStaNodes.Get (i)));
        }
    }
  else
    {
     wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (mcs),
                                    "ControlMode", StringValue ("VhtMcs0"));
          staDeviceA = wifi.Install (phy, mac, wifiStaNodes);
    }

  Ptr<NetDevice> staDevicePtr;
  for (uint16_t i = 0; i < n; i++)
    {

      staDevicePtr = staDeviceA.Get (i);
      Ptr<WifiNetDevice> wifiStaDevicePtr = staDevicePtr->GetObject<WifiNetDevice> ();
      wifiStaDevicePtr->GetPhy ()->SetChannelNumber (channelBss);
      wifiStaDevicePtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBss);
      wifiStaDevicePtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBss);
    }

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
               BooleanValue (true));

  if (mcs == "IdealWifi")
    {
      wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
    }

else if (mcs == "MaxMcs"||mcs=="proposedMcs")
    {
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                    StringValue ("VhtMcs0"), "ControlMode",
                                    StringValue ("VhtMcs0"));
    }
else
{
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                    StringValue (mcs), "ControlMode",
                                    StringValue ("VhtMcs0"));
}

  apDeviceA = wifi.Install (phy, mac, wifiApNodes.Get (bss_i-1));

  Ptr<NetDevice> apDevicePtr = apDeviceA.Get (0);
  Ptr<WifiNetDevice> wifiApDevicePtr = apDevicePtr->GetObject<WifiNetDevice> ();
  wifiApDevicePtr->GetPhy ()->SetChannelNumber (channelBss);
  wifiApDevicePtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBss);
  wifiApDevicePtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBss);

  start_i = nBss + (bss_i - 1) * n;
  end_i = nBss + (bss_i) *n;
  stmp << "/NodeList/" << bss_i - 1 << "/DeviceList/*/Phy/ChannelBondingManager/"
                                       "$ns3::ConstantThresholdChannelBondingManager/"
                                       "CcaEdThresholdSecondary";
  Config::Set (stmp.str (), DoubleValue (constantCcaEdThresholdSecondaryBss));
  for (uint16_t i = start_i; i < end_i; i++)
    {
      std::stringstream stmp1;
      stmp1 << "/NodeList/" << i << "/DeviceList/*/Phy/ChannelBondingManager/"
                                    "$ns3::ConstantThresholdChannelBondingManager/"
                                    "CcaEdThresholdSecondary";
      Config::Set (stmp1.str (), DoubleValue (constantCcaEdThresholdSecondaryBss));
    }
//std::cout<<"reached end1 "<<std::endl;
  if (nBss > 1)
    {

      // network B
      ssid = Ssid ("network-B");

    bss_i = 2;
mcs=mcs2;
wifiStaNodes=wifiStaNodesB;
primaryChannelBss=primaryChannelBssB;
channelBss=channelBssB;
ccaEdThresholdPrimaryBss=ccaEdThresholdPrimaryBssB;
constantCcaEdThresholdSecondaryBss = constantCcaEdThresholdSecondaryBssB;

//std::cout<<"reached end2a "<<std::endl;
//std::cout<<mcs<<std::endl;
  mac.SetType ("ns3::StaWifiMac", "MaxMissedBeacons", UintegerValue (maxMissedBeacons), "Ssid",
               SsidValue (ssid));
  if (mcs == "IdealWifi")
    {
      wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
          staDeviceB = wifi.Install (phy, mac, wifiStaNodes);
    }

else if (mcs == "MaxMcs"||mcs=="proposedMcs")
    {

      for (uint8_t i = 0; i < n; i++)
        {
          ossMcs.str (std::to_string (maxMcsNode[i + nBss]));
          dataRate = "VhtMcs" + ossMcs.str ();

          wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                        StringValue (dataRate), "ControlMode",
                                        StringValue ("VhtMcs0"));
          staDeviceB.Add (wifi.Install (phy, mac, wifiStaNodes.Get (i)));
        }
    }
  else
    {
     wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (mcs),
                                    "ControlMode", StringValue ("VhtMcs0"));
          staDeviceB = wifi.Install (phy, mac, wifiStaNodes);
    }

  Ptr<NetDevice> staDevicePtr;
  for (uint16_t i = 0; i < n; i++)
    {

      staDevicePtr = staDeviceB.Get (i);
      Ptr<WifiNetDevice> wifiStaDevicePtr = staDevicePtr->GetObject<WifiNetDevice> ();
      wifiStaDevicePtr->GetPhy ()->SetChannelNumber (channelBss);
      wifiStaDevicePtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBss);
      wifiStaDevicePtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBss);
    }
//std::cout<<"reached end2b "<<std::endl;
  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
               BooleanValue (true));

  if (mcs == "IdealWifi")
    {
      wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
    }

 else if (mcs == "MaxMcs"||mcs=="proposedMcs")
    {
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                    StringValue ("VhtMcs0"), "ControlMode",
                                    StringValue ("VhtMcs0"));
    }
else
{
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                    StringValue (mcs), "ControlMode",
                                    StringValue ("VhtMcs0"));
}

  apDeviceB = wifi.Install (phy, mac, wifiApNodes.Get (bss_i-1));

  Ptr<NetDevice> apDevicePtr = apDeviceB.Get (0);
  Ptr<WifiNetDevice> wifiApDevicePtr = apDevicePtr->GetObject<WifiNetDevice> ();
  wifiApDevicePtr->GetPhy ()->SetChannelNumber (channelBss);
  wifiApDevicePtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBss);
  wifiApDevicePtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBss);

  start_i = nBss + (bss_i - 1) * n;
  end_i = nBss + (bss_i) *n;
  stmp << "/NodeList/" << bss_i - 1 << "/DeviceList/*/Phy/ChannelBondingManager/"
                                       "$ns3::ConstantThresholdChannelBondingManager/"
                                       "CcaEdThresholdSecondary";
  Config::Set (stmp.str (), DoubleValue (constantCcaEdThresholdSecondaryBss));
  for (uint16_t i = start_i; i < end_i; i++)
    {
      std::stringstream stmp1;
      stmp1 << "/NodeList/" << i << "/DeviceList/*/Phy/ChannelBondingManager/"
                                    "$ns3::ConstantThresholdChannelBondingManager/"
                                    "CcaEdThresholdSecondary";
      Config::Set (stmp1.str (), DoubleValue (constantCcaEdThresholdSecondaryBss));
    }
//std::cout<<"reached end2c "<<std::endl;
    }

  if (nBss > 2)
    {
      // network C
      ssid = Ssid ("network-C");
       bss_i = 3;
mcs=mcs3;
wifiStaNodes=wifiStaNodesC;
primaryChannelBss=primaryChannelBssC;
channelBss=channelBssC;
ccaEdThresholdPrimaryBss=ccaEdThresholdPrimaryBssC;
constantCcaEdThresholdSecondaryBss = constantCcaEdThresholdSecondaryBssC;


  mac.SetType ("ns3::StaWifiMac", "MaxMissedBeacons", UintegerValue (maxMissedBeacons), "Ssid",
               SsidValue (ssid));
  if (mcs == "IdealWifi")
    {
      wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
          staDeviceC = wifi.Install (phy, mac, wifiStaNodes);
    }

else if (mcs == "MaxMcs"||mcs=="proposedMcs")
    {

      for (uint8_t i = 0; i < n; i++)
        {
          ossMcs.str (std::to_string (maxMcsNode[i + nBss]));
          dataRate = "VhtMcs" + ossMcs.str ();

          wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                        StringValue (dataRate), "ControlMode",
                                        StringValue ("VhtMcs0"));
          staDeviceC.Add (wifi.Install (phy, mac, wifiStaNodes.Get (i)));
        }
    }
  else
    {
     wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (mcs),
                                    "ControlMode", StringValue ("VhtMcs0"));
          staDeviceC = wifi.Install (phy, mac, wifiStaNodes);
    }

  Ptr<NetDevice> staDevicePtr;
  for (uint16_t i = 0; i < n; i++)
    {

      staDevicePtr = staDeviceC.Get (i);
      Ptr<WifiNetDevice> wifiStaDevicePtr = staDevicePtr->GetObject<WifiNetDevice> ();
      wifiStaDevicePtr->GetPhy ()->SetChannelNumber (channelBss);
      wifiStaDevicePtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBss);
      wifiStaDevicePtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBss);
    }

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
               BooleanValue (true));

  if (mcs == "IdealWifi")
    {
      wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
    }

 else if (mcs == "MaxMcs"||mcs=="proposedMcs")
    {
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                    StringValue ("VhtMcs0"), "ControlMode",
                                    StringValue ("VhtMcs0"));
    }
else
{
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                    StringValue (mcs), "ControlMode",
                                    StringValue ("VhtMcs0"));
}

  apDeviceC = wifi.Install (phy, mac, wifiApNodes.Get (bss_i-1));

  Ptr<NetDevice> apDevicePtr = apDeviceC.Get (0);
  Ptr<WifiNetDevice> wifiApDevicePtr = apDevicePtr->GetObject<WifiNetDevice> ();
  wifiApDevicePtr->GetPhy ()->SetChannelNumber (channelBss);
  wifiApDevicePtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBss);
  wifiApDevicePtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBss);

  start_i = nBss + (bss_i - 1) * n;
  end_i = nBss + (bss_i) *n;
  stmp << "/NodeList/" << bss_i - 1 << "/DeviceList/*/Phy/ChannelBondingManager/"
                                       "$ns3::ConstantThresholdChannelBondingManager/"
                                       "CcaEdThresholdSecondary";
  Config::Set (stmp.str (), DoubleValue (constantCcaEdThresholdSecondaryBss));
  for (uint16_t i = start_i; i < end_i; i++)
    {
      std::stringstream stmp1;
      stmp1 << "/NodeList/" << i << "/DeviceList/*/Phy/ChannelBondingManager/"
                                    "$ns3::ConstantThresholdChannelBondingManager/"
                                    "CcaEdThresholdSecondary";
      Config::Set (stmp1.str (), DoubleValue (constantCcaEdThresholdSecondaryBss));
    }
    }
  if (nBss > 3)
    {
      // network D
      ssid = Ssid ("network-D");
       bss_i = 4;
mcs=mcs4;
wifiStaNodes=wifiStaNodesD;
primaryChannelBss=primaryChannelBssD;
channelBss=channelBssD;
ccaEdThresholdPrimaryBss=ccaEdThresholdPrimaryBssD;
constantCcaEdThresholdSecondaryBss = constantCcaEdThresholdSecondaryBssD;


  mac.SetType ("ns3::StaWifiMac", "MaxMissedBeacons", UintegerValue (maxMissedBeacons), "Ssid",
               SsidValue (ssid));
  if (mcs == "IdealWifi")
    {
      wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
          staDeviceD = wifi.Install (phy, mac, wifiStaNodes);
    }

else if (mcs == "MaxMcs"||mcs=="proposedMcs")
    {

      for (uint8_t i = 0; i < n; i++)
        {
          ossMcs.str (std::to_string (maxMcsNode[i + nBss]));
          dataRate = "VhtMcs" + ossMcs.str ();

          wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                        StringValue (dataRate), "ControlMode",
                                        StringValue ("VhtMcs0"));
          staDeviceD.Add (wifi.Install (phy, mac, wifiStaNodes.Get (i)));
        }
    }
  else
    {
     wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (mcs),
                                    "ControlMode", StringValue ("VhtMcs0"));
          staDeviceD = wifi.Install (phy, mac, wifiStaNodes);
    }

  Ptr<NetDevice> staDevicePtr;
  for (uint16_t i = 0; i < n; i++)
    {

      staDevicePtr = staDeviceD.Get (i);
      Ptr<WifiNetDevice> wifiStaDevicePtr = staDevicePtr->GetObject<WifiNetDevice> ();
      wifiStaDevicePtr->GetPhy ()->SetChannelNumber (channelBss);
      wifiStaDevicePtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBss);
      wifiStaDevicePtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBss);
    }

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
               BooleanValue (true));

  if (mcs == "IdealWifi")
    {
      wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
    }

 else if (mcs == "MaxMcs"||mcs=="proposedMcs")
    {
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                    StringValue ("VhtMcs0"), "ControlMode",
                                    StringValue ("VhtMcs0"));
    }
else
{
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                    StringValue (mcs), "ControlMode",
                                    StringValue ("VhtMcs0"));
}

  apDeviceD = wifi.Install (phy, mac, wifiApNodes.Get (bss_i-1));

  Ptr<NetDevice> apDevicePtr = apDeviceD.Get (0);
  Ptr<WifiNetDevice> wifiApDevicePtr = apDevicePtr->GetObject<WifiNetDevice> ();
  wifiApDevicePtr->GetPhy ()->SetChannelNumber (channelBss);
  wifiApDevicePtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBss);
  wifiApDevicePtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBss);

  start_i = nBss + (bss_i - 1) * n;
  end_i = nBss + (bss_i) *n;
  stmp << "/NodeList/" << bss_i - 1 << "/DeviceList/*/Phy/ChannelBondingManager/"
                                       "$ns3::ConstantThresholdChannelBondingManager/"
                                       "CcaEdThresholdSecondary";
  Config::Set (stmp.str (), DoubleValue (constantCcaEdThresholdSecondaryBss));
  for (uint16_t i = start_i; i < end_i; i++)
    {
      std::stringstream stmp1;
      stmp1 << "/NodeList/" << i << "/DeviceList/*/Phy/ChannelBondingManager/"
                                    "$ns3::ConstantThresholdChannelBondingManager/"
                                    "CcaEdThresholdSecondary";
      Config::Set (stmp1.str (), DoubleValue (constantCcaEdThresholdSecondaryBss));
    }
    }
  if (nBss > 4)
    {
      // network E
       bss_i = 5;
mcs=mcs5;
wifiStaNodes=wifiStaNodesE;
primaryChannelBss=primaryChannelBssE;
channelBss=channelBssE;
ccaEdThresholdPrimaryBss=ccaEdThresholdPrimaryBssE;
constantCcaEdThresholdSecondaryBss = constantCcaEdThresholdSecondaryBssE;


  mac.SetType ("ns3::StaWifiMac", "MaxMissedBeacons", UintegerValue (maxMissedBeacons), "Ssid",
               SsidValue (ssid));
  if (mcs == "IdealWifi")
    {
      wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
          staDeviceE = wifi.Install (phy, mac, wifiStaNodes);
    }

else if (mcs == "MaxMcs"||mcs=="proposedMcs")
    {

      for (uint8_t i = 0; i < n; i++)
        {
          ossMcs.str (std::to_string (maxMcsNode[i + nBss]));
          dataRate = "VhtMcs" + ossMcs.str ();

          wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                        StringValue (dataRate), "ControlMode",
                                        StringValue ("VhtMcs0"));
          staDeviceE.Add (wifi.Install (phy, mac, wifiStaNodes.Get (i)));
        }
    }
  else
    {
     wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (mcs),
                                    "ControlMode", StringValue ("VhtMcs0"));
          staDeviceE = wifi.Install (phy, mac, wifiStaNodes);
    }

  Ptr<NetDevice> staDevicePtr;
  for (uint16_t i = 0; i < n; i++)
    {

      staDevicePtr = staDeviceE.Get (i);
      Ptr<WifiNetDevice> wifiStaDevicePtr = staDevicePtr->GetObject<WifiNetDevice> ();
      wifiStaDevicePtr->GetPhy ()->SetChannelNumber (channelBss);
      wifiStaDevicePtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBss);
      wifiStaDevicePtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBss);
    }

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
               BooleanValue (true));

  if (mcs == "IdealWifi")
    {
      wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
    }

 else if (mcs == "MaxMcs"||mcs=="proposedMcs")
    {
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                    StringValue ("VhtMcs0"), "ControlMode",
                                    StringValue ("VhtMcs0"));
    }
else
{
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                    StringValue (mcs), "ControlMode",
                                    StringValue ("VhtMcs0"));
}

  apDeviceE = wifi.Install (phy, mac, wifiApNodes.Get (bss_i-1));

  Ptr<NetDevice> apDevicePtr = apDeviceE.Get (0);
  Ptr<WifiNetDevice> wifiApDevicePtr = apDevicePtr->GetObject<WifiNetDevice> ();
  wifiApDevicePtr->GetPhy ()->SetChannelNumber (channelBss);
  wifiApDevicePtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBss);
  wifiApDevicePtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBss);

  start_i = nBss + (bss_i - 1) * n;
  end_i = nBss + (bss_i) *n;
  stmp << "/NodeList/" << bss_i - 1 << "/DeviceList/*/Phy/ChannelBondingManager/"
                                       "$ns3::ConstantThresholdChannelBondingManager/"
                                       "CcaEdThresholdSecondary";
  Config::Set (stmp.str (), DoubleValue (constantCcaEdThresholdSecondaryBss));
  for (uint16_t i = start_i; i < end_i; i++)
    {
      std::stringstream stmp1;
      stmp1 << "/NodeList/" << i << "/DeviceList/*/Phy/ChannelBondingManager/"
                                    "$ns3::ConstantThresholdChannelBondingManager/"
                                    "CcaEdThresholdSecondary";
      Config::Set (stmp1.str (), DoubleValue (constantCcaEdThresholdSecondaryBss));
    }
    }
  if (nBss > 5)
    {
      // network F
      ssid = Ssid ("network-F");
       bss_i = 6;
mcs=mcs6;
wifiStaNodes=wifiStaNodesF;
primaryChannelBss=primaryChannelBssF;
channelBss=channelBssF;
ccaEdThresholdPrimaryBss=ccaEdThresholdPrimaryBssF;
constantCcaEdThresholdSecondaryBss = constantCcaEdThresholdSecondaryBssF;


  mac.SetType ("ns3::StaWifiMac", "MaxMissedBeacons", UintegerValue (maxMissedBeacons), "Ssid",
               SsidValue (ssid));
  if (mcs == "IdealWifi")
    {
      wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
          staDeviceF = wifi.Install (phy, mac, wifiStaNodes);
    }

else if (mcs == "MaxMcs"||mcs=="proposedMcs")
    {

      for (uint8_t i = 0; i < n; i++)
        {
          ossMcs.str (std::to_string (maxMcsNode[i + nBss]));
          dataRate = "VhtMcs" + ossMcs.str ();

          wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                        StringValue (dataRate), "ControlMode",
                                        StringValue ("VhtMcs0"));
          staDeviceF.Add (wifi.Install (phy, mac, wifiStaNodes.Get (i)));
        }
    }
  else
    {
     wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (mcs),
                                    "ControlMode", StringValue ("VhtMcs0"));
          staDeviceF = wifi.Install (phy, mac, wifiStaNodes);
    }

  Ptr<NetDevice> staDevicePtr;
  for (uint16_t i = 0; i < n; i++)
    {

      staDevicePtr = staDeviceF.Get (i);
      Ptr<WifiNetDevice> wifiStaDevicePtr = staDevicePtr->GetObject<WifiNetDevice> ();
      wifiStaDevicePtr->GetPhy ()->SetChannelNumber (channelBss);
      wifiStaDevicePtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBss);
      wifiStaDevicePtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBss);
    }

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
               BooleanValue (true));

  if (mcs == "IdealWifi")
    {
      wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
    }

 else if (mcs == "MaxMcs"||mcs=="proposedMcs")
    {
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                    StringValue ("VhtMcs0"), "ControlMode",
                                    StringValue ("VhtMcs0"));
    }
else
{
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                    StringValue (mcs), "ControlMode",
                                    StringValue ("VhtMcs0"));
}

  apDeviceF = wifi.Install (phy, mac, wifiApNodes.Get (bss_i-1));

  Ptr<NetDevice> apDevicePtr = apDeviceF.Get (0);
  Ptr<WifiNetDevice> wifiApDevicePtr = apDevicePtr->GetObject<WifiNetDevice> ();
  wifiApDevicePtr->GetPhy ()->SetChannelNumber (channelBss);
  wifiApDevicePtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBss);
  wifiApDevicePtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBss);

  start_i = nBss + (bss_i - 1) * n;
  end_i = nBss + (bss_i) *n;
  stmp << "/NodeList/" << bss_i - 1 << "/DeviceList/*/Phy/ChannelBondingManager/"
                                       "$ns3::ConstantThresholdChannelBondingManager/"
                                       "CcaEdThresholdSecondary";
  Config::Set (stmp.str (), DoubleValue (constantCcaEdThresholdSecondaryBss));
  for (uint16_t i = start_i; i < end_i; i++)
    {
      std::stringstream stmp1;
      stmp1 << "/NodeList/" << i << "/DeviceList/*/Phy/ChannelBondingManager/"
                                    "$ns3::ConstantThresholdChannelBondingManager/"
                                    "CcaEdThresholdSecondary";
      Config::Set (stmp1.str (), DoubleValue (constantCcaEdThresholdSecondaryBss));
    }
    }
  if (nBss > 6)
    {
      // network G
      ssid = Ssid ("network-G");
       bss_i = 7;
mcs=mcs7;
wifiStaNodes=wifiStaNodesG;
primaryChannelBss=primaryChannelBssG;
channelBss=channelBssG;
ccaEdThresholdPrimaryBss=ccaEdThresholdPrimaryBssG;
constantCcaEdThresholdSecondaryBss = constantCcaEdThresholdSecondaryBssG;


  mac.SetType ("ns3::StaWifiMac", "MaxMissedBeacons", UintegerValue (maxMissedBeacons), "Ssid",
               SsidValue (ssid));
  if (mcs == "IdealWifi")
    {
      wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
          staDeviceG = wifi.Install (phy, mac, wifiStaNodes);
    }

else if (mcs == "MaxMcs"||mcs=="proposedMcs")
    {

      for (uint8_t i = 0; i < n; i++)
        {
          ossMcs.str (std::to_string (maxMcsNode[i + nBss]));
          dataRate = "VhtMcs" + ossMcs.str ();

          wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                        StringValue (dataRate), "ControlMode",
                                        StringValue ("VhtMcs0"));
          staDeviceG.Add (wifi.Install (phy, mac, wifiStaNodes.Get (i)));
        }
    }
  else
    {
     wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (mcs),
                                    "ControlMode", StringValue ("VhtMcs0"));
          staDeviceG = wifi.Install (phy, mac, wifiStaNodes);
    }

  Ptr<NetDevice> staDevicePtr;
  for (uint16_t i = 0; i < n; i++)
    {

      staDevicePtr = staDeviceG.Get (i);
      Ptr<WifiNetDevice> wifiStaDevicePtr = staDevicePtr->GetObject<WifiNetDevice> ();
      wifiStaDevicePtr->GetPhy ()->SetChannelNumber (channelBss);
      wifiStaDevicePtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBss);
      wifiStaDevicePtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBss);
    }

  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid), "EnableBeaconJitter",
               BooleanValue (true));

  if (mcs == "IdealWifi")
    {
      wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
    }

 else if (mcs == "MaxMcs"||mcs=="proposedMcs")
    {
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                    StringValue ("VhtMcs0"), "ControlMode",
                                    StringValue ("VhtMcs0"));
    }
else
{
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
                                    StringValue (mcs), "ControlMode",
                                    StringValue ("VhtMcs0"));
}

  apDeviceG = wifi.Install (phy, mac, wifiApNodes.Get (bss_i-1));

  Ptr<NetDevice> apDevicePtr = apDeviceG.Get (0);
  Ptr<WifiNetDevice> wifiApDevicePtr = apDevicePtr->GetObject<WifiNetDevice> ();
  wifiApDevicePtr->GetPhy ()->SetChannelNumber (channelBss);
  wifiApDevicePtr->GetPhy ()->SetPrimaryChannelNumber (primaryChannelBss);
  wifiApDevicePtr->GetPhy ()->SetCcaEdThreshold (ccaEdThresholdPrimaryBss);

  start_i = nBss + (bss_i - 1) * n;
  end_i = nBss + (bss_i) *n;
  stmp << "/NodeList/" << bss_i - 1 << "/DeviceList/*/Phy/ChannelBondingManager/"
                                       "$ns3::ConstantThresholdChannelBondingManager/"
                                       "CcaEdThresholdSecondary";
  Config::Set (stmp.str (), DoubleValue (constantCcaEdThresholdSecondaryBss));
  for (uint16_t i = start_i; i < end_i; i++)
    {
      std::stringstream stmp1;
      stmp1 << "/NodeList/" << i << "/DeviceList/*/Phy/ChannelBondingManager/"
                                    "$ns3::ConstantThresholdChannelBondingManager/"
                                    "CcaEdThresholdSecondary";
      Config::Set (stmp1.str (), DoubleValue (constantCcaEdThresholdSecondaryBss));
    }
    }
//std::cout<<"reached end3"<<std::endl;
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

  if (nBss > 1)
    {
      address.SetBase ("192.168.2.0", "255.255.255.0");

      StaInterfaceB = address.Assign (staDeviceB);

      ApInterfaceB = address.Assign (apDeviceB);
    }
  if (nBss > 2)
    {
      address.SetBase ("192.168.3.0", "255.255.255.0");

      StaInterfaceC = address.Assign (staDeviceC);

      ApInterfaceC = address.Assign (apDeviceC);
    }
  if (nBss > 3)
    {
      address.SetBase ("192.168.4.0", "255.255.255.0");

      StaInterfaceD = address.Assign (staDeviceD);

      ApInterfaceD = address.Assign (apDeviceD);
    }
  if (nBss > 4)
    {
      address.SetBase ("192.168.5.0", "255.255.255.0");

      StaInterfaceE = address.Assign (staDeviceE);

      ApInterfaceE = address.Assign (apDeviceE);
    }
  if (nBss > 5)
    {
      address.SetBase ("192.168.6.0", "255.255.255.0");

      StaInterfaceF = address.Assign (staDeviceF);

      ApInterfaceF = address.Assign (apDeviceF);
    }
  if (nBss > 6)
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

  uint32_t maxAmpduSizeBss1 = 131071;
  uint32_t maxAmpduSizeBss2 = 131071;
  uint32_t maxAmpduSizeBss3 = 131071;
  uint32_t maxAmpduSizeBss4 = 131071;
  uint32_t maxAmpduSizeBss5 = 131071;
  uint32_t maxAmpduSizeBss6 = 131071;
  uint32_t maxAmpduSizeBss7 = 131071;
  for (uint16_t i = 0; i < ((n + 1) * nBss); i++)
    {
      if (i < (n + 1)) // BSS 1
        {
          std::stringstream stmp;
          stmp << "/NodeList/" << i << "/DeviceList/*/$ns3::WifiNetDevice/Mac/BE_MaxAmpduSize";
          Config::Set (stmp.str (), UintegerValue (std::min (maxAmpduSizeBss1, 4194303u)));
        }
      else if (i < (2 * (n + 1))) // BSS 2
        {
          std::stringstream stmp;
          stmp << "/NodeList/" << i << "/DeviceList/*/$ns3::WifiNetDevice/Mac/BE_MaxAmpduSize";
          Config::Set (stmp.str (), UintegerValue (std::min (maxAmpduSizeBss2, 4194303u)));
        }
      else if (i < (3 * (n + 1))) // BSS 3
        {
          std::stringstream stmp;
          stmp << "/NodeList/" << i << "/DeviceList/*/$ns3::WifiNetDevice/Mac/BE_MaxAmpduSize";
          Config::Set (stmp.str (), UintegerValue (std::min (maxAmpduSizeBss3, 4194303u)));
        }
      else if (i < (4 * (n + 1))) // BSS 4
        {
          std::stringstream stmp;
          stmp << "/NodeList/" << i << "/DeviceList/*/$ns3::WifiNetDevice/Mac/BE_MaxAmpduSize";
          Config::Set (stmp.str (), UintegerValue (std::min (maxAmpduSizeBss4, 4194303u)));
        }
      else if (i < (5 * (n + 1))) // BSS 5
        {
          std::stringstream stmp;
          stmp << "/NodeList/" << i << "/DeviceList/*/$ns3::WifiNetDevice/Mac/BE_MaxAmpduSize";
          Config::Set (stmp.str (), UintegerValue (std::min (maxAmpduSizeBss5, 4194303u)));
        }
      else if (i < (6 * (n + 1))) // BSS 6
        {
          std::stringstream stmp;
          stmp << "/NodeList/" << i << "/DeviceList/*/$ns3::WifiNetDevice/Mac/BE_MaxAmpduSize";
          Config::Set (stmp.str (), UintegerValue (std::min (maxAmpduSizeBss6, 4194303u)));
        }
      else if (i < (7 * (n + 1))) // BSS 7
        {
          std::stringstream stmp;
          stmp << "/NodeList/" << i << "/DeviceList/*/$ns3::WifiNetDevice/Mac/BE_MaxAmpduSize";
          Config::Set (stmp.str (), UintegerValue (std::min (maxAmpduSizeBss7, 4194303u)));
        }
    }

  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::UdpServer/RxWithAddresses",
                   MakeCallback (&PacketRx));

  // phy.EnablePcap ("staA_pcap", staDeviceA);
 // phy.EnablePcap ("apA_pcap", apDeviceA);
  /*
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
  filename = "Res_" + Test + ".csv";
  std::ofstream TputFile;
  TputFile.open (filename.c_str (), std::ofstream::out | std::ofstream::trunc);
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
          TputFile << rxThroughputPerNode[k] << std::endl;

    }

  TputFile << std::endl;
  TputFile.close ();

  return 0;
}

/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 MIRKO BANCHI
 * Copyright (c) 2015 University of Washington
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
 * Authors: Mirko Banchi <mk.banchi@gmail.com>
 *          Sebastien Deronne <sebastien.deronne@gmail.com>
 *          Tom Henderson <tomhend@u.washington.edu>
 *
 * Adapted from ht-wifi-network.cc example
 */
#include <sstream>
#include <iomanip>

#include "ns3/core-module.h"
#include "ns3/config-store-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/spectrum-module.h"
#include "ns3/internet-module.h"

// This is a simple example of an IEEE 802.11n Wi-Fi network.
//
// The main use case is to enable and test different error models for
// frequency-selective channels
//
// Network topology:
//
//  Wi-Fi 192.168.1.0
//
//   STA                  AP
//    * <-- distance -->  *
//    |                   |
//    n1                  n2
//
// Users may vary the following command-line arguments in addition to the
// attributes, global values, and default values typically available:
//
//    --simulationTime:  Simulation time in seconds [10]
//    --udp:             UDP if set to 1, TCP otherwise [true]
//    --distance:        meters separation between nodes [50]
//    --index:           restrict index to single value between 0 and 31 [256]
//    --errorModelType:  select ns3::NistErrorRateModel or ns3::YansErrorRateModel or ns3::MatlabOfdmErrorRateModel [ns3::NistErrorRateModel]
//    --enablePcap:      enable pcap output [false]
//
// By default, the program will step through 32 index values, corresponding
// to the following MCS, channel width, and guard interval combinations:
//   index 0-7:    MCS 0-7, long guard interval, 20 MHz channel
//   index 8-15:   MCS 0-7, short guard interval, 20 MHz channel
//   index 16-23:  MCS 0-7, long guard interval, 40 MHz channel
//   index 24-31:  MCS 0-7, short guard interval, 40 MHz channel
// and send 1000 UDP packets using each MCS, using the SpectrumWifiPhy and the
// NistErrorRateModel, at a distance of 50 meters.  The program outputs
// results such as:
//
// wifiType: ns3::SpectrumWifiPhy distance: 50m; sent: 1000
// index   MCS Rate (Mb/s) Tput (Mb/s) Received Signal (dBm) Noise (dBm) SNR (dB)
//     0     0       6.5      0.7776    1000    -77.6633    -100.966     23.3027
//     1     1        13      0.7776    1000    -77.6633    -100.966     23.3027
//     2     2      19.5      0.7776    1000    -77.6633    -100.966     23.3027
//     3     3        26      0.7776    1000    -77.6633    -100.966     23.3027
//  ...
//
// When UDP is used, the throughput will always be 0.7776 Mb/s since the
// traffic generator does not attempt to match the maximum Phy data rate
// but instead sends at a constant rate.  When TCP is used, the TCP flow
// will exhibit different throughput depending on the index.
//

using namespace ns3;

// Global variables for use in callbacks.
double g_signalDbmAvg;
double g_noiseDbmAvg;
uint32_t g_samples;

void MonitorSniffRx (Ptr<const Packet> packet,
                     uint16_t channelFreqMhz,
                     WifiTxVector txVector,
                     MpduInfo aMpdu,
                     SignalNoiseDbm signalNoise)

{
  g_samples++;
  g_signalDbmAvg += ((signalNoise.signal - g_signalDbmAvg) / g_samples);
  g_noiseDbmAvg += ((signalNoise.noise - g_noiseDbmAvg) / g_samples);
}


NS_LOG_COMPONENT_DEFINE ("WifiSpectrumMultipathExample");

int main (int argc, char *argv[])
{       
        SeedManager::SetSeed(3);   
        SeedManager::SetRun (3); 
        bool udp = true;
        double distance = 20;
        double simulationTime = 2; //seconds
        uint16_t index = 0;
        std::string channeltype = "TGN-D";
        std::string errorModelType = "ns3::MatlabOfdmErrorRateModel";
        //std::string errorModelType = "ns3::YansErrorRateModel";

        bool enablePcap = true;

        CommandLine cmd;
        cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
        cmd.AddValue ("udp", "UDP if set to 1, TCP otherwise", udp);
        cmd.AddValue ("distance", "meters separation between nodes", distance);
        cmd.AddValue ("index", "restrict index to single value between 0 and 31", index);
        cmd.AddValue ("errorModelType", "select ns3::NistErrorRateModel or ns3::YansErrorRateModel", errorModelType);
        cmd.AddValue ("enablePcap", "enable pcap output", enablePcap);
        cmd.Parse (argc,argv);


        uint32_t payloadSize = 972; // 1000 bytes IPv4

        NodeContainer wifiStaNodes;
        wifiStaNodes.Create (2);

        SpectrumWifiPhyHelper spectrumPhy = SpectrumWifiPhyHelper::Default ();
        Config::SetDefault ("ns3::WifiPhy::CcaMode1Threshold", DoubleValue (-62.0));

        Config::SetDefault ("ns3::SpectrumWifiPhy::FastFadingChannelType", StringValue (channeltype));    
        
        Ptr<MultiModelSpectrumChannel> spectrumChannel = CreateObject<MultiModelSpectrumChannel> ();
        Ptr<FriisPropagationLossModel> lossModel = CreateObject<FriisPropagationLossModel> ();
        lossModel->SetFrequency (5.180e9);
        spectrumChannel->AddPropagationLossModel (lossModel);

        Ptr<ConstantSpeedPropagationDelayModel> delayModel
        = CreateObject<ConstantSpeedPropagationDelayModel> ();
        spectrumChannel->SetPropagationDelayModel (delayModel);

        spectrumPhy.SetChannel (spectrumChannel);
        spectrumPhy.SetErrorRateModel (errorModelType);
        spectrumPhy.Set ("Frequency", UintegerValue (5180));
        spectrumPhy.Set ("TxPowerStart", DoubleValue (1)); // dBm  (1.26 mW)
        spectrumPhy.Set ("TxPowerEnd", DoubleValue (1));     
        spectrumPhy.Set ("ShortGuardEnabled", BooleanValue (false));
        spectrumPhy.Set ("ChannelWidth", UintegerValue (20));
        // for MIMO
        /*spectrumPhy.Set ("Antennas", UintegerValue (2));
        spectrumPhy.Set ("MaxSupportedTxSpatialStreams", UintegerValue (2));
        spectrumPhy.Set ("MaxSupportedRxSpatialStreams", UintegerValue (2));*/
        WifiHelper wifi;
        wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
        WifiMacHelper mac;

        Ssid ssid = Ssid ("ns380211n");

        double datarate = 0;
        StringValue DataRate;
        DataRate = StringValue ("HtMcs7");
        datarate = 6.5;

        wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", DataRate,
                                    "ControlMode", DataRate);

        NetDeviceContainer staDevices;

        mac.SetType ("ns3::AdhocWifiMac",
                   "Ssid", SsidValue (ssid));
        staDevices = wifi.Install (spectrumPhy, mac, wifiStaNodes);

        // mobility.
        MobilityHelper mobility;
        Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
        positionAlloc->Add (Vector (0.0, 0.0, 0.0));
        positionAlloc->Add (Vector (distance, 0.0, 0.0));
        mobility.SetPositionAllocator (positionAlloc);
        mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
        mobility.Install (wifiStaNodes);

        /* Internet stack*/
        InternetStackHelper stack;
        stack.Install (wifiStaNodes);

        Ipv4AddressHelper address;

        address.SetBase ("192.168.1.0", "255.255.255.0");
        Ipv4InterfaceContainer staNodeInterfaces;

        staNodeInterfaces = address.Assign (staDevices);

        /* Setting applications */
        ApplicationContainer serverApp;
        //UDP flow
        UdpServerHelper myServer (9);
        serverApp = myServer.Install (wifiStaNodes.Get (0));
        serverApp.Start (Seconds (0.0));
        serverApp.Stop (Seconds (simulationTime + 1));

        UdpClientHelper myClient (staNodeInterfaces.GetAddress (0), 9);
        myClient.SetAttribute ("MaxPackets", UintegerValue (1));
        myClient.SetAttribute ("Interval", TimeValue (MilliSeconds (5)));
        myClient.SetAttribute ("PacketSize", UintegerValue (payloadSize));

        ApplicationContainer clientApp = myClient.Install (wifiStaNodes.Get (1));
        clientApp.Start (Seconds (1.0));
        clientApp.Stop (Seconds (simulationTime + 1));

        Config::ConnectWithoutContext ("/NodeList/0/DeviceList/*/Phy/MonitorSnifferRx", MakeCallback (&MonitorSniffRx));

        if (enablePcap)
        {
                std::stringstream ss;
                ss << "wifi-spectrum-per-example-";
                spectrumPhy.EnablePcap (ss.str (), staDevices);
        } 
        g_signalDbmAvg = 0;
        g_noiseDbmAvg = 0;
        g_samples = 0;

        Simulator::Stop (Seconds (simulationTime + 1));
        Simulator::Run ();

        double throughput = 0;
        uint32_t totalPacketsThrough = 0;

        totalPacketsThrough = DynamicCast<UdpServer> (serverApp.Get (0))->GetReceived ();
        throughput = totalPacketsThrough * payloadSize * 8 / (simulationTime * 1000000.0); //Mbit/s
        
        std::cout << "wifiType: ns3::SpectrumWifiPhy distance: " << distance << "m; sent: 1000 TxPower: 1 dBm (1.3 mW)" << std::endl;
        std::cout << std::setw (5) << "index" <<
        std::setw (6) << "MCS" <<
        std::setw (12) << "Rate (Mb/s)" <<
        std::setw (12) << "Tput (Mb/s)" <<
        std::setw (10) << "Received " <<
        std::setw (12) << "Signal (dBm)" <<
        std::setw (12) << "Noise (dBm)" <<
        std::setw (10) << "SNR (dB)" <<
        std::endl;
         

        std::cout << std::setw (5) << 0 <<
        std::setw (6) << (0 % 8) <<
        std::setw (10) << datarate <<
        std::setw (12) << throughput <<
        std::setw (8) << totalPacketsThrough;
        if (totalPacketsThrough > 0)
        {
          std::cout << std::setw (12) << g_signalDbmAvg <<
            std::setw (12) << g_noiseDbmAvg <<
            std::setw (12) << (g_signalDbmAvg - g_noiseDbmAvg) <<
            std::endl;
        }
        else
        {
          std::cout << std::setw (12) << "N/A" <<
            std::setw (12) << "N/A" <<
            std::setw (12) << "N/A" <<
            std::endl;
        }
        Simulator::Destroy ();

        return 0;
} 

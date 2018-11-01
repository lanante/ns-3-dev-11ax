/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Authors: Nicola Baldo <nbaldo@cttc.es> and Tom Henderson <tomh@tomh.org>
 */

// Explore coexistence (sharing) behavior between two wifi networks
//
// One to four APs are supported:  APs are laid out in a grid-like pattern.
//
//             AP2<--d1--->AP3
//              |           |
//            d1|           | d1 = 10m (default)
//              |           |
//             AP0<--d1--->AP1
//
//  Around each AP are 1-5 STAs, distributed according to uniform disc position
//  allocator around the AP with radius rho.
//
//     STA3
//     |
//     |
//    AP---- STA2
//     |
//     |
//     STA1
//
//
//  All devices within ED/PD range; APs doing full-buffer UDP transfer
//  to the STAs, APs set to the same channel
//
//  The program can be configured at run-time by passing
//  command-line arguments.  The command
//  ./waf --run "wifi-co-channel-networks --help"
//  will display all of the available run-time help options, and in
//  particular, the command
//  ./waf --run "wifi-co-channel-networks --PrintGlobals" should
//  display the following:
//
// Global values:
//     --ChecksumEnabled=[false]
//         A global switch to enable all checksums for all protocols
//     --RngRun=[1]
//         The run number used to modify the global seed
//     --RngSeed=[1]
//         The global seed of all rng streams
//     --SchedulerType=[ns3::MapScheduler]
//         The object class to use as the scheduler implementation
//     --SimulatorImplementationType=[ns3::DefaultSimulatorImpl]
//         The object class to use as the simulator implementation
//     --d1=[10]
//         AP separation (m)
//     --duration=[2]
//         Data transfer duration (seconds)
//     --numAps=[1]
//         number of APs (1-4)
//     --numStas=[1]
//         number of APs (1-5)
//     --outputDir=[./]
//         directory where to store simulation results
//     --pcapEnabled=[false]
//         Whether to enable pcap trace files for Wi-Fi
//     --rho=[5]
//         uniform disc radius (m)
//     --simTag=[default]
//         tag to be appended to output filenames to distinguish simulation campaigns
//
//  In addition, some other variables may be modified at compile-time.
//  For simplification, an IEEE 802.11n configuration is used with the
//  IdealWifiManager.
//
//  The following sample output is provided.
//
//  When run with no arguments:
//
//  ./waf --run "wifi-co-channel-networks"
//
//  The program will output:
//
// Running simulation for 2 sec of data transfer; 15 sec overall
// Operator A: Wi-Fi; number of cells 1; number of UEs 1
// Operator B: Wi-Fi; number of cells 0; number of UEs 0
// --------monitorA----------
// Flow 1 (11.0.0.1 -> 17.0.0.2)
//   Tx Packets: 18306
//   Tx Bytes:   18818568
//   TxOffered:  75.2743 Mbps
//   Rx Bytes:   18809316
//   Throughput: 77.5286 Mbps
//   Mean delay:  0.595566 ms
//   Mean jitter:  0.0673113 ms
//   Rx Packets: 18297
// --------monitorB----------
//
//  In general, the global values of "--RngRun", "--duration", "--numAps",
//  and "--numStasPerAp"  are most
//  likely to be changed by command-line argument.  The first changes
//  the random variable stream assigments, the second changes the duration
//  of the data flow (2 seconds of full-buffer by default)

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/scenario-helper.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiCoChannelNetworks");

// Global Values are used in place of command line arguments so that these
// values may be managed in the ns-3 ConfigStore system.
static ns3::GlobalValue g_duration ("duration",
                                    "Data transfer duration (seconds)",
                                    ns3::DoubleValue (2),
                                    ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_pcap ("pcapEnabled",
                                "Whether to enable pcap trace files for Wi-Fi",
                                ns3::BooleanValue (false),
                                ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_simTag ("simTag",
                                  "tag to be appended to output filenames to distinguish simulation campaigns",
                                  ns3::StringValue ("default"),
                                  ns3::MakeStringChecker ());

static ns3::GlobalValue g_outputDir ("outputDir",
                                     "directory where to store simulation results",
                                     ns3::StringValue ("./"),
                                     ns3::MakeStringChecker ());
static ns3::GlobalValue g_d1 ("d1",
                              "AP separation (m)",
                              ns3::DoubleValue (10),
                              ns3::MakeDoubleChecker<double> (0, 10e6));

static ns3::GlobalValue g_rho ("rho",
                              "uniform disc radius (m)",
                              ns3::DoubleValue (5),
                              ns3::MakeDoubleChecker<double> (0, 10e6));

static ns3::GlobalValue g_numAps ("numAps",
                              "number of APs (1-4)",
                              ns3::UintegerValue (1),
                              ns3::MakeUintegerChecker<uint32_t> (1, 4));

static ns3::GlobalValue g_numStas ("numStas",
                              "number of APs (1-5)",
                              ns3::UintegerValue (1),
                              ns3::MakeUintegerChecker<uint32_t> (1, 5));


int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  enum Config_e cellConfigA = WIFI;
  enum Config_e cellConfigB = WIFI;

  // Some debugging settings not exposed as command line args are below

  // To disable application data flow (e.g. for visualizing mobility
  // in the visualizer), set below to true
  bool disableApps = false;

  // Extract global values into local variables.
  DoubleValue doubleValue;
  StringValue stringValue;
  UintegerValue uintegerValue;
  GlobalValue::GetValueByName ("duration", doubleValue);
  double duration = doubleValue.Get ();
  Time durationTime = Seconds (duration);
  GlobalValue::GetValueByName ("simTag", stringValue);
  std::string simTag = stringValue.Get ();
  GlobalValue::GetValueByName ("outputDir", stringValue);
  std::string outputDir = stringValue.Get ();
  GlobalValue::GetValueByName ("d1", doubleValue);
  double d1 = doubleValue.Get ();
  GlobalValue::GetValueByName ("rho", doubleValue);
  double rho = doubleValue.Get ();
  GlobalValue::GetValueByName ("numAps", uintegerValue);
  uint32_t numAps = uintegerValue.Get ();
  GlobalValue::GetValueByName ("numStas", uintegerValue);
  uint32_t numStasPerAp = uintegerValue.Get ();

  //
  // Topology setup phase
  //

  NodeContainer bsNodesA;
  bsNodesA.Create (numAps);

  // Use an empty node container for the operator B network
  NodeContainer bsNodesB;

  // BS mobility helper
  MobilityHelper mobilityBs;
  mobilityBs.SetPositionAllocator ("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue (0),
                                  "MinY", DoubleValue (0),
                                  "DeltaX", DoubleValue (d1),
                                  "DeltaY", DoubleValue (d1),
                                  "GridWidth", UintegerValue (2),
                                  "LayoutType", StringValue ("RowFirst"));
  mobilityBs.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityBs.Install (bsNodesA);

  // Allocate UE for each cell
  NodeContainer ueNodesA;
  // Use an empty node container for the operator B network
  NodeContainer ueNodesB;

  for (uint32_t i = 0; i < numAps; i++)
    {
      NodeContainer ueNodes;
      ueNodes.Create (numStasPerAp);
      double xCoord = 0;
      double yCoord = 0;
      if (i % 2)
        {
          xCoord = d1;
        }
      if (i > 1)
        {
          yCoord = d1;
        }
      // UE mobility helper
      MobilityHelper mobilityUe;
      mobilityUe.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                      "rho", DoubleValue (rho),
                                      "X", DoubleValue (xCoord),
                                      "Y", DoubleValue (yCoord));
      mobilityUe.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobilityUe.Install (ueNodes);
      ueNodesA.Add (ueNodes);
    }

  std::ostringstream simulationParams;
  simulationParams << "";

  // Specify some physical layer parameters that will be used in scenario helper
  PhyParams phyParams;
  phyParams.m_bsTxGain = 5; // dB antenna gain
  phyParams.m_bsRxGain = 5; // dB antenna gain
  phyParams.m_bsTxPower = 18; // dBm
  phyParams.m_bsNoiseFigure = 5; // dB
  phyParams.m_ueTxGain = 0; // dB antenna gain
  phyParams.m_ueRxGain = 0; // dB antenna gain
  phyParams.m_ueTxPower = 18; // dBm
  phyParams.m_ueNoiseFigure = 9; // dB

  ConfigureAndRunScenario (cellConfigA, cellConfigB, bsNodesA, bsNodesB, ueNodesA, ueNodesB, phyParams, durationTime, UDP, "ns3::LogDistancePropagationLossModel", disableApps, 0, false, outputDir + "/wifi_co_channel_networks_" + simTag, simulationParams.str ());

  return 0;
}

/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 */

//
//  This example program can be used to experiment with wireless coexistence
//  in a simple scenario.  The scenario consists of two cells whose radio
//  coverage overlaps but representing, notionally, two operators in the
//  same region whose transmissions may impact mechanisms such as clear
//  channel assessment and adaptive modulation and coding.
//  The technologies used are LTE Licensed Assisted Access (LAA)
//  operating on EARFCN 255444 (5.180 GHz), and Wi-Fi 802.11n
//  operating on channel 36 (5.180 GHz).
//
//  The notional scenario consists of two operators A and B, each
//  operating a single cell, and each cell consisting of a base
//  station (BS) and a user equipment (UE), as represented in the following
//  diagram:
//
//
//    BS A  . .  d2 . .  UE B
//     |                   |
//  d1 |                d1 |
//     |                   |
//    UE A  . .  d2 . .  BS B
//
//  cell A              cell B
//
//  where 'd1' and 'd2' distances (in meters) separate the nodes.
//
//  When using LTE, the BS is modeled as an eNB and the UE as a UE.
//  When using Wi-Fi, the BS is modeled as an AP and the UE as a STA.
//
//  In addition, both BS are connected to a "backhaul" client node that
//  originates data transfer in the downlink direction from client to UE(s).
//  The links from client to BS are modeled as well provisioned; low latency
//  links.
//
//  The figure may be redrawn as follows:
//
//     +---------------------------- client
//     |                               |
//    BS A  . .  d2 . .  UE B          |
//     |                   |           |
//  d1 |                d1 |           |
//     |                   |           |
//    UE A  . .  d2 . .  BS B----------+
//
//
//  In general, the program can be configured at run-time by passing
//  command-line arguments.  The command
//  ./waf --run "lte-wifi-simple --help"
//  will display all of the available run-time help options, and in
//  particular, the command
//  ./waf --run "lte-wifi-simple --PrintGlobals" should
//  display the following:
//
// Global values:
//    --ChannelAccessManager=[Lbt]
//        Default, DutyCycle, Lbt
//    --ChecksumEnabled=[false]
//        A global switch to enable all checksums for all protocols
//    --RngRun=[1]
//        The run number used to modify the global seed
//    --RngSeed=[1]
//        The global seed of all rng streams
//    --SchedulerType=[ns3::MapScheduler]
//        The object class to use as the scheduler implementation
//    --SimulatorImplementationType=[ns3::DefaultSimulatorImpl]
//        The object class to use as the simulator implementation
//    --asciiEnabled=[false]
//        Whether to enable ascii trace files for Wi-Fi
//    --cellConfigA=[Laa]
//        Laa, Lte, or Wifi
//    --cellConfigB=[Wifi]
//        Laa, Lte, or Wifi
//    --clientStartTimeSeconds=[2]
//        Client start time (seconds)
//    --cwUpdateRule=[nacks80]
//        Rule that will be used to update contention window of LAA node
//    --d1=[10]
//        intra-cell separation (e.g. AP to STA)
//    --d2=[50]
//        inter-cell separation
//    --disableMibAndSibStartupTime=[2]
//        the time at which to disable mib and sib control messages (seconds)
//    --dropPackets=[false]
//        applicable to impl2; if true, Phy will drop packets upon failure to obtain channel access
//    --drsEnabled=[true]
//        if true, drs ctrl messages will be generatedif false, drs ctrl messages will not be generated
//    --drsPeriod=[80]
//        periodicity to be set for DRS ctrl messages
//    --duration=[1]
//        Data transfer duration (seconds)
//    --ftpLambda=[0.5]
//        Lambda value for FTP model 1 application
//    --generateRem=[false]
//        if true, will generate a REM and then abort the simulation;if false, will run the simulation normally (without generating any REM)
//    --laaEdThreshold=[-72]
//        CCA-ED threshold for channel access manager (dBm)
//    --lbtChannelAccessManagerInstallTime=[2]
//        LTE channel access manager install time (seconds)
//    --lbtTxop=[8]
//        TxOp for LBT devices (ms)
//    --logBackoffChanges=[false]
//        Whether to write logfile of backoff values drawn
//    --logBackoffNodeId=[4294967295]
//        Node index of specific node to filter logBackoffChanges (default will log all nodes)
//    --logBeaconArrivals=[false]
//        Whether to write logfile of beacon arrivals to STA devices
//    --logBeaconNodeId=[4294967295]
//        Node index of specific node to filter logBeaconArrivals (default will log all nodes)
//    --logCwChanges=[false]
//        Whether to write logfile of contention window changes
//    --logCwNodeId=[4294967295]
//        Node index of specific node to filter logCwChanges (default will log all nodes)
//    --logDataTx=[false]
//        Whether to write logfile when data is transmitted at eNb
//    --logHarqFeedback=[false]
//        Whether to write logfile of HARQ feedbacks per each subframe.
//    --logHarqFeedbackNodeId=[4294967295]
//        Node index of specific node to filter HARQ feedbacks (default will log all nodes)
//    --logPhyArrivals=[false]
//        Whether to write logfile of signal arrivals to SpectrumWifiPhy
//    --logPhyNodeId=[4294967295]
//        Node index of specific node to filter logPhyArrivals (default will log all nodes)
//    --logTxopNodeId=[4294967295]
//        Node index of specific node to filter logTxops (default will log all nodes)
//    --logTxops=[false]
//        Whether to write logfile of txop opportunities of eNb
//    --logWifiFailRetries=[false]
//        Log when a data packet fails to be retransmitted successfully and is discarded
//    --logWifiRetries=[false]
//        Log when a data packet requires a retry
//    --lteChannelAccessImpl=[false]
//        if true, (impl1) it will be used lte channel access implemenation that stops scheduler when no channel access, thus has 2ms delayif false, (impl2) it will be used lte channel acces implementation that does not stop scheduler, always schedules, transmits only when there is channel access, can be used in combination with drop packet attribute
//    --lteDutyCycle=[1]
//        Duty cycle value to be used for LTE
//    --mibPeriod=[10]
//        periodicity to be set for MIB ctrl messages
//    --outputDir=[./]
//        directory where to store simulation results
//    --pcapEnabled=[false]
//        Whether to enable pcap trace files for Wi-Fi
//    --remDir=[./]
//        directory where to save REM-related output files
//    --rlcAmRbsTimer=[20]
//        Set value of ReportBufferStatusTimer attribute of RlcAm.
//    --serverLingerTimeSeconds=[1]
//        Server linger time (seconds)
//    --serverStartTimeSeconds=[2]
//        Server start time (seconds)
//    --sibPeriod=[20]
//        periodicity to be set for SIB1 ctrl messages
//    --simTag=[default]
//        tag to be appended to output filenames to distinguish simulation campaigns
//    --simulationLingerTimeSeconds=[1]
//        Simulation linger time (seconds)
//    --spreadUdpLoad=[false]
//        optimization to try to spread a saturating UDP load across multiple UE and cells
//    --tcpRlcMode=[RlcAm]
//        RLC_AM, RLC_UM
//    --transport=[Udp]
//        whether to use 3GPP Ftp, Udp, or Tcp
//    --udpPacketSize=[1000]
//        Packet size of UDP application
//    --udpRate=[75000000bps]
//        Data rate of UDP application
//    --useReservationSignal=[true]
//        Defines whether reservation signal will be used when used channel access manager at LTE eNb
//    --voiceEnabled=[false]
//        Whether to enable voice
//    --wifiQueueMaxDelay=[500]
//        change from default 500 ms to change the maximum queue dwell time for Wifi packets
//    --wifiQueueMaxSize=[400]
//        change from default 400 packets to change the queue size for Wifi packets
//  Most of the above are specific to the LAA module.  In particular,
//  passing the argument '--cellConfigA=Wifi' will cause both
//  cells to use Wifi on the same channel but on a different SSID,
//  while the (default) will cause cell A to use Laa and cellB to use Wi-Fi.
//  Passing an argument  of '--cellConfigB=Laa" will cause both cells to
//  use LAA.
//
//  By default, the simulation is configured to use a constant-bit-rate
//  UDP flow in each operator network, defaulting to 75 Mb/s.
//
//  We refer to the left-most cell as 'cell A' and the right-most
//  cell as 'cell B'.  Both cells are configured to run UDP transfers from
//  client to BS.  The application data rate is 75 Mb/s, which does not
//  saturate either link since 2x2 MIMO is the default.  Usually for
//  coexistence studies, the "Ftp" or "Tcp" transport type is selected.
//
//  It is outside of the scope of this header to try to document everything
//  about this program; see the model library documentation.
//
//  The following sample output is provided.
//
//  When run with no arguments, something like this will show:
//
//  ./waf --run "lte-wifi-simple"
//
//
//
//
//Running simulation for 1 sec of data transfer; 5 sec overall
//Operator A: LAA; number of cells 1; number of UEs 1
//Operator B: Wi-Fi; number of cells 1; number of UEs 1
//Total txop duration: 0 seconds.
//Total phy arrivals duration: 0 seconds.
//--------monitorA----------
//Flow 1 (1.0.0.2:49153 -> 7.0.0.2:9) proto UDP
//  Tx Packets: 7303
//  Tx Bytes:   7507484
//  TxOffered:  60.0599 Mbps
//  Rx Bytes:   7507484
//  Throughput: 71.4195 Mbps
//  Mean delay:  27.8461 ms
//  Mean jitter:  0.201836 ms
//  Rx Packets: 7303
//--------monitorB----------
//Flow 1 (12.0.0.1:49153 -> 18.0.0.2:9) proto UDP
//  Tx Packets: 5197
//  Tx Bytes:   5342516
//  TxOffered:  42.7401 Mbps
//  Rx Bytes:   5342516
//  Throughput: 77.0265 Mbps
//  Mean delay:  0.509075 ms
//  Mean jitter:  0.0645998 ms
//  Rx Packets: 5197
//--- List of associations (time and node id) ---
//0.600294 3 associate 00:00:00:00:00:06
//
//  In this case, both LAA and Wi-Fi obtain good throughput and low latency.
//

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/internet-module.h>
#include <ns3/point-to-point-module.h>
#include <ns3/lte-module.h>
#include <ns3/wifi-module.h>
#include <ns3/config-store-module.h>
#include <ns3/spectrum-module.h>
#include <ns3/applications-module.h>
#include <ns3/flow-monitor-module.h>
#include <ns3/propagation-module.h>
#include <ns3/scenario-helper.h>
#include <ns3/lte-wifi-coexistence-helper.h>
#include <ns3/lbt-access-manager.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LteWifiCoexistenceSimple");

// Global Values are used in place of command line arguments so that these
// values may be managed in the ns-3 ConfigStore system.
static ns3::GlobalValue g_d1 ("d1",
                              "intra-cell separation (e.g. AP to STA)",
                              ns3::DoubleValue (10),
                              ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_d2 ("d2",
                              "inter-cell separation",
                              ns3::DoubleValue (50),
                              ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_duration ("duration",
                                    "Data transfer duration (seconds)",
                                    ns3::DoubleValue (1),
                                    ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_cellConfigA ("cellConfigA",
                                       "Laa, Lte, or Wifi",
                                        ns3::EnumValue (LAA),
                                        ns3::MakeEnumChecker (WIFI, "Wifi",
                                                              LTE, "Lte",
                                                              LAA, "Laa"));
static ns3::GlobalValue g_cellConfigB ("cellConfigB",
                                       "Laa, Lte, or Wifi",
                                        ns3::EnumValue (WIFI),
                                        ns3::MakeEnumChecker (WIFI, "Wifi",
                                                              LTE, "Lte",
                                                              LAA, "Laa"));

static ns3::GlobalValue g_channelAccessManager ("ChannelAccessManager",
                                                "Default, DutyCycle, Lbt",
                                                ns3::EnumValue (Lbt),
                                                ns3::MakeEnumChecker (Default, "Default",
                                                                      DutyCycle, "DutyCycle",
                                                                      Lbt, "Lbt"));

static ns3::GlobalValue g_lbtTxop ("lbtTxop",
                                   "TxOp for LBT devices (ms)",
                                   ns3::DoubleValue (8.0),
                                   ns3::MakeDoubleChecker<double> (4.0, 20.0));

static ns3::GlobalValue g_useReservationSignal ("useReservationSignal",
                                                "Defines whether reservation signal will be used when used channel access manager at LTE eNb",
                                                ns3::BooleanValue (true),
                                                ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_laaEdThreshold ("laaEdThreshold",
                                   "CCA-ED threshold for channel access manager (dBm)",
                                   ns3::DoubleValue (-72.0),
                                   ns3::MakeDoubleChecker<double> (-100.0, -50.0));

static ns3::GlobalValue g_pcap ("pcapEnabled",
                                "Whether to enable pcap trace files for Wi-Fi",
                                ns3::BooleanValue (false),
                                ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_ascii ("asciiEnabled",
                                "Whether to enable ascii trace files for Wi-Fi",
                                ns3::BooleanValue (false),
                                ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_transport ("transport",
                                     "whether to use 3GPP Ftp, Udp, or Tcp",
                                     ns3::EnumValue (UDP),
                                     ns3::MakeEnumChecker (FTP, "Ftp",
                                                           UDP, "Udp",
                                                           TCP, "Tcp"));

static ns3::GlobalValue g_lteDutyCycle ("lteDutyCycle",
                                    "Duty cycle value to be used for LTE",
                                    ns3::DoubleValue (1),
                                    ns3::MakeDoubleChecker<double> (0.0, 1.0));

static ns3::GlobalValue g_generateRem ("generateRem",
                                       "if true, will generate a REM and then abort the simulation;"
                                       "if false, will run the simulation normally (without generating any REM)",
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

static ns3::GlobalValue g_cwUpdateRule ("cwUpdateRule",
                                         "Rule that will be used to update contention window of LAA node",
                                         ns3::EnumValue (LbtAccessManager::NACKS_80_PERCENT),
                                         ns3::MakeEnumChecker (ns3::LbtAccessManager::ALL_NACKS, "all",
                                        		 	 	 	   ns3::LbtAccessManager::ANY_NACK, "any",
                                        		 	 	 	   ns3::LbtAccessManager::NACKS_10_PERCENT, "nacks10",
                                        		 	 	 	   ns3::LbtAccessManager::NACKS_80_PERCENT, "nacks80"));

// Higher lambda means faster arrival rate; values [0.5, 1, 1.5, 2, 2.5]
// recommended
static ns3::GlobalValue g_ftpLambda ("ftpLambda",
                                    "Lambda value for FTP model 1 application",
                                    ns3::DoubleValue (0.5),
                                    ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_voiceEnabled ("voiceEnabled",
                                       "Whether to enable voice",
                                       ns3::BooleanValue (false),
                                       ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_indoorLossModel ("indoorLossModel",
                                           "TypeId string of indoor propagation loss model",
                                            ns3::StringValue ("ns3::LogDistancePropagationLossModel"),
                                            ns3::MakeStringChecker ());

// Global variables for use in callbacks.
double g_signalDbmAvg[2];
double g_noiseDbmAvg[2];
uint32_t g_samples[2];
uint16_t g_channelNumber[2];
uint32_t g_rate[2];

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();
  // parse again so you can override input file default values via command line
  cmd.Parse (argc, argv);

  DoubleValue doubleValue;
  EnumValue enumValue;
  BooleanValue booleanValue;
  StringValue stringValue;

  GlobalValue::GetValueByName ("d1", doubleValue);
  double d1 = doubleValue.Get ();
  GlobalValue::GetValueByName ("d2", doubleValue);
  double d2 = doubleValue.Get ();
  GlobalValue::GetValueByName ("cellConfigA", enumValue);
  enum Config_e cellConfigA = (Config_e) enumValue.Get ();
  GlobalValue::GetValueByName ("cellConfigB", enumValue);
  enum Config_e cellConfigB = (Config_e) enumValue.Get ();
  GlobalValue::GetValueByName ("duration", doubleValue);
  double duration = doubleValue.Get ();
  GlobalValue::GetValueByName ("ChannelAccessManager", enumValue);
  enum Config_ChannelAccessManager channelAccessManager = (Config_ChannelAccessManager) enumValue.Get ();
  GlobalValue::GetValueByName ("lbtTxop", doubleValue);
  double lbtTxop = doubleValue.Get ();
  GlobalValue::GetValueByName ("laaEdThreshold", doubleValue);
  double laaEdThreshold = doubleValue.Get ();
  GlobalValue::GetValueByName ("useReservationSignal", booleanValue);
  bool useReservationSignal = booleanValue.Get ();
  GlobalValue::GetValueByName ("transport", enumValue);
  enum Transport_e transport = (Transport_e) enumValue.Get ();
  GlobalValue::GetValueByName ("lteDutyCycle", doubleValue);
  double lteDutyCycle = doubleValue.Get ();
  GlobalValue::GetValueByName ("generateRem", booleanValue);
  bool generateRem = booleanValue.Get ();
  GlobalValue::GetValueByName ("simTag", stringValue);
  std::string simTag = stringValue.Get ();
  GlobalValue::GetValueByName ("outputDir", stringValue);
  std::string outputDir = stringValue.Get ();
  GlobalValue::GetValueByName ("cwUpdateRule", enumValue);
  enum  LbtAccessManager::CWUpdateRule_t cwUpdateRule = (LbtAccessManager::CWUpdateRule_t) enumValue.Get ();
  GlobalValue::GetValueByName ("indoorLossModel", stringValue);
  std::string indoorLossModel = stringValue.Get ();

  Config::SetDefault ("ns3::LteChannelAccessManager::EnergyDetectionThreshold", DoubleValue (laaEdThreshold));
  switch (channelAccessManager)
    {
    case Lbt:
      Config::SetDefault ("ns3::LteWifiCoexistenceHelper::ChannelAccessManagerType", StringValue ("ns3::LbtAccessManager"));
      Config::SetDefault ("ns3::LbtAccessManager::Txop", TimeValue (Seconds (lbtTxop/1000.0)));
      Config::SetDefault ("ns3::LbtAccessManager::UseReservationSignal", BooleanValue(useReservationSignal));
      Config::SetDefault ("ns3::LbtAccessManager::CwUpdateRule", EnumValue(cwUpdateRule));
      break;
    case DutyCycle:
      Config::SetDefault ("ns3::LteWifiCoexistenceHelper::ChannelAccessManagerType", StringValue ("ns3::DutyCycleAccessManager"));
      Config::SetDefault ("ns3::DutyCycleAccessManager::OnDuration", TimeValue (MilliSeconds (60)));
      Config::SetDefault ("ns3::DutyCycleAccessManager::OnStartTime",TimeValue (MilliSeconds (0)));
      Config::SetDefault ("ns3::DutyCycleAccessManager::DutyCyclePeriod",TimeValue (MilliSeconds (80)));
      break;
    default:
      //default LTE channel access manager will be used, LTE always transmits
      break;
    }

  // When logging, use prefixes
  LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);

  // Create nodes and containers
  NodeContainer bsNodesA, bsNodesB;  // for APs and eNBs
  NodeContainer ueNodesA, ueNodesB;  // for STAs and UEs
  NodeContainer allWirelessNodes;  // container to hold all wireless nodes
  // Each network A and B gets one type of node each
  bsNodesA.Create (1);
  bsNodesB.Create (1);
  ueNodesA.Create (1);
  ueNodesB.Create (1);
  allWirelessNodes = NodeContainer (bsNodesA, bsNodesB, ueNodesA, ueNodesB);

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));   // eNB1/AP in cell 0
  positionAlloc->Add (Vector (d2, d1, 0.0)); // AP in cell 1
  positionAlloc->Add (Vector (0.0, d1, 0.0));  // UE1/STA in cell 0
  positionAlloc->Add (Vector (d2, 0.0, 0.0));  // STA in cell 1
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (allWirelessNodes);

  bool disableApps = false;
  Time durationTime = Seconds (duration);


  // REM settings tuned to get a nice figure for this specific scenario
  Config::SetDefault ("ns3::RadioEnvironmentMapHelper::OutputFile", StringValue ("lte-wifi-simple.rem"));
  Config::SetDefault ("ns3::RadioEnvironmentMapHelper::XMin", DoubleValue (-50));
  Config::SetDefault ("ns3::RadioEnvironmentMapHelper::XMax", DoubleValue (250));
  Config::SetDefault ("ns3::RadioEnvironmentMapHelper::YMin", DoubleValue (-50));
  Config::SetDefault ("ns3::RadioEnvironmentMapHelper::YMax", DoubleValue (250));
  Config::SetDefault ("ns3::RadioEnvironmentMapHelper::XRes", UintegerValue (600));
  Config::SetDefault ("ns3::RadioEnvironmentMapHelper::YRes", UintegerValue (600));
  Config::SetDefault ("ns3::RadioEnvironmentMapHelper::Z", DoubleValue (1.5));

  // we want deterministic behavior in this simple scenario, so we disable shadowing
  Config::SetDefault ("ns3::Ieee80211axIndoorPropagationLossModel::Sigma", DoubleValue (0));

  // Enable aggregation for AC_BE; see bug 2471 in tracker
  Config::SetDefault ("ns3::RegularWifiMac::BE_BlockAckThreshold", UintegerValue (2));

  // Specify some physical layer parameters that will be used below and
  // in the scenario helper.
  PhyParams phyParams;
  phyParams.m_bsTxGain = 5; // dB antenna gain
  phyParams.m_bsRxGain = 5; // dB antenna gain
  phyParams.m_bsTxPower = 18; // dBm
  phyParams.m_bsNoiseFigure = 5; // dB
  phyParams.m_ueTxGain = 0; // dB antenna gain
  phyParams.m_ueRxGain = 0; // dB antenna gain
  phyParams.m_ueTxPower = 18; // dBm
  phyParams.m_ueNoiseFigure = 9; // dB

  // calculate rx power corresponding to d2 for logging purposes
  // note: a separate propagation loss model instance is used, make
  // sure the settings are consistent with the ones used for the
  // simulation
  const uint32_t earfcn = 255444;
  double dlFreq = LteSpectrumValueHelper::GetCarrierFrequency (earfcn);
  Ptr<PropagationLossModel> plm = CreateObject<Ieee80211axIndoorPropagationLossModel> ();
  plm->SetAttribute ("Frequency", DoubleValue (dlFreq));
  double txPowerFactors = phyParams.m_bsTxGain + phyParams.m_ueRxGain +
                          phyParams.m_bsTxPower;
  double rxPowerDbmD1 = plm->CalcRxPower (txPowerFactors,
                                          bsNodesA.Get (0)->GetObject<MobilityModel> (),
                                          ueNodesA.Get (0)->GetObject<MobilityModel> ());
  double rxPowerDbmD2 = plm->CalcRxPower (txPowerFactors,
                                          bsNodesA.Get (0)->GetObject<MobilityModel> (),
                                          ueNodesB.Get (0)->GetObject<MobilityModel> ());

  std::ostringstream simulationParams;
  simulationParams << d1 << " " << d2 << " "
                   << rxPowerDbmD1 << " "
                   << rxPowerDbmD2 << " "
                   << lteDutyCycle << " ";

  ConfigureAndRunScenario (cellConfigA, cellConfigB, bsNodesA, bsNodesB, ueNodesA, ueNodesB, phyParams, durationTime, transport, indoorLossModel, disableApps, lteDutyCycle, generateRem, outputDir + "/lte_wifi_simple_" + simTag, simulationParams.str ());

  return 0;
}

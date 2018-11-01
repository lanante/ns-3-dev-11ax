/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 University of Washington
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
 * Author: Tom Henderson <tomh@tomh.org> and Nicola Baldo <nbaldo@cttc.es>
 */

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/scenario-helper.h>
#include <ns3/lbt-access-manager.h>
#include <ns3/lte-wifi-coexistence-helper.h>

// The topology of this simulation program follows that of
// 3GPP TR 36.889, Section A.1.2 "Outdoor scenario for LAA coexistence evaluations"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LteWifiOutdoor");


// Global Values are used in place of command line arguments so that these
// values may be managed in the ns-3 ConfigStore system.

static ns3::GlobalValue g_duration ("duration",
                                    "Data transfer duration (seconds)",
                                    ns3::DoubleValue (2),
                                    ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_cellConfigA ("cellConfigA",
                                       "Lte, Wifi, or Laa",
                                        ns3::EnumValue (WIFI),
                                        ns3::MakeEnumChecker (WIFI, "Wifi",
                                                              LTE, "Lte",
                                                              LAA, "Laa"));
static ns3::GlobalValue g_cellConfigB ("cellConfigB",
                                       "Lte, Wifi, or Laa",
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

static ns3::GlobalValue g_nMacroEnbSites ("nMacroEnbSites",
                                          "How many macro sites there are",
                                          ns3::UintegerValue (7),
                                          ns3::MakeUintegerChecker<uint32_t> ());

static ns3::GlobalValue g_nMacroEnbSitesX ("nMacroEnbSitesX",
                                           "(minimum) number of sites along the X-axis of the hex grid",
                                           ns3::UintegerValue (2),
                                           ns3::MakeUintegerChecker<uint32_t> ());

static ns3::GlobalValue g_interSiteDistance ("interSiteDistance",
                                             "min distance between two nearby macro cell sites",
                                             ns3::DoubleValue (500),
                                             ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_smallCellDroppingRadius ("smallCellDroppingRadius",
               "Radius for small cell dropping in a cluster",
               ns3::DoubleValue (50),
               ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_ueDroppingRadius ("ueDroppingRadius",
               "Radius for UE dropping in a cluster",
               ns3::DoubleValue (70),
               ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_minDistScSc ("minDistScSc",
               "minimum distance between small cell and small cell",
               ns3::DoubleValue (20),
               ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_minDistIoScSc ("minDistIoScSc",
           "minimum distance between inter-operator small cell and small cell",
           ns3::DoubleValue (10),
           ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_minDistScUe ("minDistScUe",
               "minimum distance between small cell and UE",
               ns3::DoubleValue (3),
               ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_minDistUeUe ("minDistUeUe",
               "minimum distance between UE and UE",
               ns3::DoubleValue (3),
               ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_minDistMacroCluster ("minDistMacroCluster",
                 "minimum distance between macro cell site and cluster center",
                 ns3::DoubleValue (105),
                 ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_minDistMacroUe ("minDistMacroUe",
                 "minimum distance between macro cell site and UE",
                 ns3::DoubleValue (35),
                 ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_minDistClusterCluster ("minDistClusterCluster",
             "minimum distance between two cell cluster centers",
             ns3::DoubleValue (100),
             ns3::MakeDoubleChecker<double> ());


static ns3::GlobalValue g_cwUpdateRule ("cwUpdateRule",
                                         "Rule that will be used to update contention window of LAA node",
                                         ns3::EnumValue (LbtAccessManager::NACKS_80_PERCENT),
                                         ns3::MakeEnumChecker (ns3::LbtAccessManager::ALL_NACKS, "all",
                                                     ns3::LbtAccessManager::ANY_NACK, "any",
                                                     ns3::LbtAccessManager::NACKS_10_PERCENT, "nacks10",
                                                     ns3::LbtAccessManager::NACKS_80_PERCENT, "nacks80"));


int
main (int argc, char *argv[])
{
  // change some default attributes so that they are reasonable for
  // this scenario, but do this before processing command line
  // arguments, so that the user is allowed to override these settings
  Config::SetDefault ("ns3::UdpClient::Interval", TimeValue (MilliSeconds (1)));
  Config::SetDefault ("ns3::UdpClient::MaxPackets", UintegerValue (1000000));
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (10 * 1024));
  Config::SetDefault ("ns3::RadioEnvironmentMapHelper::XRes", UintegerValue (100));
  Config::SetDefault ("ns3::RadioEnvironmentMapHelper::YRes", UintegerValue (100));


  Config::SetDefault ("ns3::RadioEnvironmentMapHelper::OutputFile", StringValue ("lte-wifi-outdoor.rem"));
  Config::SetDefault ("ns3::RadioEnvironmentMapHelper::XMin", DoubleValue (-250));
  Config::SetDefault ("ns3::RadioEnvironmentMapHelper::XMax", DoubleValue (1335));
  Config::SetDefault ("ns3::RadioEnvironmentMapHelper::YMin", DoubleValue (-360));
  Config::SetDefault ("ns3::RadioEnvironmentMapHelper::YMax", DoubleValue (1220));
  Config::SetDefault ("ns3::RadioEnvironmentMapHelper::Z", DoubleValue (0));

  // Effectively disable ARP cache entries from timing out
  Config::SetDefault ("ns3::ArpCache::AliveTimeout", TimeValue (Seconds (10000)));

  // Enable aggregation for AC_BE; see bug 2471 in tracker
  Config::SetDefault ("ns3::RegularWifiMac::BE_BlockAckThreshold", UintegerValue (2));

  CommandLine cmd;
  cmd.Parse (argc, argv);

  // used for consistent position assignment across program configurations
  int64_t streamIndex = 1;

  // This program has two operators, and nominally 4 cells per operator per cluster and
  // and 20 UEs per operator per cluster cell.  These variables can be tuned below for
  // e.g. debugging on a smaller scale scenario
  uint32_t numUePerClusterPerOperator = 20;
  uint32_t numBsPerClusterPerOperator = 4;

  // Some debugging settings not exposed as command line args are below

  // To disable application data flow (e.g. for visualizing mobility
  // in the visualizer), set below to true
  bool disableApps = false;

  // Extract global values into local variables.
  UintegerValue uintegerValue;
  IntegerValue integerValue;
  DoubleValue doubleValue;
  BooleanValue booleanValue;
  StringValue stringValue;
  EnumValue enumValue;
  GlobalValue::GetValueByName ("cellConfigA", enumValue);
  enum Config_e cellConfigA = (Config_e) enumValue.Get ();
  GlobalValue::GetValueByName ("cellConfigB", enumValue);
  enum Config_e cellConfigB = (Config_e) enumValue.Get ();
  GlobalValue::GetValueByName ("duration", doubleValue);
  double duration = doubleValue.Get ();
  Time durationTime = Seconds (duration);
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
  GlobalValue::GetValueByName ("remDir", stringValue);
  std::string remDir = stringValue.Get ();

  GlobalValue::GetValueByName ("nMacroEnbSites", uintegerValue);
  uint32_t nMacroEnbSites = uintegerValue.Get ();
  GlobalValue::GetValueByName ("nMacroEnbSitesX", uintegerValue);
  uint32_t nMacroEnbSitesX = uintegerValue.Get ();
  GlobalValue::GetValueByName ("interSiteDistance", doubleValue);
  double interSiteDistance = doubleValue.Get ();
  GlobalValue::GetValueByName ("smallCellDroppingRadius", doubleValue);
  double smallCellDroppingRadius = doubleValue.Get ();
  GlobalValue::GetValueByName ("ueDroppingRadius", doubleValue);
  double ueDroppingRadius = doubleValue.Get ();

  GlobalValue::GetValueByName ("minDistScSc", doubleValue);
  double minDistScSc = doubleValue.Get ();
  GlobalValue::GetValueByName ("minDistIoScSc", doubleValue);
  double minDistIoScSc = doubleValue.Get ();
  GlobalValue::GetValueByName ("minDistScUe", doubleValue);
  double minDistScUe = doubleValue.Get ();
  GlobalValue::GetValueByName ("minDistUeUe", doubleValue);
  double minDistUeUe = doubleValue.Get ();
  GlobalValue::GetValueByName ("minDistMacroCluster", doubleValue);
  double minDistMacroCluster = doubleValue.Get ();
  GlobalValue::GetValueByName ("minDistMacroUe", doubleValue);
  double minDistMacroUe = doubleValue.Get ();
  GlobalValue::GetValueByName ("minDistClusterCluster", doubleValue);
  double minDistClusterCluster = doubleValue.Get ();

  GlobalValue::GetValueByName ("ChannelAccessManager", enumValue);
  enum Config_ChannelAccessManager channelAccessManager = (Config_ChannelAccessManager) enumValue.Get ();
  GlobalValue::GetValueByName ("lbtTxop", doubleValue);
  double lbtTxop = doubleValue.Get ();
  GlobalValue::GetValueByName ("laaEdThreshold", doubleValue);
  double laaEdThreshold = doubleValue.Get ();

 GlobalValue::GetValueByName ("useReservationSignal", booleanValue);
  bool useReservationSignal = booleanValue.Get ();
  GlobalValue::GetValueByName ("cwUpdateRule", enumValue);
  enum  LbtAccessManager::CWUpdateRule_t cwUpdateRule = (LbtAccessManager::CWUpdateRule_t) enumValue.Get ();


 GlobalValue::GetValueByName ("asciiEnabled", booleanValue);
  if (booleanValue.Get () == true)
    {
      PacketMetadata::Enable ();
    }

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

  //
  // Topology setup phase
  //

  NodeContainer bsNodesA;
  NodeContainer bsNodesB;

  NodeContainer ueNodesA;
  NodeContainer ueNodesB;

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  Ptr<LteHexGridEnbTopologyHelper> lteHexGridEnbTopologyHelper = CreateObject<LteHexGridEnbTopologyHelper> ();

  lteHexGridEnbTopologyHelper->SetAttribute ("InterSiteDistance", DoubleValue (interSiteDistance));
  lteHexGridEnbTopologyHelper->SetAttribute ("MinX", DoubleValue (interSiteDistance/2));
  lteHexGridEnbTopologyHelper->SetAttribute ("GridWidth", UintegerValue (nMacroEnbSitesX));

  double macroCellRadius = lteHexGridEnbTopologyHelper->GetCellRadius ();

  std::ofstream topologyOutfile;
  if (generateRem)
    {
      NS_LOG_LOGIC ("generate gnuplottable topology file");
      std::string topologyFileName = remDir + "/lte-wifi-outdoor-topology.gnuplot";
      topologyOutfile.open (topologyFileName.c_str (), std::ios_base::out | std::ios_base::trunc);
      if (!topologyOutfile.is_open ())
        {
          NS_LOG_ERROR ("Can't open " << topologyFileName);
          return 1;
        }
    }

  for (uint32_t macroCellId = 0; macroCellId < (nMacroEnbSites*3); ++macroCellId)
    {
      // determine position of cluster
      Vector macroCellCenter = lteHexGridEnbTopologyHelper->GetCellCenterPosition (macroCellId);
      Vector macroCellSite = lteHexGridEnbTopologyHelper->GetSitePosition (macroCellId);
      Ptr<Min2dDistancePositionAllocator> clusterMinDistPosAlloc = CreateObject<Min2dDistancePositionAllocator> ();
      Ptr<UniformHexagonPositionAllocator> clusterHexPositionAlloc = CreateObject<UniformHexagonPositionAllocator> ();
      streamIndex += clusterHexPositionAlloc->AssignStreams (streamIndex);
      clusterHexPositionAlloc->SetAttribute ("X", DoubleValue (macroCellCenter.x));
      clusterHexPositionAlloc->SetAttribute ("Y", DoubleValue (macroCellCenter.y));
      clusterHexPositionAlloc->SetAttribute ("rho", DoubleValue (macroCellRadius - minDistClusterCluster/2));
      clusterHexPositionAlloc->SetAttribute ("theta", DoubleValue (M_PI/2));
      clusterMinDistPosAlloc->SetPositionAllocator (clusterHexPositionAlloc);
      clusterMinDistPosAlloc->AddPositionDistance (macroCellSite, minDistMacroCluster);
      Vector cluster = clusterMinDistPosAlloc->GetNext ();


      Ptr<UniformDiscPositionAllocator> bsDiscPosAlloc = CreateObject<UniformDiscPositionAllocator> ();
      bsDiscPosAlloc->SetAttribute ("X", DoubleValue (cluster.x));
      bsDiscPosAlloc->SetAttribute ("Y", DoubleValue (cluster.y));
      bsDiscPosAlloc->SetAttribute ("rho", DoubleValue (smallCellDroppingRadius));
      streamIndex += bsDiscPosAlloc->AssignStreams (streamIndex);


      // now place BSs in cluster
      // first place BSs of operator A in cluster
      NodeContainer bsNodesClusterA;
      Ptr<Min2dDistancePositionAllocator> bsMinDistPosAllocA
        = CreateObject<Min2dDistancePositionAllocator> ();
      bsMinDistPosAllocA->SetPositionAllocator (bsDiscPosAlloc);
      mobility.SetPositionAllocator (bsMinDistPosAllocA);
      // BSs are placed one by one to guarantee min distance w.r.t. previous BSs
      for (uint32_t i = 0; i < numBsPerClusterPerOperator; ++i)
        {
          NodeContainer bs;
          bs.Create (1);
          mobility.Install (bs);
          bsNodesClusterA.Add (bs);
          bsMinDistPosAllocA->AddNodesDistance (bs, minDistScSc);
        }

      // then place BSs of operator B in cluster
      NodeContainer bsNodesClusterB;
      Ptr<Min2dDistancePositionAllocator> bsMinDistPosAllocB
        = CreateObject<Min2dDistancePositionAllocator> ();
      bsMinDistPosAllocB->SetPositionAllocator (bsDiscPosAlloc);
      mobility.SetPositionAllocator (bsMinDistPosAllocB);
      // guarantee inter-operator distance
      bsMinDistPosAllocB->AddNodesDistance (bsNodesClusterA, minDistIoScSc);
      // BSs are placed one by one to guarantee min distance w.r.t. previous BSs
      for (uint32_t i = 0; i < numBsPerClusterPerOperator; ++i)
        {
          NodeContainer bs;
          bs.Create (1);
          mobility.Install (bs);
          bsNodesClusterB.Add (bs);
          bsMinDistPosAllocB->AddNodesDistance (bs, minDistScSc);
        }

      // finally place UEs

      Ptr<UniformDiscPositionAllocator> ueDiscPosAlloc = CreateObject<UniformDiscPositionAllocator> ();
      streamIndex += ueDiscPosAlloc->AssignStreams (streamIndex);
      ueDiscPosAlloc->SetAttribute ("X", DoubleValue (cluster.x));
      ueDiscPosAlloc->SetAttribute ("Y", DoubleValue (cluster.y));
      ueDiscPosAlloc->SetAttribute ("rho", DoubleValue (ueDroppingRadius));
      Ptr<Min2dDistancePositionAllocator> ueMinDistPosAlloc
        = CreateObject<Min2dDistancePositionAllocator> ();
      ueMinDistPosAlloc->SetPositionAllocator (ueDiscPosAlloc);
      mobility.SetPositionAllocator (ueMinDistPosAlloc);

      // guarantee small cell-UE distance
      ueMinDistPosAlloc->AddNodesDistance (bsNodesClusterA, minDistScUe);
      ueMinDistPosAlloc->AddNodesDistance (bsNodesClusterB, minDistScUe);
      // guarantee macro site distance
      ueMinDistPosAlloc->AddPositionDistance (macroCellSite, minDistMacroUe);
      // UEs are placed one by one to guarantee min distance w.r.t. previous UEs
      NodeContainer ueNodesClusterA;
      for (uint32_t i = 0; i < numUePerClusterPerOperator; ++i)
        {
          NodeContainer ue;
          ue.Create (1);
          mobility.Install (ue);
          ueNodesClusterA.Add (ue);
          ueMinDistPosAlloc->AddNodesDistance (ue, minDistUeUe);
        }
      NodeContainer ueNodesClusterB;
      for (uint32_t i = 0; i < numUePerClusterPerOperator; ++i)
        {
          NodeContainer ue;
          ue.Create (1);
          mobility.Install (ue);
          ueNodesClusterB.Add (ue);
          ueMinDistPosAlloc->AddNodesDistance (ue, minDistUeUe);
        }


      bsNodesA.Add (bsNodesClusterA);
      bsNodesB.Add (bsNodesClusterB);

      ueNodesA.Add (ueNodesClusterA);
      ueNodesB.Add (ueNodesClusterB);


      if (generateRem)
        {
          NS_LOG_LOGIC ("output gnuplottable hexagon for macroCellId " << macroCellId);

          DoubleValue theta;
          clusterHexPositionAlloc->GetAttribute ("theta", theta);
          DoubleValue X;
          clusterHexPositionAlloc->GetAttribute ("X", X);
          DoubleValue Y;
          clusterHexPositionAlloc->GetAttribute ("Y", Y);

          topologyOutfile << "set object " << (macroCellId*3) + 1 << " polygon from \\\n";

          for (uint32_t vertexId = 0; vertexId < 6; ++vertexId)
            {
              // angle of the vertex w.r.t. y-axis
              double a = vertexId * (M_PI/3.0) + theta.Get ();
              double x =  - macroCellRadius * sin (a) + X.Get ();
              double y =  macroCellRadius * cos (a) + Y.Get ();
              topologyOutfile << x << ", " << y << " to \\\n";
            }
          // repeat vertex 0 to close polygon
          uint32_t vertexId = 0;
          double a = vertexId * (M_PI/3.0) + theta.Get ();
          double x =  - macroCellRadius * sin (a) + X.Get ();
          double y =  macroCellRadius * cos (a) + Y.Get ();
          topologyOutfile << x << ", " << y;
          topologyOutfile << " front fs empty \n";

          NS_LOG_LOGIC ("output gnuplottable BS and UE allocation disc ");
          topologyOutfile << "set object " << (macroCellId*3 + 2)
                          << " circle at " << cluster.x << "," << cluster.y
                          << " size " << ueDroppingRadius << " front fs empty border lt 1 \n";
          topologyOutfile << "set object " << (macroCellId*3 + 3)
                          << " circle at " << cluster.x << "," << cluster.y
                          << " size " << smallCellDroppingRadius << " front fs empty border lt 1 \n";

          NS_LOG_LOGIC ("output gnuplottable arrow indicating macro cell antenna boresight");
          double arrowLength = macroCellRadius/4.0;
          double alpha = lteHexGridEnbTopologyHelper->GetAntennaOrientationDegrees (macroCellId) * M_PI / 180.0;
          topologyOutfile << "set arrow from " << macroCellSite.x << "," << macroCellSite.y
                          << " rto " << arrowLength * std::cos(alpha)
                          << "," << arrowLength * std::sin(alpha)
                          << " arrowstyle 1 \n";

        }



    }

  if (generateRem)
    {
      // only works with LTE
      cellConfigA = LTE;
      cellConfigB = LTE;
    }

  std::ostringstream simulationParams;
  simulationParams << "";

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

  ConfigureAndRunScenario (cellConfigA, cellConfigB, bsNodesA, bsNodesB, ueNodesA, ueNodesB, phyParams, durationTime, transport, "ns3::ItuUmiPropagationLossModel", disableApps, lteDutyCycle, generateRem, outputDir + "/lte_wifi_outdoor_" + simTag, simulationParams.str ());

  return 0;
}

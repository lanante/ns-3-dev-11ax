/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 *  Author: Biljana Bojovic <bbojovic@cttc.es> and Lorenza Giupponi <lgiupponi@cttc.es>
 */

#include <sstream>
#include "ns3/test.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/boolean.h"
#include "ns3/mobility-helper.h"
#include "ns3/lte-helper.h"
#include "ns3/ff-mac-scheduler.h"
#include "ns3/lte-enb-rrc.h"
#include "ns3/lte-enb-phy.h"
#include "ns3/lte-enb-net-device.h"
#include "ns3/lte-ue-phy.h"
#include "ns3/lte-ue-net-device.h"
#include "ns3/scenario-helper.h"
#include <ns3/flow-monitor-module.h>
#include "ns3/lbt-access-manager.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/lte-module.h"
#include "ns3/wifi-module.h"
#include "ns3/spectrum-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/propagation-module.h"
#include "ns3/config-store-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-wifi-coexistence-helper.h"
#include "ns3/lbt-access-manager.h"
#include "ns3/scenario-helper.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LbtTxopTest");

/*
*  We test on the following scenario that LAA node is transmitting reservation signal and the data.
*
*     UE
*      |
*   d1 |
*      |     d2
*     LAA  _ _ _  wifi monitor
*      .
*      .
*      .p2p
*      .
*     client
*
*  The following illustration refers to implementation 1. of channel access procedure (once that
*  the channel access is granted there is 2 ms delay caused by delay between mac and phy, no data
*  is ready to be transmitted, so channel is reserved with reservation signal).
*  For each txop value we expect to see the following pattern (channel access implementation 1),
*  when there is data to be transmitted:
*
*				      Txop				              Txop
*		                     starts				              ends
*			   	       |                                               |
*			   	       |				               |
*			   	       v				               v
*			               .................................................
*  Channel state     		 ||IDLE|BUSY||   BUSY   ||   BUSY    ||   DATA   ||IDLE|IDLE||
*  Subframe number	         ||    n    ||   n+1    ||   n+2     ||   n+3    ||   n+4   ||
*  Reservation signal 		 ||  - | yes||   yes    ||   yes     ||    no    ||   no    ||
*			                      <-----mac-phy delay--->
*/


class LbtTxopTestSuite : public TestSuite
{
public:
	LbtTxopTestSuite ();
};


class LbtTxopTestCase : public TestCase
{
 public:

  LbtTxopTestCase (std::string name,
		  bool useReservationSignal,
		  double txop,
		  double d1,
		  double d2,
		  Time udpInterval,
		  Transport_e traffic);
  virtual ~LbtTxopTestCase ();

  void CheckState (LbtAccessManager::LbtState state, bool duringReservationSignal);
  void DataTracedCallback (uint32_t bytes);
  void ReservationSignalCheck (Time startTime, Time duration);
  void TxopCheck (Time startTime, Time duration, Time nextSubframeStarts);

private:

  virtual void DoRun (void);
  bool m_useReservationSignal;
  double m_txop;
  double m_d1;
  double m_d2;
  Time m_udpInterval;
  Time m_startTime;
  Time m_stopTime;
  uint32_t m_txopCounter;
  Time m_ttiDuration;
  uint32_t m_bytesTransmitedCounter;
  Transport_e m_transport;
  Ptr<LteEnbPhy> m_ltePhy;
};


void
LteTestReservationSignalCallback (LbtTxopTestCase *testcase, std::string path, Time startTime, Time duration)
{
  testcase->ReservationSignalCheck (startTime, duration);
}

void
LteDataTracedCallback (LbtTxopTestCase *testcase, std::string path, uint32_t bytes)
{
  testcase->DataTracedCallback (bytes);
}



void
LteTestTxOpCallback (LbtTxopTestCase *testcase, std::string path, Time startTime, Time duration, Time nextSubframeStarts)
{
  testcase->TxopCheck (startTime, duration, nextSubframeStarts);
}


/**
 * TestSuite
 */

LbtTxopTestSuite::LbtTxopTestSuite ()
  : TestSuite ("lbt-txop-test", SYSTEM)
{
  AddTestCase (new LbtTxopTestCase ("txop=4,  d1=50, d2=50", true, 4.0, 10.0, 50.0, MicroSeconds(106), UDP ), TestCase::QUICK);
  AddTestCase (new LbtTxopTestCase ("txop=5,  d1=50, d2=50", true, 5.0, 10.0, 50.0, MicroSeconds(106), UDP), TestCase::EXTENSIVE);
  AddTestCase (new LbtTxopTestCase ("txop=10, d1=50, d2=50", true, 10.0, 10.0, 50.0, MicroSeconds(106), UDP), TestCase::EXTENSIVE);
  AddTestCase (new LbtTxopTestCase ("txop=13, d1=50, d2=50", true, 13.0, 10.0, 50.0, MicroSeconds(106), UDP), TestCase::EXTENSIVE);
  AddTestCase (new LbtTxopTestCase ("txop=20, d1=50, d2=50", true, 20.0, 10.0, 50.0, MicroSeconds(106), UDP), TestCase::EXTENSIVE);
  //AddTestCase (new LbtTxopTestCase ("txop=20, d1=50, d2=50", true, 20.0, 10.0, 50.0, MilliSeconds(10), UDP), TestCase::EXTENSIVE);

  // when testing TCP increase m_startTime and m_StopTime
  AddTestCase (new LbtTxopTestCase ("txop=4,  d1=50, d2=50", true, 4.0, 10.0, 50.0, MicroSeconds(106), TCP), TestCase::QUICK);
  AddTestCase (new LbtTxopTestCase ("txop=5,  d1=50, d2=50", true, 5.0, 10.0, 50.0, MicroSeconds(106), TCP), TestCase::EXTENSIVE);
  AddTestCase (new LbtTxopTestCase ("txop=10, d1=50, d2=50", true, 10.0, 10.0, 50.0, MicroSeconds(106), TCP), TestCase::EXTENSIVE);
  AddTestCase (new LbtTxopTestCase ("txop=13, d1=50, d2=50", true, 13.0, 10.0, 50.0, MicroSeconds(106), TCP), TestCase::EXTENSIVE);
  AddTestCase (new LbtTxopTestCase ("txop=20, d1=50, d2=50", true, 20.0, 10.0, 50.0, MicroSeconds(106), TCP), TestCase::EXTENSIVE);

}

static LbtTxopTestSuite lbtTxopTestSuite;


/**
 * TestCase
 */
LbtTxopTestCase::LbtTxopTestCase (std::string name, bool useReservationSignal, double txop, double d1, double d2, Time udpInterval, Transport_e transport)
  :TestCase (name), m_useReservationSignal (useReservationSignal), m_txop (txop), m_d1 (d1), m_d2 (d2), m_udpInterval (udpInterval), m_transport(transport)
{
  m_startTime = Seconds(0.3);
  m_stopTime = Seconds(2);
  m_ttiDuration = MilliSeconds(1);
  m_txopCounter = 0;
  m_bytesTransmitedCounter = 0;
}

LbtTxopTestCase::~LbtTxopTestCase (){}

void
LbtTxopTestCase::DoRun (void)
{
  NS_LOG_INFO (this << GetName ());

  NodeContainer eNbNode, ueNode, monitorNode, clientNode, allNodes;
  eNbNode.Create(1);
  ueNode.Create(1);
  monitorNode.Create(1);
  clientNode.Create (1);
  allNodes = NodeContainer (eNbNode, ueNode, monitorNode);

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));   // eNb
  positionAlloc->Add (Vector (0.0, m_d1, 0.0));  // ue
  positionAlloc->Add (Vector (m_d2, 0.0, 0.0));  // laa monitor

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (allNodes);

  PhyParams phyParams;
  phyParams.m_bsTxGain = 5; // dB antenna gain
  phyParams.m_bsRxGain = 5; // dB antenna gain
  phyParams.m_bsTxPower = 18; // dBm
  phyParams.m_bsNoiseFigure = 5; // dB
  phyParams.m_ueTxGain = 0; // dB antenna gain
  phyParams.m_ueRxGain = 0; // dB antenna gain
  phyParams.m_ueTxPower = 18; // dBm
  phyParams.m_ueNoiseFigure = 9; // dB

  Config::SetDefault ("ns3::LteWifiCoexistenceHelper::ChannelAccessManagerType", StringValue ("ns3::LbtAccessManager"));
  Config::SetDefault ("ns3::LbtAccessManager::UseReservationSignal", BooleanValue(m_useReservationSignal));
  Config::SetDefault ("ns3::LbtAccessManager::Txop", TimeValue (MilliSeconds(m_txop)));
  // we want deterministic behavior in this simple scenario, so we disable shadowing
  Config::SetDefault ("ns3::Ieee80211axIndoorPropagationLossModel::Sigma", DoubleValue (0));
  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (phyParams.m_bsTxPower));
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (phyParams.m_ueTxPower));
  Config::SetDefault ("ns3::LteEnbPhy::ImplWith2msDelay", BooleanValue(true));

  if (m_transport == UDP)
    {
      Config::SetDefault ("ns3::LteEnbRrc::EpsBearerToRlcMapping", EnumValue (LteEnbRrc::RLC_UM_ALWAYS));
    }
  else
    {
      Config::SetDefault ("ns3::LteEnbRrc::EpsBearerToRlcMapping", EnumValue (LteEnbRrc::RLC_AM_ALWAYS));
    }

  //Disable Uplink Power Control
  Config::SetDefault ("ns3::LteUePhy::EnableUplinkPowerControl", BooleanValue (false));
  NetDeviceContainer eNbDevice;
  NetDeviceContainer ueDevice;
  Ptr<NetDevice> laaMonitorDevice;

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisSpectrumPropagationLossModel"));
  // since LAA is using CA, RRC messages will be exchanged in the
  // licensed bands, hence we model it using the ideal RRC
  lteHelper->SetAttribute ("UseIdealRrc", BooleanValue (true));
  lteHelper->SetAttribute ("UsePdschForCqiGeneration", BooleanValue (true));

  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->Initialize ();

  // LTE requires some Internet stack configuration prior to device installation
  // Wifi configures it after device installation
  InternetStackHelper internetStackHelper;
  // Install an internet stack to all the nodes with IP
  internetStackHelper.Install (ueNode);
  internetStackHelper.Install (clientNode);

  lteHelper->SetEnbDeviceAttribute ("CsgIndication", BooleanValue (true));
  lteHelper->SetEnbDeviceAttribute ("CsgId", UintegerValue (1));
  lteHelper->SetUeDeviceAttribute ("CsgId", UintegerValue (1));
  Ipv4AddressHelper internetIpv4Helper;
  internetIpv4Helper.SetBase ("1.0.0.0", "255.0.0.0");

  // configure client node
  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  PointToPointHelper p2pHelper;
  p2pHelper.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2pHelper.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2pHelper.SetChannelAttribute ("Delay", TimeValue (Seconds (0.0)));
  NetDeviceContainer clientDevice = p2pHelper.Install (pgw, clientNode.Get(0));
  Ipv4InterfaceContainer internetIpIfaces = internetIpv4Helper.Assign (clientDevice);
  // interface 0 is localhost, 1 is the p2p device
  //Ipv4Address clientNodeAddr = internetIpIfaces.GetAddress (1);
  // make LTE and network reachable from the client node
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> clientNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting (clientNode.Get(0)->GetObject<Ipv4> ());
  clientNodeStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  // LTE configuration parametes
  lteHelper->SetSchedulerType ("ns3::PfFfMacScheduler");
  lteHelper->SetSchedulerAttribute ("UlCqiFilter", EnumValue (FfMacScheduler::PUSCH_UL_CQI));
  // LTE-U DL transmission @5180 MHz
  lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (255444));
  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (100));
  // needed for initial cell search
  lteHelper->SetUeDeviceAttribute ("DlEarfcn", UintegerValue (255444));
  // LTE calibration
  lteHelper->SetEnbAntennaModelType ("ns3::IsotropicAntennaModel");
  lteHelper->SetEnbAntennaModelAttribute ("Gain",   DoubleValue (phyParams.m_bsTxGain));

  // Create Devices and install them in the Nodes (eNBs and UEs)
  eNbDevice = lteHelper->InstallEnbDevice (eNbNode);

  m_ltePhy = eNbDevice.Get(0)->GetObject<LteEnbNetDevice> ()->GetPhy ();

  ueDevice = lteHelper->InstallUeDevice (ueNode);
  Ptr<LteWifiCoexistenceHelper> lteWifiCoexistenceHelper = CreateObject<LteWifiCoexistenceHelper> ();
  // configure LAA node
  //lteWifiCoexistenceHelper->ConfigureEnbDevicesForLbt(eNbDevice, phyParams);
  Simulator::Schedule (MilliSeconds(300), &LteWifiCoexistenceHelper::ConfigureEnbDevicesForLbt, lteWifiCoexistenceHelper, eNbDevice, phyParams);

  // configure monitor node
  // get LTE channel
  Ptr<LteEnbNetDevice> lteEnbNetDevice = eNbDevice.Get(0)->GetObject<LteEnbNetDevice> ();
  Ptr<SpectrumChannel> downlinkSpectrumChannel = lteEnbNetDevice->GetPhy ()->GetDownlinkSpectrumPhy ()->GetChannel ();
  SpectrumWifiPhyHelper spectrumPhy = SpectrumWifiPhyHelper::Default ();
  spectrumPhy.SetChannel (downlinkSpectrumChannel);
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  WifiMacHelper mac;
  spectrumPhy.Set ("ChannelWidth", UintegerValue (20));
  spectrumPhy.Set ("TxGain", DoubleValue (phyParams.m_ueTxGain));
  spectrumPhy.Set ("RxGain", DoubleValue (phyParams.m_ueRxGain));
  spectrumPhy.Set ("TxPowerStart", DoubleValue (phyParams.m_ueTxPower));
  spectrumPhy.Set ("TxPowerEnd", DoubleValue (phyParams.m_ueTxPower));
  spectrumPhy.Set ("RxNoiseFigure", DoubleValue (phyParams.m_ueNoiseFigure));
  spectrumPhy.Set ("Antennas", UintegerValue (2));
  uint32_t channelNumber = 36;
  spectrumPhy.Set ("ChannelNumber", UintegerValue (channelNumber));
  wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
  //which implements a Wi-Fi MAC that does not perform any kind of beacon generation, probing, or association
  mac.SetType ("ns3::AdhocWifiMac");
  // wifi device that is doing monitoring
  Ptr<NetDevice> monitorDevice = (wifi.Install (spectrumPhy, mac, monitorNode.Get(0))).Get (0);

  // configure UE device
  epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevice));
  // set the default gateway for the UE
  Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode.Get(0)->GetObject<Ipv4> ());
  ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
  lteHelper->Attach (ueDevice);

  // configure application at UE
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("17.0.0.0", "255.255.0.0");
  ipv4.Assign (eNbDevice);
  Ipv4InterfaceContainer ipUe = ipv4.Assign (ueDevice);
  ApplicationContainer serverApps;

  if (m_transport==UDP)
    {
      ApplicationContainer serverApps, clientApps;
      serverApps.Add (ConfigureUdpServers (ueNode, m_startTime, m_stopTime));
      clientApps.Add (ConfigureUdpClients (clientNode, ipUe, m_startTime, m_stopTime, m_udpInterval));
    }
  else
    {
      ApplicationContainer tcpServerApps, tcpClientApps;
      ApplicationContainer voiceServerApps, voiceClientApps;
      tcpServerApps.Add (ConfigureTcpServers (ueNode, m_startTime));
      tcpClientApps.Add (ConfigureTcpClients (clientNode, ipUe, m_startTime));

      // Start file transfer arrival process
      uint32_t nextClient = 0;
      Ptr<ExponentialRandomVariable> ftpArrivals = CreateObject<ExponentialRandomVariable> ();
      double firstArrival = ftpArrivals->GetValue ();
      Simulator::Schedule (m_startTime + Seconds (firstArrival), &StartFileTransfer, ftpArrivals, tcpClientApps, nextClient, m_stopTime);
    }

  FlowMonitorHelper flowmonHelper;
  NodeContainer endPointNodes;
  endPointNodes.Add (clientNode);
  endPointNodes.Add (ueNode);
  Ptr<FlowMonitor> monitor = flowmonHelper.Install (endPointNodes);
  monitor->SetAttribute ("DelayBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("JitterBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("PacketSizeBinWidth", DoubleValue (20));

  Config::Connect ("/NodeList/0/DeviceList/0/LteEnbPhy/ReservationSignal",
                   MakeBoundCallback (&LteTestReservationSignalCallback, this));
  Config::Connect ("/NodeList/0/DeviceList/0/LteEnbPhy/DataSent",
                     MakeBoundCallback (&LteDataTracedCallback, this));
  Config::Connect ("/NodeList/0/DeviceList/0/LteEnbPhy/Txop",
                     MakeBoundCallback (&LteTestTxOpCallback, this));

  Simulator::Stop (m_stopTime);
  Simulator::Run ();
  Simulator::Destroy ();
}


void
LbtTxopTestCase::CheckState (LbtAccessManager::LbtState state, bool duringReservationSignal)
{
  Ptr<LbtAccessManager> lbtManager = DynamicCast<LbtAccessManager>(m_ltePhy->GetChannelAccessManager());
  LbtAccessManager::LbtState detectedState = lbtManager->GetLbtState();
  NS_TEST_ASSERT_MSG_EQ (detectedState, state, "Failed at time " << Simulator::Now().GetSeconds ());
}


void
LbtTxopTestCase::DataTracedCallback (uint32_t bytes)
{
  NS_LOG_INFO(this<<"DATA at:"<<Simulator::Now().GetMicroSeconds()<<", size:"<<bytes<<" bytes.");
  m_bytesTransmitedCounter+=bytes;
  Simulator::Schedule (NanoSeconds(1), &LbtTxopTestCase::CheckState, this, LbtAccessManager::BUSY, true);
}

void
LbtTxopTestCase::ReservationSignalCheck (Time startTime, Time duration)
{
  NS_LOG_INFO(this<<"RS at:"<<startTime.GetMicroSeconds()<<", duration:"<<duration.GetMicroSeconds());
  Simulator::Schedule (duration/2, &LbtTxopTestCase::CheckState, this, LbtAccessManager::BUSY, true);
}

void
LbtTxopTestCase::TxopCheck (Time startTime, Time duration, Time firstSubframeStarts)
{
  NS_LOG_INFO(this<<"Txop "<<m_txopCounter<<" at:"<<Simulator::Now().GetMicroSeconds()<<" with duration of "<<duration.GetMilliSeconds()<<". Next subrame starts at:"<<firstSubframeStarts.GetMicroSeconds());
  m_txopCounter++;
  // check if reservation signal started immediately after txop
  if (m_ltePhy->GetGrantTimeout() > Simulator::Now() + NanoSeconds(1))
    {
      Simulator::Schedule ((firstSubframeStarts-startTime)/2, &LbtTxopTestCase::CheckState, this, LbtAccessManager::BUSY, false);
    }
}

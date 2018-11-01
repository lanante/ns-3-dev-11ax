/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2015 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 *
 * Authors: Biljana Bojovic <bbojovic@cttc.es>
 * Created on: Aug 18, 2015
 *
 * based upon lte-test-interference by
 *  Manuel Requena <manuel.requena@cttc.es>
 *  Nicola Baldo <nbaldo@cttc.es>
 */

#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/double.h"
#include <ns3/enum.h>
#include "ns3/boolean.h"
#include "ns3/mobility-helper.h"
#include "ns3/lte-helper.h"
#include "ns3/ff-mac-scheduler.h"
#include <ns3/lte-enb-rrc.h>
#include "ns3/lte-enb-phy.h"
#include "ns3/lte-enb-net-device.h"
#include "ns3/lte-ue-phy.h"
#include "ns3/lte-ue-net-device.h"
#include <ns3/lte-chunk-processor.h>
#include <ns3/duty-cycle-access-manager.h>
#include "test-lte-duty-cycle.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LteDutyCycleTest");


void
LteTestDlSchedulingCallback1 (LteDutyCycleTestCase *testcase, std::string path,
                              uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                              uint8_t mcsTb1, uint16_t sizeTb1, uint8_t mcsTb2, uint16_t sizeTb2)
{
  testcase->DlScheduling1 (frameNo, subframeNo, rnti, mcsTb1, sizeTb1, mcsTb2, sizeTb2);
}


void
LteTestDlSchedulingCallback2 (LteDutyCycleTestCase *testcase, std::string path,
                              uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                              uint8_t mcsTb1, uint16_t sizeTb1, uint8_t mcsTb2, uint16_t sizeTb2)
{
  testcase->DlScheduling2 (frameNo, subframeNo, rnti, mcsTb1, sizeTb1, mcsTb2, sizeTb2);
}

void
LteTestUlSchedulingCallback1 (LteDutyCycleTestCase *testcase, std::string path,
                              uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                              uint8_t mcs, uint16_t sizeTb)
{
  testcase->UlScheduling1 (frameNo, subframeNo, rnti, mcs, sizeTb);
}

void
LteTestUlSchedulingCallback2 (LteDutyCycleTestCase *testcase, std::string path,
                              uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                              uint8_t mcs, uint16_t sizeTb)
{
  testcase->UlScheduling2 (frameNo, subframeNo, rnti, mcs, sizeTb);
}


/**
 * TestSuite
 */

LteDutyCycleTestSuite::LteDutyCycleTestSuite ()
  : TestSuite ("lte-duty-cycle", SYSTEM)
{

  // duty cycle parameters are configured in such way that cell1 and cell2
  // do not transmit at the same time, thus even when they are close to each other
  // the MCS in downlink is very high = 28. Also for the uplink, MCS=28
  // since UEs are very near to the eNBs
  AddTestCase (new LteDutyCycleTestCase ("d1=50, d2=50",  50.000000, 50.000000, 80, 40, 0, 40, 40, 55.5288, 40.3453, 28, 28), TestCase::QUICK);
  AddTestCase (new LteDutyCycleTestCase ("d1=50, d2=50000",  50.000000, 500000.000000, 80, 40, 0, 40, 40, 55.5288, 40.3453, 28, 28), TestCase::QUICK);

  // duty cycle parameters are set in such a way that cell1 and cell 2 transmit at the same time
  // because of this when cells are near, d2=50, there is a lot of interference, so MCS in both
  // uplink and downlink drops to 2. This is not the case when cells are far away d2=50000,
  // when MCS is 28 for uplink and downlink.
  AddTestCase (new LteDutyCycleTestCase ("d1=50, d2=50",  50.000000, 50.000000, 80, 40, 0, 40, 0, -1.21591e-05, -0.000401085, 2, 2), TestCase::QUICK);
  AddTestCase (new LteDutyCycleTestCase ("d1=50, d2=50000",  50.000000, 500000.000000, 80, 80, 0, 80, 0, 55.5134, 40.3448, 28, 28), TestCase::QUICK);

  // duty cycle parameters are set in such a way that cell1 and cell2 do not transmit at the same time,
  // but UEs are placed more far away from its cells, and as expected MCS for uplink and downlink drop with distance d1.
  AddTestCase (new LteDutyCycleTestCase ("d1=1000, d2=50",  1000.000000, 50.000000, 80, 40, 0, 40, 40, 29.5082, 14.3247, 28, 16), TestCase::QUICK);
  AddTestCase (new LteDutyCycleTestCase ("d1=5000, d2=50",  5000.000000, 50.000000, 80, 40, 0, 40, 40, 15.5288, 0.345283, 18, 2), TestCase::QUICK);
}

static LteDutyCycleTestSuite lteDutyCycleTestSuite;


/**
 * TestCase
 */
LteDutyCycleTestCase::LteDutyCycleTestCase (std::string name, double d1, double d2, int64_t dutyCycleTime, int64_t cell1OnTime, uint64_t cell1OnStartTime, int64_t cell2OnTime, uint64_t cell2OnStartTime,
                                            double dlSinr, double ulSinr, uint16_t dlMcs, uint16_t ulMcs)
  : TestCase (BuildNameString (name, dutyCycleTime, cell1OnStartTime, cell1OnTime, cell2OnStartTime, cell2OnTime)),
    m_d1 (d1),
    m_d2 (d2),
    m_dutyCycleTime (dutyCycleTime),
    m_cell1onTime (cell1OnTime),
    m_cell1onStartTime (cell1OnStartTime),
    m_cell2onTime (cell2OnTime),
    m_cell2onStartTime (cell2OnStartTime),
    m_expectedDlSinrDb (dlSinr),
    m_expectedUlSinrDb (ulSinr),
    m_dlMcs (dlMcs),
    m_ulMcs (ulMcs)
{

  // need to allow for RRC connection establishment + SRS
  m_testCheckStartTime = MilliSeconds (330);
}


std::string
LteDutyCycleTestCase::BuildNameString (std::string name, int64_t dutyCycleTime, int64_t cell1OnStartTime, int64_t cell1OnTime, int64_t cell2OnStartTime, int64_t cell2OnTime)
{
  std::ostringstream oss;
  oss << name << ", dutyCycleTime=" << dutyCycleTime << ", cell1OnStartTime=" << cell1OnStartTime << ", cell1OnTime=" << cell1OnTime << ", cell2OnStartTime=" << cell2OnStartTime << ", cell2OnTime=" << cell2OnTime;
  return oss.str ();
}

LteDutyCycleTestCase::~LteDutyCycleTestCase ()
{
}

void LteDutyCycleTestCase::DelayedDutyCycleStart (NetDeviceContainer enbDevs)
{
  Ptr<DutyCycleAccessManager> dutyCycleAccessManager1 = CreateObject<DutyCycleAccessManager> ();
  Ptr<DutyCycleAccessManager> dutyCycleAccessManager2 = CreateObject<DutyCycleAccessManager> ();

  dutyCycleAccessManager1->SetOnDuration (MilliSeconds (m_cell1onTime));
  dutyCycleAccessManager1->SetOnStartTime (MilliSeconds (m_cell1onStartTime));
  dutyCycleAccessManager1->SetDutyCyclePeriod (MilliSeconds (m_dutyCycleTime));

  dutyCycleAccessManager2->SetOnDuration (MilliSeconds (m_cell2onTime));
  dutyCycleAccessManager2->SetOnStartTime (MilliSeconds (m_cell2onStartTime));
  dutyCycleAccessManager2->SetDutyCyclePeriod (MilliSeconds (m_dutyCycleTime));

  enbDevs.Get (0)->GetObject<LteEnbNetDevice> ()->GetPhy ()->SetChannelAccessManager (dutyCycleAccessManager1);
  enbDevs.Get (1)->GetObject<LteEnbNetDevice> ()->GetPhy ()->SetChannelAccessManager (dutyCycleAccessManager2);

}


void
LteDutyCycleTestCase::DoRun (void)
{
  NS_LOG_INFO (this << GetName ());

  Config::SetDefault ("ns3::LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::PiroEW2010));
  Config::SetDefault ("ns3::LteAmc::Ber", DoubleValue (0.00005));
  //LogComponentEnable("DutyCycleAccessManager", LOG_LEVEL_ALL);
  //LogComponentEnable("ChannelAccessManager", LOG_LEVEL_ALL);

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisSpectrumPropagationLossModel"));
  lteHelper->SetAttribute ("UseIdealRrc", BooleanValue (false));
  lteHelper->SetAttribute ("UsePdschForCqiGeneration", BooleanValue (true));

  //Disable Uplink Power Control
  Config::SetDefault ("ns3::LteUePhy::EnableUplinkPowerControl", BooleanValue (false));
  //set laa channel acccess implementation
  Config::SetDefault ("ns3::LteEnbPhy::ImplWith2msDelay", BooleanValue(true));
  Config::SetDefault ("ns3::LteEnbPhy::ChannelAccessManagerStartTime", TimeValue (MilliSeconds(300)));
  // Create Nodes: eNodeB and UE
  NodeContainer enbNodes;
  NodeContainer ueNodes1;
  NodeContainer ueNodes2;
  enbNodes.Create (2);
  ueNodes1.Create (1);
  ueNodes2.Create (1);
  NodeContainer allNodes = NodeContainer ( enbNodes, ueNodes1, ueNodes2);

  // the topology is the following:
  //         d2
  //  UE1-----------eNB2
  //   |             |
  // d1|             |d1
  //   |     d2      |
  //  eNB1----------UE2
  //
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));   // eNB1
  positionAlloc->Add (Vector (m_d2, m_d1, 0.0)); // eNB2
  positionAlloc->Add (Vector (0.0, m_d1, 0.0));  // UE1
  positionAlloc->Add (Vector (m_d2, 0.0, 0.0));  // UE2
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (allNodes);

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs1;
  NetDeviceContainer ueDevs2;
  lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");
  lteHelper->SetSchedulerAttribute ("UlCqiFilter", EnumValue (FfMacScheduler::PUSCH_UL_CQI));
  enbDevs = lteHelper->InstallEnbDevice (enbNodes);
  ueDevs1 = lteHelper->InstallUeDevice (ueNodes1);
  ueDevs2 = lteHelper->InstallUeDevice (ueNodes2);

  lteHelper->Attach (ueDevs1, enbDevs.Get (0));
  lteHelper->Attach (ueDevs2, enbDevs.Get (1));

  // Activate an EPS bearer
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueDevs1, bearer);
  lteHelper->ActivateDataRadioBearer (ueDevs2, bearer);

  // Use testing chunk processor in the PHY layer
  // It will be used to test that the SNR is as intended
  // we plug in two instances, one for DL and one for UL

  Ptr<LtePhy> ue1Phy = ueDevs1.Get (0)->GetObject<LteUeNetDevice> ()->GetPhy ()->GetObject<LtePhy> ();
  Ptr<LteAverageChunkProcessor> testDlSinr1 = Create<LteAverageChunkProcessor> ();
  LteSpectrumValueCatcher dlSinr1Catcher;
  testDlSinr1->AddCallback (MakeCallback (&LteSpectrumValueCatcher::ReportValue, &dlSinr1Catcher));
  ue1Phy->GetDownlinkSpectrumPhy ()->AddDataSinrChunkProcessor (testDlSinr1);

  Ptr<LtePhy> enb1phy = enbDevs.Get (0)->GetObject<LteEnbNetDevice> ()->GetPhy ()->GetObject<LtePhy> ();
  Ptr<LteAverageChunkProcessor> testUlSinr1 = Create<LteAverageChunkProcessor> ();
  LteSpectrumValueCatcher ulSinr1Catcher;
  testUlSinr1->AddCallback (MakeCallback (&LteSpectrumValueCatcher::ReportValue, &ulSinr1Catcher));
  enb1phy->GetUplinkSpectrumPhy ()->AddDataSinrChunkProcessor (testUlSinr1);

  Config::Connect ("/NodeList/0/DeviceList/0/LteEnbMac/DlScheduling",
                   MakeBoundCallback (&LteTestDlSchedulingCallback1, this));

  Config::Connect ("/NodeList/0/DeviceList/0/LteEnbMac/UlScheduling",
                   MakeBoundCallback (&LteTestUlSchedulingCallback1, this));


  // same as above for eNB2 and UE2
  Ptr<LtePhy> ue2Phy = ueDevs2.Get (0)->GetObject<LteUeNetDevice> ()->GetPhy ()->GetObject<LtePhy> ();
  Ptr<LteAverageChunkProcessor> testDlSinr2 = Create<LteAverageChunkProcessor> ();
  LteSpectrumValueCatcher dlSinr2Catcher;
  testDlSinr2->AddCallback (MakeCallback (&LteSpectrumValueCatcher::ReportValue, &dlSinr2Catcher));
  ue2Phy->GetDownlinkSpectrumPhy ()->AddDataSinrChunkProcessor (testDlSinr2);

  Ptr<LtePhy> enb2phy = enbDevs.Get (1)->GetObject<LteEnbNetDevice> ()->GetPhy ()->GetObject<LtePhy> ();
  Ptr<LteAverageChunkProcessor> testUlSinr2 = Create<LteAverageChunkProcessor> ();
  LteSpectrumValueCatcher ulSinr2Catcher;
  testUlSinr2->AddCallback (MakeCallback (&LteSpectrumValueCatcher::ReportValue, &ulSinr2Catcher));
  enb1phy->GetUplinkSpectrumPhy ()->AddDataSinrChunkProcessor (testUlSinr2);

  Config::Connect ("/NodeList/1/DeviceList/0/LteEnbMac/DlScheduling",
                   MakeBoundCallback (&LteTestDlSchedulingCallback2, this));

  Config::Connect ("/NodeList/1/DeviceList/0/LteEnbMac/UlScheduling",
                   MakeBoundCallback (&LteTestUlSchedulingCallback2, this));



  Simulator::Schedule (MilliSeconds (300), &LteDutyCycleTestCase::DelayedDutyCycleStart, this, enbDevs);

  //lteHelper->EnableMacTraces ();
  //lteHelper->EnablePhyTraces ();


  // consider that the ABS pattern might make attachment & connection establishment longer
  Simulator::Stop (Seconds (1));
  Simulator::Run ();

  if (m_dlMcs > m_testCheckStartTime)
    {
      double dlSinr1Db = 10.0 * std::log10 (dlSinr1Catcher.GetValue ()->operator[] (0));
      NS_TEST_ASSERT_MSG_EQ_TOL (dlSinr1Db, m_expectedDlSinrDb, 0.01, "Wrong SINR in DL! (eNB1 --> UE1)");

      double dlSinr2Db = 10.0 * std::log10 (dlSinr2Catcher.GetValue ()->operator[] (0));
      NS_TEST_ASSERT_MSG_EQ_TOL (dlSinr2Db, m_expectedDlSinrDb, 0.01, "Wrong SINR in DL! (eNB2 --> UE2)");
    }

  if (m_ulMcs > m_testCheckStartTime)
    {
      double ulSinr1Db = 10.0 * std::log10 (ulSinr1Catcher.GetValue ()->operator[] (0));
      NS_TEST_ASSERT_MSG_EQ_TOL (ulSinr1Db, m_expectedUlSinrDb, 0.01, "Wrong SINR in UL!  (UE1 --> eNB1)");

      double ulSinr2Db = 10.0 * std::log10 (ulSinr2Catcher.GetValue ()->operator[] (0));
      NS_TEST_ASSERT_MSG_EQ_TOL (ulSinr2Db, m_expectedUlSinrDb, 0.01, "Wrong SINR in UL!  (UE2 --> eNB2)");
    }

  Simulator::Destroy ();

}


void
LteDutyCycleTestCase::DlScheduling1 (uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                                     uint8_t mcsTb1, uint16_t sizeTb1, uint8_t mcsTb2, uint16_t sizeTb2)
{
  NS_LOG_FUNCTION (frameNo << subframeNo << rnti << (uint32_t) mcsTb1 << sizeTb1 << (uint32_t) mcsTb2 << sizeTb2);
  uint64_t tInt = Simulator::Now ().GetMilliSeconds () % (m_dutyCycleTime);
  // need to allow for RRC connection establishment + SRS
  if (Simulator::Now () > m_testCheckStartTime)
  {
     NS_TEST_ASSERT_MSG_EQ (((tInt > m_cell1onTime + m_cell1onStartTime)or (tInt < m_cell1onStartTime)), false, "cell 1: DL TX scheduled in offTime");
  }
  // do not consider first 8ms of each grant period, until MCS gets its real value. This is because there might not
  // be transmission because of delay between MAC and PHY for first 2 ms of each grant period, so interference will
  // not be present. While MCS is not updated there is delay of 4ms, and 2 ms while this MCS is actual
  if (Simulator::Now () > m_testCheckStartTime && tInt>8)
    {
      NS_TEST_ASSERT_MSG_EQ ((uint32_t)mcsTb1, (uint32_t)m_dlMcs, "Wrong DL MCS ");
    }
}

void
LteDutyCycleTestCase::DlScheduling2 (uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                                     uint8_t mcsTb1, uint16_t sizeTb1, uint8_t mcsTb2, uint16_t sizeTb2)
{
  NS_LOG_FUNCTION (frameNo << subframeNo << rnti << (uint32_t) mcsTb1 << sizeTb1 << (uint32_t) mcsTb2 << sizeTb2);
  uint64_t tInt = Simulator::Now ().GetMilliSeconds () % (m_dutyCycleTime);
  // wait for some time until channel access manager is set
  if (Simulator::Now () > m_testCheckStartTime)
  {
     NS_TEST_ASSERT_MSG_EQ (((tInt > m_cell2onTime + m_cell2onStartTime)or (tInt < m_cell2onStartTime)), false, "cell 2: DL TX scheduled in offTime");
  }

  // do not consider first 8ms of each grant period, until MCS gets its real value. This is because there might not
  // be transmission because of delay between MAC and PHY for first 2 ms of each grant period, so interference will
  // not be present. While MCS is not updated there is delay of 4ms, and 2 ms while this MCS is actual
  if (Simulator::Now () > m_testCheckStartTime && tInt>8)
    {
      NS_TEST_ASSERT_MSG_EQ ((uint32_t)mcsTb1, (uint32_t)m_dlMcs, "Wrong DL MCS ");
    }
}

void
LteDutyCycleTestCase::UlScheduling1 (uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                                     uint8_t mcs, uint16_t sizeTb)
{
  NS_LOG_FUNCTION (frameNo << subframeNo << rnti << (uint32_t) mcs << sizeTb);

  if (Simulator::Now () > m_testCheckStartTime)
    {
      NS_TEST_ASSERT_MSG_EQ ((uint32_t)mcs, (uint32_t)m_ulMcs, "Wrong UL MCS");
    }
}

void
LteDutyCycleTestCase::UlScheduling2 (uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                                     uint8_t mcs, uint16_t sizeTb)
{
  NS_LOG_FUNCTION (frameNo << subframeNo << rnti << (uint32_t) mcs << sizeTb);

  if (Simulator::Now () > m_testCheckStartTime)
    {
      NS_TEST_ASSERT_MSG_EQ ((uint32_t)mcs, (uint32_t)m_ulMcs, "Wrong UL MCS");
    }

}



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
 * Authors: Nicola Baldo
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

#include "test-lte-interference-abs.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LteInterferenceAbsTest");

void
LteTestDlSchedulingCallback1 (LteInterferenceAbsTestCase *testcase, std::string path,
                             uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                             uint8_t mcsTb1, uint16_t sizeTb1, uint8_t mcsTb2, uint16_t sizeTb2)
{
  testcase->DlScheduling1 (frameNo, subframeNo, rnti, mcsTb1, sizeTb1, mcsTb2, sizeTb2);
}


void
LteTestDlSchedulingCallback2 (LteInterferenceAbsTestCase *testcase, std::string path,
                             uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                             uint8_t mcsTb1, uint16_t sizeTb1, uint8_t mcsTb2, uint16_t sizeTb2)
{
  testcase->DlScheduling2 (frameNo, subframeNo, rnti, mcsTb1, sizeTb1, mcsTb2, sizeTb2);
}

void
LteTestUlSchedulingCallback1 (LteInterferenceAbsTestCase *testcase, std::string path,
                             uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                             uint8_t mcs, uint16_t sizeTb)
{
  testcase->UlScheduling1 (frameNo, subframeNo, rnti, mcs, sizeTb);
}

void
LteTestUlSchedulingCallback2 (LteInterferenceAbsTestCase *testcase, std::string path,
                             uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                             uint8_t mcs, uint16_t sizeTb)
{
  testcase->UlScheduling2 (frameNo, subframeNo, rnti, mcs, sizeTb);
}


/**
 * TestSuite
 */

LteInterferenceAbsTestSuite::LteInterferenceAbsTestSuite ()
  : TestSuite ("lte-interference-abs", SYSTEM)
{
  AddTestCase (new LteInterferenceAbsTestCase ("d1=50, d2=50",  50.000000, 50.000000, "1101011000101000100100010110000010100101", "1101011000101000100100010110000010100101", 0.999997, 0.999907,  0.239828, 0.239808, 2, 2), TestCase::QUICK);
  AddTestCase (new LteInterferenceAbsTestCase ("d1=50, d2=50",  50.000000, 50.000000, "0011101001000110010101000101000000010100", "0011101001000110010101000101000000010100", 0.999997, 0.999907,  0.239828, 0.239808, 2, 2), TestCase::EXTENSIVE);
  AddTestCase (new LteInterferenceAbsTestCase ("d1=50, d2=50",  50.000000, 50.000000, "0110110100100101011111010111010011001100", "0110110100100101011111010111010011001100", 0.999997, 0.999907,  0.239828, 0.239808, 2, 2), TestCase::EXTENSIVE);
  AddTestCase (new LteInterferenceAbsTestCase ("d1=50, d2=50",  50.000000, 50.000000, "0010010111001110110110001011110000000110", "1101101000110001001001110100001111111001", 356449.932732, 10803.280215,  15.976248, 10.932806, 28, 28), TestCase::QUICK);
  AddTestCase (new LteInterferenceAbsTestCase ("d1=50, d2=50",  50.000000, 50.000000, "1001110011110000000011100101100110010011", "0110001100001111111100011010011001101100", 356449.932732, 10803.280215,  15.976248, 10.932806, 28, 28), TestCase::EXTENSIVE);
  AddTestCase (new LteInterferenceAbsTestCase ("d1=50, d2=50",  50.000000, 50.000000, "0010111011011100110001100011111111111111", "1101000100100011001110011100000000000000", 356449.932732, 10803.280215,  15.976248, 10.932806, 28, 28), TestCase::EXTENSIVE);
  AddTestCase (new LteInterferenceAbsTestCase ("d1=50, d2=200",  50.000000, 200.000000, "0001011110011111011011001010001010010100", "0001011110011111011011001010001010010100", 15.999282, 15.976339,  1.961072, 1.959533, 14, 14), TestCase::EXTENSIVE);
  AddTestCase (new LteInterferenceAbsTestCase ("d1=50, d2=200",  50.000000, 200.000000, "0101001100001110101001101001001111010111", "0101001100001110101001101001001111010111", 15.999282, 15.976339,  1.961072, 1.959533, 14, 14), TestCase::EXTENSIVE);
  AddTestCase (new LteInterferenceAbsTestCase ("d1=50, d2=200",  50.000000, 200.000000, "1001110001011111010100010101101011010101", "1001110001011111010100010101101011010101", 15.999282, 15.976339,  1.961072, 1.959533, 14, 14), TestCase::EXTENSIVE);
  AddTestCase (new LteInterferenceAbsTestCase ("d1=50, d2=200",  50.000000, 200.000000, "1001111010100110110110110011111000100000", "0110000101011001001001001100000111011111", 356449.932732, 10803.280215,  15.976248, 10.932806, 28, 28), TestCase::EXTENSIVE);
  AddTestCase (new LteInterferenceAbsTestCase ("d1=50, d2=200",  50.000000, 200.000000, "0010111010111011011010000000010100101000", "1101000101000100100101111111101011010111", 356449.932732, 10803.280215,  15.976248, 10.932806, 28, 28), TestCase::EXTENSIVE);
  AddTestCase (new LteInterferenceAbsTestCase ("d1=50, d2=200",  50.000000, 200.000000, "1110101110101110110101110101110100110111", "0001010001010001001010001010001011001000", 356449.932732, 10803.280215,  15.976248, 10.932806, 28, 28), TestCase::EXTENSIVE);
  AddTestCase (new LteInterferenceAbsTestCase ("d1=50, d2=500",  50.000000, 500.000000, "0011010011000011100010001101101001110010", "0011010011000011100010001101101001110010", 99.971953, 99.082845,  4.254003, 4.241793, 22, 22), TestCase::EXTENSIVE);
  AddTestCase (new LteInterferenceAbsTestCase ("d1=50, d2=500",  50.000000, 500.000000, "0110100010011101111010000000000011000111", "0110100010011101111010000000000011000111", 99.971953, 99.082845,  4.254003, 4.241793, 22, 22), TestCase::EXTENSIVE);
  AddTestCase (new LteInterferenceAbsTestCase ("d1=50, d2=500",  50.000000, 500.000000, "1001000110001110010011010000010101100110", "1001000110001110010011010000010101100110", 99.971953, 99.082845,  4.254003, 4.241793, 22, 22), TestCase::EXTENSIVE);
  AddTestCase (new LteInterferenceAbsTestCase ("d1=50, d2=500",  50.000000, 500.000000, "1001101001110011000000110000011111110001", "0110010110001100111111001111100000001110", 356449.932732, 10803.280215,  15.976248, 10.932806, 28, 28), TestCase::EXTENSIVE);
  AddTestCase (new LteInterferenceAbsTestCase ("d1=50, d2=500",  50.000000, 500.000000, "1111111100000001010011000000100001100111", "0000000011111110101100111111011110011000", 356449.932732, 10803.280215,  15.976248, 10.932806, 28, 28), TestCase::EXTENSIVE);
  AddTestCase (new LteInterferenceAbsTestCase ("d1=50, d2=500",  50.000000, 500.000000, "1000111101111011111101110011101011100001", "0111000010000100000010001100010100011110", 356449.932732, 10803.280215,  15.976248, 10.932806, 28, 28), TestCase::EXTENSIVE);
}

static LteInterferenceAbsTestSuite lteLinkAdaptationWithInterferenceTestSuite;


/**
 * TestCase
 */

LteInterferenceAbsTestCase::LteInterferenceAbsTestCase (std::string name, double d1, double d2, std::string absPattern1, std::string absPattern2, double dlSinr, double ulSinr, double dlSe, double ulSe, uint16_t dlMcs, uint16_t ulMcs)
  : TestCase (name + ", absPattern1=" + absPattern1 + ", absPattern2=" + absPattern2),
    m_d1 (d1),
    m_d2 (d2),
    m_absPattern1 (absPattern1),
    m_absPattern2 (absPattern2),
    m_expectedDlSinrDb (10 * std::log10 (dlSinr)),
    m_expectedUlSinrDb (10 * std::log10 (ulSinr)),
    m_dlMcs (dlMcs),
    m_ulMcs (ulMcs)
{
}

LteInterferenceAbsTestCase::~LteInterferenceAbsTestCase ()
{
}

void
LteInterferenceAbsTestCase::DoRun (void)
{
  NS_LOG_INFO (this << GetName ());
  
  Config::SetDefault ("ns3::LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::PiroEW2010));
  Config::SetDefault ("ns3::LteAmc::Ber", DoubleValue (0.00005));
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisSpectrumPropagationLossModel"));
  lteHelper->SetAttribute ("UseIdealRrc", BooleanValue (false));
  lteHelper->SetAttribute ("UsePdschForCqiGeneration", BooleanValue (true));

  //Disable Uplink Power Control
  Config::SetDefault ("ns3::LteUePhy::EnableUplinkPowerControl", BooleanValue (false));

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

  // Configure Almost Blank Subframe (ABS) patterns
  enbDevs.Get (0)->GetObject<LteEnbNetDevice> ()->GetRrc ()->SetAbsPattern (m_absPattern1);
  enbDevs.Get (1)->GetObject<LteEnbNetDevice> ()->GetRrc ()->SetAbsPattern (m_absPattern2);


  lteHelper->EnableMacTraces ();
  lteHelper->EnablePhyTraces ();

  // need to allow for RRC connection establishment + SRS
  // consider that the ABS pattern might make attachment & connection establishment longer
  Simulator::Stop (Seconds (0.4));
  Simulator::Run ();

  if (m_dlMcs > 0)
    {
      double dlSinr1Db = 10.0 * std::log10 (dlSinr1Catcher.GetValue ()->operator[] (0));
      NS_TEST_ASSERT_MSG_EQ_TOL (dlSinr1Db, m_expectedDlSinrDb, 0.01, "Wrong SINR in DL! (eNB1 --> UE1)");

      double dlSinr2Db = 10.0 * std::log10 (dlSinr2Catcher.GetValue ()->operator[] (0));
      NS_TEST_ASSERT_MSG_EQ_TOL (dlSinr2Db, m_expectedDlSinrDb, 0.01, "Wrong SINR in DL! (eNB2 --> UE2)");
    }
  if (m_ulMcs > 0)
    {
      double ulSinr1Db = 10.0 * std::log10 (ulSinr1Catcher.GetValue ()->operator[] (0));
      NS_TEST_ASSERT_MSG_EQ_TOL (ulSinr1Db, m_expectedUlSinrDb, 0.01, "Wrong SINR in UL!  (UE1 --> eNB1)");
      
      double ulSinr2Db = 10.0 * std::log10 (ulSinr2Catcher.GetValue ()->operator[] (0));
      NS_TEST_ASSERT_MSG_EQ_TOL (ulSinr2Db, m_expectedUlSinrDb, 0.01, "Wrong SINR in UL!  (UE2 --> eNB2)");
    }

  Simulator::Destroy ();

}


void
LteInterferenceAbsTestCase::DlScheduling1 (uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                                       uint8_t mcsTb1, uint16_t sizeTb1, uint8_t mcsTb2, uint16_t sizeTb2)
{
  NS_LOG_FUNCTION (frameNo << subframeNo << rnti << (uint32_t) mcsTb1 << sizeTb1 << (uint32_t) mcsTb2 << sizeTb2);
  NS_TEST_ASSERT_MSG_EQ (m_absPattern1[(frameNo % 4) * 10 + ((subframeNo - 1) % 10)], 
                         false, 
                         "cell 1: DL TX scheduled in ABS");
  // need to allow for RRC connection establishment + CQI feedback reception + persistent data transmission
  // consider that the ABS pattern might make attachment & connection establishment longer
  if (Simulator::Now () > MilliSeconds (300))
    {
      NS_TEST_ASSERT_MSG_EQ ((uint32_t)mcsTb1, (uint32_t)m_dlMcs, "Wrong DL MCS ");
    }
}

void
LteInterferenceAbsTestCase::DlScheduling2 (uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                                       uint8_t mcsTb1, uint16_t sizeTb1, uint8_t mcsTb2, uint16_t sizeTb2)
{
  NS_LOG_FUNCTION (frameNo << subframeNo << rnti << (uint32_t) mcsTb1 << sizeTb1 << (uint32_t) mcsTb2 << sizeTb2);
  NS_TEST_ASSERT_MSG_EQ (m_absPattern2[(frameNo % 4) * 10 + ((subframeNo - 1) % 10)], 
                         false, 
                         "cell 2: DL TX scheduled in ABS");
  // need to allow for RRC connection establishment + CQI feedback reception + persistent data transmission
  // consider that the ABS pattern might make attachment & connection establishment longer
  if (Simulator::Now () > MilliSeconds (300))
    {
      NS_TEST_ASSERT_MSG_EQ ((uint32_t)mcsTb1, (uint32_t)m_dlMcs, "Wrong DL MCS ");
    }
}

void
LteInterferenceAbsTestCase::UlScheduling1 (uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                                       uint8_t mcs, uint16_t sizeTb)
{
  NS_LOG_FUNCTION (frameNo << subframeNo << rnti << (uint32_t) mcs << sizeTb);

  // if we have PUSCH in this subframe, it means that exactly
  // UL_PUSCH_TTIS_DELAY TTIs before we could receive the UL DCIs,
  // hence that was not an ABS.
  NS_TEST_ASSERT_MSG_EQ (m_absPattern1[(frameNo * 10 + (subframeNo - 1) - UL_PUSCH_TTIS_DELAY) % 40], 
                         false, 
                         "cell 1: UL TX scheduled but corresponding DCIs fall in an ABS");

  // need to allow for RRC connection establishment + SRS transmission
  // consider that the ABS pattern might make attachment & connection establishment longer

  if (Simulator::Now () > MilliSeconds (300))
    {
      NS_TEST_ASSERT_MSG_EQ ((uint32_t)mcs, (uint32_t)m_ulMcs, "Wrong UL MCS");
    }
}

void
LteInterferenceAbsTestCase::UlScheduling2 (uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                                       uint8_t mcs, uint16_t sizeTb)
{
  NS_LOG_FUNCTION (frameNo << subframeNo << rnti << (uint32_t) mcs << sizeTb);

  // if we have PUSCH in this subframe, it means that exactly
  // UL_PUSCH_TTIS_DELAY TTIs before we could receive the UL DCIs,
  // hence that was not an ABS.
  NS_TEST_ASSERT_MSG_EQ (m_absPattern2[(frameNo * 10 + (subframeNo - 1) - UL_PUSCH_TTIS_DELAY) % 40], 
                         false, 
                         "cell 2: UL TX scheduled but corresponding DCIs fall in an ABS");

  // need to allow for RRC connection establishment + SRS transmission
  // consider that the ABS pattern might make attachment & connection establishment longer

  if (Simulator::Now () > MilliSeconds (300))
    {
      NS_TEST_ASSERT_MSG_EQ ((uint32_t)mcs, (uint32_t)m_ulMcs, "Wrong UL MCS");
    }
}

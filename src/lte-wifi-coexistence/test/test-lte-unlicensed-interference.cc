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
#include <ns3/abort.h>
#include "ns3/string.h"
#include "ns3/double.h"
#include <ns3/enum.h>
#include "ns3/boolean.h"
#include "ns3/mobility-helper.h"
#include "ns3/lte-helper.h"
#include "ns3/ff-mac-scheduler.h"

#include "ns3/lte-enb-phy.h"
#include "ns3/lte-enb-net-device.h"

#include "ns3/lte-ue-phy.h"
#include "ns3/lte-ue-net-device.h"

#include <ns3/lte-chunk-processor.h>

#include <ns3/spectrum-module.h>

#include "test-lte-unlicensed-interference.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TestLteUnlicensedInterference");


Ptr<SpectrumModel> SpectrumModelWifi5180MHz;

class static_SpectrumModelWifi5180MHz_initializer
{
public:
  static_SpectrumModelWifi5180MHz_initializer ()
  {
    BandInfo bandInfo;
    bandInfo.fc = 5180e6;
    bandInfo.fl = 5180e6 - 10e6;
    bandInfo.fh = 5180e6 + 10e6;

    Bands bands;
    bands.push_back (bandInfo);
    
    SpectrumModelWifi5180MHz = Create<SpectrumModel> (bands);
  }

} static_SpectrumModelWifi5180MHz_initializer_instance;





void
LteTestDlSchedulingCallback (LteUnlicensedInterferenceTestCase *testcase, std::string path,
                             uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                             uint8_t mcsTb1, uint16_t sizeTb1, uint8_t mcsTb2, uint16_t sizeTb2)
{
  testcase->DlScheduling (frameNo, subframeNo, rnti, mcsTb1, sizeTb1, mcsTb2, sizeTb2);
}

void
LteTestUlSchedulingCallback (LteUnlicensedInterferenceTestCase *testcase, std::string path,
                             uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                             uint8_t mcs, uint16_t sizeTb)
{
  testcase->UlScheduling (frameNo, subframeNo, rnti, mcs, sizeTb);
}

/**
 * TestSuite
 */

LteUnlicensedInterferenceTestSuite::LteUnlicensedInterferenceTestSuite ()
  : TestSuite ("lte-unlicensed-interference", SYSTEM)
{
  AddTestCase (new LteUnlicensedInterferenceTestCase ("d1=20, d2=20",  20.000000, 20.000000,  0.999989, 0.999941,  0.239826, 0.239816, 2, 2), TestCase::QUICK);
  AddTestCase (new LteUnlicensedInterferenceTestCase ("d1=20, d2=50",  20.000000, 50.000000,  6.249581, 6.247687,  1.091025, 1.090793, 8, 8), TestCase::QUICK);
  AddTestCase (new LteUnlicensedInterferenceTestCase ("d1=20, d2=200",  20.000000, 200.000000,  99.892921, 99.411076,  4.252922, 4.246313, 22, 22), TestCase::QUICK);
}

static LteUnlicensedInterferenceTestSuite lteLinkAdaptationWithInterferenceTestSuite;


/**
 * TestCase
 */

LteUnlicensedInterferenceTestCase::LteUnlicensedInterferenceTestCase (std::string name, double d1, double d2, double dlSinr, double ulSinr, double dlSe, double ulSe, uint16_t dlMcs, uint16_t ulMcs)
  : TestCase (name),
    m_d1 (d1),
    m_d2 (d2),
    m_expectedDlSinrDb (10 * std::log10 (dlSinr)),
    m_expectedUlSinrDb (10 * std::log10 (ulSinr)),
    m_dlMcs (dlMcs),
    m_ulMcs (ulMcs)
{
}

LteUnlicensedInterferenceTestCase::~LteUnlicensedInterferenceTestCase ()
{
}

void
LteUnlicensedInterferenceTestCase::DoRun (void)
{
  NS_LOG_INFO (this << GetName ());
  
  Config::SetDefault ("ns3::LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::PiroEW2010));
  Config::SetDefault ("ns3::LteAmc::Ber", DoubleValue (0.00005));

  // LTE-U DL transmission @5180 MHz
  Config::SetDefault ("ns3::LteEnbNetDevice::DlEarfcn", UintegerValue (255444));
  Config::SetDefault ("ns3::LteEnbNetDevice::DlBandwidth", UintegerValue (100));


  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisSpectrumPropagationLossModel"));
  lteHelper->SetAttribute ("UseIdealRrc", BooleanValue (false));
  lteHelper->SetAttribute ("UsePdschForCqiGeneration", BooleanValue (true));

  //Disable Uplink Power Control
  Config::SetDefault ("ns3::LteUePhy::EnableUplinkPowerControl", BooleanValue (false));

  // Create Nodes: eNodeB and UE
  NodeContainer enbNodes;
  NodeContainer wgNodes;
  NodeContainer ueNodes1;
  enbNodes.Create (1);
  wgNodes.Create (1);
  ueNodes1.Create (1);
  NodeContainer allNodes = NodeContainer (enbNodes, wgNodes, ueNodes1);

  // the topology is the following:
  //         d2
  //  UE1-----------WaveformGenerator
  //   |             |
  // d1|             |d1
  //   |     d2      |
  //  eNB1----------
  //
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));   // eNB1
  positionAlloc->Add (Vector (m_d2, m_d1, 0.0)); // WaveformGenerator
  positionAlloc->Add (Vector (0.0, m_d1, 0.0));  // UE1
  positionAlloc->Add (Vector (m_d2, 0.0, 0.0));  // unused
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (allNodes);

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs1;
  lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");
  lteHelper->SetSchedulerAttribute ("UlCqiFilter", EnumValue (FfMacScheduler::PUSCH_UL_CQI));
  enbDevs = lteHelper->InstallEnbDevice (enbNodes);
  ueDevs1 = lteHelper->InstallUeDevice (ueNodes1);

  lteHelper->Attach (ueDevs1, enbDevs.Get (0));

  // Activate an EPS bearer
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueDevs1, bearer);

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
                   MakeBoundCallback (&LteTestDlSchedulingCallback, this));

  // no UL testing since interference is in the DL only
  //Config::Connect ("/NodeList/0/DeviceList/0/LteEnbMac/UlScheduling",
  //                MakeBoundCallback (&LteTestUlSchedulingCallback, this));

  // Configure waveform generator

  Ptr<SpectrumValue> wgPsd = Create<SpectrumValue> (SpectrumModelWifi5180MHz);
  // 30 dBm over 100 RBs (18MHz total)
  *wgPsd = 1.0 / (100*180000); 
  NS_LOG_INFO ("wgPsd : " << *wgPsd);
  
  WaveformGeneratorHelper waveformGeneratorHelper;
  Config::MatchContainer match = Config::LookupMatches ("/ChannelList/0");
  if (match.GetN () != 1)
    {
      NS_FATAL_ERROR ("Lookup " << "/ChannelList/0" << " should have exactly one match");
    }
  Ptr<SpectrumChannel> dlChannel = match.Get (0)->GetObject<SpectrumChannel> ();
  NS_ABORT_MSG_IF (dlChannel == 0, "object is not of type SpectrumChannel");
  waveformGeneratorHelper.SetChannel (dlChannel);
  waveformGeneratorHelper.SetTxPowerSpectralDensity (wgPsd);
  
  waveformGeneratorHelper.SetPhyAttribute ("Period", TimeValue (Seconds (0.0007)));    waveformGeneratorHelper.SetPhyAttribute ("DutyCycle", DoubleValue (1));
  NetDeviceContainer waveformGeneratorDevices = waveformGeneratorHelper.Install (wgNodes);

  Simulator::Schedule (Seconds (0.002), &WaveformGenerator::Start,
                       waveformGeneratorDevices.Get (0)->GetObject<NonCommunicatingNetDevice> ()->GetPhy ()->GetObject<WaveformGenerator> ());
  
  

// need to allow for RRC connection establishment + SRS
  Simulator::Stop (Seconds (0.100));
  Simulator::Run ();

  if (m_dlMcs > 0)
    {
      double dlSinr1Db = 10.0 * std::log10 (dlSinr1Catcher.GetValue ()->operator[] (0));
      NS_TEST_ASSERT_MSG_EQ_TOL (dlSinr1Db, m_expectedDlSinrDb, 0.01, "Wrong SINR in DL! (eNB1 --> UE1)");
    }


  Simulator::Destroy ();

}


void
LteUnlicensedInterferenceTestCase::DlScheduling (uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                                       uint8_t mcsTb1, uint16_t sizeTb1, uint8_t mcsTb2, uint16_t sizeTb2)
{
  NS_LOG_FUNCTION (frameNo << subframeNo << rnti << (uint32_t) mcsTb1 << sizeTb1 << (uint32_t) mcsTb2 << sizeTb2);
  // need to allow for RRC connection establishment + CQI feedback reception + persistent data transmission
  if (Simulator::Now () > MilliSeconds (65))
    {
      NS_TEST_ASSERT_MSG_EQ ((uint32_t)mcsTb1, (uint32_t)m_dlMcs, "Wrong DL MCS ");
    }
}

void
LteUnlicensedInterferenceTestCase::UlScheduling (uint32_t frameNo, uint32_t subframeNo, uint16_t rnti,
                                       uint8_t mcs, uint16_t sizeTb)
{
  NS_LOG_FUNCTION (frameNo << subframeNo << rnti << (uint32_t) mcs << sizeTb);
  // need to allow for RRC connection establishment + SRS transmission
  if (Simulator::Now () > MilliSeconds (50))
    {
      NS_TEST_ASSERT_MSG_EQ ((uint32_t)mcs, (uint32_t)m_ulMcs, "Wrong UL MCS");
    }
}

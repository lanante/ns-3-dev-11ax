/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 University of Washington
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
 * Authors: Sébastien Deronne <sebastien.deronne@gmail.com>
 *          Scott Carpenter <scarpenter44@windstream.net>
 */

#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/config.h"
#include "ns3/ssid.h"
#include "ns3/wifi-phy-listener.h"
#include "ns3/mobility-helper.h"
#include "ns3/wifi-net-device.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/ieee-80211ax-indoor-propagation-loss-model.h"
#include "ns3/constant-obss-pd-algorithm.h"
#include "ns3/he-configuration.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("InterBssTestSuite");

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief HE Phy Listener
 */
class InterBssPhyListener : public ns3::WifiPhyListener
{
public:
  /**
   * Create a test PhyListener
   *
   */
  InterBssPhyListener (void)
    : m_notifyRxStart (0),
      m_notifyRxEndOk (0),
      m_notifyRxEndError (0),
      m_notifyMaybeCcaBusyStart (0),
      m_notifyTxStart (0),
      m_phy (0)
      // ,m_currentState (WifiPhyState::IDLE)
  {
  }

  virtual ~InterBssPhyListener ()
  {
    m_phy = 0;
  }

  WifiPhyState GetState (void)
  {
    if (m_phy != 0)
      {
        PointerValue ptr;
        m_phy->GetAttribute ("State", ptr);
        Ptr <WifiPhyStateHelper> state = DynamicCast <WifiPhyStateHelper> (ptr.Get<WifiPhyStateHelper> ());
        // std::cout << "At " << Simulator::Now() << " PHY state is " << state->GetState () << std::endl;
        return state->GetState ();
      }
    else
      {
        // ERROR
        return WifiPhyState::IDLE;
      }
  }

  virtual void NotifyRxStart (Time duration)
  {
    NS_LOG_FUNCTION (this << duration);
    ++m_notifyRxStart;
  }

  virtual void NotifyRxEndOk (void)
  {
    NS_LOG_FUNCTION (this);
    ++m_notifyRxEndOk;
  }

  virtual void NotifyRxEndError (void)
  {
    NS_LOG_FUNCTION (this);
    ++m_notifyRxEndError;
  }

  virtual void NotifyTxStart (Time duration, double txPowerDbm)
  {
    NS_LOG_FUNCTION (this << duration << txPowerDbm);
    ++m_notifyTxStart;
  }

  virtual void NotifyMaybeCcaBusyStart (Time duration)
  {
    NS_LOG_FUNCTION (this);
    ++m_notifyMaybeCcaBusyStart;
  }

  virtual void NotifySwitchingStart (Time duration)
  {
  }

  virtual void NotifySleep (void)
  {
  }

  virtual void NotifyOff (void)
  {
  }

  virtual void NotifyWakeup (void)
  {
  }

  virtual void NotifyOn (void)
  {
  }

  uint32_t m_notifyRxStart; ///< notify receive start
  uint32_t m_notifyRxEndOk; ///< notify receive end OK
  uint32_t m_notifyRxEndError; ///< notify receive end error
  uint32_t m_notifyMaybeCcaBusyStart; ///< notify maybe CCA busy start
  uint32_t m_notifyTxStart; ///< notify trasnmit start
  Ptr<WifiPhy> m_phy; ///< the PHY being listened to
};

// Parse context strings of the form "/NodeList/3/DeviceList/1/Mac/Assoc"
// to extract the NodeId
uint32_t
ConvertContextToNodeId (std::string context)
{
  std::string sub = context.substr (10);
  uint32_t pos = sub.find ("/Device");
  uint32_t nodeId = atoi (sub.substr (0, pos).c_str ());
  return nodeId;
}

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief Wifi Test
 *
 * This test case tests the transmission of a inter-BSS cases
 * Specifically:
 * There are two networks, each with one AP and one STA, with topology as:
 * The topology for this test case is 2 STAs and 2 APs:
 *  STA1  --d1--  AP1  --d2--  AP2  --d3-- STA2
 *  RX1           TX1          TX2         RX2
 *
 * Goal: This test case is designed to verify whether the simulator can correctly reset
 * the PHY when receiving an InterBSS PPDU.
 *
 * Assumptions: TX1 and TX2 are full buffer.
 *
 * Parameters:
 *  OBSS_PD level = -72dbm
 *  Received Power by TX1 from TX2 = [-62dbm, -82dbm]
 *  Received SINR by RX1 from TX1 > 3dB (enough to pass MCS0 reception)
 *  Received SINR by RX2 from TX2 > 3dB (enough to pass MCS0 reception)
 *  TX1/RX1 BSS Color =1
 *  TX2/RX2 transmission PPDU BSS Color =[ 2 0]
 *  PHY = 11ax, MCS 0, 80MHz
 *  TX2 PHY header SR Field SRP_AND_NON-SRG _OBSS-PD_PROHIBITED = [ 0 1]
 *
 */

class TestInterBssConstantObssPdAlgo : public TestCase
{
public:
  TestInterBssConstantObssPdAlgo ();

  virtual void DoRun (void);

  /**
   * Send one packet function
   * \param tx_dev the transmitting device
   * \param rx_dev the receiving device
   * \param payloadSize the payload size
   */
  void SendOnePacket (Ptr<WifiNetDevice> tx_dev, Ptr<WifiNetDevice> rx_dev, uint32_t payloadSize);

private:
  /**
   * Get the number of STAs
   */
  uint32_t GetNumberOfStas ();

  /**
   * Get the number of APs
   */
  uint32_t GetNumberOfAps ();

  /**
   * Allocate the node positions
   */
  Ptr<ListPositionAllocator> AllocatePositions ();

  /**
   * Setup the simulation
   */
  void SetupSimulation ();

  /**
   * Check the results
   */
  void CheckResults ();

  /// Run one function
  void RunOne (void);

  /**
   * Check if the Phy State for a node is an expected value
   * \param idx is 1 for AP, 0 for STA
   * \param idx expectedState the expected WifiPhyState
   */
  void CheckPhyState (uint32_t idx, WifiPhyState expectedState);

  unsigned int m_numStaPacketsSent; ///< number of sent packets
  unsigned int m_numApPacketsSent; ///< number of sent packets
  unsigned int m_totalReceivedPackets; ///< total number of recevied packets, regardless of pkt size
  unsigned int m_payloadSize1; ///< size in bytes of packet #1 payload
  unsigned int m_payloadSize2; ///< size in bytes of packet #2 payload
  Time m_firstTransmissionTime; ///< first transmission time
  SpectrumWifiPhyHelper m_phy; ///< the PHY
  NetDeviceContainer m_staDevices;
  NetDeviceContainer m_apDevices;
  bool m_receivedPayload1;
  bool m_receivedPayload2;
  NodeContainer m_allNodes;

  InterBssPhyListener* m_listener; ///< listener

  /**
   * Notify Phy transmit begin
   * \param context the context
   * \param p the packet
   */
  void NotifyPhyTxBegin (std::string context, Ptr<const Packet> p);

  /**
   * Notify Phy receive endsn
   * \param context the context
   * \param p the packet
   */
  void NotifyPhyRxEnd (std::string context, Ptr<const Packet> p);

  /**
   * Notify end of HE preamble
   * \param rssi the rssi of the received packet
   * \param bssColor the BSS color
   */
  void NotifyEndOfHePreamble (std::string context, HePreambleParameters params);

  /**
   * Get ptr to the OBB PD algorithm object
   */
  Ptr<ObssPdAlgorithm> GetObssPdAlgorithm (Ptr<Node>);
};

TestInterBssConstantObssPdAlgo::TestInterBssConstantObssPdAlgo ()
  : TestCase ("InterBssConstantObssPd"),
    m_numStaPacketsSent (0),
    m_numApPacketsSent (0),
    m_totalReceivedPackets (0),
    m_payloadSize1 (1500),
    m_payloadSize2 (1510),
    m_receivedPayload1 (false),
    m_receivedPayload2 (false)
{
  m_firstTransmissionTime = Seconds (0);
  m_phy = SpectrumWifiPhyHelper::Default ();
}

// The topology for this test case is 2 STAs and 2 APs:
//  STA1  --d1--  AP1  --d2--  AP2  --d3-- STA2
//  RX1           TX1          TX2         RX2
//
//  TX1 and TX2 are full buffer
//
// this test case confirms TBD

// @TODO:
// This is intended to be the base class for the test suite of OBSS PD test cases
// as defined by Leonardo Lanante.  The basics of Test1a is given here, although
// is may need to be ported to another derived class, if there is a need to separate
// the functionality among those tests.
// For now, this test only sets up 2 APs and 2 STAs, with the APs separated by some
// distance, d2, and then each AP sends just 1 packet.  This test case will therefore
// need to be enhanced to send packets full stream, and check throughput.

uint32_t
TestInterBssConstantObssPdAlgo::GetNumberOfStas ()
{
  uint32_t nStas = 2;
  return nStas;
}

uint32_t
TestInterBssConstantObssPdAlgo::GetNumberOfAps ()
{
  uint32_t nAps = 2;
  return nAps;
}

Ptr<ListPositionAllocator>
TestInterBssConstantObssPdAlgo::AllocatePositions ()
{
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  double d1 = 10.0;
  double d2 = 100.0;
  double d3 = 10.0;

  positionAlloc->Add (Vector (d1,       0.0, 0.0));  // AP1
  positionAlloc->Add (Vector (d1 + d2,    0.0, 0.0));  // AP1
  positionAlloc->Add (Vector (0.0,      0.0, 0.0));  // STA1
  positionAlloc->Add (Vector (d1 + d2 + d3, 0.0, 0.0));  // STA2

  return positionAlloc;
}

void
TestInterBssConstantObssPdAlgo::SetupSimulation ()
{
  Ptr<WifiNetDevice> ap_device1 = DynamicCast<WifiNetDevice> (m_apDevices.Get (0));
  Ptr<WifiNetDevice> ap_device2 = DynamicCast<WifiNetDevice> (m_apDevices.Get (1));
  Ptr<WifiNetDevice> sta_device1 = DynamicCast<WifiNetDevice> (m_staDevices.Get (0));
  Ptr<WifiNetDevice> sta_device2 = DynamicCast<WifiNetDevice> (m_staDevices.Get (1));

  // the STA will send packet #1 after 1s (allowing the Wifi network to reach some steady state)
  Simulator::Schedule (Seconds (1.0), &TestInterBssConstantObssPdAlgo::SendOnePacket, this, ap_device1, sta_device1, m_payloadSize1);

  // the STA will send packet #2 0.5s later, does not lead to a collision
  Simulator::Schedule (Seconds (1.5), &TestInterBssConstantObssPdAlgo::SendOnePacket, this, ap_device2, sta_device2, m_payloadSize2);

  Simulator::Stop (Seconds (1.6));
}

void
TestInterBssConstantObssPdAlgo::CheckResults ()
{
  // TODO
  // Test case is not fully complete.  For now, each AP sends only 1 packet.  The inter-node
  // distances are such that the sent packet is received by one STA (e.g., AP1 --> STA1, and AP2 --> STA2).
  // Further implementation of OBSS_PD algorithm is needed so that pkts get "dropped" under the correct
  // conditions, in order to further these test cases.

  // expect 2 packets successfully sent, one by each AP
  NS_TEST_ASSERT_MSG_EQ (m_numStaPacketsSent, 0, "The number of packets sent by STAs is not correct!");
  NS_TEST_ASSERT_MSG_EQ (m_numApPacketsSent, 2, "The number of packets sent by APs is not correct!");
  NS_TEST_ASSERT_MSG_EQ (m_receivedPayload1, false, "The payload for STA1 was received, and should not have been!");
  NS_TEST_ASSERT_MSG_EQ (m_receivedPayload2, false, "The payload for STA2 was received, and should not have been!");
}

void
TestInterBssConstantObssPdAlgo::NotifyPhyTxBegin (std::string context, Ptr<const Packet> p)
{
  uint32_t idx = ConvertContextToNodeId (context);
  // get the packet size
  uint32_t pktSize = p->GetSize ();

  // only count packets originated from the STAs that match our payloadSize
  uint32_t nStas = GetNumberOfStas ();
  if ((idx < nStas) && (pktSize >= m_payloadSize1))
    {
      if (m_numStaPacketsSent == 0)
        {
          // this is the first packet
          m_firstTransmissionTime = Simulator::Now ();
          NS_ASSERT_MSG (m_firstTransmissionTime >= Time (Seconds (1)), "Packet 0 not transmitted at 1 second");

          // relative to the start of receiving the packet...
          // The PHY will remain in the IDLE state from 0..4 us
          // After the preamble is received (takes 24us), then the PHY will switch to RX
          // The PHY will remain in RX after the headeer is successfully decoded

          // at 4us, AP PHY STATE should be IDLE
          Simulator::Schedule (MicroSeconds (4.0), &TestInterBssConstantObssPdAlgo::CheckPhyState, this, 1, WifiPhyState::IDLE);

          // at 20us, AP PHY STATE should be in RX if the preamble detection succeeded
          Simulator::Schedule (MicroSeconds (20.0), &TestInterBssConstantObssPdAlgo::CheckPhyState, this, 1, WifiPhyState::RX);

          // in 40us, AP PHY STATE should be in RX if the header decoded successfully (IDLE if not)
          Simulator::Schedule (MicroSeconds (40.0), &TestInterBssConstantObssPdAlgo::CheckPhyState, this, 1, WifiPhyState::RX);
        }

      m_numStaPacketsSent++;
    }
  else
    {
      if (pktSize >= m_payloadSize1)
        {
          m_numApPacketsSent++;
        }
    }
}

void
TestInterBssConstantObssPdAlgo::NotifyPhyRxEnd (std::string context, Ptr<const Packet> p)
{
  m_totalReceivedPackets++;

  uint32_t idx = ConvertContextToNodeId (context);
  // get the packet size
  uint32_t pktSize = p->GetSize ();

  // only count packets originated from the STAs that are received at the AP and that match our payloadSize
  uint32_t nStas = GetNumberOfStas ();
  if ((idx == nStas) && (pktSize >= m_payloadSize1))
    {
      if (pktSize == (m_payloadSize1 + 42))
        {
          m_receivedPayload1 = true;
        }
      else if (pktSize == (m_payloadSize2 + 42))
        {
          m_receivedPayload2 = true;
        }
    }
}

Ptr <ObssPdAlgorithm>
TestInterBssConstantObssPdAlgo::GetObssPdAlgorithm (Ptr<Node> node)
{
  NS_ASSERT (node != 0);
  // get ptr to the node's device
  Ptr<WifiNetDevice> device = DynamicCast<WifiNetDevice> (node->GetDevice (0));
  NS_ASSERT (device != 0);
  // from the device, get ptr to the ObssPdAlgorithm
  Ptr<ObssPdAlgorithm> obssPdAlgorithm = device->GetObject<ObssPdAlgorithm> ();
  return obssPdAlgorithm;
}

void
TestInterBssConstantObssPdAlgo::NotifyEndOfHePreamble (std::string context, HePreambleParameters params)
{
  // get the node id from context
  uint32_t idx = ConvertContextToNodeId (context);
  // get ptr to the node
  Ptr<Node> node = m_allNodes.Get (idx);
  // get the ptr to the OBSS PD algorithm object for this node
  Ptr<ObssPdAlgorithm> obssPdAlgorithm = GetObssPdAlgorithm (node);
  if (obssPdAlgorithm)
    {
      // call the OBSS PD algorithm for evaluation
      obssPdAlgorithm->ReceiveHeSigA (params);
    }
}

void
TestInterBssConstantObssPdAlgo::SendOnePacket (Ptr<WifiNetDevice> tx_dev, Ptr<WifiNetDevice> rx_dev, uint32_t payloadSize)
{
  Ptr<Packet> p = Create<Packet> (payloadSize);
  tx_dev->Send (p, rx_dev->GetAddress (), 1);
}

void
TestInterBssConstantObssPdAlgo::CheckPhyState (uint32_t idx, WifiPhyState expectedState)
{
  if (idx == 1) // AP
    {
      WifiPhyState state = m_listener->GetState ();
      std::ostringstream ossMsg;
      ossMsg << "PHY State " << state << " does not match expected state " << expectedState << " at " << Simulator::Now ();
      NS_TEST_ASSERT_MSG_EQ (state, expectedState, ossMsg.str ());
    }
}

void
TestInterBssConstantObssPdAlgo::RunOne (void)
{
  // initializations
  m_numStaPacketsSent = 0;
  m_firstTransmissionTime = Seconds (0);

  // 1 STA
  uint32_t nStas = GetNumberOfStas ();
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nStas);

  // 1 AP
  uint32_t nAps = GetNumberOfAps ();
  NodeContainer wifiApNodes;
  wifiApNodes.Create (nAps);

  m_allNodes.Add (wifiStaNodes);
  m_allNodes.Add (wifiApNodes);

  // PHY setup
  Ptr<MultiModelSpectrumChannel> channel
    = CreateObject<MultiModelSpectrumChannel> ();
  Ptr<ConstantSpeedPropagationDelayModel> delayModel
    = CreateObject<ConstantSpeedPropagationDelayModel> ();
  channel->SetPropagationDelayModel (delayModel);

  // Ideally we do not need to use the Ieee80211ax loss models.  But, the combination
  // of which propagation model and the stream number causes the test cases to otherwise fail,
  // mainly the two packet tests in which the second packet should not be received incurs a backoff and
  // then IS received.  For now, am leaving the Ieee80211ax loss model in place, and will debug further
  // later.

  // Use TGax Indoor propagation loss model
  Config::SetDefault ("ns3::Ieee80211axIndoorPropagationLossModel::DistanceBreakpoint", DoubleValue (10.0));
  Config::SetDefault ("ns3::Ieee80211axIndoorPropagationLossModel::Walls", DoubleValue (0.0));
  Config::SetDefault ("ns3::Ieee80211axIndoorPropagationLossModel::WallsFactor", DoubleValue (0.0));

  uint32_t streamNumber = 100;

  Ptr<Ieee80211axIndoorPropagationLossModel> lossModel = CreateObject<Ieee80211axIndoorPropagationLossModel> ();
  streamNumber += lossModel->AssignStreams (streamNumber);
  channel->AddPropagationLossModel (lossModel);
  m_phy.SetChannel (channel);

  // 802.11ax
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211ax_5GHZ);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue ("HeMcs0"),
                                "ControlMode", StringValue ("HeMcs0"));

  // assign STA MAC
  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  m_staDevices = wifi.Install (m_phy, mac, wifiStaNodes);

  /*for (uint32_t i = 0; i < m_staDevices.GetN (); i++)
    {
      Ptr<WifiNetDevice> wifiNetDevice = DynamicCast<WifiNetDevice> (m_staDevices.Get (i));
      Ptr<HeConfiguration> heConfiguration = wifiNetDevice->GetHeConfiguration ();
          
      heConfiguration->SetAttribute ("ObssPdThreshold", DoubleValue (-99.0));
      heConfiguration->SetAttribute ("ObssPdThresholdMin", DoubleValue (-82.0));
      heConfiguration->SetAttribute ("ObssPdThresholdMax", DoubleValue (-62.0));
    }*/

  wifi.AssignStreams (m_staDevices, streamNumber);

  // assign AP MAC
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "BeaconGeneration", BooleanValue (true));

  // install Wifi
  m_apDevices = wifi.Install (m_phy, mac, wifiApNodes);

  wifi.AssignStreams (m_apDevices, streamNumber);

  for (uint32_t i = 0; i < m_apDevices.GetN (); i++)
    {
      Ptr<WifiNetDevice> wifiNetDevice = DynamicCast<WifiNetDevice> (m_apDevices.Get (i));
      Ptr<HeConfiguration> heConfiguration = wifiNetDevice->GetHeConfiguration ();
      heConfiguration->SetAttribute ("BssColor", UintegerValue (i + 1));
      //heConfiguration->SetAttribute ("ObssPdThreshold", DoubleValue (-99.0));
      //heConfiguration->SetAttribute ("ObssPdThresholdMin", DoubleValue (-82.0));
      //heConfiguration->SetAttribute ("ObssPdThresholdMax", DoubleValue (-62.0));
    }

  // fixed positions
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = AllocatePositions ();

  mobility.SetPositionAllocator (positionAlloc);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNodes);
  mobility.Install (wifiStaNodes);

  Ptr<WifiNetDevice> ap_device = DynamicCast<WifiNetDevice> (m_apDevices.Get (0));

  // Create a PHY listener for the AP's PHY.  This will track state changes and be used
  // to confirm at certain times that the AP is in the right state
  m_listener = new InterBssPhyListener;
  Ptr<WifiPhy> apPhy = DynamicCast<WifiPhy> (ap_device->GetPhy ());
  m_listener->m_phy = apPhy;
  apPhy->RegisterListener (m_listener);

  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxBegin", MakeCallback (&TestInterBssConstantObssPdAlgo::NotifyPhyTxBegin, this));
  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxEnd", MakeCallback (&TestInterBssConstantObssPdAlgo::NotifyPhyRxEnd, this));

  SetupSimulation ();

  Simulator::Run ();
  Simulator::Destroy ();
  delete m_listener;

  CheckResults ();
}

void
TestInterBssConstantObssPdAlgo::DoRun (void)
{
  LogComponentEnableAll (LOG_PREFIX_TIME);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  LogComponentEnable ("ConstantObssPdAlgorithm", LOG_LEVEL_ALL);
  LogComponentEnable ("ObssPdAlgorithm", LOG_LEVEL_ALL);
  LogComponentEnable ("WifiPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("MacLow", LOG_LEVEL_ALL);

  m_payloadSize1 = 1000;
  m_payloadSize2 = 1500;

  RunOne ();
}

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief Inter BSS Test Suite
 */

class InterBssTestSuite : public TestSuite
{
public:
  InterBssTestSuite ();
};

InterBssTestSuite::InterBssTestSuite ()
  : TestSuite ("wifi-inter-bss", UNIT)
{
  AddTestCase (new TestInterBssConstantObssPdAlgo, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static InterBssTestSuite interBssTestSuite;

/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
 *               2010      NICTA
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
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Quincy Tse <quincy.tse@nicta.com.au>
 *          Sébastien Deronne <sebastien.deronne@gmail.com>
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

NS_LOG_COMPONENT_DEFINE ("WifiHeTestSuite");

// This test suite contains the following test cases:
//
//   AddTestCase (new TestSinglePacketTxTimings, TestCase::QUICK);
//   This test case tests a single STA sending a single packet to an AP
//   that is successfully received and that the PHY state transitions of the frame
//   received occur at the expected times, specifically the PHY remains in IDLE for 4us at
//   which time there is an attempt to detect the preamble, then the PHY remains in RX for
//   20us (from the start of the frame) after which time there is an attempt to detect the packet
//
//   AddTestCase (new TestTwoPacketsNoCollision, TestCase::QUICK);
//   This test case tests 2 STAs each sending one packet to the AP.  The 2nd STA sends its packet
//   0.5s after the first STA has sent its packet.  In this scenario, the AP successfully receives
//   both packets.
//
//   AddTestCase (new TestTwoPacketsCollisionStrongFirstFrame, TestCase::QUICK);
//   This test case tests 2 STAs each sending one packet to the AP.  The 2nd STA sends its packet
//   1us after the first STA has sent its packet.  In this scenario, the AP successfully receives
//   and keeps only 1 packet, which is the first packet, the stronger signalled oneboth packets.
//
//   AddTestCase (new TestTwoPacketsCollisionWeakFirstFrame, TestCase::QUICK);
//   This test case tests 2 STAs each sending one packet to the AP.  The 2nd STA sends its packet
//   1us after the first STA has sent its packet.  In this scenario, the AP successfully receives
//   and keeps only 1 packet, which is the first packet, the weaker signalled one.  Thus, this test
//   slightly reverses the above test, in which the ordering of "strong packet send first" has been
//   changed to "weaker packet sent first".


// Parse context strings of the form "/NodeList/3/DeviceList/1/Mac/Assoc"
// to extract the NodeId
uint32_t
ContextToNodeId (std::string context)
{
  std::string sub = context.substr (10);  // skip "/NodeList/"
  uint32_t pos = sub.find ("/Device");
  uint32_t nodeId = atoi (sub.substr (0, pos).c_str ());
  // NS_LOG_DEBUG ("Found NodeId " << nodeId << " from context " << context);
  return nodeId;
}

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief HE Phy Listener
 */
class HePhyListener : public ns3::WifiPhyListener
{
public:
  /**
   * Create a test PhyListener
   *
   */
  HePhyListener (void)
    : m_notifyRxStart (0),
      m_notifyRxEndOk (0),
      m_notifyRxEndError (0),
      m_notifyMaybeCcaBusyStart (0),
      m_notifyTxStart (0),
      m_phy (0)
      // ,m_currentState (WifiPhyState::IDLE)
  {
  }

  virtual ~HePhyListener ()
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
  // WifiPhyState m_currentState; ///< the current WifiPhyState
};

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief Wifi-He Test Case base class
 *
 * This class sets up generic information for Wifi-He test cases
 * so that they are all configured similarly and run consistently
 */
class WifiHeTestCase : public TestCase
{
public:
  WifiHeTestCase ();

  virtual void DoRun (void);

  /**
   * Send one packet function
   * \param dev the device
   */
  void SendOnePacket (Ptr<WifiNetDevice> dev, uint32_t payloadSize);

protected:
  virtual void DoSetup (void);

protected:
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
  bool m_enableHeConfiguration;
  uint32_t m_expectedBssColor;
  NodeContainer m_allNodes;

  HePhyListener* m_listener; ///< listener

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

  // derived test case classes need to override these methods to control behaviors.

  /**
   * Get the number of STAs
   */
  virtual uint32_t GetNumberOfStas ();

  /**
   * Get the number of APs
   */
  virtual uint32_t GetNumberOfAps ();

  /**
   * Allocate the node positions
   */
  virtual Ptr<ListPositionAllocator> AllocatePositions ();

  /**
   * Setup the simulation
   */
  virtual void SetupSimulation ();

  /**
   * Check the results
   */
  virtual void CheckResults ();

private:
  /**
   * Get ptr to the OBB PD algorithm object
   */
  Ptr<ObssPdAlgorithm> GetObssPdAlgorithm (Ptr<Node>);


};

WifiHeTestCase::WifiHeTestCase ()
  : TestCase ("WifiHe"),
    m_numStaPacketsSent (0),
    m_numApPacketsSent (0),
    m_totalReceivedPackets (0),
    m_payloadSize1 (1500),
    m_payloadSize2 (1510),
    m_receivedPayload1 (false),
    m_receivedPayload2 (false),
    m_enableHeConfiguration (false),
    m_expectedBssColor (0)
{
  m_firstTransmissionTime = Seconds (0);
  m_phy = SpectrumWifiPhyHelper::Default ();
}

void
WifiHeTestCase::NotifyPhyTxBegin (std::string context, Ptr<const Packet> p)
{
  uint32_t idx = ContextToNodeId (context);
  // get the packet size
  uint32_t pktSize = p->GetSize ();

  // only count packets originated from the STAs that match our payloadSize
  uint32_t nStas = GetNumberOfStas ();
  if ((idx < nStas) && (pktSize >= m_payloadSize1))
    {
      // std::cout << "PhyTxBegin at " << Simulator::Now() << " idx=" << idx << " pktSize=" << pktSize << " " << context << " pkt: " << p << std::endl;
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
          Simulator::Schedule (MicroSeconds (4.0), &WifiHeTestCase::CheckPhyState, this, 1, WifiPhyState::IDLE);

          // at 20us, AP PHY STATE should be in RX if the preamble detection succeeded
          Simulator::Schedule (MicroSeconds (20.0), &WifiHeTestCase::CheckPhyState, this, 1, WifiPhyState::RX);

          // in 40us, AP PHY STATE should be in RX if the header decoded successfully (IDLE if not)
          Simulator::Schedule (MicroSeconds (40.0), &WifiHeTestCase::CheckPhyState, this, 1, WifiPhyState::RX);
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
WifiHeTestCase::NotifyPhyRxEnd (std::string context, Ptr<const Packet> p)
{
  m_totalReceivedPackets++;

  uint32_t idx = ContextToNodeId (context);
  // get the packet size
  uint32_t pktSize = p->GetSize ();

/**
// Debugging.  TODO remove
  if (pktSize >= m_payloadSize1)
    {
      std::cout << "PhyRxEnd at " << Simulator::Now() << " " << pktSize << " " << context << " pkt: " << p << std::endl;
    }
**/

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
WifiHeTestCase::GetObssPdAlgorithm (Ptr<Node> node)
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
WifiHeTestCase::NotifyEndOfHePreamble (std::string context, HePreambleParameters params)
{
  // get the node id from context
  uint32_t idx = ContextToNodeId (context);
  // get ptr to the node
  Ptr<Node> node = m_allNodes.Get (idx);
  // get the ptr to the OBSS PD algorithm object for this node
  Ptr<ObssPdAlgorithm> obssPdAlgorithm = GetObssPdAlgorithm (node);
  if (obssPdAlgorithm)
    {
      // call the OBSS PD algorithm for evaluation
      obssPdAlgorithm->ReceiveHeSigA (params);
    }

  // std::cout << "NotifyEndOfHePreamble has fired. rssi=" << rssi << " BSS color=" << ((uint32_t) bssColor) << std::endl;
}

void
WifiHeTestCase::SendOnePacket (Ptr<WifiNetDevice> dev, uint32_t payloadSize)
{

  Ptr<Packet> p = Create<Packet> (payloadSize);
  dev->Send (p, dev->GetBroadcast (), 1);
}

void
WifiHeTestCase::CheckPhyState (uint32_t idx, WifiPhyState expectedState)
{
  if (idx == 1) // AP
    {
      WifiPhyState state = m_listener->GetState ();
      std::ostringstream ossMsg;
      ossMsg << "PHY State " << state << " does not match expected state " << expectedState << " at " << Simulator::Now ();
      NS_TEST_ASSERT_MSG_EQ (state, expectedState, ossMsg.str ());
      // std::cout << "Checking PHY STATE at " << Simulator::Now() << " Expected " << expectedState << " got " << state << std::endl;
    }
}

uint32_t
WifiHeTestCase::GetNumberOfStas ()
{
  uint32_t nStas = 1;
  return nStas;
}

uint32_t
WifiHeTestCase::GetNumberOfAps ()
{
  uint32_t nAps = 1;
  return nAps;
}

Ptr<ListPositionAllocator>
WifiHeTestCase::AllocatePositions ()
{
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  positionAlloc->Add (Vector (0.0, 0.0, 0.0));  // AP1
  positionAlloc->Add (Vector (5.0, 0.0, 0.0));  // STA1

  return positionAlloc;
}

void
WifiHeTestCase::SetupSimulation ()
{
  Ptr<WifiNetDevice> sta1_device = DynamicCast<WifiNetDevice> (m_staDevices.Get (0));

  // the STA will send 1 packet after 1s (allowing the Wifi network to reach some steady state)
  Simulator::Schedule (Seconds (1.0), &WifiHeTestCase::SendOnePacket, this, sta1_device, m_payloadSize1);

  // 2s should be enough time ot complete the simulation...
  Simulator::Stop (Seconds (2.0));
}

void
WifiHeTestCase::CheckResults ()
{
  // expect only 1 packet successfully sent
  NS_TEST_ASSERT_MSG_EQ (m_numStaPacketsSent, 1, "The number of sent packets is not correct!");
}

void
WifiHeTestCase::RunOne (void)
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
  //Ptr<FriisPropagationLossModel> lossModel = CreateObject<FriisPropagationLossModel> ();
  streamNumber += lossModel->AssignStreams (streamNumber);
  channel->AddPropagationLossModel (lossModel);
  m_phy.SetChannel (channel);

  // RTS/CTS disabled by default

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

  if (m_enableHeConfiguration)
    {
      for (uint32_t i = 0; i < m_staDevices.GetN (); i++)
        {
          Ptr<WifiNetDevice> wifiNetDevice = DynamicCast<WifiNetDevice> (m_staDevices.Get (i));
          Ptr<HeConfiguration> heConfiguration = wifiNetDevice->GetHeConfiguration ();
          
          heConfiguration->SetAttribute ("ObssPdThreshold", DoubleValue (-99.0));
          heConfiguration->SetAttribute ("ObssPdThresholdMin", DoubleValue (-82.0));
          heConfiguration->SetAttribute ("ObssPdThresholdMax", DoubleValue (-62.0));
        }
    }

  wifi.AssignStreams (m_staDevices, streamNumber);

  // assign AP MAC
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "BeaconGeneration", BooleanValue (true));

  // install Wifi
  m_apDevices = wifi.Install (m_phy, mac, wifiApNodes);

  wifi.AssignStreams (m_apDevices, streamNumber);

  if (m_enableHeConfiguration)
    {
      for (uint32_t i = 0; i < m_apDevices.GetN (); i++)
        {
          Ptr<WifiNetDevice> wifiNetDevice = DynamicCast<WifiNetDevice> (m_apDevices.Get (i));
          Ptr<HeConfiguration> heConfiguration = wifiNetDevice->GetHeConfiguration ();
          
          heConfiguration->SetAttribute ("BssColor", UintegerValue (m_expectedBssColor));
          heConfiguration->SetAttribute ("ObssPdThreshold", DoubleValue (-99.0));
          heConfiguration->SetAttribute ("ObssPdThresholdMin", DoubleValue (-82.0));
          heConfiguration->SetAttribute ("ObssPdThresholdMax", DoubleValue (-62.0));
        }
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
  m_listener = new HePhyListener;
  Ptr<WifiPhy> apPhy = DynamicCast<WifiPhy> (ap_device->GetPhy ());
  m_listener->m_phy = apPhy;
  apPhy->RegisterListener (m_listener);

  // PCAP (for debugging)
  // m_phy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
  // m_phy.EnablePcap ("wifi-he-test", sta_device);

  // traces:
  // PhyTxBegin - used to test when AP senses STA begins Tx a packet
  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxBegin", MakeCallback (&WifiHeTestCase::NotifyPhyTxBegin, this));
  // PhyRxEnd - used to test when AP receives a packet from a STA
  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxEnd", MakeCallback (&WifiHeTestCase::NotifyPhyRxEnd, this));

  SetupSimulation ();

  Simulator::Run ();
  Simulator::Destroy ();
  delete m_listener;

  CheckResults ();
}

void
WifiHeTestCase::DoSetup (void)
{
  // m_listener = new HePhyListener;
  // m_phy.m_state->RegisterListener (m_listener);
}

void
WifiHeTestCase::DoRun (void)
{
  m_payloadSize1 = 1500;
  m_payloadSize2 = 1510;

  RunOne ();
}

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief Wifi Test
 *
 * This test case tests the transmission of a single packet in a Wifi HE network (802.11ax),
 * from a STA, that is successfully received by an AP.
 * The Tx timing transistions are tested to confirm that they conform to expectations.
 * Specifically:
 * The STA (sender) submits the packet for sending at 1.0s
 * The AP (receiver) senses the channel as WifiPhyState::Tx shortly thereafter.
 * Then, relative to the start of the frame:
 * at time 4.0 us, PHY is still in IDLE state.
 * at time 20.0us, PHY has switched to RX state, if preamble detected, then start decode packet.
 * at time 20.0us, PHY still in RX state if packet decode has been successful
 */
class TestSinglePacketTxTimings : public WifiHeTestCase
{
public:
  TestSinglePacketTxTimings ();

protected:
  /**
   * Get the number of STAs
   */
  virtual uint32_t GetNumberOfStas ();

  /**
   * Get the number of APs
   */
  virtual uint32_t GetNumberOfAps ();

  /**
   * Allocate the node positions
   */
  virtual Ptr<ListPositionAllocator> AllocatePositions ();

  /**
   * Setup the simulation
   */
  virtual void SetupSimulation ();

  /**
   * Check the results
   */
  virtual void CheckResults ();

};

TestSinglePacketTxTimings::TestSinglePacketTxTimings ()
  : WifiHeTestCase ()
{
}

// The topology for this test case is 1 STA to 1 AP:
//  AP  --5m--  STA1
//
// at t=1.0s, STA1 sends one packet to AP
//
// this test case confirms the transitions from PHY state IDLE to RX occur at expected times
// and confirms that 1 packet was successfull received

uint32_t
TestSinglePacketTxTimings::GetNumberOfStas ()
{
  uint32_t nStas = 1;
  return nStas;
}

uint32_t
TestSinglePacketTxTimings::GetNumberOfAps ()
{
  uint32_t nAps = 1;
  return nAps;
}

Ptr<ListPositionAllocator>
TestSinglePacketTxTimings::AllocatePositions ()
{
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  positionAlloc->Add (Vector (0.0, 0.0, 0.0));  // AP1
  positionAlloc->Add (Vector (5.0, 0.0, 0.0));  // STA1

  return positionAlloc;
}

void
TestSinglePacketTxTimings::SetupSimulation ()
{
  Ptr<WifiNetDevice> sta1_device = DynamicCast<WifiNetDevice> (m_staDevices.Get (0));

  // the STA will send 1 packet after 1s (allowing the Wifi network to reach some steady state)
  Simulator::Schedule (Seconds (1.0), &TestSinglePacketTxTimings::SendOnePacket, this, sta1_device, m_payloadSize1);

  // 2s should be enough time ot complete the simulation...
  Simulator::Stop (Seconds (2.0));
}

void
TestSinglePacketTxTimings::CheckResults ()
{
  // expect only 1 packet successfully sent, and only 1 packet successfully received, from the first STA
  NS_TEST_ASSERT_MSG_EQ (m_numStaPacketsSent, 1, "The number of sent packets is not correct!");
  NS_TEST_ASSERT_MSG_EQ (m_receivedPayload1, true, "The payload for STA1 was not received!");
  NS_TEST_ASSERT_MSG_EQ (m_receivedPayload2, false, "The payload for STA2 was received, and should not have been received!");
}

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief Wifi Test
 *
 * This test case tests the transmission of a two packets sent 0.5s apart that do not collide in a Wifi HE network (802.11ax),
 * from a STA, that is successfully received by an AP.
 * The Tx timing transistions are tested to confirm that they conform to expectations.
 * Specifically:
 * The STA (sender) submits the packet for sending at 1.0s
 * The AP (receiver) senses the channel as WifiPhyState::Tx shortly thereafter.
 */

class TestTwoPacketsNoCollision : public WifiHeTestCase
{
public:
  TestTwoPacketsNoCollision ();

protected:
  /**
   * Get the number of STAs
   */
  virtual uint32_t GetNumberOfStas ();

  /**
   * Get the number of APs
   */
  virtual uint32_t GetNumberOfAps ();

  /**
   * Allocate the node positions
   */
  virtual Ptr<ListPositionAllocator> AllocatePositions ();

  /**
   * Setup the simulation
   */
  virtual void SetupSimulation ();

  /**
   * Check the results
   */
  virtual void CheckResults ();

};

TestTwoPacketsNoCollision::TestTwoPacketsNoCollision ()
  : WifiHeTestCase ()
{
}

// The topology for this test case is 2 STAs to 1 AP:
//  STA1  --10m--  AP  --20m--  STA2
//
// at t=4.0s, STA1 sends one packet to AP
// at t=4.5s, STA2 sends one packet to AP
//
// this test case confirms the transitions from PHY state IDLE to RX occur at expected times
// and confirms that 2 packets are successfully received

uint32_t
TestTwoPacketsNoCollision::GetNumberOfStas ()
{
  uint32_t nStas = 2;
  return nStas;
}

uint32_t
TestTwoPacketsNoCollision::GetNumberOfAps ()
{
  uint32_t nAps = 1;
  return nAps;
}

Ptr<ListPositionAllocator>
TestTwoPacketsNoCollision::AllocatePositions ()
{
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  positionAlloc->Add (Vector (10.0, 0.0, 0.0));  // AP1
  positionAlloc->Add (Vector ( 0.0, 0.0, 0.0));  // STA1
  positionAlloc->Add (Vector (20.0, 0.0, 0.0));  // STA2

  return positionAlloc;
}

void
TestTwoPacketsNoCollision::SetupSimulation ()
{
  Ptr<WifiNetDevice> sta_device1 = DynamicCast<WifiNetDevice> (m_staDevices.Get (0));
  Ptr<WifiNetDevice> sta_device2 = DynamicCast<WifiNetDevice> (m_staDevices.Get (1));

  // the STA will send packet #1 after 1s (allowing the Wifi network to reach some steady state)
  Simulator::Schedule (MicroSeconds (4000000), &TestTwoPacketsNoCollision::SendOnePacket, this, sta_device1, m_payloadSize1);

  // the STA will send packet #2 0.5s later, does not lead to a collision
  Simulator::Schedule (MicroSeconds (4500000), &TestTwoPacketsNoCollision::SendOnePacket, this, sta_device2, m_payloadSize2);

  // 2s should be enough time to complete the simulation...
  Simulator::Stop (Seconds (5.0));
}

void
TestTwoPacketsNoCollision::CheckResults ()
{
  // expect 2 packets successfully sent and both received
  NS_TEST_ASSERT_MSG_EQ (m_numStaPacketsSent, 2, "The number of sent packets is not correct!");
  NS_TEST_ASSERT_MSG_EQ (m_receivedPayload1, true, "The payload for STA1 was not received!");
  NS_TEST_ASSERT_MSG_EQ (m_receivedPayload2, true, "The payload for STA2 was not received!");
}

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief Wifi Test
 *
 * This test case tests the transmission of a two packets sent 1us apart that collide in a Wifi HE network (802.11ax),
 * from a STA, that is successfully received by an AP.
 * The Tx timing transistions are tested to confirm that they conform to expectations.
 */

class TestTwoPacketsCollisionStrongFirstFrame : public WifiHeTestCase
{
public:
  TestTwoPacketsCollisionStrongFirstFrame ();

protected:
  /**
   * Get the number of STAs
   */
  virtual uint32_t GetNumberOfStas ();

  /**
   * Get the number of APs
   */
  virtual uint32_t GetNumberOfAps ();

  /**
   * Allocate the node positions
   */
  virtual Ptr<ListPositionAllocator> AllocatePositions ();

  /**
   * Setup the simulation
   */
  virtual void SetupSimulation ();

  /**
   * Check the results
   */
  virtual void CheckResults ();

};

TestTwoPacketsCollisionStrongFirstFrame::TestTwoPacketsCollisionStrongFirstFrame ()
  : WifiHeTestCase ()
{
}

// The topology for this test case is 2 STAs to 1 AP:
//  STA1  --10m--  AP  --20m--  STA2
//
// at t=4.0s, STA1 sends one packet to AP
// at t=4.000001s, (1us later) STA2 sends one packet to AP
//
// this test case confirms the transitions from PHY state IDLE to RX occur at expected times.
// Frame #1 will be stronger than Frame #2 (due to distance from AP).
// The first frame will be dropped, and only the second frame will be received.
// This test onfirms that 2 packets are sent but only 1 is successfully received.

uint32_t
TestTwoPacketsCollisionStrongFirstFrame::GetNumberOfStas ()
{
  uint32_t nStas = 2;
  return nStas;
}

uint32_t
TestTwoPacketsCollisionStrongFirstFrame::GetNumberOfAps ()
{
  uint32_t nAps = 1;
  return nAps;
}

Ptr<ListPositionAllocator>
TestTwoPacketsCollisionStrongFirstFrame::AllocatePositions ()
{
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  positionAlloc->Add (Vector (10.0, 0.0, 0.0));  // AP1
  positionAlloc->Add (Vector ( 0.0, 0.0, 0.0));  // STA1
  positionAlloc->Add (Vector (100.0, 0.0, 0.0));  // STA2

  return positionAlloc;
}

void
TestTwoPacketsCollisionStrongFirstFrame::SetupSimulation ()
{
  Ptr<WifiNetDevice> sta_device1 = DynamicCast<WifiNetDevice> (m_staDevices.Get (0));
  Ptr<WifiNetDevice> sta_device2 = DynamicCast<WifiNetDevice> (m_staDevices.Get (1));

  // the STA will send packet #1 after 1s (allowing the Wifi network to reach some steady state)
  Simulator::Schedule (MicroSeconds (4000000), &TestTwoPacketsCollisionStrongFirstFrame::SendOnePacket, this, sta_device1, m_payloadSize1);

  // the STA will send packet #2 1us later, leading to a collision
  Simulator::Schedule (MicroSeconds (4000001), &TestTwoPacketsCollisionStrongFirstFrame::SendOnePacket, this, sta_device2, m_payloadSize2);

  // 2s should be enough time to complete the simulation...
  // Simulator::Stop (Seconds (4.002));
  Simulator::Stop (Seconds (5.0));
}

void
TestTwoPacketsCollisionStrongFirstFrame::CheckResults ()
{
  // expect 1 packet successfully sent, and 1 successfully received, from the first STA (stronger signal, first arriving packet)
  NS_TEST_ASSERT_MSG_EQ (m_numStaPacketsSent, 1, "The number of sent packets is not correct!");
  NS_TEST_ASSERT_MSG_EQ (m_receivedPayload1, true, "The payload for STA1 was received, and should have been dropped!");
  NS_TEST_ASSERT_MSG_EQ (m_receivedPayload2, false, "The payload for STA2 was not received!");
}

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief Wifi Test
 *
 * This test case tests the transmission of a two packets sent 1us apart that collide in a Wifi HE network (802.11ax),
 * from a STA, that is successfully received by an AP.
 * The Tx timing transistions are tested to confirm that they conform to expectations.
 */

class TestTwoPacketsCollisionWeakFirstFrame : public WifiHeTestCase
{
public:
  TestTwoPacketsCollisionWeakFirstFrame ();

protected:
  /**
   * Get the number of STAs
   */
  virtual uint32_t GetNumberOfStas ();

  /**
   * Get the number of APs
   */
  virtual uint32_t GetNumberOfAps ();

  /**
   * Allocate the node positions
   */
  virtual Ptr<ListPositionAllocator> AllocatePositions ();

  /**
   * Setup the simulation
   */
  virtual void SetupSimulation ();

  /**
   * Check the results
   */
  virtual void CheckResults ();

};

TestTwoPacketsCollisionWeakFirstFrame::TestTwoPacketsCollisionWeakFirstFrame ()
  : WifiHeTestCase ()
{
}

// The topology for this test case is 2 STAs to 1 AP:
//  STA1  --10m--  AP  --20m--  STA2
//
// at t=4.0s, STA1 sends one packet to AP
// at t=4.000001s, (1us later) STA2 sends one packet to AP
//
// this test case confirms the transitions from PHY state IDLE to RX occur at expected times.
// Frame #1 will be stronger than Frame #2 (due to distance from AP).
// The first frame will be dropped, and only the second frame will be received.
// This test onfirms that 2 packets are sent but only 1 is successfully received.

uint32_t
TestTwoPacketsCollisionWeakFirstFrame::GetNumberOfStas ()
{
  uint32_t nStas = 2;
  return nStas;
}

uint32_t
TestTwoPacketsCollisionWeakFirstFrame::GetNumberOfAps ()
{
  uint32_t nAps = 1;
  return nAps;
}

Ptr<ListPositionAllocator>
TestTwoPacketsCollisionWeakFirstFrame::AllocatePositions ()
{
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  positionAlloc->Add (Vector (90.0, 0.0, 0.0));  // AP1
  positionAlloc->Add (Vector ( 0.0, 0.0, 0.0));  // STA1
  positionAlloc->Add (Vector (100.0, 0.0, 0.0));  // STA2

  return positionAlloc;
}

void
TestTwoPacketsCollisionWeakFirstFrame::SetupSimulation ()
{
  Ptr<WifiNetDevice> sta_device1 = DynamicCast<WifiNetDevice> (m_staDevices.Get (0));
  Ptr<WifiNetDevice> sta_device2 = DynamicCast<WifiNetDevice> (m_staDevices.Get (1));

  // the STA will send packet #1 after 1s (allowing the Wifi network to reach some steady state)
  Simulator::Schedule (MicroSeconds (4000000), &TestTwoPacketsCollisionStrongFirstFrame::SendOnePacket, this, sta_device1, m_payloadSize1);

  // the STA will send packet #2 1us later, leading to a collision
  Simulator::Schedule (MicroSeconds (4000001), &TestTwoPacketsCollisionStrongFirstFrame::SendOnePacket, this, sta_device2, m_payloadSize2);

  // 2s should be enough time to complete the simulation...
  // Simulator::Stop (Seconds (4.002));
  Simulator::Stop (Seconds (5.0));
}

void
TestTwoPacketsCollisionWeakFirstFrame::CheckResults ()
{
  // expect 1 packets successfully sent and 1 successfully received, from the second STA (weaker signal, but arrives at AP first)
  NS_TEST_ASSERT_MSG_EQ (m_numStaPacketsSent, 1, "The number of sent packets is not correct!");
  NS_TEST_ASSERT_MSG_EQ (m_receivedPayload1, false, "The payload for STA1 was received, and should have been dropped!");
  NS_TEST_ASSERT_MSG_EQ (m_receivedPayload2, true, "The payload for STA2 was not received!");
}

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief Wifi Test
 *
 * This test case tests the transmission of a single packet in a Wifi HE network (802.11ax),
 * from a STA, that is successfully received by an AP.
 * This test confirms that the EndOfHePreamble event fires
 */
class TestSinglePacketEndOfHePreambleNoBssColor : public WifiHeTestCase
{
public:
  TestSinglePacketEndOfHePreambleNoBssColor ();

protected:
  /**
   * Notify end of HE preamble
   * \param params the HE preamble parameters
   */
  void NotifyEndOfHePreamble (std::string context, HePreambleParameters params);

  /**
   * Get the number of STAs
   */
  virtual uint32_t GetNumberOfStas ();

  /**
   * Get the number of APs
   */
  virtual uint32_t GetNumberOfAps ();

  /**
   * Allocate the node positions
   */
  virtual Ptr<ListPositionAllocator> AllocatePositions ();

  /**
   * Setup the simulation
   */
  virtual void SetupSimulation ();

  /**
   * Check the results
   */
  virtual void CheckResults ();
};

TestSinglePacketEndOfHePreambleNoBssColor::TestSinglePacketEndOfHePreambleNoBssColor ()
  : WifiHeTestCase ()
{
  m_expectedBssColor = 0;
}

// The topology for this test case is 1 STA to 1 AP:
//  AP  --5m--  STA1
//
// at t=1.0s, STA1 sends one packet to AP
//
// this test case confirms the transitions from PHY state IDLE to RX occur at expected times
// and confirms that 1 packet was successfull received

uint32_t
TestSinglePacketEndOfHePreambleNoBssColor::GetNumberOfStas ()
{
  uint32_t nStas = 1;
  return nStas;
}

uint32_t
TestSinglePacketEndOfHePreambleNoBssColor::GetNumberOfAps ()
{
  uint32_t nAps = 1;
  return nAps;
}

Ptr<ListPositionAllocator>
TestSinglePacketEndOfHePreambleNoBssColor::AllocatePositions ()
{
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  positionAlloc->Add (Vector (0.0, 0.0, 0.0));  // AP1
  positionAlloc->Add (Vector (5.0, 0.0, 0.0));  // STA1

  return positionAlloc;
}

void
TestSinglePacketEndOfHePreambleNoBssColor::NotifyEndOfHePreamble (std::string context, HePreambleParameters params)
{
  // call base class
  WifiHeTestCase::NotifyEndOfHePreamble (context, params);

  //  uint32_t idx = ContextToNodeId (context);

  NS_TEST_ASSERT_MSG_EQ (m_expectedBssColor, (uint32_t) params.bssColor, "The received packet HE BSS Color is not the expected color!");
}

void
TestSinglePacketEndOfHePreambleNoBssColor::SetupSimulation ()
{
  // PhyEndOfHePreamble - used to test that the PHY EndOfHePreamble event has fired
  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/EndOfHePreamble", MakeCallback (&TestSinglePacketEndOfHePreambleNoBssColor::NotifyEndOfHePreamble, this));


  Ptr<WifiNetDevice> sta1_device = DynamicCast<WifiNetDevice> (m_staDevices.Get (0));

  // the STA will send 1 packet after 1s (allowing the Wifi network to reach some steady state)
  Simulator::Schedule (Seconds (1.0), &TestSinglePacketEndOfHePreambleNoBssColor::SendOnePacket, this, sta1_device, m_payloadSize1);

  // 2s should be enough time ot complete the simulation...
  Simulator::Stop (Seconds (2.0));
}

void
TestSinglePacketEndOfHePreambleNoBssColor::CheckResults ()
{
  // expect only 1 packet successfully sent, and only 1 packet successfully received, from the first STA
  NS_TEST_ASSERT_MSG_EQ (m_numStaPacketsSent, 1, "The number of sent packets is not correct!");
  NS_TEST_ASSERT_MSG_EQ (m_receivedPayload1, true, "The payload for STA1 was not received!");
  NS_TEST_ASSERT_MSG_EQ (m_receivedPayload2, false, "The payload for STA2 was received, and should not have been received!");
}

/**
 * \ingroup wifi-he-test-suite
 * \ingroup tests
 *
 * \brief Wifi-HE Test Suite
 */

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief Wifi Test
 *
 * This test case tests the transmission of a single packet in a Wifi HE network (802.11ax),
 * from a STA, that is successfully received by an AP.
 * This test confirms that the EndOfHePreamble event fires
 */
class TestSinglePacketEndOfHePreambleCorrectBssColor : public WifiHeTestCase
{
public:
  TestSinglePacketEndOfHePreambleCorrectBssColor ();

protected:
  /**
   * Notify end of HE preamble
   * \param rssi the rssi of the received packet
   * \param bssColor the BSS color
   */
  void NotifyEndOfHePreamble (std::string context, HePreambleParameters params);

  /**
   * Get the number of STAs
   */
  virtual uint32_t GetNumberOfStas ();

  /**
   * Get the number of APs
   */
  virtual uint32_t GetNumberOfAps ();

  /**
   * Allocate the node positions
   */
  virtual Ptr<ListPositionAllocator> AllocatePositions ();

  /**
   * Setup the simulation
   */
  virtual void SetupSimulation ();

  /**
   * Check the results
   */
  virtual void CheckResults ();

};

TestSinglePacketEndOfHePreambleCorrectBssColor::TestSinglePacketEndOfHePreambleCorrectBssColor ()
  : WifiHeTestCase ()
{
  m_enableHeConfiguration = true;
  m_expectedBssColor = 1;
}

// The topology for this test case is 1 STA to 1 AP:
//  AP  --5m--  STA1
//
// at t=1.0s, STA1 sends one packet to AP
//
// this test case confirms the transitions from PHY state IDLE to RX occur at expected times
// and confirms that 1 packet was successfull received

uint32_t
TestSinglePacketEndOfHePreambleCorrectBssColor::GetNumberOfStas ()
{
  uint32_t nStas = 1;
  return nStas;
}

uint32_t
TestSinglePacketEndOfHePreambleCorrectBssColor::GetNumberOfAps ()
{
  uint32_t nAps = 1;
  return nAps;
}

Ptr<ListPositionAllocator>
TestSinglePacketEndOfHePreambleCorrectBssColor::AllocatePositions ()
{
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  positionAlloc->Add (Vector (0.0, 0.0, 0.0));  // AP1
  positionAlloc->Add (Vector (5.0, 0.0, 0.0));  // STA1

  return positionAlloc;
}

void
TestSinglePacketEndOfHePreambleCorrectBssColor::NotifyEndOfHePreamble (std::string context, HePreambleParameters params)
{
  // call base class
  WifiHeTestCase::NotifyEndOfHePreamble (context, params);

  uint32_t idx = ContextToNodeId (context);

  if (idx == 1)
    {
      // The AP should have the expected BSS color
      NS_TEST_ASSERT_MSG_EQ (m_expectedBssColor, (uint32_t) params.bssColor, "The AP received packet HE BSS Color is not the expected color!");
    }
  else
    {
      if (m_totalReceivedPackets < 1 )
        {
          // the STA's first received packet will have a color of 0, since the BSS color association is not complete
          NS_TEST_ASSERT_MSG_EQ (0, (uint32_t) params.bssColor, "The STA received packet HE BSS Color is not the expected color!");
        }
      else
        {
          // after 0.25s, assume the network has settled, and the BSS color asisgnment has been completed.
          // first packet is not sent until 1.0s
          Time now = Simulator::Now ();
          if (now > Seconds (0.25))
            {
              // Also, each STA should have the expected BSS color
              NS_TEST_ASSERT_MSG_EQ (m_expectedBssColor, (uint32_t) params.bssColor, "The STA received packet HE BSS Color is not the expected color!");
            }
        }
    }
}

void
TestSinglePacketEndOfHePreambleCorrectBssColor::SetupSimulation ()
{
  // PhyEndOfHePreamble - used to test that the PHY EndOfHePreamble event has fired
  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/EndOfHePreamble", MakeCallback (&TestSinglePacketEndOfHePreambleCorrectBssColor::NotifyEndOfHePreamble, this));


  Ptr<WifiNetDevice> sta1_device = DynamicCast<WifiNetDevice> (m_staDevices.Get (0));

  // the STA will send 1 packet after 1s (allowing the Wifi network to reach some steady state)
  Simulator::Schedule (Seconds (1.0), &TestSinglePacketEndOfHePreambleCorrectBssColor::SendOnePacket, this, sta1_device, m_payloadSize1);

  // 2s should be enough time ot complete the simulation...
  Simulator::Stop (Seconds (2.0));
}

void
TestSinglePacketEndOfHePreambleCorrectBssColor::CheckResults ()
{
  // expect only 1 packet successfully sent, and only 1 packet successfully received, from the first STA
  NS_TEST_ASSERT_MSG_EQ (m_numStaPacketsSent, 1, "The number of sent packets is not correct!");
  NS_TEST_ASSERT_MSG_EQ (m_receivedPayload1, true, "The payload for STA1 was not received!");
  NS_TEST_ASSERT_MSG_EQ (m_receivedPayload2, false, "The payload for STA2 was received, and should not have been received!");
}

/**
 * \ingroup wifi-he-test-suite
 * \ingroup tests
 *
 * \brief Wifi-HE Test Suite
 */

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief Wifi Test
 *
 * This test case tests the transmission of a single packet in a Wifi HE network (802.11ax),
 * from a STA, that is successfully received by an AP.
 * This test confirms that the EndOfHePreamble event fires
 */
class TestSinglePacketEndOfHePreambleResetPhyOnMagicBssColor : public WifiHeTestCase
{
public:
  TestSinglePacketEndOfHePreambleResetPhyOnMagicBssColor ();

protected:
  /**
   * Notify end of HE preamble
   * \param rssi the rssi of the received packet
   * \param bssColor the BSS color
   */
  void NotifyEndOfHePreamble (std::string context, HePreambleParameters params);

  /**
   * Get the number of STAs
   */
  virtual uint32_t GetNumberOfStas ();

  /**
   * Get the number of APs
   */
  virtual uint32_t GetNumberOfAps ();

  /**
   * Allocate the node positions
   */
  virtual Ptr<ListPositionAllocator> AllocatePositions ();

  /**
   * Setup the simulation
   */
  virtual void SetupSimulation ();

  /**
   * Check the results
   */
  virtual void CheckResults ();

};

TestSinglePacketEndOfHePreambleResetPhyOnMagicBssColor::TestSinglePacketEndOfHePreambleResetPhyOnMagicBssColor ()
  : WifiHeTestCase ()
{
  m_enableHeConfiguration = true;
  // Use a magic BSS Color = 54 to denote nodes that will reset t he PHY upon notifiction of EndOfHeCPreamble event
  m_expectedBssColor = 54;
}

// The topology for this test case is 1 STA to 1 AP:
//  AP  --5m--  STA1
//
// at t=1.0s, STA1 sends one packet to AP
//
// this test case confirms the transitions from PHY state IDLE to RX occur at expected times
// and confirms that 1 packet was successfull received

uint32_t
TestSinglePacketEndOfHePreambleResetPhyOnMagicBssColor::GetNumberOfStas ()
{
  uint32_t nStas = 1;
  return nStas;
}

uint32_t
TestSinglePacketEndOfHePreambleResetPhyOnMagicBssColor::GetNumberOfAps ()
{
  uint32_t nAps = 1;
  return nAps;
}

Ptr<ListPositionAllocator>
TestSinglePacketEndOfHePreambleResetPhyOnMagicBssColor::AllocatePositions ()
{
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  positionAlloc->Add (Vector (0.0, 0.0, 0.0));  // AP1
  positionAlloc->Add (Vector (5.0, 0.0, 0.0));  // STA1

  return positionAlloc;
}

void
TestSinglePacketEndOfHePreambleResetPhyOnMagicBssColor::NotifyEndOfHePreamble (std::string context, HePreambleParameters params)
{
  // call base class
  WifiHeTestCase::NotifyEndOfHePreamble (context, params);

  uint32_t idx = ContextToNodeId (context);

  if (idx == 1)
    {
      // The AP should have the expected BSS color
      NS_TEST_ASSERT_MSG_EQ (m_expectedBssColor, (uint32_t) params.bssColor, "The AP received packet HE BSS Color is not the expected color!");
      if (params.bssColor == 54)
        {

          // current PHY state should be RX
          WifiPhyState state = m_listener->GetState ();
          NS_TEST_ASSERT_MSG_EQ (state, WifiPhyState::RX, "The PHY is not in the RX state!");

          // this is the AP, and we have received notification of End of HE Preamble, and the BSS color is our
          // magic number.  Reset the PHY, and then check that the  PHY returns to IDLE shortly thereafter
          m_listener->m_phy->ResetCca ();

          // at 4us, AP PHY STATE should be IDLE
          Simulator::Schedule (MicroSeconds (4.0), &TestSinglePacketEndOfHePreambleResetPhyOnMagicBssColor::CheckPhyState, this, 1, WifiPhyState::IDLE);
        }
    }
  else
    {
      if (m_totalReceivedPackets < 1 )
        {
          // the STA's first received packet will have a color of 0, since the BSS color association is not complete
          NS_TEST_ASSERT_MSG_EQ (0, (uint32_t) params.bssColor, "The STA received packet HE BSS Color is not the expected color!");
        }
      else
        {
          Time now = Simulator::Now ();
          // after 0.25s, assume the network has settled, and the BSS color asisgnment has been completed.
          // first packet is not sent until 1.0s
          if (now > Seconds (0.25))
            {
              // Also, each STA should have the expected BSS color
              NS_TEST_ASSERT_MSG_EQ (m_expectedBssColor, (uint32_t) params.bssColor, "The STA received packet HE BSS Color is not the expected color!");
            }
        }
    }
}

void
TestSinglePacketEndOfHePreambleResetPhyOnMagicBssColor::SetupSimulation ()
{
  // PhyEndOfHePreamble - used to test that the PHY EndOfHePreamble event has fired
  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/EndOfHePreamble", MakeCallback (&TestSinglePacketEndOfHePreambleResetPhyOnMagicBssColor::NotifyEndOfHePreamble, this));


  Ptr<WifiNetDevice> sta1_device = DynamicCast<WifiNetDevice> (m_staDevices.Get (0));

  // the STA will send 1 packet after 1s (allowing the Wifi network to reach some steady state)
  Simulator::Schedule (Seconds (1.0), &TestSinglePacketEndOfHePreambleResetPhyOnMagicBssColor::SendOnePacket, this, sta1_device, m_payloadSize1);

  // 2s should be enough time ot complete the simulation...
  Simulator::Stop (Seconds (2.0));
}

void
TestSinglePacketEndOfHePreambleResetPhyOnMagicBssColor::CheckResults ()
{
  // expect only 1 packet successfully sent, and only 1 packet successfully received, from the first STA
  NS_TEST_ASSERT_MSG_EQ (m_numStaPacketsSent, 0, "The number of sent packets is not correct!");
  //NS_TEST_ASSERT_MSG_EQ (m_receivedPayload1, false, "The payload for STA1 was not received!");
  //NS_TEST_ASSERT_MSG_EQ (m_receivedPayload2, false, "The payload for STA2 was received, and should not have been received!");
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

class TestInterBss : public WifiHeTestCase
{
public:
  TestInterBss ();

protected:
  /**
   * Get the number of STAs
   */
  virtual uint32_t GetNumberOfStas ();

  /**
   * Get the number of APs
   */
  virtual uint32_t GetNumberOfAps ();

  /**
   * Allocate the node positions
   */
  virtual Ptr<ListPositionAllocator> AllocatePositions ();

  /**
   * Setup the simulation
   */
  virtual void SetupSimulation ();

  /**
   * Check the results
   */
  virtual void CheckResults ();

};

TestInterBss::TestInterBss ()
  : WifiHeTestCase ()
{
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
TestInterBss::GetNumberOfStas ()
{
  uint32_t nStas = 2;
  return nStas;
}

uint32_t
TestInterBss::GetNumberOfAps ()
{
  uint32_t nAps = 2;
  return nAps;
}

Ptr<ListPositionAllocator>
TestInterBss::AllocatePositions ()
{
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  double d1 = 10.0;
  double d2 = 5000.0;
  double d3 = 10.0;

  positionAlloc->Add (Vector (d1,       0.0, 0.0));  // AP1
  positionAlloc->Add (Vector (d1 + d2,    0.0, 0.0));  // AP1
  positionAlloc->Add (Vector (0.0,      0.0, 0.0));  // STA2
  positionAlloc->Add (Vector (d1 + d2 + d3, 0.0, 0.0));  // STA2

  return positionAlloc;
}

void
TestInterBss::SetupSimulation ()
{
  Ptr<WifiNetDevice> ap_device1 = DynamicCast<WifiNetDevice> (m_apDevices.Get (0));
  Ptr<WifiNetDevice> ap_device2 = DynamicCast<WifiNetDevice> (m_apDevices.Get (1));

  // the STA will send packet #1 after 1s (allowing the Wifi network to reach some steady state)
  Simulator::Schedule (MicroSeconds (4000000), &TestInterBss::SendOnePacket, this, ap_device1, m_payloadSize1);

  // the STA will send packet #2 0.5s later, does not lead to a collision
  Simulator::Schedule (MicroSeconds (4500000), &TestInterBss::SendOnePacket, this, ap_device2, m_payloadSize2);

  // 2s should be enough time to complete the simulation...
  Simulator::Stop (Seconds (5.0));
}

void
TestInterBss::CheckResults ()
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

/**
 * \ingroup wifi-he-test-suite
 * \ingroup tests
 *
 * \brief Wifi-HE Test Suite
 */

class WifiHeTestSuite : public TestSuite
{
public:
  WifiHeTestSuite ();
};

WifiHeTestSuite::WifiHeTestSuite ()
  : TestSuite ("wifi-he", UNIT)
{
  AddTestCase (new TestSinglePacketTxTimings, TestCase::QUICK);
  AddTestCase (new TestTwoPacketsNoCollision, TestCase::QUICK);
  AddTestCase (new TestTwoPacketsCollisionStrongFirstFrame, TestCase::QUICK);
  AddTestCase (new TestTwoPacketsCollisionWeakFirstFrame, TestCase::QUICK);
  AddTestCase (new TestSinglePacketEndOfHePreambleNoBssColor, TestCase::QUICK);
  AddTestCase (new TestSinglePacketEndOfHePreambleCorrectBssColor, TestCase::QUICK);
  //AddTestCase (new TestSinglePacketEndOfHePreambleResetPhyOnMagicBssColor, TestCase::QUICK);
  AddTestCase (new TestInterBss, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static WifiHeTestSuite wifiHeTestSuite;

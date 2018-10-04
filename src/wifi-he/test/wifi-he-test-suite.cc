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

#include "ns3/spectrum-phy.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/wifi-spectrum-value-helper.h"
#include "ns3/spectrum-wifi-phy.h"
#include "ns3/nist-error-rate-model.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/wifi-mac-trailer.h"
#include "ns3/wifi-phy-tag.h"
#include "ns3/wifi-spectrum-signal-parameters.h"
#include "ns3/wifi-phy-listener.h"
#include "ns3/log.h"

#include "ns3/string.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/wifi-net-device.h"
#include "ns3/adhoc-wifi-mac.h"
#include "ns3/ap-wifi-mac.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/yans-error-rate-model.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/test.h"
#include "ns3/pointer.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/config.h"
#include "ns3/packet-socket-server.h"
#include "ns3/packet-socket-client.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/wifi-spectrum-signal-parameters.h"
#include "ns3/wifi-phy-tag.h"
#include "ns3/yans-wifi-phy.h"
#include "ns3/mgt-headers.h"
#include "ns3/node-list.h"
#include "ns3/ieee-80211ax-indoor-propagation-loss-model.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiHeTestSuite");

/*****
//Helper function to assign streams to random variables, to control
//randomness in the tests
static void
AssignWifiRandomStreams (Ptr<WifiMac> mac, int64_t stream)
{
  int64_t currentStream = stream;
  Ptr<RegularWifiMac> rmac = DynamicCast<RegularWifiMac> (mac);
  if (rmac)
    {
      PointerValue ptr;
      rmac->GetAttribute ("Txop", ptr);
      Ptr<Txop> txop = ptr.Get<Txop> ();
      currentStream += txop->AssignStreams (currentStream);

      rmac->GetAttribute ("VO_Txop", ptr);
      Ptr<QosTxop> vo_txop = ptr.Get<QosTxop> ();
      currentStream += vo_txop->AssignStreams (currentStream);

      rmac->GetAttribute ("VI_Txop", ptr);
      Ptr<QosTxop> vi_txop = ptr.Get<QosTxop> ();
      currentStream += vi_txop->AssignStreams (currentStream);

      rmac->GetAttribute ("BE_Txop", ptr);
      Ptr<QosTxop> be_txop = ptr.Get<QosTxop> ();
      currentStream += be_txop->AssignStreams (currentStream);

      rmac->GetAttribute ("BK_Txop", ptr);
      Ptr<QosTxop> bk_txop = ptr.Get<QosTxop> ();
      bk_txop->AssignStreams (currentStream);
    }
}
*****/

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
 * \brief Test Phy Listener
 */
class TestPhyListener : public ns3::WifiPhyListener
{
public:
  /**
   * Create a test PhyListener
   *
   */
  TestPhyListener (void)
    : m_notifyRxStart (0),
      m_notifyRxEndOk (0),
      m_notifyRxEndError (0),
      m_notifyMaybeCcaBusyStart (0),
      m_phy (0),
      m_currentState (WifiPhyState::IDLE)
  {
  }

  virtual ~TestPhyListener ()
  {
  }

  WifiPhyState GetState (void)
  {
    if (m_phy != 0)
      {
        PointerValue ptr;
        m_phy->GetAttribute ("State", ptr);
        Ptr <WifiPhyStateHelper> state = DynamicCast <WifiPhyStateHelper> (ptr.Get<WifiPhyStateHelper> ());
        std::cout << "At " << Simulator::Now() << " PHY state is " << state->GetState () << std::endl;
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
  Ptr<WifiPhy> m_phy; ///< the PHY being listened to
  WifiPhyState m_currentState; ///< the current WifiPhyState

private:
};

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
 * at time 2.0 us, the receiver state == IDLE.
 * TBD:
 * at time 4.0 us, preamble detection.
 * at time 6.0us, if preamble succeeded, then state == RX.
 * at time 20.0 us, check header
 * at time 40.0 us, if header is decoded successfully, then state == RX, else state == IDLE
 */
class TestSinglePacketTxTimings : public TestCase
{
public:
  TestSinglePacketTxTimings ();

  virtual void DoRun (void);

protected:
  virtual void DoSetup (void);

private:
  /// Run one function
  void RunOne (void);

  /**
   * Send one packet function
   * \param dev the device
   */
  void SendOnePacket (Ptr<WifiNetDevice> dev);

  /**
   * Check if the Phy State for a node is an expected value
   * \param idx is 1 for AP, 0 for STA
   * \param idx expectedState the expected WifiPhyState
   */
  void CheckPhyState (uint32_t idx, WifiPhyState expectedState);

  unsigned int m_numSentPackets; ///< number of sent packets
  unsigned int m_payloadSize; ///< size in bytes of packet payload
  Time m_firstTransmissionTime; ///< first transmission time
  SpectrumWifiPhyHelper m_phy; ///< the PHY

  TestPhyListener* m_listener; ///< listener

  /**
   * Notify Phy transmit begin
   * \param context the context
   * \param p the packet
   */
  void NotifyPhyTxBegin (std::string context, Ptr<const Packet> p);
};

TestSinglePacketTxTimings::TestSinglePacketTxTimings ()
  : TestCase ("WifiHe"),
  m_numSentPackets (0),
  m_payloadSize (1500)
{
  m_firstTransmissionTime = Seconds (0);
  m_phy = SpectrumWifiPhyHelper::Default ();
}

void
TestSinglePacketTxTimings::NotifyPhyTxBegin (std::string context, Ptr<const Packet> p)
{
  uint32_t idx = ContextToNodeId (context);
  // get the packet size
  uint32_t pktSize = p->GetSize ();

  // only count packets originated from the STA (idx==0) that match our payloadSize
  if ((idx == 0) && (pktSize >= m_payloadSize))
    {
      std::cout << "PhyTxBegin at " << Simulator::Now() << " " << pktSize << " " << context << " pkt: " << p << std::endl;
      if (m_numSentPackets == 0)
        {
          // this is the first packet
          m_firstTransmissionTime = Simulator::Now();
          NS_ASSERT_MSG (m_firstTransmissionTime >= Time (Seconds (1)), "Packet 0 not transmitted at 1 second");

          // relative to the start of receiving the packet...
          // The PHY will remain in the IDLE state from 0..4 us
          // After the preamble is received (takes 24us), then the PHY will switch to RX
          // The PHY will remain in RX after the headeer is successfully decoded

          // at 4us, AP PHY STATE should be IDLE
          Simulator::Schedule (MicroSeconds (4.0), &TestSinglePacketTxTimings::CheckPhyState, this, 1, WifiPhyState::IDLE);

          // at 20us, AP PHY STATE should be in RX if the preamble detection succeeded
          Simulator::Schedule (MicroSeconds (20.0), &TestSinglePacketTxTimings::CheckPhyState, this, 1, WifiPhyState::RX);

          // in 40us, AP PHY STATE should be in RX if the header decoded successfully (IDLE if not)
          Simulator::Schedule (MicroSeconds (40.0), &TestSinglePacketTxTimings::CheckPhyState, this, 1, WifiPhyState::RX);
        }

      m_numSentPackets++;
    }
}

void
TestSinglePacketTxTimings::SendOnePacket (Ptr<WifiNetDevice> dev)
{

  Ptr<Packet> p = Create<Packet> (m_payloadSize);
  dev->Send (p, dev->GetBroadcast (), 1);
}

void
TestSinglePacketTxTimings::CheckPhyState (uint32_t idx, WifiPhyState expectedState)
{
  if (idx == 1) // AP
    {
      WifiPhyState state = m_listener->GetState ();
      std::ostringstream ossMsg;
      ossMsg << "PHY State " << state << " does not match expected state " << expectedState << " at " << Simulator::Now();
      NS_TEST_ASSERT_MSG_EQ (state, expectedState, ossMsg.str ());
      // std::cout << "Checking PHY STATE at " << Simulator::Now() << " Expected " << expectedState << " got " << state << std::endl;
    }
}

void
TestSinglePacketTxTimings::RunOne (void)
{
  // initializations
  m_numSentPackets = 0;
  m_firstTransmissionTime = Seconds (0);

  // 1 STA
  NodeContainer wifiStaNode;
  wifiStaNode.Create (1);

  // 1 AP
  NodeContainer wifiApNode;
  wifiApNode.Create (1);

  // PHY setup
  Ptr<MultiModelSpectrumChannel> channel
    = CreateObject<MultiModelSpectrumChannel> ();
  Ptr<ConstantSpeedPropagationDelayModel> delayModel
    = CreateObject<ConstantSpeedPropagationDelayModel> ();
  channel->SetPropagationDelayModel (delayModel);

  // Use TGax Indoor propagation loss model
  Config::SetDefault ("ns3::Ieee80211axIndoorPropagationLossModel::DistanceBreakpoint", DoubleValue (10.0));
  Config::SetDefault ("ns3::Ieee80211axIndoorPropagationLossModel::Walls", DoubleValue (0.0));
  Config::SetDefault ("ns3::Ieee80211axIndoorPropagationLossModel::WallsFactor", DoubleValue (0.0));

  Ptr<Ieee80211axIndoorPropagationLossModel> lossModel = CreateObject<Ieee80211axIndoorPropagationLossModel> ();
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

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (m_phy, mac, wifiStaNode);

  // assign AP MAC
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "BeaconGeneration", BooleanValue (true));

  // install Wifi
  NetDeviceContainer apDevices;
  apDevices = wifi.Install (m_phy, mac, wifiApNode);

  // fixed positions
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  positionAlloc->Add (Vector (0.0, 0.0, 0.0));  // AP1
  positionAlloc->Add (Vector (5.0, 0.0, 0.0));  // STA1
  mobility.SetPositionAllocator (positionAlloc);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);
  mobility.Install (wifiStaNode);

  Ptr<WifiNetDevice> ap_device = DynamicCast<WifiNetDevice> (apDevices.Get (0));
  Ptr<WifiNetDevice> sta_device = DynamicCast<WifiNetDevice> (staDevices.Get (0));

  // Create a PHY listener for the AP's PHY.  This will track state changes and be used
  // to confirm at certain times that the AP is in the right state
  m_listener = new TestPhyListener;
  Ptr<WifiPhy> apPhy = DynamicCast<WifiPhy> (ap_device->GetPhy());
  m_listener->m_phy = apPhy;
  apPhy->RegisterListener (m_listener);

  // PCAP (for debugging)
  // m_phy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
  // m_phy.EnablePcap ("wifi-he-test", sta_device);

  // traces:
  // PhyTxBegin - used to test when AP senses STA begins Tx a packet 
  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxBegin", MakeCallback (&TestSinglePacketTxTimings::NotifyPhyTxBegin, this));

  // the STA will send 1 packet after 1s (allowing the Wifi network to reach some steady state)
  Simulator::Schedule (Seconds (1.0), &TestSinglePacketTxTimings::SendOnePacket, this, sta_device);

  // 2s should be enough time ot complete the simulation...
  Simulator::Stop (Seconds (2.0));

  Simulator::Run ();
  Simulator::Destroy ();
  delete m_listener;

  // expect only 1 packet successfully sent
  NS_TEST_ASSERT_MSG_EQ (m_numSentPackets, 1, "The number of sent packets is not correct!");
}

void
TestSinglePacketTxTimings::DoSetup (void)
{
  // m_listener = new TestPhyListener;
  // m_phy.m_state->RegisterListener (m_listener);
}

void
TestSinglePacketTxTimings::DoRun (void)
{
  m_payloadSize = 1500;

  RunOne ();
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
  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new TestSinglePacketTxTimings, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static WifiHeTestSuite wifiHeTestSuite;


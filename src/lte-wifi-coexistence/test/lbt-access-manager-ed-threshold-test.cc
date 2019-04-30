/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

#include "ns3/test.h"
#include "ns3/packet.h"
#include "ns3/tag.h"
#include "ns3/log.h"
#include "ns3/packet-burst.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "ns3/wifi-spectrum-value-helper.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/spectrum-wifi-phy.h"
#include "ns3/interference-helper.h"
#include "ns3/nist-error-rate-model.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/wifi-mac-trailer.h"
#include "ns3/wifi-phy-tag.h"
#include "ns3/wifi-phy-standard.h"
#include "ns3/wifi-phy-listener.h"
#include "ns3/wifi-spectrum-signal-parameters.h"
#include "ns3/lbt-access-manager.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LbtAccessManagerEdThresholdTest");

static const uint16_t CHANNEL_NUMBER = 36;
static const uint32_t FREQUENCY = 5180; // MHz
static const uint16_t CHANNEL_WIDTH = 20; // MHz
static const uint16_t GUARD_WIDTH = CHANNEL_WIDTH; // MHz (expanded to channel width to model spectrum mask)

class LbtAccessManagerEdThresholdTest : public TestCase
{
public:
  LbtAccessManagerEdThresholdTest ();
  LbtAccessManagerEdThresholdTest (std::string name);
  virtual ~LbtAccessManagerEdThresholdTest ();
protected:
  virtual void DoSetup (void);
  Ptr<SpectrumWifiPhy> m_phy;
  Ptr<LbtAccessManager> m_lbt;
  Ptr<SpectrumSignalParameters> MakeSignal (double txPowerWatts);
  void SendSignal (double txPowerWatts);
  void SpectrumWifiPhyRxSuccess (Ptr<Packet> p, double snr, double rxPower, WifiTxVector txVector, std::vector<bool> statusPerMpdu);
  void SpectrumWifiPhyRxFailure (Ptr<Packet> p, double snr);
private:
  virtual void DoRun (void);
  void ReceiveAccessGranted (Time duration);
  void RequestAccess (void);
  void SetEdThreshold (double edThreshold);
  void CheckState (LbtAccessManager::LbtState state);
};

LbtAccessManagerEdThresholdTest::LbtAccessManagerEdThresholdTest ()
  : TestCase ("LbtAccessManager testing of ED threshold")
{
}

LbtAccessManagerEdThresholdTest::LbtAccessManagerEdThresholdTest (std::string name)
  : TestCase (name)
{
}

// Make a Wi-Fi signal to inject directly to the StartRx() method
Ptr<SpectrumSignalParameters>
LbtAccessManagerEdThresholdTest::MakeSignal (double txPowerWatts)
{
  WifiTxVector txVector = WifiTxVector (WifiPhy::GetOfdmRate6Mbps (), 0, WIFI_PREAMBLE_LONG, false, 1, 1, 0, 20, false, false);
  MpduType mpdutype = NORMAL_MPDU;

  Ptr<Packet> pkt = Create<Packet> (1000);
  WifiMacHeader hdr;
  WifiMacTrailer trailer;

  hdr.SetType (WIFI_MAC_QOSDATA);
  hdr.SetQosTid (0);
  uint32_t size = pkt->GetSize () + hdr.GetSize () + trailer.GetSerializedSize ();
  Time txDuration = m_phy->CalculateTxDuration (size, txVector, m_phy->GetFrequency(), mpdutype, 0);
  NS_LOG_DEBUG ("Signal duration " << txDuration.GetSeconds ());
  hdr.SetDuration (txDuration);

  pkt->AddHeader (hdr);
  pkt->AddTrailer (trailer);
  WifiPhyTag tag (txVector.GetPreambleType (), txVector.GetMode ().GetModulationClass (), 1);
  pkt->AddPacketTag (tag);
  Ptr<SpectrumValue> txPowerSpectrum = WifiSpectrumValueHelper::CreateOfdmTxPowerSpectralDensity (FREQUENCY, CHANNEL_WIDTH, txPowerWatts, GUARD_WIDTH);
  Ptr<WifiSpectrumSignalParameters> txParams = Create<WifiSpectrumSignalParameters> ();
  txParams->psd = txPowerSpectrum;
  txParams->txPhy = 0;
  txParams->duration = txDuration;
  txParams->packet = pkt;
  return txParams;
}

// Make a Wi-Fi signal to inject directly to the StartRx() method
void
LbtAccessManagerEdThresholdTest::SendSignal (double txPowerWatts)
{
  m_phy->StartRx (MakeSignal (txPowerWatts));
}

void
LbtAccessManagerEdThresholdTest::SpectrumWifiPhyRxSuccess (Ptr<Packet> p, double snr, double rxPower, WifiTxVector txVector, std::vector<bool> statusPerMpdu)
{
  NS_FATAL_ERROR ("Should be unreachable; Wi-Fi rx reception disabled");
}

void
LbtAccessManagerEdThresholdTest::SpectrumWifiPhyRxFailure (Ptr<Packet> p, double snr)
{
  NS_FATAL_ERROR ("Should be unreachable; Wi-Fi rx reception disabled");
}

void
LbtAccessManagerEdThresholdTest::SetEdThreshold (double edThreshold)
{
  NS_LOG_FUNCTION (this << edThreshold);
  bool ok = m_lbt->SetAttributeFailSafe ("EnergyDetectionThreshold", DoubleValue (edThreshold));
  NS_TEST_ASSERT_MSG_EQ (ok, true, "Could not set attribute");
  // change the underlying SpectrumWifiPhy attribute via the LbtAccessManager
  m_lbt->SetWifiPhy (m_phy);
}

LbtAccessManagerEdThresholdTest::~LbtAccessManagerEdThresholdTest ()
{
}

void
LbtAccessManagerEdThresholdTest::RequestAccess ()
{
  m_lbt->RequestAccess ();
}

void
LbtAccessManagerEdThresholdTest::ReceiveAccessGranted (Time duration)
{
  NS_FATAL_ERROR ("Should be unreachable; LTE not yet part of this test");
}

// Create necessary objects, and inject signals.  Test that the expected
// number of packet receptions occur.
void
LbtAccessManagerEdThresholdTest::DoSetup (void)
{
  m_phy = CreateObject<SpectrumWifiPhy> ();
  m_phy->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  Ptr<ErrorRateModel> error = CreateObject<NistErrorRateModel> ();
  m_phy->SetErrorRateModel (error);
  m_phy->SetChannelNumber (CHANNEL_NUMBER);
  m_phy->SetFrequency (FREQUENCY);
  m_phy->SetReceiveOkCallback (MakeCallback (&LbtAccessManagerEdThresholdTest::SpectrumWifiPhyRxSuccess, this));
  m_phy->SetReceiveErrorCallback (MakeCallback (&LbtAccessManagerEdThresholdTest::SpectrumWifiPhyRxFailure, this));
  m_phy->SetAttribute ("DisableWifiReception", BooleanValue (true));
  m_phy->SetAttribute ("RxGain", DoubleValue (0.0));

  m_lbt = CreateObject<LbtAccessManager> ();
  m_lbt->SetWifiPhy (m_phy);
  m_lbt->SetAccessGrantedCallback (MakeCallback (&LbtAccessManagerEdThresholdTest::ReceiveAccessGranted, this));
}

void
LbtAccessManagerEdThresholdTest::CheckState (LbtAccessManager::LbtState state)
{
  NS_LOG_DEBUG ("actual: " << m_lbt->GetLbtState () << ", limit: " << state);
  NS_TEST_ASSERT_MSG_EQ (state, m_lbt->GetLbtState (), "Failed at time " << Simulator::Now());
}

void
LbtAccessManagerEdThresholdTest::DoRun (void)
{
  double txPowerWatts = 1e-9;  // -60.0282 dBm
  // Signal duration is 1400 microseconds
  // Check that state starts in IDLE
  Simulator::Schedule (Seconds (0.5), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::IDLE);

  // Send a packet above threshold and see if it causes Lbt state to go BUSY
  // just for the duration of the packet, then return to IDLE
  m_phy->SetAttribute ("CcaEdThreshold", DoubleValue (-62.0));
  Simulator::Schedule (Seconds (1), &LbtAccessManagerEdThresholdTest::SendSignal, this, txPowerWatts);
  Simulator::Schedule (MicroSeconds (1001399), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::BUSY);
  Simulator::Schedule (MicroSeconds (1001401), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::IDLE);

  // Move below ED threshold of -62 dBm.  LbtAccessManager shouldn't hear it
  txPowerWatts = 5e-10;  // -63.0385 dBm
  Simulator::Schedule (Seconds (2), &LbtAccessManagerEdThresholdTest::SendSignal, this, txPowerWatts);
  Simulator::Schedule (MicroSeconds (2001000), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::IDLE);

  // Hit almost exactly the ED threshold with one packet
  txPowerWatts = 6.36e-10;  // -61.9936 dBm
  Simulator::Schedule (Seconds (3), &LbtAccessManagerEdThresholdTest::SendSignal, this, txPowerWatts);
  Simulator::Schedule (MicroSeconds (3001000), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::BUSY);
  // Double check state at the same time
  Simulator::Schedule (MicroSeconds (3001000), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::BUSY);

  // Hit exactly the ED threshold with two packets sent 1 us apart
  txPowerWatts = 5e-10;  // -63.0385 dBm
  Simulator::Schedule (MicroSeconds (4000000), &LbtAccessManagerEdThresholdTest::SendSignal, this, txPowerWatts);
  Simulator::Schedule (MicroSeconds (4000001), &LbtAccessManagerEdThresholdTest::SendSignal, this, txPowerWatts);
  Simulator::Schedule (MicroSeconds (4001000), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::BUSY);

  // Two packets sent 700 us apart.  Before 700 us into the first packet, the
  // state should be IDLE.  After second packet arrives, the state should be
  // busy for 596 us more, then should return to IDLE.
  Simulator::Schedule (MicroSeconds (5000000), &LbtAccessManagerEdThresholdTest::SendSignal, this, txPowerWatts);
  Simulator::Schedule (MicroSeconds (5000700), &LbtAccessManagerEdThresholdTest::SendSignal, this, txPowerWatts);
  // Check around the boundaries
  Simulator::Schedule (MicroSeconds (5000699), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::IDLE);
  Simulator::Schedule (MicroSeconds (5000701), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::BUSY);
  Simulator::Schedule (MicroSeconds (5001399), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::BUSY);
  Simulator::Schedule (MicroSeconds (5001401), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::IDLE);

  // Change the LBT ED threshold attribute to -72 dBm.
  Simulator::Schedule (MicroSeconds (6000000), &LbtAccessManagerEdThresholdTest::SetEdThreshold, this, -72.0);
  // Check that signal above -72 dBm but below -62 dBm triggers a BUSY
  txPowerWatts = 1.58e-10;  // -68.0416 dBm
  Simulator::Schedule (MicroSeconds (6001000), &LbtAccessManagerEdThresholdTest::SendSignal, this, txPowerWatts);
  Simulator::Schedule (MicroSeconds (6001050), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::BUSY);
  txPowerWatts = 5e-11;  // -73.0385 dBm
  Simulator::Schedule (MicroSeconds (7001000), &LbtAccessManagerEdThresholdTest::SendSignal, this, txPowerWatts);
  Simulator::Schedule (MicroSeconds (7001050), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::IDLE);

  Simulator::Stop (Seconds (10));
  Simulator::Run ();
  Simulator::Destroy ();
}

class TestPhyListener : public ns3::WifiPhyListener
{
public:
  /**
   * Create a test PhyListener
   *
   */
  TestPhyListener (void) :
    m_notifyRxStart (0),
    m_notifyRxEndOk (0),
    m_notifyRxEndError (0),
    m_notifyMaybeCcaBusyStart (0)
  {
  }
  virtual ~TestPhyListener ()
  {
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
private:
};

class LbtAccessManagerEdThresholdTestSuite : public TestSuite
{
public:
  LbtAccessManagerEdThresholdTestSuite ();
};

LbtAccessManagerEdThresholdTestSuite::LbtAccessManagerEdThresholdTestSuite ()
  : TestSuite ("lbt-access-manager-ed-threshold", UNIT)
{
  AddTestCase (new LbtAccessManagerEdThresholdTest, TestCase::QUICK);
}

static LbtAccessManagerEdThresholdTestSuite lbtAccessManagerTestSuite;


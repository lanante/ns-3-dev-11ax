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

// Logs are enabled when running a debug build through 'test-runner'
NS_LOG_COMPONENT_DEFINE ("LbtAccessManagerTest");

static const uint16_t CHANNEL_NUMBER = 36;
static const uint32_t FREQUENCY = 5180; // MHz
static const uint16_t CHANNEL_WIDTH = 20; // MHz
static const uint16_t GUARD_WIDTH = CHANNEL_WIDTH; // MHz (expanded to channel width to model spectrum mask)

/**
 * Base test class for test cases
 */
class LbtAccessManagerBaseTestCase : public TestCase
{
public:
  LbtAccessManagerBaseTestCase ();
  LbtAccessManagerBaseTestCase (std::string name);
  virtual ~LbtAccessManagerBaseTestCase ();

  void SendSignal (double txPowerWatts);
  void ReceiveAccessGranted (Time duration);
  void RequestAccess (void);
protected:
  virtual void DoSetup (void);
  Ptr<SpectrumWifiPhy> m_phy;
  Ptr<LbtAccessManager> m_lbt;
  Ptr<SpectrumSignalParameters> MakeSignal (double txPowerWatts);
  void SpectrumWifiPhyRxSuccess (Ptr<Packet> p, double snr, double rxPower, WifiTxVector txVector, std::vector<bool> statusPerMpdu);
  void SpectrumWifiPhyRxFailure (Ptr<Packet> p, double snr);
  uint32_t m_count;
  std::vector<Time> m_accessGrantedTimes;
  Time m_durationTotal;
  virtual void DoRun (void);
private:
};

LbtAccessManagerBaseTestCase::LbtAccessManagerBaseTestCase ()
  : TestCase ("LbtAccessManager test case receives one packet"),
    m_count (0),
    m_durationTotal (Seconds (0))
{
}

LbtAccessManagerBaseTestCase::LbtAccessManagerBaseTestCase (std::string name)
  : TestCase (name),
    m_count (0),
    m_durationTotal (Seconds (0))
{
}

// Make a Wi-Fi signal to inject directly to the StartRx() method
Ptr<SpectrumSignalParameters>
LbtAccessManagerBaseTestCase::MakeSignal (double txPowerWatts)
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

  LSigHeader sig;
  pkt->AddHeader (sig);

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
LbtAccessManagerBaseTestCase::SendSignal (double txPowerWatts)
{
  m_phy->StartRx (MakeSignal (txPowerWatts));
}

void
LbtAccessManagerBaseTestCase::SpectrumWifiPhyRxSuccess (Ptr<Packet> p, double snr, double rxPower, WifiTxVector txVector, std::vector<bool> statusPerMpdu)
{
  NS_LOG_FUNCTION (this << p << snr << rxPower << txVector);
  m_count++;
}

void
LbtAccessManagerBaseTestCase::SpectrumWifiPhyRxFailure (Ptr<Packet> p, double snr)
{
  NS_LOG_FUNCTION (this << p << snr);
  m_count++;
}

LbtAccessManagerBaseTestCase::~LbtAccessManagerBaseTestCase ()
{
}

void
LbtAccessManagerBaseTestCase::RequestAccess ()
{
  m_lbt->RequestAccess ();
}

void
LbtAccessManagerBaseTestCase::ReceiveAccessGranted (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  m_accessGrantedTimes.push_back (Simulator::Now ());
  m_durationTotal += duration;
}

// Create necessary objects, and inject signals.  Test that the expected
// number of packet receptions occur.
void
LbtAccessManagerBaseTestCase::DoSetup (void)
{
  m_phy = CreateObject<SpectrumWifiPhy> ();
  m_phy->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  Ptr<ErrorRateModel> error = CreateObject<NistErrorRateModel> ();
  m_phy->SetErrorRateModel (error);
  m_phy->SetChannelNumber (CHANNEL_NUMBER);
  m_phy->SetFrequency (FREQUENCY);
  m_phy->SetReceiveOkCallback (MakeCallback (&LbtAccessManagerBaseTestCase::SpectrumWifiPhyRxSuccess, this));
  m_phy->SetReceiveErrorCallback (MakeCallback (&LbtAccessManagerBaseTestCase::SpectrumWifiPhyRxFailure, this));

  m_lbt = CreateObject<LbtAccessManager> ();
  m_lbt->SetWifiPhy (m_phy);
  uint32_t numAssigned = m_lbt->AssignStreams (1);
  NS_TEST_ASSERT_MSG_EQ (1, numAssigned, "Number of assigned streams");

  // For this test, we enable Wi-Fi reception that the above call to
  // SetWifiPhy has disabled
  m_phy->SetAttribute ("DisableWifiReception", BooleanValue (false));
  m_lbt->SetAccessGrantedCallback (MakeCallback (&LbtAccessManagerBaseTestCase::ReceiveAccessGranted, this));
}
void
LbtAccessManagerBaseTestCase::DoRun (void)
{
  NS_FATAL_ERROR ("Calling base class DoRun()");
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

// Define tests below
//
class LbtReceiveOneRequestOne : public LbtAccessManagerBaseTestCase
{
public:
  LbtReceiveOneRequestOne ();
  virtual ~LbtReceiveOneRequestOne ();
protected:
  virtual void DoRun (void);
};
  
LbtReceiveOneRequestOne::LbtReceiveOneRequestOne ()
  : LbtAccessManagerBaseTestCase ("LbtAccessManager receive one request one")
{
}

LbtReceiveOneRequestOne::~LbtReceiveOneRequestOne ()
{
}

// Test that the expected number of packet receptions occur.
void
LbtReceiveOneRequestOne::DoRun (void)
{
  double txPowerWatts = 0.010;
  // Send one packet
  Simulator::Schedule (Seconds (1), &LbtAccessManagerBaseTestCase::SendSignal, this, txPowerWatts); 
  // Request access once; check if access was ever granted
  Simulator::Schedule (MicroSeconds (6000001), &LbtAccessManagerBaseTestCase::RequestAccess, this);
  Simulator::Stop (Seconds (10));
  Simulator::Run ();
  Simulator::Destroy ();

  NS_TEST_ASSERT_MSG_EQ (1, m_count, "Didn't receive right number of packets");
  NS_TEST_ASSERT_MSG_EQ (1, m_accessGrantedTimes.size (), "Didn't receive right number of access grants");
}

// Test that access is granted immediately when have been idle longer than 
// defer time
class LbtTransmitImmediatelyAfterRequest : public LbtAccessManagerBaseTestCase
{
public:
  LbtTransmitImmediatelyAfterRequest ();
  virtual ~LbtTransmitImmediatelyAfterRequest () { }
protected:
  virtual void DoRun (void);
private:
};
  
LbtTransmitImmediatelyAfterRequest::LbtTransmitImmediatelyAfterRequest ()
  : LbtAccessManagerBaseTestCase ("LbtAccessManager test transmit immediately after request if previously idle")
{
}

// Test that the access is granted 
void
LbtTransmitImmediatelyAfterRequest::DoRun (void)
{
  // Request access once; check time of access granted
  Simulator::Schedule (MicroSeconds (6000001), &LbtAccessManagerBaseTestCase::RequestAccess, this);

  // Send signal that has ended more than m_deferTime prior to next request
  double txPowerWatts = 0.010;
  // Send one packet
  Simulator::Schedule (MicroSeconds (7000000),  &LbtAccessManagerBaseTestCase::SendSignal, this, txPowerWatts); 

  //Signal lasts for 1400 microseconds, then we wait for 43 microsecond defer
  //Should be able to send immediately at time 7001443 but not before
  Simulator::Schedule (MicroSeconds (7001443), &LbtAccessManagerBaseTestCase::RequestAccess, this);

  Simulator::Stop (Seconds (10));
  Simulator::Run ();
  Simulator::Destroy ();

  NS_TEST_ASSERT_MSG_EQ (m_accessGrantedTimes[0].GetMicroSeconds (), 6000001, "Access provided too early or late");
  NS_TEST_ASSERT_MSG_EQ (m_accessGrantedTimes[1].GetMicroSeconds (), 7001443, "Access provided too early or late");
}

// Test that defer is handled correctly, even when interrupted
class LbtDeferAndBackoff : public LbtAccessManagerBaseTestCase
{
public:
  LbtDeferAndBackoff ();
  virtual ~LbtDeferAndBackoff () {}
protected:
  virtual void DoRun (void);
private:
  void GetBackoff (void);
  std::vector<uint32_t> m_backoff;
};
  
LbtDeferAndBackoff::LbtDeferAndBackoff ()
  : LbtAccessManagerBaseTestCase ("LbtAccessManager defer and backoff after request")
{
}

void
LbtDeferAndBackoff::GetBackoff ()
{
  m_backoff.push_back (m_lbt->GetCurrentBackoffCount ());
  NS_LOG_DEBUG ("Backoff count is " << m_lbt->GetCurrentBackoffCount ());
}

// Test that the access is granted 
void
LbtDeferAndBackoff::DoRun (void)
{
  // Send signal at time 6
  double txPowerWatts = 0.010;
  // Send one packet
  Simulator::Schedule (MicroSeconds (6000000),  &LbtAccessManagerBaseTestCase::SendSignal, this, txPowerWatts); 
  // Request access once at time 6000010 (10 us later)
  // The LBT should be busy until 6001292, then defer for 43, then backoff
  // Backoff in this case is 11 slots
  // So check that access granted is at 6000000 + 1400 + 43 + 11*9 = 6001542
  Simulator::Schedule (MicroSeconds (6000010), &LbtAccessManagerBaseTestCase::RequestAccess, this);
  // LbtAccessManager will get backoff count immediately after request
  Simulator::Schedule (MicroSeconds (6000011), &LbtDeferAndBackoff::GetBackoff, this);


  // Test that signal coming in while in DEFER will cause defer to occur again
  // Send signal at time 7
  Simulator::Schedule (MicroSeconds (7000000),  &LbtAccessManagerBaseTestCase::SendSignal, this, txPowerWatts); 
  // Request access once at time 7000010 (10 us later)
  Simulator::Schedule (MicroSeconds (7000010), &LbtAccessManagerBaseTestCase::RequestAccess, this);
  Simulator::Schedule (MicroSeconds (7000011), &LbtDeferAndBackoff::GetBackoff, this);
  // The LBT should be busy until 7001400, then defer for 43, then backoff
  // But interrupt the defer at 7001440 with another signal, which holds it busy
  // again until 7002982
  Simulator::Schedule (MicroSeconds (7001440),  &LbtAccessManagerBaseTestCase::SendSignal, this, txPowerWatts); 
  // Then defer (43) and backoff 

  Simulator::Stop (Seconds (10));
  Simulator::Run ();
  Simulator::Destroy ();

  NS_TEST_ASSERT_MSG_EQ (m_accessGrantedTimes.size (), 2, "missing access granted?");
  int64_t expectedExpiration0 = 6000000 + 1400 + 43 + 9 * m_backoff[0];
  int64_t expectedExpiration1 = 7001440 + 1400 + 43 + 9 * m_backoff[1];

  NS_TEST_ASSERT_MSG_EQ  (m_accessGrantedTimes[0].GetMicroSeconds (), expectedExpiration0, "Access provided too early or late");
  NS_TEST_ASSERT_MSG_EQ (m_accessGrantedTimes[1].GetMicroSeconds (), expectedExpiration1, "Access provided too early or late");
}

// Test that backoff is suspended correctly
class LbtSuspendBackoff : public LbtAccessManagerBaseTestCase
{
public:
  LbtSuspendBackoff ();
  virtual ~LbtSuspendBackoff () {}
protected:
  virtual void DoRun (void);
private:
  void GetBackoff (void);
  std::vector<uint32_t> m_backoff;
};
  
LbtSuspendBackoff::LbtSuspendBackoff ()
  : LbtAccessManagerBaseTestCase ("LbtAccessManager suspend backoff")
{
}

void
LbtSuspendBackoff::GetBackoff ()
{
  m_backoff.push_back (m_lbt->GetCurrentBackoffCount ());
  NS_LOG_DEBUG ("Backoff count is " << m_lbt->GetCurrentBackoffCount ());
}

void
LbtSuspendBackoff::DoRun (void)
{
  // Send signal at time 8
  double txPowerWatts = 0.010;
  // Send one packet
  Simulator::Schedule (MicroSeconds (8000000),  &LbtAccessManagerBaseTestCase::SendSignal, this, txPowerWatts); 
  // Request access once at time 8000010 (10 us later)
  // The LBT should be busy until 8001400, then defer for 43, then backoff
  // starts at 6001443
  // Backoff in this case is 11 slots
  Simulator::Schedule (MicroSeconds (8000010), &LbtAccessManagerBaseTestCase::RequestAccess, this);
  // LbtAccessManager will get backoff count immediately after request
  Simulator::Schedule (MicroSeconds (8000011), &LbtSuspendBackoff::GetBackoff, this);
  // Send during backoff; one slot has elapsed and we are into the second slot
  Simulator::Schedule (MicroSeconds (8001455),  &LbtAccessManagerBaseTestCase::SendSignal, this, txPowerWatts); 
  // Store the value of backoff counter (should be decremented by two)
  Simulator::Schedule (MicroSeconds (8001456), &LbtSuspendBackoff::GetBackoff, this);


  Simulator::Stop (Seconds (10));
  Simulator::Run ();
  Simulator::Destroy ();

  NS_TEST_ASSERT_MSG_EQ (m_accessGrantedTimes.size (), 1, "missing access granted?");
  NS_TEST_ASSERT_MSG_EQ (m_backoff[0] - m_backoff[1], 2,  "Backoff didn't decreent by two");
  int64_t expectedExpiration0 = 8000000 + 1455 + 1400 + 43 + 9 * m_backoff[1];

  NS_TEST_ASSERT_MSG_EQ  (m_accessGrantedTimes[0].GetMicroSeconds (), expectedExpiration0, "Access provided too early or late");
}

class LbtAccessManagerTestSuite : public TestSuite
{
public:
  LbtAccessManagerTestSuite ();
};

LbtAccessManagerTestSuite::LbtAccessManagerTestSuite ()
  : TestSuite ("lbt-access-manager", UNIT)
{
  AddTestCase (new LbtReceiveOneRequestOne, TestCase::QUICK);
  AddTestCase (new LbtTransmitImmediatelyAfterRequest, TestCase::QUICK);
  AddTestCase (new LbtDeferAndBackoff, TestCase::QUICK);
  AddTestCase (new LbtSuspendBackoff, TestCase::QUICK);
}

static LbtAccessManagerTestSuite lbtAccessManagerTestSuite;


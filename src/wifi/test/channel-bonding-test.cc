/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 University of Washington
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
 * Author: Sébastien Deronne <sebastien.deronne@gmail.com>
 */

#include <algorithm>
#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/wifi-spectrum-value-helper.h"
#include "ns3/spectrum-wifi-phy.h"
#include "ns3/nist-error-rate-model.h"
#include "ns3/wifi-psdu.h"
#include "ns3/wifi-phy-header.h"
#include "ns3/wifi-utils.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/constant-threshold-channel-bonding-manager.h"
#include "ns3/dynamic-threshold-channel-bonding-manager.h"
#include "ns3/waveform-generator.h"
#include "ns3/non-communicating-net-device.h"
#include "ns3/mobility-helper.h"
#include "ns3/wifi-net-device.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiChannelBondingTest");

class BondingTestSpectrumWifiPhy : public SpectrumWifiPhy
{
public:
  using SpectrumWifiPhy::SpectrumWifiPhy;
  using SpectrumWifiPhy::GetBand;
};

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief SNR tests for static channel bonding
 *
 * In this test, we have four 802.11ac transmitters and four 802.11ac receivers.
 * A BSS is composed of one transmitter and one receiver.
 *
 * The first BSS occupies channel 36 and a channel width of 20 MHz.
 * The second BSS operates on channel 40 with a channel width of 20 MHz.
 * Both BSS 3 and BSS 4 makes uses of channel bonding with a 40 MHz channel width
 * and operates on channel 38 (= 36 + 40). The only difference between them is that
 * BSS 3 has channel 36 as primary channel, whereas BSS 4 has channel 40 has primary channel.
 *
 */
class TestStaticChannelBondingSnr : public TestCase
{
public:
  TestStaticChannelBondingSnr ();
  virtual ~TestStaticChannelBondingSnr ();

private:
  virtual void DoSetup (void);
  virtual void DoRun (void);

  /**
   * Create BSS
   * \param channel the Spectrum channel
   * \param channelWidth the channel width
   * \param channelNumber the operating channel number
   * \param frequency the operating frequency
   * \param primaryChannelNumber the channel number of the primary 20 MHz
   */
  void CreateBss (const Ptr<MultiModelSpectrumChannel> channel,
                  uint16_t channelWidth, uint8_t channelNumber,
                  uint16_t frequency, uint8_t primaryChannelNumber);

  /**
   * Check the PHY state
   * \param expectedState the expected PHY state
   * \param bss the BSS number
   */
  void CheckPhyState (WifiPhyState expectedState, uint8_t bss);
  /**
   * Check the secondary channel status
   * \param expectedIdle flag whether the secondary channel is expected to be deemed IDLE
   * \param bss the BSS number
   * \param channelWidth the channel width to check
   */
  void CheckSecondaryChannelStatus (bool expectedIdle, uint8_t bss, uint16_t channelWidth);

  /**
   * Send packet function
   * \param bss the BSS of the transmitter belongs to
   */
  void SendPacket (uint8_t bss);

  /**
   * Callback triggered when a packet is starting to be processed for reception
   * \param context the context
   * \param p the received packet
   * \param rxPowersW the received power per channel band in watts
   */
  void RxCallback (std::string context, Ptr<const Packet> p, RxPowerWattPerChannelBand rxPowersW);

  /**
   * Callback triggered when a packet has been successfully received
   * \param context the context
   * \param p the received packet
   * \param snr the signal to noise ratio
   * \param mode the mode used for the transmission
   * \param preamble the preamble used for the transmission
   */
  void RxOkCallback (std::string context, Ptr<const Packet> p, double snr, WifiMode mode, WifiPreamble preamble);

  /**
   * Callback triggered when a packet has been unsuccessfully received
   * \param context the context
   * \param p the packet
   * \param snr the signal to noise ratio
   */
  void RxErrorCallback (std::string context, Ptr<const Packet> p, double snr);

  /**
   * Set expected SNR ((in dB) for a given BSS before the test case is run
   * \param snr the expected signal to noise ratio in dB
   * \param bss the BSS number
   */
  void SetExpectedSnrForBss (double snr, uint8_t bss);
  /**
   * Verify packet reception once the test case is run
   * \param expectedReception whether the reception should have occured
   * \param bss the BSS number
   */
  void VerifyResultsForBss (bool expectedReception, bool expectedPhyPayloadSuccess, uint8_t bss);

  /**
   * Reset the results
   */
  void Reset (void);

  std::vector<Ptr<BondingTestSpectrumWifiPhy> > m_rxPhys; ///< RX PHYs
  std::vector<Ptr<BondingTestSpectrumWifiPhy> > m_txPhys; ///< TX PHYs

  std::map<uint8_t /* bss */, double /* snr */> m_expectedSnrPerBss; ///< Expected SNR per BSS

  std::vector<bool> m_reception; ///< Flags whether a reception occured for a given BSS
  std::vector<bool> m_phyPayloadReceivedSuccess; ///< Flags whether the PHY payload has been successfully received for a given BSS
};

TestStaticChannelBondingSnr::TestStaticChannelBondingSnr ()
  : TestCase ("SNR tests for static channel bonding")
{
  LogLevel logLevel = (LogLevel)(LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_ALL);
  LogComponentEnable ("WifiChannelBondingTest", logLevel);
  //LogComponentEnable ("WifiSpectrumValueHelper", logLevel);
  //LogComponentEnable ("WifiPhy", logLevel);
  //LogComponentEnable ("SpectrumWifiPhy", logLevel);
  //LogComponentEnable ("InterferenceHelper", logLevel);
  //LogComponentEnable ("MultiModelSpectrumChannel", logLevel);
}

void
TestStaticChannelBondingSnr::Reset (void)
{
  m_expectedSnrPerBss.clear ();
  std::fill(m_reception.begin(), m_reception.end(), false);
  std::fill(m_phyPayloadReceivedSuccess.begin(), m_phyPayloadReceivedSuccess.end(), false);
}

void
TestStaticChannelBondingSnr::SetExpectedSnrForBss (double snr, uint8_t bss)
{
  auto it = m_expectedSnrPerBss.find (bss);
  if (it != m_expectedSnrPerBss.end ())
    {
      it->second = snr;
    }
  else
    {
      m_expectedSnrPerBss.insert ({bss, snr});
    }
}

void
TestStaticChannelBondingSnr::VerifyResultsForBss (bool expectedReception, bool expectedPhyPayloadSuccess, uint8_t bss)
{
  if (bss == 1)
    {
      NS_TEST_ASSERT_MSG_EQ (m_reception.at (0), expectedReception, "m_receptionBss1 is not equal to expectedReception");
      NS_TEST_ASSERT_MSG_EQ (m_phyPayloadReceivedSuccess.at (0), expectedPhyPayloadSuccess, "m_phyPayloadReceivedSuccess is not equal to expectedPhyPayloadSuccess for BSS 1");
    }
  else if (bss == 2)
    {
      NS_TEST_ASSERT_MSG_EQ (m_reception.at (1), expectedReception, "m_receptionBss2 is not equal to expectedReception");
      NS_TEST_ASSERT_MSG_EQ (m_phyPayloadReceivedSuccess.at (1), expectedPhyPayloadSuccess, "m_phyPayloadReceivedSuccess is not equal to expectedPhyPayloadSuccess for BSS 2");
    }
  else if (bss == 3)
    {
      NS_TEST_ASSERT_MSG_EQ (m_reception.at (2), expectedReception, "m_receptionBss3 is not equal to expectedReception");
      NS_TEST_ASSERT_MSG_EQ (m_phyPayloadReceivedSuccess.at (2), expectedPhyPayloadSuccess, "m_phyPayloadReceivedSuccess is not equal to expectedPhyPayloadSuccess for BSS 3");
    }
  else if (bss == 4)
    {
      NS_TEST_ASSERT_MSG_EQ (m_reception.at (3), expectedReception, "m_receptionBss4 is not equal to expectedReception");
      NS_TEST_ASSERT_MSG_EQ (m_phyPayloadReceivedSuccess.at (3), expectedPhyPayloadSuccess, "m_phyPayloadReceivedSuccess is not equal to expectedPhyPayloadSuccess for BSS 4");
    }
}

void
TestStaticChannelBondingSnr::CheckPhyState (WifiPhyState expectedState, uint8_t bss)
{
  Ptr<BondingTestSpectrumWifiPhy> phy = m_rxPhys.at (bss - 1);
  WifiPhyState currentState = phy->GetPhyState ();
  NS_TEST_ASSERT_MSG_EQ (currentState, expectedState, "PHY State " << currentState << " does not match expected state " << expectedState << " at " << Simulator::Now ());
}

void
TestStaticChannelBondingSnr::CheckSecondaryChannelStatus (bool expectedIdle, uint8_t bss, uint16_t channelWidth)
{
  Ptr<BondingTestSpectrumWifiPhy> phy = m_rxPhys.at (bss - 1);
  NS_ASSERT (phy->GetChannelWidth () >= 40);
  bool currentlyIdle = phy->IsStateIdle (channelWidth, WToDbm (phy->GetDefaultCcaEdThresholdSecondary ()));
  NS_TEST_ASSERT_MSG_EQ (currentlyIdle, expectedIdle, "Secondary channel status " << currentlyIdle << " does not match expected status " << expectedIdle << " at " << Simulator::Now ());
}

void
TestStaticChannelBondingSnr::SendPacket (uint8_t bss)
{
  Ptr<BondingTestSpectrumWifiPhy> phy = m_txPhys.at (bss - 1);
  uint16_t channelWidth = 20;
  uint32_t payloadSize = 1000;
  if (bss == 1)
    {
      channelWidth = 20;
      payloadSize = 1001;
    }
  else if (bss == 2)
    {
      channelWidth = 20;
      payloadSize = 1002;
    }
  else if (bss == 3)
    {
      channelWidth = 40;
      payloadSize = 2100; //This is chosen such that the transmission time on 40 MHz will be the same as for packets sent on 20 MHz
    }
  else if (bss == 4)
    {
      channelWidth = 40;
      payloadSize = 2101; //This is chosen such that the transmission time on 40 MHz will be the same as for packets sent on 20 MHz
    }
  else if (bss == 5)
    {
      channelWidth = 20;
      payloadSize = 1005;
    }
  else if (bss == 6)
    {
      channelWidth = 20;
      payloadSize = 1006;
    }
  else if (bss == 7)
    {
      channelWidth = 40;
      payloadSize = 2107; //This is chosen such that the transmission time on 40 MHz will be the same as for packets sent on 20 MHz
    }
  else if (bss == 8)
    {
      channelWidth = 40;
      payloadSize = 2108; //This is chosen such that the transmission time on 40 MHz will be the same as for packets sent on 20 MHz
    }
  else
    {
      channelWidth = 80;
      payloadSize = 4000; //This is chosen such that the transmission time on 80 MHz will be the same as for packets sent on 20 MHz
    }
  
  WifiTxVector txVector = WifiTxVector (WifiPhy::GetVhtMcs7 (), 0, WIFI_PREAMBLE_HT_MF, 800, 1, 1, 0, channelWidth, false, false);

  Ptr<Packet> pkt = Create<Packet> (payloadSize);
  WifiMacHeader hdr;
  hdr.SetType (WIFI_MAC_QOSDATA);

  Ptr<WifiPsdu> psdu = Create<WifiPsdu> (pkt, hdr);
  phy->Send (WifiPsduMap ({std::make_pair (SU_STA_ID, psdu)}), txVector);
}

void
TestStaticChannelBondingSnr::RxCallback (std::string context, Ptr<const Packet> p, RxPowerWattPerChannelBand rxPowersW)
{
  uint32_t size = p->GetSize ();
  NS_LOG_INFO (context << " received packet with size " << size);
  if (context == "BSS1") //RX is in BSS 1
    {
      auto band = m_rxPhys.at (0)->GetBand (20, 0);
      auto it = std::find_if (rxPowersW.begin (), rxPowersW.end(),
          [&band](const std::pair<WifiSpectrumBand, double>& element){ return element.first == band; } );
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 1 received packet with size " << size << " and power in 20 MHz band: " << WToDbm (it->second));
      if (size == 1031) //TX is in BSS 1
        {
          double expectedRxPowerMin = - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 1 RX PHY is too low");
        }
      else if (size == 1032) //TX is in BSS 2
        {
          double expectedRxPowerMax = - 40 /* rejection */ - 50 /* loss */;
          NS_TEST_EXPECT_MSG_LT (WToDbm (it->second), expectedRxPowerMax, "Received power for BSS 1 RX PHY is too high");
        }
      else if (size == 2130) //TX is in BSS 3
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 1 RX PHY is too low");
        }
      else if (size == 2131) //TX is in BSS 4
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 1 RX PHY is too low");
        }
    }
  else if (context == "BSS2") //RX is in BSS 2
    {
      auto band = m_rxPhys.at (1)->GetBand (20, 0);
      auto it = std::find_if (rxPowersW.begin (), rxPowersW.end(),
          [&band](const std::pair<WifiSpectrumBand, double>& element){ return element.first == band; } );
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 2 received packet with size " << size << " and power in 20 MHz band: " << WToDbm (it->second));
      if (size == 1031) //TX is in BSS 1
        {
          double expectedRxPowerMax = - 40 /* rejection */ - 50 /* loss */;
          NS_TEST_EXPECT_MSG_LT (WToDbm (it->second), expectedRxPowerMax, "Received power for BSS 2 RX PHY is too high");
        }
      else if (size == 1032) //TX is in BSS 2
        {
          double expectedRxPowerMin = - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 2 RX PHY is too low");
        }
      else if (size == 2130) //TX is in BSS 3
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 2 RX PHY is too low");
        }
      else if (size == 2131) //TX is in BSS 4
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 2 RX PHY is too low");
        }
    }
  else if (context == "BSS3") //RX is in BSS 3
    {
      auto band = m_rxPhys.at (2)->GetBand (20, 0);
      auto it = std::find_if (rxPowersW.begin (), rxPowersW.end(),
          [&band](const std::pair<WifiSpectrumBand, double>& element){ return element.first == band; } );
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 3 received packet with size " << size << " and power in primary 20 MHz band: " << WToDbm (it->second));
      if (size == 1031) //TX is in BSS 1
        {
          double expectedRxPowerMin = - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power in primary channel for BSS 3 RX PHY is too low");
        }
      else if (size == 1032) //TX is in BSS 2
        {
          double expectedRxPowerMax = - 40 /* rejection */ - 50 /* loss */;
          NS_TEST_EXPECT_MSG_LT (WToDbm (it->second), expectedRxPowerMax, "Received power for BSS 3 RX PHY is too high");
        }
      else if (size == 2130) //TX is in BSS 3
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 3 RX PHY is too low");
        }
      else if (size == 2131) //TX is in BSS 4
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 3 RX PHY is too low");
        }

      band = m_rxPhys.at (2)->GetBand (20, 1);
      it = std::find_if (rxPowersW.begin (), rxPowersW.end(),
          [&band](const std::pair<WifiSpectrumBand, double>& element){ return element.first == band; } );
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 3 received packet with size " << size << " and power in secondary 20 MHz band: " << WToDbm (it->second));
      if (size == 1031) //TX is in BSS 1
        {
          double expectedRxPowerMax = - 40 /* rejection */ - 50 /* loss */;
          NS_TEST_EXPECT_MSG_LT (WToDbm (it->second), expectedRxPowerMax, "Received power for BSS 3 RX PHY is too high");
        }
      else if (size == 1032) //TX is in BSS 2
        {
          double expectedRxPowerMin = - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power in primary channel for BSS 3 RX PHY is too low");
        }
      else if (size == 2130) //TX is in BSS 3
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 3 RX PHY is too low");
        }
      else if (size == 2131) //TX is in BSS 4
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 3 RX PHY is too low");
        }
    }
  else if (context == "BSS4") //RX is in BSS 4
    {
      auto band = m_rxPhys.at (3)->GetBand (20, 1);
      auto it = std::find_if (rxPowersW.begin (), rxPowersW.end(),
          [&band](const std::pair<WifiSpectrumBand, double>& element){ return element.first == band; } );
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 4 received packet with size " << size << " and power in primary 20 MHz band: " << WToDbm (it->second));
      if (size == 1031) //TX is in BSS 1
        {
          double expectedRxPowerMax = - 40 /* rejection */ - 50 /* loss */;
          NS_TEST_EXPECT_MSG_LT (WToDbm (it->second), expectedRxPowerMax, "Received power for BSS 4 RX PHY is too high");
        }
      else if (size == 1032) //TX is in BSS 2
        {
          double expectedRxPowerMin = - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power in primary channel for BSS 4 RX PHY is too low");
        }
      else if (size == 2130) //TX is in BSS 3
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 4 RX PHY is too low");
        }
      else if (size == 2131) //TX is in BSS 4
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 4 RX PHY is too low");
        }

      band = m_rxPhys.at (3)->GetBand (20, 0);
      it = std::find_if (rxPowersW.begin (), rxPowersW.end(),
          [&band](const std::pair<WifiSpectrumBand, double>& element){ return element.first == band; } );
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 4 received packet with size " << size << " and power in secondary 20 MHz band: " << WToDbm (it->second));
      if (size == 1031) //TX is in BSS 1
        {
          double expectedRxPowerMin = - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power in primary channel for BSS 4 RX PHY is too low");
        }
      else if (size == 1032) //TX is in BSS 2
        {
          double expectedRxPowerMax = - 40 /* rejection */ - 50 /* loss */;
          NS_TEST_EXPECT_MSG_LT (WToDbm (it->second), expectedRxPowerMax, "Received power for BSS 4 RX PHY is too high");
        }
      else if (size == 2130) //TX is in BSS 3
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 4 RX PHY is too low");
        }
      else if (size == 2131) //TX is in BSS 4
        {
          double expectedRxPowerMin = - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 4 RX PHY is too low");
        }
    }
}

void
TestStaticChannelBondingSnr::RxOkCallback (std::string context, Ptr<const Packet> p, double snr, WifiMode mode, WifiPreamble preamble)
{
  uint32_t size = p->GetSize ();
  uint8_t txBss = 0;
  if (size == 1031)
    {
      txBss = 1;
    }
  else if (size == 1032)
    {
      txBss = 2;
    }
  else if (size == 2130)
    {
      txBss = 3;
    }
  else if (size == 2131)
    {
      txBss = 4;
    }
  uint8_t rxBss = std::stoi (context.substr (3, 2));
  NS_LOG_INFO ("RxOkCallback: TX BSS=" << +txBss << " RX BSS=" << +rxBss << " SNR=" << RatioToDb (snr));
  if (txBss != rxBss)
    {
      return;
    }
  m_reception.at (rxBss - 1) = true;
  m_phyPayloadReceivedSuccess.at (rxBss - 1) = true;
  auto it = m_expectedSnrPerBss.find (rxBss);
  if (it != m_expectedSnrPerBss.end ())
    {
      NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), it->second, 0.2, "Unexpected SNR value");
    }
}

void
TestStaticChannelBondingSnr::RxErrorCallback (std::string context, Ptr<const Packet> p, double snr)
{
  uint32_t size = p->GetSize ();
  uint8_t txBss = 0;
  if (size == 1031)
    {
      txBss = 1;
    }
  else if (size == 1032)
    {
      txBss = 2;
    }
  else if (size == 2130)
    {
      txBss = 3;
    }
  else if (size == 2131)
    {
      txBss = 4;
    }
  uint8_t rxBss = std::stoi (context.substr (3, 2));
  NS_LOG_INFO ("RxOkCallback: TX BSS=" << +txBss << " RX BSS=" << +rxBss << " SNR=" << RatioToDb (snr));
  if (txBss != rxBss)
    {
      return;
    }
  m_reception.at (rxBss - 1) = true;
  m_phyPayloadReceivedSuccess.at (rxBss - 1) = false;
  auto it = m_expectedSnrPerBss.find (rxBss);
  if (it != m_expectedSnrPerBss.end ())
    {
      NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), it->second, 0.2, "Unexpected SNR value");
    }
}

TestStaticChannelBondingSnr::~TestStaticChannelBondingSnr ()
{
  for (auto & rxPhy : m_rxPhys)
    {
      rxPhy = 0;
    }
  for (auto & txPhy : m_txPhys)
    {
      txPhy = 0;
    }
  m_rxPhys.clear ();
  m_txPhys.clear ();
}

void
TestStaticChannelBondingSnr::CreateBss (const Ptr<MultiModelSpectrumChannel> channel,
                                        uint16_t channelWidth, uint8_t channelNumber,
                                        uint16_t frequency, uint8_t primaryChannelNumber)
{
  uint8_t bssNumber = m_rxPhys.size () + 1;

  Ptr<ErrorRateModel> error = CreateObject<NistErrorRateModel> ();

  Ptr<BondingTestSpectrumWifiPhy> rxPhy = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> rxMobility = CreateObject<ConstantPositionMobilityModel> ();
  rxMobility->SetPosition (Vector (1.0, 1.0 * (bssNumber - 1), 0.0));
  rxPhy->SetMobility (rxMobility);
  rxPhy->ConfigureStandard (WIFI_PHY_STANDARD_80211ac);
  rxPhy->CreateWifiSpectrumPhyInterface (nullptr);
  rxPhy->SetChannel (channel);
  rxPhy->SetErrorRateModel (error);
  rxPhy->SetChannelWidth (channelWidth);
  rxPhy->SetChannelNumber (channelNumber);
  rxPhy->SetPrimaryChannelNumber (primaryChannelNumber);
  rxPhy->SetFrequency (frequency);
  rxPhy->SetTxPowerStart (0.0);
  rxPhy->SetTxPowerEnd (0.0);
  rxPhy->SetRxSensitivity (-91.0);
  rxPhy->SetAttribute ("TxMaskInnerBandMinimumRejection", DoubleValue (-40.0));
  rxPhy->SetAttribute ("TxMaskOuterBandMinimumRejection", DoubleValue (-56.0));
  rxPhy->SetAttribute ("TxMaskOuterBandMaximumRejection", DoubleValue (-80.0));
  rxPhy->Initialize ();

  std::ostringstream bss;
  bss << "BSS" << +bssNumber;

  rxPhy->TraceConnect ("PhyRxBegin", bss.str (), MakeCallback (&TestStaticChannelBondingSnr::RxCallback, this));
  rxPhy->GetState()->TraceConnect ("RxOk", bss.str (), MakeCallback (&TestStaticChannelBondingSnr::RxOkCallback, this));
  rxPhy->GetState()->TraceConnect ("RxError", bss.str (), MakeCallback (&TestStaticChannelBondingSnr::RxErrorCallback, this));
  
  m_rxPhys.push_back (rxPhy);

  Ptr<BondingTestSpectrumWifiPhy> txPhy = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> txMobility = CreateObject<ConstantPositionMobilityModel> ();
  txMobility->SetPosition (Vector (0.0, 1.0 * (bssNumber - 1), 0.0));
  txPhy->SetMobility (txMobility);
  txPhy->ConfigureStandard (WIFI_PHY_STANDARD_80211ac);
  txPhy->CreateWifiSpectrumPhyInterface (nullptr);
  txPhy->SetChannel (channel);
  txPhy->SetErrorRateModel (error);
  txPhy->SetChannelWidth (channelWidth);
  txPhy->SetChannelNumber (channelNumber);
  txPhy->SetPrimaryChannelNumber (primaryChannelNumber);
  txPhy->SetFrequency (frequency);
  txPhy->SetTxPowerStart (0.0);
  txPhy->SetTxPowerEnd (0.0);
  txPhy->SetRxSensitivity (-91.0);
  txPhy->SetAttribute ("TxMaskInnerBandMinimumRejection", DoubleValue (-40.0));
  txPhy->SetAttribute ("TxMaskOuterBandMinimumRejection", DoubleValue (-56.0));
  txPhy->SetAttribute ("TxMaskOuterBandMaximumRejection", DoubleValue (-80.0));
  txPhy->Initialize ();

  m_txPhys.push_back (txPhy);
}

void
TestStaticChannelBondingSnr::DoSetup (void)
{
  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();

  Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel> ();
  lossModel->SetDefaultLoss (50); // set default loss to 50 dB for all links
  channel->AddPropagationLossModel (lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  channel->SetPropagationDelayModel (delayModel);

  //Create BSS #1 operating on channel 36 (20 MHz)
  CreateBss (channel, 20 /* channel width */, 36 /* channel number */, 5180 /* frequency */, 36 /* primary channel number */);

  //Create BSS #2 operating on channel 40 (20 MHz)
  CreateBss (channel, 20 /* channel width */, 40 /* channel number */, 5200 /* frequency */, 40 /* primary channel number */);

  //Create BSS #3 operating on channel 38, with primary channel 36 (40 MHz)
  CreateBss (channel, 40 /* channel width */, 38 /* channel number */, 5190 /* frequency */, 36 /* primary channel number */);

  //Create BSS #4 operating on channel 38, with primary channel 40 (40 MHz)
  CreateBss (channel, 40 /* channel width */, 38 /* channel number */, 5190 /* frequency */, 40 /* primary channel number */);

  //Create BSS #5 operating on channel 44 (20 MHz)
  CreateBss (channel, 20 /* channel width */, 44 /* channel number */, 5220 /* frequency */, 44 /* primary channel number */);

  //Create BSS #6 operating on channel 48 (20 MHz)
  CreateBss (channel, 20 /* channel width */, 48 /* channel number */, 5240 /* frequency */, 48 /* primary channel number */);

  //Create BSS #7 operating on channel 46, with primary channel 44 (40 MHz)
  CreateBss (channel, 40 /* channel width */, 46 /* channel number */, 5230 /* frequency */, 44 /* primary channel number */);

  //Create BSS #8 operating on channel 46, with primary channel 48 (40 MHz)
  CreateBss (channel, 40 /* channel width */, 46 /* channel number */, 5230 /* frequency */, 48 /* primary channel number */);
  
  //Create BSS #9 operating on channel 42, with primary channel 36 (80 MHz)
  CreateBss (channel, 80 /* channel width */, 42 /* channel number */, 5210 /* frequency */, 36 /* primary channel number */);

  //Create BSS #10 operating on channel 42, with primary channel 40 (80 MHz)
  CreateBss (channel, 80 /* channel width */, 42 /* channel number */, 5210 /* frequency */, 40 /* primary channel number */);

  //Create BSS #11 operating on channel 42, with primary channel 44 (80 MHz)
  CreateBss (channel, 80 /* channel width */, 42 /* channel number */, 5210 /* frequency */, 44 /* primary channel number */);

  //Create BSS #12 operating on channel 42, with primary channel 48 (80 MHz)
  CreateBss (channel, 80 /* channel width */, 42 /* channel number */, 5210 /* frequency */, 48 /* primary channel number */);

  m_reception = std::vector<bool> (12, false);
  m_phyPayloadReceivedSuccess = std::vector<bool> (12, false);
}

void
TestStaticChannelBondingSnr::DoRun (void)
{
  RngSeedManager::SetSeed (1);
  RngSeedManager::SetRun (1);
  int64_t streamNumber = 0;
  for (auto & rxPhy : m_rxPhys)
    {
      rxPhy->AssignStreams (streamNumber);
    }
  for (auto & txPhy : m_txPhys)
    {
      txPhy->AssignStreams (streamNumber);
    }

  //CASE 1: each BSS send a packet on its channel to verify the received power per band for each receiver
  //and whether the packet is successfully received or not.*/

  //CASE 1A: BSS 1
  Simulator::Schedule (Seconds (0.9), &TestStaticChannelBondingSnr::Reset, this);
  Simulator::Schedule (Seconds (1.0), &TestStaticChannelBondingSnr::SendPacket, this, 1);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 1);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 3);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 9);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 10);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 3, 40); //secondary channel should be deemed busy for BSS 3
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 4, 40); //secondary channel should be deemed busy for BSS 4
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should not be deemed busy for BSS 7
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should not be deemed busy for BSS 8
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 40); //primary 40 MHz channel should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 40); //primary 40 MHz channel should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 80); //secondary channels should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 80); //secondary channels should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 40); //primary 40 MHz channel should not be deemed busy for BSS 11
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 40); //primary 40 MHz channel should not be deemed busy for BSS 12
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 11, 80); //secondary channels should be deemed busy for BSS 11
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 12, 80); //secondary channels should be deemed busy for BSS 12

  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 9);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 10);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 3, 40); //secondary channel should be deemed idle for BSS 3
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 4, 40); //secondary channel should be deemed idle for BSS 4
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should be deemed idle for BSS 7
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should be deemed idle for BSS 8
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 9, 80); //secondary channels should be deemed idle for BSS 9
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 10, 80); //secondary channels should be deemed idle for BSS 10
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 80); //secondary channels should be deemed idle for BSS 11
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 80); //secondary channels should be deemed idle for BSS 12

  Simulator::Schedule (Seconds (1.5), &TestStaticChannelBondingSnr::VerifyResultsForBss, this, true, true, 1); // successfull reception for BSS 1


  //CASE 1B: BSS 2
  Simulator::Schedule (Seconds (1.9), &TestStaticChannelBondingSnr::Reset, this);
  Simulator::Schedule (Seconds (2.0), &TestStaticChannelBondingSnr::SendPacket, this, 2);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 2);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 4);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 9);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 10);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 3, 40); //secondary channel should be deemed busy for BSS 3
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 4, 40); //secondary channel should be deemed busy for BSS 4
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should not be deemed busy for BSS 7
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should not be deemed busy for BSS 8
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 40); //primary 40 MHz channel should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 40); //primary 40 MHz channel should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 80); //secondary channels should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 80); //secondary channels should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 40); //primary 40 MHz channel should not be deemed busy for BSS 11
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 40); //primary 40 MHz channel should not be deemed busy for BSS 12
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 11, 80); //secondary channels should be deemed busy for BSS 11
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 12, 80); //secondary channels should be deemed busy for BSS 12

  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 9);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 10);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 3, 40); //secondary channel should be deemed idle for BSS 3
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 4, 40); //secondary channel should be deemed idle for BSS 4
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should be deemed idle for BSS 7
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should be deemed idle for BSS 8
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 9, 80); //secondary channels should be deemed idle for BSS 9
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 10, 80); //secondary channels should be deemed idle for BSS 10
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 80); //secondary channels should be deemed idle for BSS 11
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 80); //secondary channels should be deemed idle for BSS 12

  Simulator::Schedule (Seconds (2.5), &TestStaticChannelBondingSnr::VerifyResultsForBss, this, true, true, 2); // successfull reception for BSS 2


  //CASE 1C: BSS 3
  Simulator::Schedule (Seconds (2.9), &TestStaticChannelBondingSnr::Reset, this);
  Simulator::Schedule (Seconds (3.0), &TestStaticChannelBondingSnr::SendPacket, this, 3);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 1);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 2);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 3);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 4);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 9);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 10);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 3, 40); //secondary channel should be deemed busy for BSS 3
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 4, 40); //secondary channel should be deemed busy for BSS 4
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should not be deemed busy for BSS 7
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should not be deemed busy for BSS 8
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 40); //primary 40 MHz channel should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 40); //primary 40 MHz channel should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 80); //secondary channels should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 80); //secondary channels should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 40); //primary 40 MHz channel should be deemed busy for BSS 11
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 40); //primary 40 MHz channel should be deemed busy for BSS 12
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 11, 80); //secondary channels should be deemed busy for BSS 11
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 12, 80); //secondary channels should be deemed busy for BSS 12

  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 9);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 10);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 3, 40); //secondary channel should be deemed idle for BSS 3
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 4, 40); //secondary channel should be deemed idle for BSS 4
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should be deemed idle for BSS 7
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should be deemed idle for BSS 8
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 9, 80); //secondary channels should be deemed idle for BSS 9
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 10, 80); //secondary channels should be deemed idle for BSS 10
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 80); //secondary channels should be deemed idle for BSS 11
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 80); //secondary channels should be deemed idle for BSS 12

  Simulator::Schedule (Seconds (3.5), &TestStaticChannelBondingSnr::VerifyResultsForBss, this, true, true, 3); // successfull reception for BSS 3

  //CASE 1D: BSS 4
  Simulator::Schedule (Seconds (3.9), &TestStaticChannelBondingSnr::Reset, this);
  Simulator::Schedule (Seconds (4.0), &TestStaticChannelBondingSnr::SendPacket, this, 4);

  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 1);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 2);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 3);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 4);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 9);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 10);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 3, 40); //secondary channel should be deemed busy for BSS 3
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 4, 40); //secondary channel should be deemed busy for BSS 4
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should not be deemed busy for BSS 7
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should not be deemed busy for BSS 8
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 40); //primary 40 MHz channel should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 40); //primary 40 MHz channel should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 80); //secondary channels should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 80); //secondary channels should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 40); //primary 40 MHz channel should be deemed busy for BSS 11
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 40); //primary 40 MHz channel should be deemed busy for BSS 12
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 11, 80); //secondary channels should be deemed busy for BSS 11
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 12, 80); //secondary channels should be deemed busy for BSS 12

  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 9);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 10);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 3, 40); //secondary channel should be deemed idle for BSS 3
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 4, 40); //secondary channel should be deemed idle for BSS 4
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should be deemed idle for BSS 7
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should be deemed idle for BSS 8
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 9, 80); //secondary channels should be deemed idle for BSS 9
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 10, 80); //secondary channels should be deemed idle for BSS 10
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 80); //secondary channels should be deemed idle for BSS 11
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 80); //secondary channels should be deemed idle for BSS 12

  Simulator::Schedule (Seconds (4.5), &TestStaticChannelBondingSnr::VerifyResultsForBss, this, true, true, 4); // successfull reception for BSS 4


  //CASE 2: verify reception on channel 36 (BSS 1) when channel 40 is used (BSS 2) at the same time
  Simulator::Schedule (Seconds (4.9), &TestStaticChannelBondingSnr::Reset, this);
  Simulator::Schedule (Seconds (5.0), &TestStaticChannelBondingSnr::SendPacket, this, 1);
  Simulator::Schedule (Seconds (5.0), &TestStaticChannelBondingSnr::SendPacket, this, 2);
  
  Simulator::Schedule (Seconds (5.0), &TestStaticChannelBondingSnr::SetExpectedSnrForBss, this, 44.0, 1); // BSS 1 expects SNR around 44 dB
  Simulator::Schedule (Seconds (5.0), &TestStaticChannelBondingSnr::SetExpectedSnrForBss, this, 44.0, 2); // BSS 2 expects SNR around 44 dB

  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 1);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 2);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 3);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 4);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 9);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 10);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 3, 40); //secondary channel should be deemed busy for BSS 3
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 4, 40); //secondary channel should be deemed busy for BSS 4
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should not be deemed busy for BSS 7
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should not be deemed busy for BSS 8
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 40); //primary 40 MHz channel should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 40); //primary 40 MHz channel should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 80); //secondary channels should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 80); //secondary channels should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 40); //primary 40 MHz channel should be deemed busy for BSS 11
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 40); //primary 40 MHz channel should be deemed busy for BSS 12
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 11, 80); //secondary channels should be deemed busy for BSS 11
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 12, 80); //secondary channels should be deemed busy for BSS 12

  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 9);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 10);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 3, 40); //secondary channel should be deemed idle for BSS 3
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 4, 40); //secondary channel should be deemed idle for BSS 4
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should be deemed idle for BSS 7
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should be deemed idle for BSS 8
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 9, 80); //secondary channels should be deemed idle for BSS 9
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 10, 80); //secondary channels should be deemed idle for BSS 10
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 80); //secondary channels should be deemed idle for BSS 11
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 80); //secondary channels should be deemed idle for BSS 12

  Simulator::Schedule (Seconds (5.5), &TestStaticChannelBondingSnr::VerifyResultsForBss, this, true, true, 1); // successfull reception for BSS 1


  //CASE 3: verify reception on channel 38 (BSS 3) when channel 36 is used (BSS 1) at the same time
  Simulator::Schedule (Seconds (5.9), &TestStaticChannelBondingSnr::Reset, this);
  Simulator::Schedule (Seconds (6.0), &TestStaticChannelBondingSnr::SendPacket, this, 3);
  Simulator::Schedule (Seconds (6.0), &TestStaticChannelBondingSnr::SendPacket, this, 1);

  Simulator::Schedule (Seconds (6.0), &TestStaticChannelBondingSnr::SetExpectedSnrForBss, this, 3.0, 1); // BSS 1 expects SNR around 3 dB

  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 1);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 2);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 3);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 4);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 9);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 10);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 3, 40); //secondary channel should be deemed busy for BSS 3
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 4, 40); //secondary channel should be deemed busy for BSS 4
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should not be deemed busy for BSS 7
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should not be deemed busy for BSS 8
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 40); //primary 40 MHz channel should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 40); //primary 40 MHz channel should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 80); //secondary channels should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 80); //secondary channels should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 40); //primary 40 MHz channel should be deemed busy for BSS 11
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 40); //primary 40 MHz channel should be deemed busy for BSS 12
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 11, 80); //secondary channels should be deemed busy for BSS 11
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 12, 80); //secondary channels should be deemed busy for BSS 12

  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 9);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 10);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 3, 40); //secondary channel should be deemed idle for BSS 3
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 4, 40); //secondary channel should be deemed idle for BSS 4
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should be deemed idle for BSS 7
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should be deemed idle for BSS 8
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 9, 80); //secondary channels should be deemed idle for BSS 9
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 10, 80); //secondary channels should be deemed idle for BSS 10
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 80); //secondary channels should be deemed idle for BSS 11
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 80); //secondary channels should be deemed idle for BSS 12

  Simulator::Schedule (Seconds (6.5), &TestStaticChannelBondingSnr::VerifyResultsForBss, this, true, false, 1); // PHY header passed but payload failed for BSS 1
  Simulator::Schedule (Seconds (6.5), &TestStaticChannelBondingSnr::VerifyResultsForBss, this, false, false, 3); // packet from BSS 3 dropped because power from BSS 1 is higher in the primary 20 MHz


  //CASE 4: verify reception on channel 38 (BSS 3) when channel 40 is used (BSS 2) at the same time
  Simulator::Schedule (Seconds (6.9), &TestStaticChannelBondingSnr::Reset, this);
  Simulator::Schedule (Seconds (7.0), &TestStaticChannelBondingSnr::SendPacket, this, 3);
  Simulator::Schedule (Seconds (7.0), &TestStaticChannelBondingSnr::SendPacket, this, 2);

  Simulator::Schedule (Seconds (7.0), &TestStaticChannelBondingSnr::SetExpectedSnrForBss, this, 3.0, 2); // BSS 2 expects SNR around 3 dB

  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 1);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 2);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 3);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 4);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 9);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 10);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 3, 40); //secondary channel should be deemed busy for BSS 3
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 4, 40); //secondary channel should be deemed busy for BSS 4
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should not be deemed busy for BSS 7
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should not be deemed busy for BSS 8
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 40); //primary 40 MHz channel should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 40); //primary 40 MHz channel should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 80); //secondary channels should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 80); //secondary channels should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 40); //primary 40 MHz channel should be deemed busy for BSS 11
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 40); //primary 40 MHz channel should be deemed busy for BSS 12
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 11, 80); //secondary channels should be deemed busy for BSS 11
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 12, 80); //secondary channels should be deemed busy for BSS 12

  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 9);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 10);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 3, 40); //secondary channel should be deemed idle for BSS 3
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 4, 40); //secondary channel should be deemed idle for BSS 4
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should be deemed idle for BSS 7
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should be deemed idle for BSS 8
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 9, 80); //secondary channels should be deemed idle for BSS 9
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 10, 80); //secondary channels should be deemed idle for BSS 10
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 80); //secondary channels should be deemed idle for BSS 11
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 80); //secondary channels should be deemed idle for BSS 12

  Simulator::Schedule (Seconds (7.5), &TestStaticChannelBondingSnr::VerifyResultsForBss, this, true, false, 2); // PHY header passed but payload failed for BSS 2
  Simulator::Schedule (Seconds (7.5), &TestStaticChannelBondingSnr::VerifyResultsForBss, this, true, false, 3); // PHY header passed but payload failed for BSS 3


  //CASE 5: verify reception on channel 38 (BSS 4) when channel 36 is used (BSS 1) at the same time
  Simulator::Schedule (Seconds (7.9), &TestStaticChannelBondingSnr::Reset, this);
  Simulator::Schedule (Seconds (8.0), &TestStaticChannelBondingSnr::SendPacket, this, 4);
  Simulator::Schedule (Seconds (8.0), &TestStaticChannelBondingSnr::SendPacket, this, 1);

  Simulator::Schedule (Seconds (8.0), &TestStaticChannelBondingSnr::SetExpectedSnrForBss, this, 3.0, 1); // BSS 1 expects SNR around 3 dB

  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 1);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 2);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 3);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 4);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 9);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 10);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 3, 40); //secondary channel should be deemed busy for BSS 3
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 4, 40); //secondary channel should be deemed busy for BSS 4
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should not be deemed busy for BSS 7
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should not be deemed busy for BSS 8
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 40); //primary 40 MHz channel should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 40); //primary 40 MHz channel should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 80); //secondary channels should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 80); //secondary channels should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 40); //primary 40 MHz channel should be deemed busy for BSS 11
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 40); //primary 40 MHz channel should be deemed busy for BSS 12
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 11, 80); //secondary channels should be deemed busy for BSS 11
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 12, 80); //secondary channels should be deemed busy for BSS 12

  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 9);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 10);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 3, 40); //secondary channel should be deemed idle for BSS 3
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 4, 40); //secondary channel should be deemed idle for BSS 4
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should be deemed idle for BSS 7
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should be deemed idle for BSS 8
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 9, 80); //secondary channels should be deemed idle for BSS 9
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 10, 80); //secondary channels should be deemed idle for BSS 10
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 80); //secondary channels should be deemed idle for BSS 11
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 80); //secondary channels should be deemed idle for BSS 12

  Simulator::Schedule (Seconds (8.5), &TestStaticChannelBondingSnr::VerifyResultsForBss, this, true, false, 1); // PHY header passed but payload failed for BSS 1
  Simulator::Schedule (Seconds (8.5), &TestStaticChannelBondingSnr::VerifyResultsForBss, this, true, false, 4); // PHY header passed but payload failed for BSS 4


  //CASE 6: verify reception on channel 38 (BSS 4) when channel 40 is used (BSS 2) at the same time
  Simulator::Schedule (Seconds (8.9), &TestStaticChannelBondingSnr::Reset, this);
  Simulator::Schedule (Seconds (9.0), &TestStaticChannelBondingSnr::SendPacket, this, 4);
  Simulator::Schedule (Seconds (9.0), &TestStaticChannelBondingSnr::SendPacket, this, 2);

  Simulator::Schedule (Seconds (9.0), &TestStaticChannelBondingSnr::SetExpectedSnrForBss, this, 3.0, 2); // BSS 2 expects SNR around 3 dB

  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 1);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 2);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 3);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 4);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 9);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 10);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 3, 40); //secondary channel should be deemed busy for BSS 3
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 4, 40); //secondary channel should be deemed busy for BSS 4
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should not be deemed busy for BSS 7
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should not be deemed busy for BSS 8
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 40); //primary 40 MHz channel should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 40); //primary 40 MHz channel should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 80); //secondary channels should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 80); //secondary channels should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 40); //primary 40 MHz channel should be deemed busy for BSS 11
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 40); //primary 40 MHz channel should be deemed busy for BSS 12
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 11, 80); //secondary channels should be deemed busy for BSS 11
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 12, 80); //secondary channels should be deemed busy for BSS 12

  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 9);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 10);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 3, 40); //secondary channel should be deemed idle for BSS 3
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 4, 40); //secondary channel should be deemed idle for BSS 4
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should be deemed idle for BSS 7
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should be deemed idle for BSS 8
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 9, 80); //secondary channels should be deemed idle for BSS 9
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 10, 80); //secondary channels should be deemed idle for BSS 10
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 80); //secondary channels should be deemed idle for BSS 11
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 80); //secondary channels should be deemed idle for BSS 12

  Simulator::Schedule (Seconds (9.5), &TestStaticChannelBondingSnr::VerifyResultsForBss, this, true, false, 2); // PHY header passed but payload failed for BSS 2
  Simulator::Schedule (Seconds (9.5), &TestStaticChannelBondingSnr::VerifyResultsForBss, this, false, false, 4); // packet from BSS 4 dropped because power from BSS 2 is higher in the primary 20 MHz


  //CASE 7: verify reception on channel 38 (BSS 3) when channels 36 (BSS 1) and 40 (BSS 2) are used at the same time
  Simulator::Schedule (Seconds (9.9), &TestStaticChannelBondingSnr::Reset, this);
  Simulator::Schedule (Seconds (10.0), &TestStaticChannelBondingSnr::SendPacket, this, 3);
  Simulator::Schedule (Seconds (10.0), &TestStaticChannelBondingSnr::SendPacket, this, 1);
  Simulator::Schedule (Seconds (10.0), &TestStaticChannelBondingSnr::SendPacket, this, 2);

  Simulator::Schedule (Seconds (10.0), &TestStaticChannelBondingSnr::SetExpectedSnrForBss, this, 3.0, 1); // BSS 1 expects SNR around 3 dB
  Simulator::Schedule (Seconds (10.0), &TestStaticChannelBondingSnr::SetExpectedSnrForBss, this, 3.0, 2); // BSS 2 expects SNR around 3 dB

  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 1);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 2);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 3);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 4);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 9);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 10);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 3, 40); //secondary channel should be deemed busy for BSS 3
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 4, 40); //secondary channel should be deemed busy for BSS 4
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should not be deemed busy for BSS 7
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should not be deemed busy for BSS 8
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 40); //primary 40 MHz channel should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 40); //primary 40 MHz channel should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 80); //secondary channels should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 80); //secondary channels should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 40); //primary 40 MHz channel should be deemed busy for BSS 11
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 40); //primary 40 MHz channel should be deemed busy for BSS 12
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 11, 80); //secondary channels should be deemed busy for BSS 11
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 12, 80); //secondary channels should be deemed busy for BSS 12

  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 9);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 10);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 3, 40); //secondary channel should be deemed idle for BSS 3
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 4, 40); //secondary channel should be deemed idle for BSS 4
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should be deemed idle for BSS 7
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should be deemed idle for BSS 8
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 9, 80); //secondary channels should be deemed idle for BSS 9
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 10, 80); //secondary channels should be deemed idle for BSS 10
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 80); //secondary channels should be deemed idle for BSS 11
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 80); //secondary channels should be deemed idle for BSS 12

  Simulator::Schedule (Seconds (10.5), &TestStaticChannelBondingSnr::VerifyResultsForBss, this, true, false, 1); // PHY header passed but payload failed for BSS 1
  Simulator::Schedule (Seconds (10.5), &TestStaticChannelBondingSnr::VerifyResultsForBss, this, true, false, 2); // PHY header passed but payload failed for BSS 2
  Simulator::Schedule (Seconds (10.5), &TestStaticChannelBondingSnr::VerifyResultsForBss, this, false, false, 3); // packet from BSS 3 dropped because power from BSS 1 is higher in the primary 20 MHz


  //CASE 8: verify reception on channel 38 (BSS 4) when channels 36 (BSS 1) and 40 (BSS 2) are used at the same time
  Simulator::Schedule (Seconds (10.9), &TestStaticChannelBondingSnr::Reset, this);
  Simulator::Schedule (Seconds (11.0), &TestStaticChannelBondingSnr::SendPacket, this, 4);
  Simulator::Schedule (Seconds (11.0), &TestStaticChannelBondingSnr::SendPacket, this, 1);
  Simulator::Schedule (Seconds (11.0), &TestStaticChannelBondingSnr::SendPacket, this, 2);

  Simulator::Schedule (Seconds (11.0), &TestStaticChannelBondingSnr::SetExpectedSnrForBss, this, 3.0, 1); // BSS 1 expects SNR around 3 dB
  Simulator::Schedule (Seconds (11.0), &TestStaticChannelBondingSnr::SetExpectedSnrForBss, this, 3.0, 2); // BSS 2 expects SNR around 3 dB

  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 1);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 2);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 3);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 4);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 9);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::RX, 10);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 3, 40); //secondary channel should be deemed busy for BSS 3
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 4, 40); //secondary channel should be deemed busy for BSS 4
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should not be deemed busy for BSS 7
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should not be deemed busy for BSS 8
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 40); //primary 40 MHz channel should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 40); //primary 40 MHz channel should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 9, 80); //secondary channels should be deemed busy for BSS 9
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 10, 80); //secondary channels should be deemed busy for BSS 10
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 40); //primary 40 MHz channel should be deemed busy for BSS 11
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 40); //primary 40 MHz channel should be deemed busy for BSS 12
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 11, 80); //secondary channels should be deemed busy for BSS 11
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, false, 12, 80); //secondary channels should be deemed busy for BSS 12

  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 1);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 3);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 2);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 4);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 5);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 6);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 7);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 8);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 9);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 10);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 11);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckPhyState, this, WifiPhyState::IDLE, 12);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 3, 40); //secondary channel should be deemed idle for BSS 3
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 4, 40); //secondary channel should be deemed idle for BSS 4
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 7, 40); //secondary channel should be deemed idle for BSS 7
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 8, 40); //secondary channel should be deemed idle for BSS 8
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 9, 80); //secondary channels should be deemed idle for BSS 9
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 10, 80); //secondary channels should be deemed idle for BSS 10
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 11, 80); //secondary channels should be deemed idle for BSS 11
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (165.0), &TestStaticChannelBondingSnr::CheckSecondaryChannelStatus, this, true, 12, 80); //secondary channels should be deemed idle for BSS 12

  Simulator::Schedule (Seconds (11.5), &TestStaticChannelBondingSnr::VerifyResultsForBss, this, true, false, 1); // PHY header passed but payload failed for BSS 1
  Simulator::Schedule (Seconds (11.5), &TestStaticChannelBondingSnr::VerifyResultsForBss, this, true, false, 2); // PHY header passed but payload failed for BSS 2
  Simulator::Schedule (Seconds (11.5), &TestStaticChannelBondingSnr::VerifyResultsForBss, this, false, false, 4); // packet from BSS 4 dropped because power from BSS 2 is higher in the primary 20 MHz

  Simulator::Run ();
  Simulator::Destroy ();
}

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief constant threshold dynamic channel bonding
 *
 * In this test, we have three 802.11ac transmitters and three 802.11ac receivers.
 * A BSS is composed of one transmitter and one receiver.
 *
 * The first BSS makes uses of channel bonding on channel 38 (= 36 + 40),
 * with channel 36 as primary 20 MHz channel.
 * The second BSS operates on channel 40 with a channel width of 20 MHz.
 * The third BSS makes uses of channel bonding on channel 38 (= 36 + 40),
 * with channel 40 as primary 20 MHz channel.
 */
class TestConstantThresholdDynamicChannelBonding : public TestCase
{
public:
  TestConstantThresholdDynamicChannelBonding ();
  virtual ~TestConstantThresholdDynamicChannelBonding ();

private:
  virtual void DoSetup (void);
  virtual void DoRun (void);

  /**
   * Create BSS
   * \param channel the Spectrum channel
   * \param channelWidth the channel width
   * \param channelNumber the operating channel number
   * \param frequency the operating frequency
   * \param primaryChannelNumber the channel number of the primary 20 MHz
   */
  void CreateBss (const Ptr<MultiModelSpectrumChannel> channel,
                  uint16_t channelWidth, uint8_t channelNumber,
                  uint16_t frequency, uint8_t primaryChannelNumber);

  /**
   * Send packet function
   * \param bss the BSS of the transmitter belongs to
   * \param expectedChannelWidth the expected channel width used for the transmission
   */
  void SendPacket (uint8_t bss, uint16_t expectedChannelWidth);

  std::vector<Ptr<BondingTestSpectrumWifiPhy> > m_rxPhys; ///< RX PHYs
  std::vector<Ptr<BondingTestSpectrumWifiPhy> > m_txPhys; ///< TX PHYs
};

TestConstantThresholdDynamicChannelBonding::TestConstantThresholdDynamicChannelBonding ()
  : TestCase ("Constant threshold dynamic channel bonding test")
{
  LogLevel logLevel = (LogLevel)(LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_ALL);
  LogComponentEnable ("WifiChannelBondingTest", logLevel);
  //LogComponentEnable ("ConstantThresholdChannelBondingManager", logLevel);
  //LogComponentEnable ("WifiPhy", logLevel);
}

TestConstantThresholdDynamicChannelBonding::~TestConstantThresholdDynamicChannelBonding ()
{
  for (auto & rxPhy : m_rxPhys)
    {
      rxPhy = 0;
    }
  for (auto & txPhy : m_txPhys)
    {
      txPhy = 0;
    }
  m_rxPhys.clear ();
  m_txPhys.clear ();
}

void
TestConstantThresholdDynamicChannelBonding::SendPacket (uint8_t bss, uint16_t expectedChannelWidth)
{
  Ptr<BondingTestSpectrumWifiPhy> phy = m_txPhys.at (bss - 1);
  uint32_t payloadSize = 1000 + bss;

  WifiMode mode = WifiPhy::GetVhtMcs7 ();
  uint16_t channelWidth = phy->GetUsableChannelWidth (mode);
  NS_TEST_ASSERT_MSG_EQ (channelWidth, expectedChannelWidth, "selected channel width is not as expected");

  WifiTxVector txVector = WifiTxVector (mode, 0, WIFI_PREAMBLE_HT_MF, 800, 1, 1, 0, channelWidth, false, false);

  Ptr<Packet> pkt = Create<Packet> (payloadSize);
  WifiMacHeader hdr;
  hdr.SetType (WIFI_MAC_QOSDATA);

  Ptr<WifiPsdu> psdu = Create<WifiPsdu> (pkt, hdr);
  phy->Send (WifiPsduMap ({std::make_pair (SU_STA_ID, psdu)}), txVector);
}

void
TestConstantThresholdDynamicChannelBonding::CreateBss (const Ptr<MultiModelSpectrumChannel> channel,
                                      uint16_t channelWidth, uint8_t channelNumber,
                                      uint16_t frequency, uint8_t primaryChannelNumber)
{
  uint8_t bssNumber = m_rxPhys.size () + 1;

  Ptr<ErrorRateModel> error = CreateObject<NistErrorRateModel> ();

  Ptr<BondingTestSpectrumWifiPhy> rxPhy = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> rxMobility = CreateObject<ConstantPositionMobilityModel> ();
  rxMobility->SetPosition (Vector (1.0, 1.0 * (bssNumber - 1), 0.0));
  rxPhy->SetMobility (rxMobility);
  rxPhy->ConfigureStandard (WIFI_PHY_STANDARD_80211ac);
  rxPhy->CreateWifiSpectrumPhyInterface (nullptr);
  rxPhy->SetChannel (channel);
  rxPhy->SetErrorRateModel (error);
  rxPhy->SetChannelWidth (channelWidth);
  rxPhy->SetChannelNumber (channelNumber);
  rxPhy->SetPrimaryChannelNumber (primaryChannelNumber);
  rxPhy->SetFrequency (frequency);
  rxPhy->SetTxPowerStart (0.0);
  rxPhy->SetTxPowerEnd (0.0);
  rxPhy->SetRxSensitivity (-91.0);
  rxPhy->SetAttribute ("TxMaskInnerBandMinimumRejection", DoubleValue (-40.0));
  rxPhy->SetAttribute ("TxMaskOuterBandMinimumRejection", DoubleValue (-56.0));
  rxPhy->SetAttribute ("TxMaskOuterBandMaximumRejection", DoubleValue (-80.0));
  rxPhy->Initialize ();
  
  m_rxPhys.push_back (rxPhy);

  Ptr<BondingTestSpectrumWifiPhy> txPhy = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> txMobility = CreateObject<ConstantPositionMobilityModel> ();
  txMobility->SetPosition (Vector (0.0, 1.0 * (bssNumber - 1), 0.0));
  txPhy->SetMobility (txMobility);
  txPhy->ConfigureStandard (WIFI_PHY_STANDARD_80211ac);
  txPhy->CreateWifiSpectrumPhyInterface (nullptr);
  txPhy->SetChannel (channel);
  txPhy->SetErrorRateModel (error);
  txPhy->SetChannelWidth (channelWidth);
  txPhy->SetChannelNumber (channelNumber);
  txPhy->SetPrimaryChannelNumber (primaryChannelNumber);
  txPhy->SetFrequency (frequency);
  txPhy->SetTxPowerStart (0.0);
  txPhy->SetTxPowerEnd (0.0);
  txPhy->SetRxSensitivity (-91.0);
  txPhy->SetAttribute ("TxMaskInnerBandMinimumRejection", DoubleValue (-40.0));
  txPhy->SetAttribute ("TxMaskOuterBandMinimumRejection", DoubleValue (-56.0));
  txPhy->SetAttribute ("TxMaskOuterBandMaximumRejection", DoubleValue (-80.0));
  txPhy->Initialize ();

  Ptr<ConstantThresholdChannelBondingManager> channelBondingManager = CreateObject<ConstantThresholdChannelBondingManager> ();
  txPhy->SetChannelBondingManager (channelBondingManager);
  txPhy->SetPifs (MicroSeconds (25));

  m_txPhys.push_back (txPhy);
}

void
TestConstantThresholdDynamicChannelBonding::DoSetup (void)
{
  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();

  Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel> ();
  lossModel->SetDefaultLoss (50); // set default loss to 50 dB for all links
  channel->AddPropagationLossModel (lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  channel->SetPropagationDelayModel (delayModel);

  //Create BSS #1 operating on channel 38, with primary channel 36
  CreateBss (channel, 40 /* channel width */, 38 /* channel number */, 5190 /* frequency */, 36 /* primary channel number */);

  //Create BSS #2 operating on channel 40
  CreateBss (channel, 20 /* channel width */, 40 /* channel number */, 5200 /* frequency */, 40 /* primary channel number */);

  //Create BSS #3 operating on channel 38, with primary channel 40
  CreateBss (channel, 40 /* channel width */, 38 /* channel number */, 5190 /* frequency */, 40 /* primary channel number */);
}

void
TestConstantThresholdDynamicChannelBonding::DoRun (void)
{
  RngSeedManager::SetSeed (1);
  RngSeedManager::SetRun (1);
  int64_t streamNumber = 0;
  for (auto & rxPhy : m_rxPhys)
    {
      rxPhy->AssignStreams (streamNumber);
    }
  for (auto & txPhy : m_txPhys)
    {
      txPhy->AssignStreams (streamNumber);
    }

  //CASE 1: send on free channel, so BSS 1 PHY shall select the full supported channel width of 40 MHz
  Simulator::Schedule (Seconds (1.0), &TestConstantThresholdDynamicChannelBonding::SendPacket, this, 1, 40);

  //CASE 2: send when secondary channel is free for more than PIFS, so BSS 1 PHY shall select the full supported channel width of 40 MHz
  Simulator::Schedule (Seconds (2.0), &TestConstantThresholdDynamicChannelBonding::SendPacket, this, 2, 20);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (164) /* transmission time of previous packet sent by BSS 2 */ + MicroSeconds (50) /* > PIFS */, &TestConstantThresholdDynamicChannelBonding::SendPacket, this, 1, 40);

  //CASE 3: send when secondary channel is free for less than PIFS, so BSS 1 PHY shall limit its channel width to 20 MHz
  Simulator::Schedule (Seconds (3.0), &TestConstantThresholdDynamicChannelBonding::SendPacket, this, 2, 20);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (164) /* transmission time of previous packet sent by BSS 2 */ + MicroSeconds (20) /* < PIFS */, &TestConstantThresholdDynamicChannelBonding::SendPacket, this, 1, 20);

  //CASE 4: both transmitters send at the same time when channel was previously idle, BSS 1 shall anyway transmit at 40 MHz since it shall already indicate the selected channel width in its PHY header
  Simulator::Schedule (Seconds (4.0), &TestConstantThresholdDynamicChannelBonding::SendPacket, this, 2, 20);
  Simulator::Schedule (Seconds (4.0), &TestConstantThresholdDynamicChannelBonding::SendPacket, this, 1, 40);

  //CASE 5: send when secondary channel is free for more than PIFS, so BSS 1 PHY shall select the full supported channel width of 40 MHz
  Simulator::Schedule (Seconds (5.0), &TestConstantThresholdDynamicChannelBonding::SendPacket, this, 3, 40);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (100) /* transmission time of previous packet sent by BSS 2 */ + MicroSeconds (50) /* > PIFS */, &TestConstantThresholdDynamicChannelBonding::SendPacket, this, 1, 40);

  //CASE 6: send when secondary channel is free for more than PIFS, so BSS 3 PHY shall select the full supported channel width of 40 MHz
  Simulator::Schedule (Seconds (6.0), &TestConstantThresholdDynamicChannelBonding::SendPacket, this, 1, 40);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (100) /* transmission time of previous packet sent by BSS 2 */ + MicroSeconds (50) /* > PIFS */, &TestConstantThresholdDynamicChannelBonding::SendPacket, this, 3, 40);

  //CASE 7: send when secondary channel is free for less than PIFS, so BSS 1 PHY shall limit its channel width to 20 MHz
  Simulator::Schedule (Seconds (7.0), &TestConstantThresholdDynamicChannelBonding::SendPacket, this, 3, 40);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (100) /* transmission time of previous packet sent by BSS 2 */ + MicroSeconds (20) /* < PIFS */, &TestConstantThresholdDynamicChannelBonding::SendPacket, this, 1, 20);

  //CASE 8: send when secondary channel is free for less than PIFS, so BSS 3 PHY shall limit its channel width to 20 MHz
  Simulator::Schedule (Seconds (8.0), &TestConstantThresholdDynamicChannelBonding::SendPacket, this, 1, 40);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (100) /* transmission time of previous packet sent by BSS 2 */ + MicroSeconds (20) /* < PIFS */, &TestConstantThresholdDynamicChannelBonding::SendPacket, this, 3, 20);

  Simulator::Run ();
  Simulator::Destroy ();
}

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief dynamic threshold dynamic channel bonding
 *
 * In this test, we have three 802.11ac transmitters and three 802.11ac receivers.
 * A BSS is composed of one transmitter and one receiver.
 *
 * The first BSS makes uses of channel bonding on channel 38 (= 36 + 40),
 * with channel 36 as primary 20 MHz channel.
 * The second BSS operates on channel 40 with a channel width of 20 MHz.
 * The third BSS makes uses of channel bonding on channel 38 (= 36 + 40),
 * with channel 40 as primary 20 MHz channel.
 */
class TestDynamicThresholdDynamicChannelBonding : public TestCase
{
public:
  TestDynamicThresholdDynamicChannelBonding ();
  virtual ~TestDynamicThresholdDynamicChannelBonding ();

private:
  virtual void DoSetup (void);
  virtual void DoRun (void);

  /**
   * Run one function
   * \param mcs the MCS to use for the tests
   * \param interferenceAboveThreshold  flag whether the interference is above the CCA threshold in the secondary channel
   */
  void RunOne (uint8_t mcs, bool interferenceAboveThreshold);

  /**
   * Create BSS
   * \param channel the Spectrum channel
   * \param channelWidth the channel width
   * \param channelNumber the operating channel number
   * \param frequency the operating frequency
   * \param primaryChannelNumber the channel number of the primary 20 MHz
   */
  void CreateBss (const Ptr<MultiModelSpectrumChannel> channel,
                  uint16_t channelWidth, uint8_t channelNumber,
                  uint16_t frequency, uint8_t primaryChannelNumber);

  /**
   * Send packet function
   * \param bss the BSS of the transmitter belongs to
   * \param mcs the MCS to use for the transmission
   * \param expectedChannelWidth the expected channel width used for the transmission
   */
  void SendPacket (uint8_t bss, uint8_t mcs, uint16_t expectedChannelWidth);

  std::vector<Ptr<BondingTestSpectrumWifiPhy> > m_rxPhys; ///< RX PHYs
  std::vector<Ptr<BondingTestSpectrumWifiPhy> > m_txPhys; ///< TX PHYs
};

TestDynamicThresholdDynamicChannelBonding::TestDynamicThresholdDynamicChannelBonding ()
  : TestCase ("dynamic threshold dynamic channel bonding test")
{
  LogLevel logLevel = (LogLevel)(LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_ALL);
  LogComponentEnable ("WifiChannelBondingTest", logLevel);
  //LogComponentEnable ("ConstantThresholdChannelBondingManager", logLevel);
  //LogComponentEnable ("WifiPhy", logLevel);
}

TestDynamicThresholdDynamicChannelBonding::~TestDynamicThresholdDynamicChannelBonding ()
{
  for (auto & rxPhy : m_rxPhys)
    {
      rxPhy = 0;
    }
  for (auto & txPhy : m_txPhys)
    {
      txPhy = 0;
    }
  m_rxPhys.clear ();
  m_txPhys.clear ();
}

void
TestDynamicThresholdDynamicChannelBonding::SendPacket (uint8_t bss, uint8_t mcs, uint16_t expectedChannelWidth)
{
  Ptr<BondingTestSpectrumWifiPhy> phy = m_txPhys.at (bss - 1);
  uint32_t payloadSize = 1000 + bss;

  WifiMode mode = WifiPhy::GetVhtMcs (mcs);
  uint16_t channelWidth = phy->GetUsableChannelWidth (mode);
  NS_TEST_ASSERT_MSG_EQ (channelWidth, expectedChannelWidth, "selected channel width is not as expected");

  WifiTxVector txVector = WifiTxVector (mode, 0, WIFI_PREAMBLE_HT_MF, 800, 1, 1, 0, channelWidth, false, false);

  Ptr<Packet> pkt = Create<Packet> (payloadSize);
  WifiMacHeader hdr;
  hdr.SetType (WIFI_MAC_QOSDATA);

  Ptr<WifiPsdu> psdu = Create<WifiPsdu> (pkt, hdr);
  phy->Send (WifiPsduMap ({std::make_pair (SU_STA_ID, psdu)}), txVector);
}

void
TestDynamicThresholdDynamicChannelBonding::CreateBss (const Ptr<MultiModelSpectrumChannel> channel,
                                      uint16_t channelWidth, uint8_t channelNumber,
                                      uint16_t frequency, uint8_t primaryChannelNumber)
{
  uint8_t bssNumber = m_rxPhys.size () + 1;

  Ptr<ErrorRateModel> error = CreateObject<NistErrorRateModel> ();

  Ptr<BondingTestSpectrumWifiPhy> rxPhy = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> rxMobility = CreateObject<ConstantPositionMobilityModel> ();
  rxMobility->SetPosition (Vector (1.0, 1.0 * (bssNumber - 1), 0.0));
  rxPhy->SetMobility (rxMobility);
  rxPhy->ConfigureStandard (WIFI_PHY_STANDARD_80211ac);
  rxPhy->CreateWifiSpectrumPhyInterface (nullptr);
  rxPhy->SetChannel (channel);
  rxPhy->SetErrorRateModel (error);
  rxPhy->SetChannelWidth (channelWidth);
  rxPhy->SetChannelNumber (channelNumber);
  rxPhy->SetPrimaryChannelNumber (primaryChannelNumber);
  rxPhy->SetFrequency (frequency);
  rxPhy->SetTxPowerStart (0.0);
  rxPhy->SetTxPowerEnd (0.0);
  rxPhy->SetRxSensitivity (-91.0);
  rxPhy->SetAttribute ("TxMaskInnerBandMinimumRejection", DoubleValue (-40.0));
  rxPhy->SetAttribute ("TxMaskOuterBandMinimumRejection", DoubleValue (-56.0));
  rxPhy->SetAttribute ("TxMaskOuterBandMaximumRejection", DoubleValue (-80.0));
  rxPhy->Initialize ();
  
  m_rxPhys.push_back (rxPhy);

  Ptr<BondingTestSpectrumWifiPhy> txPhy = CreateObject<BondingTestSpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> txMobility = CreateObject<ConstantPositionMobilityModel> ();
  txMobility->SetPosition (Vector (0.0, 1.0 * (bssNumber - 1), 0.0));
  txPhy->SetMobility (txMobility);
  txPhy->ConfigureStandard (WIFI_PHY_STANDARD_80211ac);
  txPhy->CreateWifiSpectrumPhyInterface (nullptr);
  txPhy->SetChannel (channel);
  txPhy->SetErrorRateModel (error);
  txPhy->SetChannelWidth (channelWidth);
  txPhy->SetChannelNumber (channelNumber);
  txPhy->SetPrimaryChannelNumber (primaryChannelNumber);
  txPhy->SetFrequency (frequency);
  txPhy->SetTxPowerStart (0.0);
  txPhy->SetTxPowerEnd (0.0);
  txPhy->SetRxSensitivity (-91.0);
  txPhy->SetAttribute ("TxMaskInnerBandMinimumRejection", DoubleValue (-40.0));
  txPhy->SetAttribute ("TxMaskOuterBandMinimumRejection", DoubleValue (-56.0));
  txPhy->SetAttribute ("TxMaskOuterBandMaximumRejection", DoubleValue (-80.0));
  txPhy->Initialize ();

  Ptr<DynamicThresholdChannelBondingManager> channelBondingManager = CreateObject<DynamicThresholdChannelBondingManager> ();
  channelBondingManager->SetCcaEdThresholdSecondaryForMode (WifiPhy::GetVhtMcs7 (), -72);
  channelBondingManager->SetCcaEdThresholdSecondaryForMode (WifiPhy::GetVhtMcs0 (), -22);
  txPhy->SetChannelBondingManager (channelBondingManager);
  txPhy->SetPifs (MicroSeconds (25));

  m_txPhys.push_back (txPhy);
}

void
TestDynamicThresholdDynamicChannelBonding::DoSetup (void)
{
  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();

  Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel> ();
  lossModel->SetDefaultLoss (50); // set default loss to 50 dB for all links
  channel->AddPropagationLossModel (lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  channel->SetPropagationDelayModel (delayModel);

  //Create BSS #1 operating on channel 38, with primary channel 36
  CreateBss (channel, 40 /* channel width */, 38 /* channel number */, 5190 /* frequency */, 36 /* primary channel number */);

  //Create BSS #2 operating on channel 40
  CreateBss (channel, 20 /* channel width */, 40 /* channel number */, 5200 /* frequency */, 40 /* primary channel number */);

  //Create BSS #3 operating on channel 38, with primary channel 40
  CreateBss (channel, 40 /* channel width */, 38 /* channel number */, 5190 /* frequency */, 40 /* primary channel number */);
}

void
TestDynamicThresholdDynamicChannelBonding::RunOne (uint8_t mcs, bool interferenceAboveThreshold)
{
  RngSeedManager::SetSeed (1);
  RngSeedManager::SetRun (1);
  int64_t streamNumber = 0;
  for (auto & rxPhy : m_rxPhys)
    {
      rxPhy->AssignStreams (streamNumber);
    }
  for (auto & txPhy : m_txPhys)
    {
      txPhy->AssignStreams (streamNumber);
    }

  //CASE 1: send on free channel, so BSS 1 PHY shall select the full supported channel width of 40 MHz
  Simulator::Schedule (Seconds (1.0), &TestDynamicThresholdDynamicChannelBonding::SendPacket, this, 1, mcs, 40);

  //CASE 2: send when secondary channel is free for more than PIFS, so BSS 1 PHY shall select the full supported channel width of 40 MHz
  Simulator::Schedule (Seconds (2.0), &TestDynamicThresholdDynamicChannelBonding::SendPacket, this, 2, 7, 20);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (164) /* transmission time of previous packet sent by BSS 2 */ + MicroSeconds (50) /* > PIFS */, &TestDynamicThresholdDynamicChannelBonding::SendPacket, this, 1, mcs, 40);

  //CASE 3: send when secondary channel is free for less than PIFS, so BSS 1 PHY shall limit its channel width to 20 MHz if interference is above the CCA threshold
  Simulator::Schedule (Seconds (3.0), &TestDynamicThresholdDynamicChannelBonding::SendPacket, this, 2, 7, 20);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (164) /* transmission time of previous packet sent by BSS 2 */ + MicroSeconds (20) /* < PIFS */, &TestDynamicThresholdDynamicChannelBonding::SendPacket, this, 1, mcs, interferenceAboveThreshold ? 20 : 40);

  //CASE 4: both transmitters send at the same time when channel was previously idle, BSS 1 shall anyway transmit at 40 MHz since it shall already indicate the selected channel width in its PHY header
  Simulator::Schedule (Seconds (4.0), &TestDynamicThresholdDynamicChannelBonding::SendPacket, this, 2, 7, 20);
  Simulator::Schedule (Seconds (4.0), &TestDynamicThresholdDynamicChannelBonding::SendPacket, this, 1, mcs, 40);

  //CASE 5: send when secondary channel is free for more than PIFS, so BSS 1 PHY shall select the full supported channel width of 40 MHz
  Simulator::Schedule (Seconds (5.0), &TestDynamicThresholdDynamicChannelBonding::SendPacket, this, 3, 7, 40);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (100) /* transmission time of previous packet sent by BSS 2 */ + MicroSeconds (50) /* > PIFS */, &TestDynamicThresholdDynamicChannelBonding::SendPacket, this, 1, mcs, 40);

  //CASE 6: send when secondary channel is free for more than PIFS, so BSS 3 PHY shall select the full supported channel width of 40 MHz
  Simulator::Schedule (Seconds (6.0), &TestDynamicThresholdDynamicChannelBonding::SendPacket, this, 1, 7, 40);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (100) /* transmission time of previous packet sent by BSS 2 */ + MicroSeconds (50) /* > PIFS */, &TestDynamicThresholdDynamicChannelBonding::SendPacket, this, 3, mcs, 40);

  //CASE 7: send when secondary channel is free for less than PIFS, so BSS 1 PHY shall limit its channel width to 20 MHz
  Simulator::Schedule (Seconds (7.0), &TestDynamicThresholdDynamicChannelBonding::SendPacket, this, 3, 7, 40);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (100) /* transmission time of previous packet sent by BSS 2 */ + MicroSeconds (20) /* < PIFS */, &TestDynamicThresholdDynamicChannelBonding::SendPacket, this, 1, mcs, 20);

  //CASE 8: send when secondary channel is free for less than PIFS, so BSS 3 PHY shall limit its channel width to 20 MHz
  Simulator::Schedule (Seconds (8.0), &TestDynamicThresholdDynamicChannelBonding::SendPacket, this, 1, 7, 40);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (100) /* transmission time of previous packet sent by BSS 2 */ + MicroSeconds (20) /* < PIFS */, &TestDynamicThresholdDynamicChannelBonding::SendPacket, this, 3, mcs, 20);

  Simulator::Run ();
}

void
TestDynamicThresholdDynamicChannelBonding::DoRun (void)
{
  /* run test for MCS 7, where RX power > CCA threshold */
  RunOne (7, true);

  /* run test for MCS 0, where RX power < CCA threshold */
  RunOne (0, false);

  Simulator::Destroy ();
}

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief effective SNR calculations
 */
class TestEffectiveSnrCalculations : public TestCase
{
public:
  TestEffectiveSnrCalculations ();
  virtual ~TestEffectiveSnrCalculations ();

private:
  virtual void DoSetup (void);
  virtual void DoRun (void);

  struct InterferenceInfo
  {
    uint16_t frequency; ///< Interference frequency in MHz
    uint16_t channelWidth; ///< Interference channel width in MHz
    double powerDbm; ///< Interference power in dBm
    InterferenceInfo(uint16_t freq, uint16_t width, double pow) : frequency (freq), channelWidth (width), powerDbm (pow) {}
  };

  /**
   * Run one function
   */
  void RunOne (void);

  /**
   * Generate interference function
   * \param phy the PHY to use to generate the interference
   * \param interference the structure holding the interference info
   */
  void GenerateInterference (Ptr<WaveformGenerator> phy, InterferenceInfo interference);
  /**
   * Stop interference function
   * \param phy the PHY to stop
   */
  void StopInterference (Ptr<WaveformGenerator> phy);

  /**
   * Send packet function
   */
  void SendPacket (void);

  /**
   * Callback triggered when a packet has been successfully received
   * \param p the received packet
   * \param snr the signal to noise ratio
   * \param mode the mode used for the transmission
   * \param preamble the preamble used for the transmission
   */
  void RxOkCallback (Ptr<const Packet> p, double snr, WifiMode mode, WifiPreamble preamble);

  /**
   * Callback triggered when a packet has been unsuccessfully received
   * \param p the packet
   * \param snr the signal to noise ratio
   */
  void RxErrorCallback (Ptr<const Packet> p, double snr);

  Ptr<BondingTestSpectrumWifiPhy> m_rxPhy = 0; ///< Rx Phy
  Ptr<BondingTestSpectrumWifiPhy> m_txPhy = 0; ///< Tx Phy
  std::vector<Ptr<WaveformGenerator> > m_interferersPhys; ///< Interferers Phys
  uint16_t m_signalFrequency; ///< Signal frequency in MHz
  uint8_t m_signalChannelNumber; ///< Signal channel number
  uint16_t m_signalChannelWidth; ///< Signal channel width in MHz
  double m_expectedSnrDb; ///< Expected SNR in dB
  uint32_t m_rxCount; ///< Counter for both RxOk and RxError callbacks

  std::vector<InterferenceInfo> m_interferences;
};

TestEffectiveSnrCalculations::TestEffectiveSnrCalculations ()
  : TestCase ("Effective SNR calculations test"),
    m_signalFrequency (5180),
    m_signalChannelNumber (36),
    m_signalChannelWidth (20),
    m_rxCount (0)
{
}

TestEffectiveSnrCalculations::~TestEffectiveSnrCalculations ()
{
  m_rxPhy = 0;
  m_txPhy = 0;
  for (auto it = m_interferersPhys.begin (); it != m_interferersPhys.end (); it++)
    {
      *it = 0;
    }
  m_interferersPhys.clear ();
}

void
TestEffectiveSnrCalculations::GenerateInterference (Ptr<WaveformGenerator> phy, TestEffectiveSnrCalculations::InterferenceInfo interference)
{
  NS_LOG_INFO ("GenerateInterference: PHY=" << phy << " frequency=" << interference.frequency << " channelWidth=" << interference.channelWidth << " powerDbm=" << interference.powerDbm);
  BandInfo bandInfo;
  bandInfo.fc = interference.frequency * 1e6;
  bandInfo.fl = bandInfo.fc - (((interference.channelWidth / 2) + 1) * 1e6);
  bandInfo.fh = bandInfo.fc + (((interference.channelWidth / 2) - 1) * 1e6);
  Bands bands;
  bands.push_back (bandInfo);

  Ptr<SpectrumModel> spectrumInterference = Create<SpectrumModel> (bands);
  Ptr<SpectrumValue> interferencePsd = Create<SpectrumValue> (spectrumInterference);
  *interferencePsd = DbmToW (interference.powerDbm) / ((interference.channelWidth - 1) * 1e6);

  Time interferenceDuration = MilliSeconds (100);

  phy->SetTxPowerSpectralDensity (interferencePsd);
  phy->SetPeriod (interferenceDuration);
  phy->Start ();

  Simulator::Schedule (interferenceDuration, &TestEffectiveSnrCalculations::StopInterference, this, phy);
}

void
TestEffectiveSnrCalculations::StopInterference (Ptr<WaveformGenerator> phy)
{
  phy->Stop();
}

void
TestEffectiveSnrCalculations::SendPacket (void)
{
  WifiTxVector txVector = WifiTxVector (WifiPhy::GetVhtMcs7 (), 0, WIFI_PREAMBLE_VHT_SU, 800, 1, 1, 0, m_signalChannelWidth, false, false);

  Ptr<Packet> pkt = Create<Packet> (1000);
  WifiMacHeader hdr;
  hdr.SetType (WIFI_MAC_QOSDATA);

  Ptr<WifiPsdu> psdu = Create<WifiPsdu> (pkt, hdr);
  m_txPhy->Send (WifiPsduMap ({std::make_pair (SU_STA_ID, psdu)}), txVector);
}

void
TestEffectiveSnrCalculations::RxOkCallback (Ptr<const Packet> p, double snr, WifiMode mode, WifiPreamble preamble)
{
  NS_LOG_INFO ("RxOkCallback: SNR=" << RatioToDb (snr) << " dB expected_SNR=" << m_expectedSnrDb << " dB");
  m_rxCount++;
  NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), m_expectedSnrDb, 0.1, "SNR is different than expected");
}

void
TestEffectiveSnrCalculations::RxErrorCallback (Ptr<const Packet> p, double snr)
{
  NS_LOG_INFO ("RxErrorCallback: SNR=" << RatioToDb (snr) << " dB expected_SNR=" << m_expectedSnrDb << " dB");
  m_rxCount++;
  NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), m_expectedSnrDb, 0.1, "SNR is different than expected");
}

void
TestEffectiveSnrCalculations::DoSetup (void)
{
  LogLevel logLevel = (LogLevel)(LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_ALL);
  LogComponentEnable ("WifiChannelBondingTest", logLevel);

  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();

  Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel> ();
  lossModel->SetDefaultLoss (0); // set default loss to 0 dB for simplicity, so RX power = TX power
  channel->AddPropagationLossModel (lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  channel->SetPropagationDelayModel (delayModel);

  Ptr<ErrorRateModel> error = CreateObject<NistErrorRateModel> ();

  Ptr<Node> rxNode = CreateObject<Node> ();
  Ptr<WifiNetDevice> rxDev = CreateObject<WifiNetDevice> ();
  m_rxPhy = CreateObject<BondingTestSpectrumWifiPhy> ();
  m_rxPhy->CreateWifiSpectrumPhyInterface (rxDev);
  m_rxPhy->ConfigureStandard (WIFI_PHY_STANDARD_80211ac);
  m_rxPhy->SetChannelNumber (m_signalChannelNumber);
  m_rxPhy->SetFrequency (m_signalFrequency);
  m_rxPhy->SetChannelWidth (m_signalChannelWidth);
  m_rxPhy->SetErrorRateModel (error);
  m_rxPhy->SetDevice (rxDev);
  m_rxPhy->SetChannel (channel);
  Ptr<ConstantPositionMobilityModel> rxMobility = CreateObject<ConstantPositionMobilityModel> ();
  rxMobility->SetPosition (Vector (1.0, 0.0, 0.0));
  m_rxPhy->SetMobility (rxMobility);
  rxDev->SetPhy (m_rxPhy);
  rxNode->AggregateObject (rxMobility);
  rxNode->AddDevice (rxDev);

  Ptr<Node> txNode = CreateObject<Node> ();
  Ptr<WifiNetDevice> txDev = CreateObject<WifiNetDevice> ();
  m_txPhy = CreateObject<BondingTestSpectrumWifiPhy> ();
  m_txPhy->CreateWifiSpectrumPhyInterface (txDev);
  m_txPhy->ConfigureStandard (WIFI_PHY_STANDARD_80211ac);
  m_txPhy->SetChannelNumber (m_signalChannelNumber);
  m_txPhy->SetFrequency (m_signalFrequency);
  m_txPhy->SetChannelWidth (m_signalChannelWidth);
  m_txPhy->SetErrorRateModel (error);
  m_txPhy->SetDevice (txDev);
  m_txPhy->SetChannel (channel);
  Ptr<ConstantPositionMobilityModel> txMobility = CreateObject<ConstantPositionMobilityModel> ();
  txMobility->SetPosition (Vector (0.0, 0.0, 0.0));
  m_txPhy->SetMobility (txMobility);
  txDev->SetPhy (m_txPhy);
  txNode->AggregateObject (txMobility);
  txNode->AddDevice (txDev);

  for (unsigned int i = 0; i < (160 / 20); i++)
    {
      Ptr<Node> interfererNode = CreateObject<Node> ();
      Ptr<NonCommunicatingNetDevice> interfererDev = CreateObject<NonCommunicatingNetDevice> ();
      Ptr<WaveformGenerator> phy = CreateObject<WaveformGenerator> ();
      phy->SetDevice (interfererDev);
      phy->SetChannel (channel);
      phy->SetDutyCycle (1);
      interfererNode->AddDevice (interfererDev);
      m_interferersPhys.push_back (phy);
    }

  m_rxPhy->GetState()->TraceConnectWithoutContext ("RxOk", MakeCallback (&TestEffectiveSnrCalculations::RxOkCallback, this));
  m_rxPhy->GetState()->TraceConnectWithoutContext ("RxError", MakeCallback (&TestEffectiveSnrCalculations::RxErrorCallback, this));
}

void
TestEffectiveSnrCalculations::RunOne (void)
{
  RngSeedManager::SetSeed (1);
  RngSeedManager::SetRun (1);
  int64_t streamNumber = 0;
  m_rxPhy->AssignStreams (streamNumber);
  m_txPhy->AssignStreams (streamNumber);

  m_txPhy->SetTxPowerStart (18);
  m_txPhy->SetTxPowerEnd (18);

  m_txPhy->SetChannelNumber (m_signalChannelNumber);
  m_txPhy->SetChannelWidth (m_signalChannelWidth);
  m_txPhy->SetFrequency (m_signalFrequency);

  m_rxPhy->SetChannelNumber (m_signalChannelNumber);
  m_rxPhy->SetChannelWidth (m_signalChannelWidth);
  m_rxPhy->SetFrequency (m_signalFrequency);

  Simulator::Schedule (Seconds (1.0), &TestEffectiveSnrCalculations::SendPacket, this);
  unsigned i = 0;
  for (auto const& interference : m_interferences)
    {
      Simulator::Schedule (Seconds (1.0) + MicroSeconds (40) + MicroSeconds (i), &TestEffectiveSnrCalculations::GenerateInterference, this, m_interferersPhys.at (i), interference);
      i++;
    }

  Simulator::Run ();

  m_interferences.clear ();
}

void
TestEffectiveSnrCalculations::DoRun (void)
{
  // Case 1: 20 MHz transmission: Reference case
  {
    m_signalFrequency = 5180;
    m_signalChannelNumber = 36;
    m_signalChannelWidth = 20;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5180, 20, 15));
    // SNR eff = SNR = 18 - 15 = 3 dB
    m_expectedSnrDb = 3;
    RunOne ();
  }

  // Case 2: 40 MHz transmission: I1 = I2
  {
    m_signalFrequency = 5190;
    m_signalChannelNumber = 38;
    m_signalChannelWidth = 40;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5190, 40, 15));
    // SNR eff,m = min ((18 - 3) - (15 - 3), (18 - 3) - (15 - 3)) = min (3 dB, 3 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(2)) = 12.5 = 10.9 dB
    m_expectedSnrDb = 10.9;
    RunOne ();
  }

  // Case 3: 40 MHz transmission: I2 = 0
  {
    m_signalFrequency = 5190;
    m_signalChannelNumber = 38;
    m_signalChannelWidth = 40;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5180, 20, 12));
    // SNR eff,m = min ((18 - 3) - 12, (18 - 3) - (-94)) min (3 dB, 109 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(2)) = 12.4 = 10.9 dB
    m_expectedSnrDb = 10.9;
    RunOne ();
  }

  // Case 4: 40 MHz transmission: I2 = 1/2 I1
  {
    m_signalFrequency = 5190;
    m_signalChannelNumber = 38;
    m_signalChannelWidth = 40;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5180, 20, 12));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5200, 20, 9));
    // SNR eff,m = min ((18 - 3) - 12, (18 - 3) - 9) = min (3 dB, 6 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(2)) = 12.4 = 10.9 dB
    m_expectedSnrDb = 10.9;
    RunOne ();
  }

  // Case 5: 80 MHz transmission: I1 = I2 = I3 = I4
  {
    m_signalFrequency = 5210;
    m_signalChannelNumber = 42;
    m_signalChannelWidth = 80;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5210, 80, 15));
    // SNR eff,m = min ((18 - 6) - (15 - 6), (18 - 6) - (15 - 6), (18 - 6) - (15 - 6), (18 - 6) - (15 - 6)) = min (3 dB, 3 dB, 3 dB, 3 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(4)) = 22.8 = 13.6 dB
    m_expectedSnrDb = 13.6;
    RunOne ();
  }

  // Case 6: 80 MHz transmission: I2 = I3 = I4 = 0
  {
    m_signalFrequency = 5210;
    m_signalChannelNumber = 42;
    m_signalChannelWidth = 80;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5180, 20, 9));
    // SNR eff,m = min ((18 - 6) - 9, (18 - 6) - (-94), (18 - 6) - (-94), (18 - 6) - (-94)) = min (3 dB, 106 dB, 106 dB, 106 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(4)) = 22.8 = 13.6 dB
    m_expectedSnrDb = 13.6;
    RunOne ();
  }

  // Case 7: 80 MHz transmission: I2 = 1/2 I1, I3 = I4 = 0
  {
    m_signalFrequency = 5210;
    m_signalChannelNumber = 42;
    m_signalChannelWidth = 80;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5180, 20, 9));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5200, 20, 6));
    // SNR eff,m = min ((18 - 6) - 9, (18 - 6) - 6, (18 - 6) - (-94), (18 - 6) - (-94)) = min (3 dB, 6 dB, 106 dB, 106 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(4)) = 22.8 = 13.6 dB
    m_expectedSnrDb = 13.6;
    RunOne ();
  }

  // Case 8: 80 MHz transmission: I2 = I3 = I4 = 1/2 I1
  {
    m_signalFrequency = 5210;
    m_signalChannelNumber = 42;
    m_signalChannelWidth = 80;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5180, 20, 9));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5200, 20, 6));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5220, 20, 6));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5240, 20, 6));
    // SNR eff,m = min ((18 - 6) - 9, (18 - 6) - 6, (18 - 6) - 6, (18 - 6) - 6) = min (3 dB, 6 dB, 6 dB, 6 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(4)) = 22.8 = 13.6 dB
    m_expectedSnrDb = 13.6;
    RunOne ();
  }

  // Case 9: 160 MHz transmission: I1 = I2 = I3 = I4 = I5 = I6 = I7 = I8
  {
    m_signalFrequency = 5250;
    m_signalChannelNumber = 50;
    m_signalChannelWidth = 160;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5250, 160, 15));
    // SNR eff,m = min ((18 - 9) - (15 - 9), (18 - 9) - (15 - 9), (18 - 9) - (15 - 9), (18 - 9) - (15 - 9), (18 - 9) - (15 - 9), (18 - 9) - (15 - 9), (18 - 9) - (15 - 9), (18 - 9) - (15 - 9))
    //           = min (3 dB, 3 dB, 3 dB, 3 dB, 3 dB, 3 dB, 3 dB, 3 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(8)) = 33.2 = 15.2 dB
    m_expectedSnrDb = 15.2;
    RunOne ();
  }

  // Case 10: 160 MHz transmission: I2 = I3 = I4 = I5 = I6 = I7 = I8 = 0
  {
    m_signalFrequency = 5250;
    m_signalChannelNumber = 50;
    m_signalChannelWidth = 160;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5180, 20, 6));
    // SNR eff,m = min ((18 - 9) - 6, (18 - 9) - (-94), (18 - 9) - (-94), (18 - 9) - (-94), (18 - 9) - (-94), (18 - 9) - (-94), (18 - 9) - (-94), (18 - 9) - (-94))
    //           = min (3 dB, 103 dB, 103 dB, 103 dB, 103 dB, 103 dB, 103 dB, 103 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(8)) = 33.2 = 15.2 dB
    m_expectedSnrDb = 15.2;
    RunOne ();
  }

  // Case 11: 160 MHz transmission: I2 = I3 = I4 = 1/2 I1, I5 = I6 = I7 = I8 = 0
  {
    m_signalFrequency = 5250;
    m_signalChannelNumber = 50;
    m_signalChannelWidth = 160;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5180, 20, 6));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5200, 20, 3));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5220, 20, 3));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5240, 20, 3));
    // SNR eff,m = min ((18 - 9) - 6, (18 - 9) - 3, (18 - 9) - 3, (18 - 9) - 3, (18 - 9) - (-94), (18 - 9) - (-94), (18 - 9) - (-94), (18 - 9) - (-94))
    //           = min (3 dB, 6 dB, 6 dB, 6 dB, 103 dB, 103 dB, 103 dB, 103 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(8)) = 33.2 = 15.2 dB
    m_expectedSnrDb = 15.2;
    RunOne ();
  }

  // Case 12: 160 MHz transmission: I2 = I3 = I4 = I5 = I6 = I7 = I8 = 1/2 I1
  {
    m_signalFrequency = 5250;
    m_signalChannelNumber = 50;
    m_signalChannelWidth = 160;
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5180, 20, 6));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5200, 20, 3));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5220, 20, 3));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5240, 20, 3));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5260, 20, 3));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5280, 20, 3));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5300, 20, 3));
    m_interferences.push_back (TestEffectiveSnrCalculations::InterferenceInfo (5320, 20, 3));
    // SNR eff,m = min ((18 - 9) - 6, (18 - 9) - 3, (18 - 9) - 3, (18 - 9) - 3, (18 - 9) - 3, (18 - 9) - 3, (18 - 9) - 3, (18 - 9) - 3)
    //           = min (3 dB, 6 dB, 6 dB, 6 dB, 6 dB, 6 dB, 6 dB, 6 dB) = 3 dB = 2
    // SNR eff = 2 + (15 * ln(8)) = 33.2 = 15.2 dB
    m_expectedSnrDb = 15.2;
    RunOne ();
  }

  NS_TEST_EXPECT_MSG_EQ (m_rxCount, 12, "12 packets should have been received!");

  Simulator::Destroy ();
}

/**
 * todo
 */

class TestStaticChannelBondingChannelAccess : public TestCase
{
public:
  TestStaticChannelBondingChannelAccess ();
  virtual ~TestStaticChannelBondingChannelAccess ();

private:
  virtual void DoRun (void);

  /**
   * Triggers the arrival of a burst of 1000 Byte-long packets in the source device
   * \param sourceDevice pointer to the source NetDevice
   * \param destination address of the destination device
   */
  void SendPacket (Ptr<NetDevice> sourceDevice, Address& destination) const;

  /**
   * Check the PHY state
   * \param expectedState the expected PHY state
   * \param device the device to check
   */
  void CheckPhyState (WifiPhyState expectedState, Ptr<NetDevice> device);

  /**
   * Check the secondary channel status
   * \param expectedIdle flag whether the secondary channel is expected to be deemed IDLE
   * \param device the device to check
   * \param channelWidth the channel width to check
   */
  void CheckSecondaryChannelStatus (bool expectedIdle, Ptr<NetDevice> device, uint16_t channelWidth);
};

TestStaticChannelBondingChannelAccess::TestStaticChannelBondingChannelAccess ()
  : TestCase ("Test case for channel access when static channel bonding")
{
}

TestStaticChannelBondingChannelAccess::~TestStaticChannelBondingChannelAccess ()
{
}

void
TestStaticChannelBondingChannelAccess::SendPacket (Ptr<NetDevice> sourceDevice, Address& destination) const
{
  Ptr<Packet> pkt = Create<Packet> (1000);  // 1000 dummy bytes of data
  sourceDevice->Send (pkt, destination, 0);
}

void
TestStaticChannelBondingChannelAccess::CheckPhyState (WifiPhyState expectedState, Ptr<NetDevice> device)
{
  Ptr<WifiNetDevice> wifiDevicePtr = device->GetObject <WifiNetDevice> ();
  WifiPhyState currentState = wifiDevicePtr->GetPhy ()->GetPhyState ();
  NS_TEST_ASSERT_MSG_EQ (currentState, expectedState, "PHY State " << currentState << " does not match expected state " << expectedState << " at " << Simulator::Now ());
}

void
TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus (bool expectedIdle, Ptr<NetDevice> device, uint16_t channelWidth)
{
  Ptr<WifiNetDevice> wifiDevicePtr = device->GetObject <WifiNetDevice> ();
  Ptr<WifiPhy> phy = wifiDevicePtr->GetPhy ();
  bool currentlyIdle = phy->IsStateIdle (channelWidth, WToDbm (phy->GetDefaultCcaEdThresholdSecondary ()));
  NS_TEST_ASSERT_MSG_EQ (currentlyIdle, expectedIdle, "Secondary channel status " << currentlyIdle << " does not match expected status " << expectedIdle << " at " << Simulator::Now ());
}

void
TestStaticChannelBondingChannelAccess::DoRun (void)
{
  RngSeedManager::SetSeed (5);
  RngSeedManager::SetRun (1);
  int64_t streamNumber = 0;

  //BSS #1 operating on channel 38 (40 MHz)
  NodeContainer wifiNodesBss1;
  wifiNodesBss1.Create (2);

  //BSS #2 operating on channel 36 (20 MHz)
  NodeContainer wifiNodesBss2;
  wifiNodesBss2.Create (2);

  //BSS #3 operating on channel 40 (20 MHz)
  NodeContainer wifiNodesBss3;
  wifiNodesBss3.Create (2);

  //BSS #4 operating on channel 46 (40 MHz)
  NodeContainer wifiNodesBss4;
  wifiNodesBss4.Create (2);

  //BSS #5 operating on channel 42 (80 MHz)
  NodeContainer wifiNodesBss5;
  wifiNodesBss5.Create (2);

  SpectrumWifiPhyHelper spectrumPhy = SpectrumWifiPhyHelper::Default ();
  Ptr<MultiModelSpectrumChannel> spectrumChannel = CreateObject<MultiModelSpectrumChannel> ();
  Ptr<FriisPropagationLossModel> lossModel = CreateObject<FriisPropagationLossModel> ();
  lossModel->SetFrequency (5e9);
  spectrumChannel->AddPropagationLossModel (lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  spectrumChannel->SetPropagationDelayModel (delayModel);

  spectrumPhy.SetChannel (spectrumChannel);
  spectrumPhy.SetErrorRateModel ("ns3::NistErrorRateModel");
  spectrumPhy.Set ("TxPowerStart", DoubleValue (10));
  spectrumPhy.Set ("TxPowerEnd", DoubleValue (10));
  //Configure very strong rejection to be close to ideal filter conditions
  spectrumPhy.Set ("TxMaskInnerBandMinimumRejection", DoubleValue (-80.0));
  spectrumPhy.Set ("TxMaskOuterBandMinimumRejection", DoubleValue (-112.0));
  spectrumPhy.Set ("TxMaskOuterBandMaximumRejection", DoubleValue (-160.0));
  
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue ("VhtMcs7"),
                                "ControlMode", StringValue ("VhtMcs7"));

  WifiMacHelper mac;
  mac.SetType ("ns3::AdhocWifiMac");

  NetDeviceContainer bss1Devices = wifi.Install (spectrumPhy, mac, wifiNodesBss1);

  // Assign fixed streams to random variables in use
  wifi.AssignStreams (bss1Devices, streamNumber);

  Ptr<NetDevice> devicePtr = bss1Devices.Get (0);
  Ptr<WifiNetDevice> wifiDevicePtr = devicePtr->GetObject <WifiNetDevice> ();
  wifiDevicePtr->GetPhy ()->SetAttribute ("ChannelWidth", UintegerValue (40));
  wifiDevicePtr->GetPhy ()->SetAttribute ("ChannelNumber", UintegerValue (38));
  wifiDevicePtr->GetPhy ()->SetAttribute ("Frequency", UintegerValue (5190));

  devicePtr = bss1Devices.Get (1);
  wifiDevicePtr = devicePtr->GetObject <WifiNetDevice> ();
  wifiDevicePtr->GetPhy ()->SetAttribute ("ChannelWidth", UintegerValue (40));
  wifiDevicePtr->GetPhy ()->SetAttribute ("ChannelNumber", UintegerValue (38));
  wifiDevicePtr->GetPhy ()->SetAttribute ("Frequency", UintegerValue (5190));

  NetDeviceContainer bss2Devices = wifi.Install (spectrumPhy, mac, wifiNodesBss2);

  // Assign fixed streams to random variables in use
  wifi.AssignStreams (bss2Devices, streamNumber);

  devicePtr = bss2Devices.Get (0);
  wifiDevicePtr = devicePtr->GetObject <WifiNetDevice> ();
  wifiDevicePtr->GetPhy ()->SetAttribute ("ChannelWidth", UintegerValue (20));
  wifiDevicePtr->GetPhy ()->SetAttribute ("ChannelNumber", UintegerValue (36));
  wifiDevicePtr->GetPhy ()->SetAttribute ("Frequency", UintegerValue (5180));

  devicePtr = bss2Devices.Get (1);
  wifiDevicePtr = devicePtr->GetObject <WifiNetDevice> ();
  wifiDevicePtr->GetPhy ()->SetAttribute ("ChannelWidth", UintegerValue (20));
  wifiDevicePtr->GetPhy ()->SetAttribute ("ChannelNumber", UintegerValue (36));
  wifiDevicePtr->GetPhy ()->SetAttribute ("Frequency", UintegerValue (5180));

  NetDeviceContainer bss3Devices = wifi.Install (spectrumPhy, mac, wifiNodesBss3);

  // Assign fixed streams to random variables in use
  wifi.AssignStreams (bss3Devices, streamNumber);

  devicePtr = bss3Devices.Get (0);
  wifiDevicePtr = devicePtr->GetObject <WifiNetDevice> ();
  wifiDevicePtr->GetPhy ()->SetAttribute ("ChannelWidth", UintegerValue (20));
  wifiDevicePtr->GetPhy ()->SetAttribute ("ChannelNumber", UintegerValue (40));
  wifiDevicePtr->GetPhy ()->SetAttribute ("Frequency", UintegerValue (5200));

  devicePtr = bss3Devices.Get (1);
  wifiDevicePtr = devicePtr->GetObject <WifiNetDevice> ();
  wifiDevicePtr->GetPhy ()->SetAttribute ("ChannelWidth", UintegerValue (20));
  wifiDevicePtr->GetPhy ()->SetAttribute ("ChannelNumber", UintegerValue (40));
  wifiDevicePtr->GetPhy ()->SetAttribute ("Frequency", UintegerValue (5200));

  NetDeviceContainer bss4Devices = wifi.Install (spectrumPhy, mac, wifiNodesBss4);

  // Assign fixed streams to random variables in use
  wifi.AssignStreams (bss4Devices, streamNumber);

  devicePtr = bss4Devices.Get (0);
  wifiDevicePtr = devicePtr->GetObject <WifiNetDevice> ();
  wifiDevicePtr->GetPhy ()->SetAttribute ("ChannelWidth", UintegerValue (40));
  wifiDevicePtr->GetPhy ()->SetAttribute ("ChannelNumber", UintegerValue (46));
  wifiDevicePtr->GetPhy ()->SetAttribute ("Frequency", UintegerValue (5230));

  devicePtr = bss4Devices.Get (1);
  wifiDevicePtr = devicePtr->GetObject <WifiNetDevice> ();
  wifiDevicePtr->GetPhy ()->SetAttribute ("ChannelWidth", UintegerValue (40));
  wifiDevicePtr->GetPhy ()->SetAttribute ("ChannelNumber", UintegerValue (46));
  wifiDevicePtr->GetPhy ()->SetAttribute ("Frequency", UintegerValue (5230));

  NetDeviceContainer bss5Devices = wifi.Install (spectrumPhy, mac, wifiNodesBss5);

  // Assign fixed streams to random variables in use
  wifi.AssignStreams (bss5Devices, streamNumber);

  devicePtr = bss5Devices.Get (0);
  wifiDevicePtr = devicePtr->GetObject <WifiNetDevice> ();
  wifiDevicePtr->GetPhy ()->SetAttribute ("ChannelWidth", UintegerValue (80));
  wifiDevicePtr->GetPhy ()->SetAttribute ("ChannelNumber", UintegerValue (42));
  wifiDevicePtr->GetPhy ()->SetAttribute ("Frequency", UintegerValue (5210));

  devicePtr = bss5Devices.Get (1);
  wifiDevicePtr = devicePtr->GetObject <WifiNetDevice> ();
  wifiDevicePtr->GetPhy ()->SetAttribute ("ChannelWidth", UintegerValue (80));
  wifiDevicePtr->GetPhy ()->SetAttribute ("ChannelNumber", UintegerValue (42));
  wifiDevicePtr->GetPhy ()->SetAttribute ("Frequency", UintegerValue (5210));

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (1.0, 0.0, 0.0));
  positionAlloc->Add (Vector (2.0, 0.0, 0.0));
  positionAlloc->Add (Vector (3.0, 0.0, 0.0));
  positionAlloc->Add (Vector (4.0, 0.0, 0.0));
  positionAlloc->Add (Vector (5.0, 0.0, 0.0));
  positionAlloc->Add (Vector (6.0, 0.0, 0.0));
  positionAlloc->Add (Vector (7.0, 0.0, 0.0));
  positionAlloc->Add (Vector (8.0, 0.0, 0.0));
  positionAlloc->Add (Vector (9.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiNodesBss1);
  mobility.Install (wifiNodesBss2);
  mobility.Install (wifiNodesBss3);
  mobility.Install (wifiNodesBss4);
  mobility.Install (wifiNodesBss5);

  //Make sure ADDBA are established
  Simulator::Schedule (Seconds (0.0), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss1Devices.Get (0), bss1Devices.Get (1)->GetAddress ());
  Simulator::Schedule (Seconds (0.1), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss2Devices.Get (0), bss2Devices.Get (1)->GetAddress ());
  Simulator::Schedule (Seconds (0.2), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss3Devices.Get (0), bss3Devices.Get (1)->GetAddress ());
  Simulator::Schedule (Seconds (0.3), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss4Devices.Get (0), bss4Devices.Get (1)->GetAddress ());
  Simulator::Schedule (Seconds (0.4), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss5Devices.Get (0), bss5Devices.Get (1)->GetAddress ());


  //CASE 1: channel 36 only (20 MHz)
  Simulator::Schedule (Seconds (1.0), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss2Devices.Get (0), bss2Devices.Get (1)->GetAddress ());

  //BSS 2: transmitter should be in TX state and receiver should be in RX state
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::TX, bss2Devices.Get (0));
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss2Devices.Get (1));

  //BSS 1 and BSS 5: they should be in RX state since the PPDU is received on the primary channel
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss1Devices.Get (0));
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss1Devices.Get (1));
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss5Devices.Get (0));
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss5Devices.Get (1));
  
  //BSS 3 and BSS 4: they should be in IDLE state since no PPDU is received on their channel(s)
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss3Devices.Get (0));
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss3Devices.Get (1));
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (0));
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (1));

  //Secondary channel CCA for BSS 1 and BBS 5 should be deemed BUSY during transmission or reception of a 20 MHz PPDU in the primary channel
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (0), 40);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (1), 40);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 40);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 40);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 80);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 80);

  //Secondary channel CCA for BSS 4 should not be deemed BUSY
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (0), 40);
  Simulator::Schedule (Seconds (1.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (1), 40);


  //CASE 2: channel 40 only (20 MHz)
  Simulator::Schedule (Seconds (2.0), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss3Devices.Get (0), bss3Devices.Get (1)->GetAddress ());

  //BSS 3: transmitter should be in TX state and receiver should be in RX state
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::TX, bss3Devices.Get (0));
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss3Devices.Get (1));

  //BSS 1 and BSS 5: they should be in IDLE state since PPDU is received on the secondary 20 MHz channel
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss1Devices.Get (0));
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss1Devices.Get (1));
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss5Devices.Get (0));
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss5Devices.Get (1));

  //BSS 2 and BSS 4: they should be in IDLE state since no PPDU is received on their channel(s)
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss2Devices.Get (0));
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss2Devices.Get (1));
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (0));
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (1));

  //Secondary channel CCA for BSS 1 and BSS 5 should be deemed BUSY since the energy in the secondary channel is above the corresponding CCA threshold.
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (0), 40);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (1), 40);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 40);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 40);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 80);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 80);

  //Secondary channel CCA for BSS 4 should not be deemed BUSY
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (0), 40);
  Simulator::Schedule (Seconds (2.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (1), 40);


  //CASE 3: channel 38 only (40 MHz)
  Simulator::Schedule (Seconds (3.0), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss1Devices.Get (0), bss1Devices.Get (1)->GetAddress ());

  //BSS 1: transmitter should be in TX state and receiver should be in RX state
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::TX, bss1Devices.Get (0));
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss1Devices.Get (1));

  //BSS 2 and BSS 3: they should be in CCA_BUSY state since a 40 MHz PPDU is transmitted whereas receivers only support 20 MHz PPDUs
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss2Devices.Get (0));
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss2Devices.Get (1));
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss3Devices.Get (0));
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss3Devices.Get (1));

  //BSS 4: they should be in IDLE state since no PPDU is received on their channel(s)
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (0));
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (1));

  //BSS 5: they should be in in RX state
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss5Devices.Get (0));
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss5Devices.Get (1));

  //Secondary channel CCA for BSS 1 is deemed BUSY during transmission of a 40 MHz PPDU.
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (0), 40);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (1), 40);

  //Secondary channel CCA for BSS 5 is deemed BUSY during reception of a 40 MHz PPDU.
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 40);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 40);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 80);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 80);
  
  //Secondary channel CCA for BSS 4 should not be deemed BUSY
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (0), 40);
  Simulator::Schedule (Seconds (3.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (1), 40);


  //CASE 4: channel 36 (20 MHz) then channel 40 (20 MHz)
  Simulator::Schedule (Seconds (4.0), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss2Devices.Get (0), bss2Devices.Get (1)->GetAddress ());
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (5), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss3Devices.Get (0), bss3Devices.Get (1)->GetAddress ());

  //BSS 2: transmitter should be in TX state and receiver should be in RX state
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::TX, bss2Devices.Get (0));
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss2Devices.Get (1));

  //BSS 3: transmitter should be in TX state and receiver should be in RX state
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::TX, bss3Devices.Get (0));
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss3Devices.Get (1));

  //BSS 1 and BSS 5: they should be in RX state since a PPDU is received on the primary channel
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss1Devices.Get (0));
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss1Devices.Get (1));
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss5Devices.Get (0));
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss5Devices.Get (1));

  //BSS 4: they should be in IDLE state since no PPDU is received on their channel(s)
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (0));
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (1));

  //Secondary channel CCA for BSS 1 and BSS 5 is deemed BUSY during reception of a 20 MHz PPDU in the primary channel.
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (0), 40);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (1), 40);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 40);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 40);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 80);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 80);

  //Secondary channel CCA for BSS 4 should not be deemed BUSY
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (0), 40);
  Simulator::Schedule (Seconds (4.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (1), 40);


  //CASE 5: channel 40 (20 MHz) then channel 36 (20 MHz)
  Simulator::Schedule (Seconds (5.0), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss3Devices.Get (0), bss3Devices.Get (1)->GetAddress ());
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (5), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss2Devices.Get (0), bss2Devices.Get (1)->GetAddress ());

  //BSS 2: transmitter should be in TX state and receiver should be in RX state
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::TX, bss2Devices.Get (0));
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss2Devices.Get (1));

  //BSS 3: transmitter should be in TX state and receiver should be in RX state
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::TX, bss3Devices.Get (0));
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss3Devices.Get (1));

  //BSS 1 and BSS 5: they should be in RX state since a PPDU is received on the primary channel
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss1Devices.Get (0));
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss1Devices.Get (1));
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss5Devices.Get (0));
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss5Devices.Get (1));

  //BSS 4: they should be in IDLE state since no PPDU is received on their channel(s)
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (0));
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (1));

  //Secondary channel CCA for BSS 1 and BBS 5 should be deemed BUSY during transmission or reception of a 20 MHz PPDU in the primary channel
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (0), 40);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (1), 40);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 40);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 40);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 80);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 80);

  //Secondary channel CCA for BSS 4 should not be deemed BUSY
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (0), 40);
  Simulator::Schedule (Seconds (5.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (1), 40);


  //CASE 6: channel 36 (20 MHz) then channel 38 (40 MHz)
  Simulator::Schedule (Seconds (6.0), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss2Devices.Get (0), bss2Devices.Get (1)->GetAddress ());
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (5), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss1Devices.Get (0), bss1Devices.Get (1)->GetAddress ());

  //BSS 2: transmitter should be in TX state and receiver should be in RX state
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::TX, bss2Devices.Get (0));
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss2Devices.Get (1));

  //BSS 1 and BSS 5: they should be in RX state since the PPDU is received on the primary channel
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss1Devices.Get (0));
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss1Devices.Get (1));
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss1Devices.Get (0));
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss1Devices.Get (1));

  //BSS 3: they should be in IDLE state since no PPDU is received on that channel
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss3Devices.Get (0));
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss3Devices.Get (1));

  //BSS 4: they should be in IDLE state since no PPDU is received on their channel(s)
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (0));
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (1));

  //Secondary channel CCA for BSS 1 and BSS 5 should be deemed BUSY during reception of a 20 MHz PPDU in the primary channel.
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (0), 40);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (1), 40);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 40);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 40);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 80);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 80);
  
  //Secondary channel CCA for BSS 4 should not be deemed BUSY
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (0), 40);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (1), 40);

  //BSS 1: transmitter should be in TX state and receiver should be in RX state
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::TX, bss1Devices.Get (0));
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss1Devices.Get (1));

  //BSS 2 and BSS 3: they should be in CCA_BUSY state since a 40 MHz PPDU is transmitted whereas receivers only support 20 MHz PPDUs
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss2Devices.Get (0));
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss2Devices.Get (1));
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss3Devices.Get (0));
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss3Devices.Get (1));

  //BSS 4: they should be in IDLE state since no PPDU is received on their channel(s)
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (0));
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (1));

  //BSS 5: they should be in RX state
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss5Devices.Get (0));
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss5Devices.Get (1));

  //Secondary channel CCA for BSS 1 and BSS 5 should be deemed BUSY during transmission or reception of a 40 MHz PPDU.
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (0), 40);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (1), 40);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 40);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 40);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 80);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 80);

  //Secondary channel CCA for BSS 4 should not be deemed BUSY
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (0), 40);
  Simulator::Schedule (Seconds (6.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (1), 40);


  //CASE 7: channel 38 (40 MHz) then channel 36 (20 MHz)
  Simulator::Schedule (Seconds (7.0), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss1Devices.Get (0), bss1Devices.Get (1)->GetAddress ());
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (5), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss2Devices.Get (0), bss2Devices.Get (1)->GetAddress ());

  //BSS 1: transmitter should be in TX state and receiver should be in RX state
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::TX, bss1Devices.Get (0));
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss1Devices.Get (1));

  //BSS 2 and BSS 3: they should be in CCA_BUSY state since a 40 MHz PPDU is transmitted whereas receivers only support 20 MHz PPDUs
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss2Devices.Get (0));
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss2Devices.Get (1));
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss3Devices.Get (0));
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss3Devices.Get (1));

  //BSS 5: they should be in RX state
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss5Devices.Get (0));
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss5Devices.Get (1));

  //Secondary channel CCA for BSS 1 and BSS 5 should be deemed BUSY during transmission or reception of a 40 MHz PPDU.
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (0), 40);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (1), 40);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 40);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 40);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 80);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 80);

  //Secondary channel CCA for BSS 4 should not be deemed BUSY
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (0), 40);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (1), 40);

  //BSS 2: transmitter should be in TX state and receiver should be in RX state
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::TX, bss2Devices.Get (0));
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss2Devices.Get (1));

  //BSS 1 and BSS 5: they should be in RX state since the PPDU is received on the primary channel
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss1Devices.Get (0));
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss1Devices.Get (1));
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss1Devices.Get (0));
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss1Devices.Get (1));

  //BSS 3: they should be in IDLE state since no PPDU is received on that channel
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss3Devices.Get (0));
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss3Devices.Get (1));

  //BSS 4: they should be in IDLE state since no PPDU is received on their channel(s)
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (0));
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (1));

  //Secondary channel CCA for BSS 1 and BSS 5 should be deemed BUSY during reception of a 20 MHz PPDU in the primary channel.
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (0), 40);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (1), 40);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 40);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 40);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 80);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 80);
  
  //Secondary channel CCA for BSS 4 should not be deemed BUSY
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (0), 40);
  Simulator::Schedule (Seconds (7.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (1), 40);


  //CASE 8: channel 40 (20 MHz) then channel 38 (40 MHz)
  Simulator::Schedule (Seconds (8.0), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss3Devices.Get (0), bss3Devices.Get (1)->GetAddress ());
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (5), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss1Devices.Get (0), bss1Devices.Get (1)->GetAddress ());

  //BSS 3: transmitter should be in TX state and receiver should be in RX state
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::TX, bss3Devices.Get (0));
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss3Devices.Get (1));

  //BSS 1 and BSS 5: they should be in IDLE state since PPDU is received on the secondary 20 MHz channel
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss1Devices.Get (0));
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss1Devices.Get (1));
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss5Devices.Get (0));
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss5Devices.Get (1));

  //BSS 2 and BSS 4: they should be in IDLE state since no PPDU is received on their channel(s)
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss2Devices.Get (0));
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss2Devices.Get (1));
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (0));
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (1));

  //Secondary channel CCA for BSS 1 and BSS 5 should be deemed BUSY since the energy in the secondary channel is above the corresponding CCA threshold.
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (0), 40);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (1), 40);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 40);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 40);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 80);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 80);

  //Secondary channel CCA for BSS 4 should not be deemed BUSY
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (0), 40);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (1), 40);

  //BSS 1: transmitter should be in TX state and receiver should be in RX state
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (400.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::TX, bss1Devices.Get (0));
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (400.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss1Devices.Get (1));

  //BSS 2 and BSS 3: they should be in CCA_BUSY state since a 40 MHz PPDU is transmitted whereas receivers only support 20 MHz PPDUs
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (400.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss2Devices.Get (0));
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (400.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss2Devices.Get (1));
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (400.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss3Devices.Get (0));
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (400.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss3Devices.Get (1));

  //BSS 5: they should be in RX state
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (400.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss5Devices.Get (0));
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (400.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss5Devices.Get (1));

  //Secondary channel CCA for BSS 1 and BSS 5 should be deemed BUSY during transmission or reception of a 40 MHz PPDU.
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (400.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (0), 40);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (400.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (1), 40);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (400.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 40);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (400.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 40);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (400.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 80);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (400.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 80);

  //Secondary channel CCA for BSS 4 should not be deemed BUSY
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (400.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (0), 40);
  Simulator::Schedule (Seconds (8.0) + MicroSeconds (400.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (1), 40);


  //CASE 9: channel 38 (40 MHz) then channel 40 (20 MHz)
  Simulator::Schedule (Seconds (9.0), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss1Devices.Get (0), bss1Devices.Get (1)->GetAddress ());
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (5), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss3Devices.Get (0), bss3Devices.Get (1)->GetAddress ());

  //BSS 1: transmitter should be in TX state and receiver should be in RX state
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::TX, bss1Devices.Get (0));
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss1Devices.Get (1));

  //BSS 2 and BSS 3: they should be in CCA_BUSY state since a 40 MHz PPDU is transmitted whereas receivers only support 20 MHz PPDUs
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss2Devices.Get (0));
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss2Devices.Get (1));
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss3Devices.Get (0));
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss3Devices.Get (1));

  //BSS 5: they should be in RX state
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss5Devices.Get (0));
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss5Devices.Get (1));

  //Secondary channel CCA for BSS 1 and BSS 5 should be deemed BUSY during transmission or reception of a 40 MHz PPDU.
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (0), 40);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (1), 40);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 40);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 40);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 80);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 80);

  //Secondary channel CCA for BSS 4 should not be deemed BUSY
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (0), 40);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (1), 40);

  //BSS 3: transmitter should be in TX state and receiver should be in RX state
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::TX, bss3Devices.Get (0));
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss3Devices.Get (1));

  //BSS 1 and BSS 5: they should be in IDLE state since PPDU is received on the secondary 20 MHz channel
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss1Devices.Get (0));
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss1Devices.Get (1));
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss5Devices.Get (0));
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss5Devices.Get (1));

  //BSS 2 and BSS 4: they should be in IDLE state since no PPDU is received on their channel(s)
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss2Devices.Get (0));
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss2Devices.Get (1));
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (0));
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (1));

  //Secondary channel CCA for BSS 1 and BSS 5 should be deemed BUSY since the energy in the secondary channel is above the corresponding CCA threshold.
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (0), 40);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (1), 40);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 40);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 40);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 80);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 80);

  //Secondary channel CCA for BSS 4 should not be deemed BUSY
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (0), 40);
  Simulator::Schedule (Seconds (9.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (1), 40);


  //CASE 10: channel 46 (40 MHz) then channel 42 (80 MHz)
  Simulator::Schedule (Seconds (10.0), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss4Devices.Get (0), bss4Devices.Get (1)->GetAddress ());
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (5), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss5Devices.Get (0), bss5Devices.Get (1)->GetAddress ());

  //BSS 4: transmitter should be in TX state and receiver should be in RX state
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::TX, bss4Devices.Get (0));
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss4Devices.Get (1));

  //BSS 5: they should be in IDLE state since PPDU is received on the secondary 40 MHz channel
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss5Devices.Get (0));
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss5Devices.Get (1));

  //BSS 1, BSS 2 and BSS 3: they should be in IDLE state since no PPDU is received on their channel(s)
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss1Devices.Get (0));
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss1Devices.Get (1));
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss2Devices.Get (0));
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss2Devices.Get (1));
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss3Devices.Get (0));
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss3Devices.Get (1));

  //Secondary channel CCA for BSS 1 should not be deemed BUSY
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss1Devices.Get (0), 40);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss1Devices.Get (1), 40);

  //Secondary channel CCA for BSS 4 should be deemed BUSY during transmission or reception of a 40 MHz PPDU
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss4Devices.Get (0), 40);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss4Devices.Get (1), 40);

  //Secondary channel CCA for BSS 5 should be deemed BUSY since the energy in the 40 MHz secondary channel is above the corresponding CCA threshold
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 80);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 80);

  //Secondary channel CCA for BSS 5 should not be deemed BUSY for the primary 40 MHz channel
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss5Devices.Get (0), 40);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss5Devices.Get (1), 40);

  //BSS 5: transmitter should be in TX state and receiver should be in RX state
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (250.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::TX, bss5Devices.Get (0));
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (250.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss5Devices.Get (1));

  //BSS 1, BSS 2, BSS 3 and BSS 4: they should be in CCA_BUSY state since a 80 MHz PPDU is transmitted whereas receivers does not support reception of 80 MHz PPDUs
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (250.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss1Devices.Get (0));
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (250.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss1Devices.Get (1));
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (250.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss2Devices.Get (0));
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (250.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss2Devices.Get (1));
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (250.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss3Devices.Get (0));
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (250.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss3Devices.Get (1));
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (250.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss4Devices.Get (0));
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (250.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss4Devices.Get (1));

  //Secondary channel CCA for all BSSs should be deemed BUSY during transmission or reception of a 80 MHz PPDU.
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (250.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (0), 40);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (250.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (1), 40);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (250.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss4Devices.Get (0), 40);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (250.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss4Devices.Get (1), 40);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (250.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 40);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (250.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 40);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (250.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 80);
  Simulator::Schedule (Seconds (10.0) + MicroSeconds (250.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 80);

  
  //CASE 11: channel 40 (20 MHz) then channel 42 (80 MHz)
  Simulator::Schedule (Seconds (11.0), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss3Devices.Get (0), bss3Devices.Get (1)->GetAddress ());
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (5), &TestStaticChannelBondingChannelAccess::SendPacket, this, bss5Devices.Get (0), bss5Devices.Get (1)->GetAddress ());

  //BSS 3: transmitter should be in TX state and receiver should be in RX state
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::TX, bss3Devices.Get (0));
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss3Devices.Get (1));

  //BSS 1 and BSS 5: they should be in IDLE state since PPDU is received on the secondary 20 MHz channel
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss1Devices.Get (0));
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss1Devices.Get (1));
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss5Devices.Get (0));
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss5Devices.Get (1));

  //BSS 2 and BSS 4: they should be in IDLE state since no PPDU is received on their channel(s)
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss2Devices.Get (0));
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss2Devices.Get (1));
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (0));
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::IDLE, bss4Devices.Get (1));

  //Secondary channel CCA for BSS 1 and BSS 5 should be deemed BUSY since the energy in the secondary channel is above the corresponding CCA threshold.
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (0), 40);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (1), 40);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 40);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 40);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 80);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 80);

  //Secondary channel CCA for BSS 4 should not be deemed BUSY
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (0), 40);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (50.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, true, bss4Devices.Get (1), 40);

  //BSS 5: transmitter should be in TX state and receiver should be in RX state
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::TX, bss5Devices.Get (0));
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::RX, bss5Devices.Get (1));

  //BSS 1, BSS 2, BSS 3 and BSS 4: they should be in CCA_BUSY state since a 80 MHz PPDU is transmitted whereas receivers does not support reception of 80 MHz PPDUs
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss1Devices.Get (0));
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss1Devices.Get (1));
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss2Devices.Get (0));
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss2Devices.Get (1));
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss3Devices.Get (0));
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss3Devices.Get (1));
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss4Devices.Get (0));
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckPhyState, this, WifiPhyState::CCA_BUSY, bss4Devices.Get (1));

  //Secondary channel CCA for all BSSs should be deemed BUSY during transmission or reception of a 80 MHz PPDU.
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (0), 40);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss1Devices.Get (1), 40);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss4Devices.Get (0), 40);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss4Devices.Get (1), 40);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 40);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 40);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (0), 80);
  Simulator::Schedule (Seconds (11.0) + MicroSeconds (350.0), &TestStaticChannelBondingChannelAccess::CheckSecondaryChannelStatus, this, false, bss5Devices.Get (1), 80);

  Simulator::Run ();

  Simulator::Destroy ();
}

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief wifi channel bonding test suite
 */
class WifiChannelBondingTestSuite : public TestSuite
{
public:
  WifiChannelBondingTestSuite ();
};

WifiChannelBondingTestSuite::WifiChannelBondingTestSuite ()
  : TestSuite ("wifi-channel-bonding", UNIT)
{
  AddTestCase (new TestStaticChannelBondingSnr, TestCase::QUICK);
  AddTestCase (new TestStaticChannelBondingChannelAccess, TestCase::QUICK);
  AddTestCase (new TestConstantThresholdDynamicChannelBonding, TestCase::QUICK);
  AddTestCase (new TestDynamicThresholdDynamicChannelBonding, TestCase::QUICK);
  AddTestCase (new TestEffectiveSnrCalculations, TestCase::QUICK);
}

static WifiChannelBondingTestSuite wifiChannelBondingTestSuite; ///< the test suite

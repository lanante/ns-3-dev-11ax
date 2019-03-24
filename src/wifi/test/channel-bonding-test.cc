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

#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/wifi-spectrum-value-helper.h"
#include "ns3/spectrum-wifi-phy.h"
#include "ns3/nist-error-rate-model.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/wifi-mac-trailer.h"
#include "ns3/wifi-phy-header.h"
#include "ns3/wifi-phy-tag.h"
#include "ns3/wifi-utils.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/constant-position-mobility-model.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiChannelBondingTest");

static const uint8_t CHANNEL_NUMBER_BSS1 = 36;
static const uint8_t CHANNEL_NUMBER_BSS2 = 40;
static const uint8_t CHANNEL_NUMBER_BSS3 = 38;
static const uint8_t CHANNEL_NUMBER_BSS4 = 38;
static const uint32_t FREQUENCY_BSS1 = 5180; // MHz
static const uint32_t FREQUENCY_BSS2 = 5200; // MHz
static const uint32_t FREQUENCY_BSS3 = 5190; // MHz
static const uint32_t FREQUENCY_BSS4 = 5190; // MHz
static const uint16_t CHANNEL_WIDTH_BSS1 = 20; // MHz
static const uint16_t CHANNEL_WIDTH_BSS2 = 20; // MHz
static const uint16_t CHANNEL_WIDTH_BSS3 = 40; // MHz
static const uint16_t CHANNEL_WIDTH_BSS4 = 40; // MHz
static const SecondaryChannelOffset SECONDARY_BSS3 = UPPER;
static const SecondaryChannelOffset SECONDARY_BSS4 = LOWER;

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief channel bonding
 *
 * In this test, we have four 802.11n transmitters and four 802.11n receivers.
 * A BSS is composed by one transmitter and one receiver.
 *
 * The first BSS occupies channel 36 and a channel width of 20 MHz.
 * The second BSS operates on channel 40 with a channel width of 20 MHz.
 * Both BSS 3 and BSS 4 makes uses of channel bonding with a 40 MHz channel width
 * and operates on channel 38 (= 36 + 40). The only difference between them is that
 * BSS 3 has channel 36 as primary channel, whereas BSS 4 has channel 40 has primary channel.
 *
 */
class TestChannelBonding : public TestCase
{
public:
  TestChannelBonding ();
  virtual ~TestChannelBonding ();

protected:
  virtual void DoSetup (void);
  Ptr<SpectrumWifiPhy> m_rxPhyBss1; ///< RX Phy BSS #1
  Ptr<SpectrumWifiPhy> m_rxPhyBss2; ///< RX Phy BSS #2
  Ptr<SpectrumWifiPhy> m_rxPhyBss3; ///< RX Phy BSS #3
  Ptr<SpectrumWifiPhy> m_rxPhyBss4; ///< RX Phy BSS #4
  Ptr<SpectrumWifiPhy> m_txPhyBss1; ///< TX Phy BSS #1
  Ptr<SpectrumWifiPhy> m_txPhyBss2; ///< TX Phy BSS #2
  Ptr<SpectrumWifiPhy> m_txPhyBss3; ///< TX Phy BSS #3
  Ptr<SpectrumWifiPhy> m_txPhyBss4; ///< TX Phy BSS #4

  double m_expectedSnrBss1;  ///< Expected SNR for RX Phy #1
  double m_expectedSnrBss2;  ///< Expected SNR for RX Phy #2
  double m_expectedSnrBss3;  ///< Expected SNR for RX Phy #3
  double m_expectedSnrBss4;  ///< Expected SNR for RX Phy #4
  bool m_initializedSnrBss1; ///< Flag whether expected SNR for BSS 1 has been set
  bool m_initializedSnrBss2; ///< Flag whether expected SNR for BSS 2 has been set
  bool m_initializedSnrBss3; ///< Flag whether expected SNR for BSS 3 has been set
  bool m_initializedSnrBss4; ///< Flag whether expected SNR for BSS 4 has been set

  bool m_payloadReceivedSuccessBss1; ///< Flag whether a packet has been successfully received by BSS 1
  bool m_payloadReceivedSuccessBss2; ///< Flag whether a packet has been successfully received by BSS 2
  bool m_payloadReceivedSuccessBss3; ///< Flag whether a packet has been successfully received by BSS 3
  bool m_payloadReceivedSuccessBss4; ///< Flag whether a packet has been successfully received by BSS 4
  bool m_payloadReceivedErrorBss1;   ///< Flag whether a packet has been unsuccessfully received by BSS 1
  bool m_payloadReceivedErrorBss2;   ///< Flag whether a packet has been unsuccessfully received by BSS 2
  bool m_payloadReceivedErrorBss3;   ///< Flag whether a packet has been unsuccessfully received by BSS 3
  bool m_payloadReceivedErrorBss4;   ///< Flag whether a packet has been unsuccessfully received by BSS 4

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
   * \param expectedPayloadSuccess whether the payload should have been successfully received
   * \param expectedPayloadError whether the payload should have been unsuccessfully received
   * \param bss the BSS number
   */
  void VerifyResultsForBss (bool expectedPayloadSuccess, bool expectedPayloadError, uint8_t bss);

  /**
   * Reset the results
   */
  void Reset (void);

private:
  virtual void DoRun (void);
};

TestChannelBonding::TestChannelBonding ()
  : TestCase ("Channel bonding test"),
    m_expectedSnrBss1 (0.0),
    m_expectedSnrBss2 (0.0),
    m_expectedSnrBss3 (0.0),
    m_expectedSnrBss4 (0.0),
    m_initializedSnrBss1 (false),
    m_initializedSnrBss2 (false),
    m_initializedSnrBss3 (false),
    m_initializedSnrBss4 (false),
    m_payloadReceivedSuccessBss1 (false),
    m_payloadReceivedSuccessBss2 (false),
    m_payloadReceivedSuccessBss3 (false),
    m_payloadReceivedSuccessBss4 (false),
    m_payloadReceivedErrorBss1 (false),
    m_payloadReceivedErrorBss2 (false),
    m_payloadReceivedErrorBss3 (false),
    m_payloadReceivedErrorBss4 (false)
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
TestChannelBonding::Reset (void)
{
  m_expectedSnrBss1 = 0.0;
  m_expectedSnrBss2 = 0.0;
  m_expectedSnrBss3 = 0.0;
  m_expectedSnrBss4 = 0.0;
  m_initializedSnrBss1 = false;
  m_initializedSnrBss2 = false;
  m_initializedSnrBss3 = false;
  m_initializedSnrBss4 = false;
  m_payloadReceivedSuccessBss1 = false;
  m_payloadReceivedSuccessBss2 = false;
  m_payloadReceivedSuccessBss3 = false;
  m_payloadReceivedSuccessBss4 = false;
  m_payloadReceivedErrorBss1 = false;
  m_payloadReceivedErrorBss2 = false;
  m_payloadReceivedErrorBss3 = false;
  m_payloadReceivedErrorBss4 = false;
}

void
TestChannelBonding::SetExpectedSnrForBss (double snr, uint8_t bss)
{
  if (bss == 1)
    {
      m_expectedSnrBss1 = snr;
      m_initializedSnrBss1 = true;
    }
  else if (bss == 2)
    {
      m_expectedSnrBss2 = snr;
      m_initializedSnrBss2 = true;
    }
  else if (bss == 3)
    {
      m_expectedSnrBss3 = snr;
      m_initializedSnrBss3 = true;
    }
  else if (bss == 4)
    {
      m_expectedSnrBss4 = snr;
      m_initializedSnrBss4 = true;
    }
}

void
TestChannelBonding::VerifyResultsForBss (bool expectedPayloadSuccess, bool expectedPayloadError, uint8_t bss)
{
  if (bss == 1)
    {
      NS_TEST_ASSERT_MSG_EQ (m_payloadReceivedSuccessBss1, expectedPayloadSuccess, "m_payloadReceivedSuccessBss1 is not equal to expectedPayloadSuccess");
      NS_TEST_ASSERT_MSG_EQ (m_payloadReceivedErrorBss1, expectedPayloadError, "m_payloadReceivedErrorBss1 is not equal to expectedPayloadError");
    }
  else if (bss == 2)
    {
      NS_TEST_ASSERT_MSG_EQ (m_payloadReceivedSuccessBss2, expectedPayloadSuccess, "m_payloadReceivedSuccessBss2 is not equal to expectedPayloadSuccess");
      NS_TEST_ASSERT_MSG_EQ (m_payloadReceivedErrorBss2, expectedPayloadError, "m_payloadReceivedErrorBss2 is not equal to expectedPayloadError");
    }
  else if (bss == 3)
    {
      NS_TEST_ASSERT_MSG_EQ (m_payloadReceivedSuccessBss3, expectedPayloadSuccess, "m_payloadReceivedSuccessBss3 is not equal to expectedPayloadSuccess");
      NS_TEST_ASSERT_MSG_EQ (m_payloadReceivedErrorBss3, expectedPayloadError, "m_payloadReceivedErrorBss3 is not equal to expectedPayloadError");
    }
  else if (bss == 4)
    {
      NS_TEST_ASSERT_MSG_EQ (m_payloadReceivedSuccessBss4, expectedPayloadSuccess, "m_payloadReceivedSuccessBss4 is not equal to expectedPayloadSuccess");
      NS_TEST_ASSERT_MSG_EQ (m_payloadReceivedErrorBss4, expectedPayloadError, "m_payloadReceivedErrorBss4 is not equal to expectedPayloadError");
    }
}

void
TestChannelBonding::SendPacket (uint8_t bss)
{
  Ptr<SpectrumWifiPhy> phy;
  uint16_t channelWidth = 20;
  uint32_t payloadSize = 1000;
  if (bss == 1)
    {
      phy = m_txPhyBss1;
      channelWidth = CHANNEL_WIDTH_BSS1;
      payloadSize = 1001;
    }
  else if (bss == 2)
    {
      phy = m_txPhyBss2;
      channelWidth = CHANNEL_WIDTH_BSS2;
      payloadSize = 1002;
    }
  else if (bss == 3)
    {
      phy = m_txPhyBss3;
      channelWidth = CHANNEL_WIDTH_BSS3;
      payloadSize = 2100; //This is chosen such that the transmission time on 40 MHz will be the same as for packets sent on 20 MHz
    }
  else if (bss == 4)
    {
      phy = m_txPhyBss4;
      channelWidth = CHANNEL_WIDTH_BSS4;
      payloadSize = 2101; //This is chosen such that the transmission time on 40 MHz will be the same as for packets sent on 20 MHz
    }

  WifiTxVector txVector = WifiTxVector (WifiPhy::GetHtMcs7 (), 0, WIFI_PREAMBLE_HT_MF, 800, 1, 1, 0, channelWidth, false, false);
  MpduType mpdutype = NORMAL_MPDU;

  Ptr<Packet> pkt = Create<Packet> (payloadSize);
  WifiMacHeader hdr;
  WifiMacTrailer trailer;

  hdr.SetType (WIFI_MAC_QOSDATA);
  hdr.SetQosTid (0);
  uint32_t size = pkt->GetSize () + hdr.GetSize () + trailer.GetSerializedSize ();
  Time txDuration = phy->CalculateTxDuration (size, txVector, phy->GetFrequency (), mpdutype, 0);
  hdr.SetDuration (txDuration);

  pkt->AddHeader (hdr);
  pkt->AddTrailer (trailer);

  HtSigHeader htSig;
  htSig.SetMcs (txVector.GetMode ().GetMcsValue ());
  htSig.SetChannelWidth (channelWidth);
  htSig.SetHtLength (size);
  htSig.SetAggregation (txVector.IsAggregation ());
  htSig.SetShortGuardInterval (txVector.GetGuardInterval () == 400);
  pkt->AddHeader (htSig);

  LSigHeader sig;
  pkt->AddHeader (sig);

  WifiPhyTag tag (txVector.GetPreambleType (), txVector.GetMode ().GetModulationClass (), 1);
  pkt->AddPacketTag (tag);

  phy->StartTx (pkt, txVector, txDuration);
}

void
TestChannelBonding::RxCallback (std::string context, Ptr<const Packet> p, RxPowerWattPerChannelBand rxPowersW)
{
  uint32_t size = p->GetSize ();
  if (context == "BSS1") //RX is in BSS 1
    {
      auto band = std::make_pair (FREQUENCY_BSS1, 20);
      auto it = rxPowersW.find (band);
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 1 received packet with size " << size << " and power in 20 MHz band: " << WToDbm (it->second));
      if (size == 1031) //TX is in BSS 1
        {
          double expectedRxPowerMin = - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 1 RX PHY is too low");
        }
      else if (size == 1032) //TX is in BSS 2
        {
          double expectedRxPowerMax = - 20 /* rejection */ - 50 /* loss */;
          NS_TEST_EXPECT_MSG_LT (WToDbm (it->second), expectedRxPowerMax, "Received power for BSS 2 RX PHY is too high");
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
      auto band = std::make_pair (FREQUENCY_BSS2, 20);
      auto it = rxPowersW.find (band);
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 2 received packet with size " << size << " and power in 20 MHz band: " << WToDbm (it->second));
      if (size == 1031) //TX is in BSS 1
        {
          double expectedRxPowerMax = - 20 /* rejection */ - 50 /* loss */;
          NS_TEST_EXPECT_MSG_LT (WToDbm (it->second), expectedRxPowerMax, "Received power for BSS 2 RX PHY is too high");
        }
      else if (size == 1032) //TX is in BSS 2
        {
          double expectedRxPowerMin = - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 1 RX PHY is too low");
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
  else if (context == "BSS3") //RX is in BSS 3
    {
      auto band = std::make_pair (FREQUENCY_BSS1, 20);
      auto it = rxPowersW.find (band);
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 3 received packet with size " << size << " and power in primary 20 MHz band: " << WToDbm (it->second));
      if (size == 1031) //TX is in BSS 1
        {
          double expectedRxPowerMin = - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power in primary channel for BSS 3 RX PHY is too low");
        }
      else if (size == 1032) //TX is in BSS 2
        {
          double expectedRxPowerMax = - 20 /* rejection */ - 50 /* loss */;
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

      band = std::make_pair (FREQUENCY_BSS2, 20);
      it = rxPowersW.find (band);
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 3 received packet with size " << size << " and power in secondary 20 MHz band: " << WToDbm (it->second));
      if (size == 1031) //TX is in BSS 1
        {
          double expectedRxPowerMax = - 20 /* rejection */ - 50 /* loss */;
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

      band = std::make_pair (FREQUENCY_BSS3, 40);
      it = rxPowersW.find (band);
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 3 received packet with size " << size << " and power in 40 MHz band: " << WToDbm (it->second));
      double expectedRxPowerMin = - 50 /* loss */ - 1 /* precision */;
      NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 3 RX PHY is too low");
    }
  else if (context == "BSS4") //RX is in BSS 4
    {
      auto band = std::make_pair (FREQUENCY_BSS2, 20);
      auto it = rxPowersW.find (band);
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 4 received packet with size " << size << " and power in primary 20 MHz band: " << WToDbm (it->second));
      if (size == 1031) //TX is in BSS 1
        {
          double expectedRxPowerMax = - 20 /* rejection */ - 50 /* loss */;
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

      band = std::make_pair (FREQUENCY_BSS1, 20);
      it = rxPowersW.find (band);
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 4 received packet with size " << size << " and power in secondary 20 MHz band: " << WToDbm (it->second));
      if (size == 1031) //TX is in BSS 1
        {
          double expectedRxPowerMin = - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power in primary channel for BSS 4 RX PHY is too low");
        }
      else if (size == 1032) //TX is in BSS 2
        {
          double expectedRxPowerMax = - 20 /* rejection */ - 50 /* loss */;
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

      band = std::make_pair (FREQUENCY_BSS3, 40);
      it = rxPowersW.find (band);
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 4 received packet with size " << size << " and power in 40 MHz band: " << WToDbm (it->second));
      double expectedRxPowerMin = - 50 /* loss */ - 1 /* precision */;
      NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 4 RX PHY is too low");
    }
}

void
TestChannelBonding::RxOkCallback (std::string context, Ptr<const Packet> p, double snr, WifiMode mode, WifiPreamble preamble)
{
  NS_LOG_INFO ("RxOkCallback: BSS=" << context << " SNR=" << RatioToDb (snr));
  if (context == "BSS1")
    {
      m_payloadReceivedSuccessBss1 = true;
      if (m_initializedSnrBss1)
        {
          NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), m_expectedSnrBss1, 0.2, "Unexpected SNR value");
        }
    }
  else if (context == "BSS2")
    {
      m_payloadReceivedSuccessBss2 = true;
      if (m_initializedSnrBss2)
        {
          NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), m_expectedSnrBss2, 0.2, "Unexpected SNR value");
        }
    }
  else if (context == "BSS3")
    {
      m_payloadReceivedSuccessBss3 = true;
      if (m_initializedSnrBss3)
        {
          NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), m_expectedSnrBss3, 0.2, "Unexpected SNR value");
        }
    }
  else if (context == "BSS4")
    {
      m_payloadReceivedSuccessBss4 = true;
      if (m_initializedSnrBss4)
        {
          NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), m_expectedSnrBss4, 0.2, "Unexpected SNR value");
        }
    }
}

void
TestChannelBonding::RxErrorCallback (std::string context, Ptr<const Packet> p, double snr)
{
  NS_LOG_INFO ("RxErrorCallback: BSS=" << context << " SNR=" << RatioToDb (snr));
  if ((context == "BSS1") && m_initializedSnrBss1)
    {
      m_payloadReceivedErrorBss1 = true;
      NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), m_expectedSnrBss1, 0.2, "Unexpected SNR value");
    }
  else if ((context == "BSS2") && m_initializedSnrBss2)
    {
      m_payloadReceivedErrorBss2 = true;
      NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), m_expectedSnrBss2, 0.2, "Unexpected SNR value");
    }
  else if ((context == "BSS3") && m_initializedSnrBss3)
    {
      m_payloadReceivedErrorBss3 = true;
      NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), m_expectedSnrBss3, 0.2, "Unexpected SNR value");
    }
  else if ((context == "BSS4") && m_initializedSnrBss4)
    {
      m_payloadReceivedErrorBss4 = true;
      NS_TEST_EXPECT_MSG_EQ_TOL (RatioToDb (snr), m_expectedSnrBss4, 0.2, "Unexpected SNR value");
    }
}

TestChannelBonding::~TestChannelBonding ()
{
  m_rxPhyBss1 = 0;
  m_rxPhyBss2 = 0;
  m_rxPhyBss3 = 0;
  m_rxPhyBss4 = 0;
  m_txPhyBss1 = 0;
  m_txPhyBss2 = 0;
  m_txPhyBss3 = 0;
  m_txPhyBss4 = 0;
}

void
TestChannelBonding::DoSetup (void)
{
  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();

  Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel> ();
  lossModel->SetDefaultLoss (50); // set default loss to 50 dB for all links
  channel->AddPropagationLossModel (lossModel);

  Ptr<ConstantSpeedPropagationDelayModel> delayModel
    = CreateObject<ConstantSpeedPropagationDelayModel> ();
  channel->SetPropagationDelayModel (delayModel);

  Ptr<ErrorRateModel> error = CreateObject<NistErrorRateModel> ();

  m_rxPhyBss1 = CreateObject<SpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> rxMobilityBss1 = CreateObject<ConstantPositionMobilityModel> ();
  rxMobilityBss1->SetPosition (Vector (1.0, 0.0, 0.0));
  m_rxPhyBss1->SetMobility (rxMobilityBss1);
  m_rxPhyBss1->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_rxPhyBss1->CreateWifiSpectrumPhyInterface (nullptr);
  m_rxPhyBss1->SetChannel (channel);
  m_rxPhyBss1->SetErrorRateModel (error);
  m_rxPhyBss1->SetChannelNumber (CHANNEL_NUMBER_BSS1);
  m_rxPhyBss1->SetFrequency (FREQUENCY_BSS1);
  m_rxPhyBss1->SetChannelWidth (CHANNEL_WIDTH_BSS1);
  m_rxPhyBss1->SetTxPowerStart(0);
  m_rxPhyBss1->SetTxPowerEnd(0);
  m_rxPhyBss1->Initialize ();

  m_txPhyBss1 = CreateObject<SpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> txMobilityBss1 = CreateObject<ConstantPositionMobilityModel> ();
  txMobilityBss1->SetPosition (Vector (0.0, 0.0, 0.0));
  m_txPhyBss1->SetMobility (txMobilityBss1);
  m_txPhyBss1->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_txPhyBss1->CreateWifiSpectrumPhyInterface (nullptr);
  m_txPhyBss1->SetChannel (channel);
  m_txPhyBss1->SetErrorRateModel (error);
  m_txPhyBss1->SetChannelNumber (CHANNEL_NUMBER_BSS1);
  m_txPhyBss1->SetFrequency (FREQUENCY_BSS1);
  m_txPhyBss1->SetChannelWidth (CHANNEL_WIDTH_BSS1);
  m_txPhyBss1->SetTxPowerStart(0);
  m_txPhyBss1->SetTxPowerEnd(0);
  m_txPhyBss1->Initialize ();

  m_rxPhyBss2 = CreateObject<SpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> rxMobilityBss2 = CreateObject<ConstantPositionMobilityModel> ();
  rxMobilityBss2->SetPosition (Vector (1.0, 10.0, 0.0));
  m_rxPhyBss2->SetMobility (rxMobilityBss2);
  m_rxPhyBss2->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_rxPhyBss2->CreateWifiSpectrumPhyInterface (nullptr);
  m_rxPhyBss2->SetChannel (channel);
  m_rxPhyBss2->SetErrorRateModel (error);
  m_rxPhyBss2->SetChannelNumber (CHANNEL_NUMBER_BSS2);
  m_rxPhyBss2->SetFrequency (FREQUENCY_BSS2);
  m_rxPhyBss2->SetChannelWidth (CHANNEL_WIDTH_BSS2);
  m_rxPhyBss2->SetTxPowerStart(0);
  m_rxPhyBss2->SetTxPowerEnd(0);
  m_rxPhyBss2->Initialize ();

  m_txPhyBss2 = CreateObject<SpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> txMobilityBss2 = CreateObject<ConstantPositionMobilityModel> ();
  txMobilityBss2->SetPosition (Vector (0.0, 10.0, 0.0));
  m_txPhyBss2->SetMobility (txMobilityBss2);
  m_txPhyBss2->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_txPhyBss2->CreateWifiSpectrumPhyInterface (nullptr);
  m_txPhyBss2->SetChannel (channel);
  m_txPhyBss2->SetErrorRateModel (error);
  m_txPhyBss2->SetChannelNumber (CHANNEL_NUMBER_BSS2);
  m_txPhyBss2->SetFrequency (FREQUENCY_BSS2);
  m_txPhyBss2->SetChannelWidth (CHANNEL_WIDTH_BSS2);
  m_txPhyBss2->SetTxPowerStart(0);
  m_txPhyBss2->SetTxPowerEnd(0);
  m_txPhyBss2->Initialize ();

  m_rxPhyBss3 = CreateObject<SpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> rxMobilityBss3 = CreateObject<ConstantPositionMobilityModel> ();
  rxMobilityBss3->SetPosition (Vector (1.0, 20.0, 0.0));
  m_rxPhyBss3->SetMobility (rxMobilityBss3);
  m_rxPhyBss3->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_rxPhyBss3->CreateWifiSpectrumPhyInterface (nullptr);
  m_rxPhyBss3->SetChannel (channel);
  m_rxPhyBss3->SetErrorRateModel (error);
  m_rxPhyBss3->SetChannelNumber (CHANNEL_NUMBER_BSS3);
  m_rxPhyBss3->SetFrequency (FREQUENCY_BSS3);
  m_rxPhyBss3->SetChannelWidth (CHANNEL_WIDTH_BSS3);
  m_rxPhyBss3->SetSecondaryChannelOffset (SECONDARY_BSS3);
  m_rxPhyBss3->SetTxPowerStart(0);
  m_rxPhyBss3->SetTxPowerEnd(0);
  m_rxPhyBss3->Initialize ();

  m_txPhyBss3 = CreateObject<SpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> txMobilityBss3 = CreateObject<ConstantPositionMobilityModel> ();
  txMobilityBss3->SetPosition (Vector (0.0, 20.0, 0.0));
  m_txPhyBss3->SetMobility (txMobilityBss3);
  m_txPhyBss3->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_txPhyBss3->CreateWifiSpectrumPhyInterface (nullptr);
  m_txPhyBss3->SetChannel (channel);
  m_txPhyBss3->SetErrorRateModel (error);
  m_txPhyBss3->SetChannelNumber (CHANNEL_NUMBER_BSS3);
  m_txPhyBss3->SetFrequency (FREQUENCY_BSS3);
  m_txPhyBss3->SetChannelWidth (CHANNEL_WIDTH_BSS3);
  m_txPhyBss3->SetSecondaryChannelOffset (SECONDARY_BSS3);
  m_txPhyBss3->SetTxPowerStart(0);
  m_txPhyBss3->SetTxPowerEnd(0);
  m_txPhyBss3->Initialize ();

  m_rxPhyBss4 = CreateObject<SpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> rxMobilityBss4 = CreateObject<ConstantPositionMobilityModel> ();
  rxMobilityBss4->SetPosition (Vector (1.0, 30.0, 0.0));
  m_rxPhyBss4->SetMobility (rxMobilityBss4);
  m_rxPhyBss4->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_rxPhyBss4->CreateWifiSpectrumPhyInterface (nullptr);
  m_rxPhyBss4->SetChannel (channel);
  m_rxPhyBss4->SetErrorRateModel (error);
  m_rxPhyBss4->SetChannelNumber (CHANNEL_NUMBER_BSS4);
  m_rxPhyBss4->SetFrequency (FREQUENCY_BSS4);
  m_rxPhyBss4->SetChannelWidth (CHANNEL_WIDTH_BSS4);
  m_rxPhyBss4->SetSecondaryChannelOffset (SECONDARY_BSS4);
  m_rxPhyBss4->SetTxPowerStart(0);
  m_rxPhyBss4->SetTxPowerEnd(0);
  m_rxPhyBss4->Initialize ();

  m_txPhyBss4 = CreateObject<SpectrumWifiPhy> ();
  Ptr<ConstantPositionMobilityModel> txMobilityBss4 = CreateObject<ConstantPositionMobilityModel> ();
  txMobilityBss4->SetPosition (Vector (0.0, 30.0, 0.0));
  m_txPhyBss4->SetMobility (txMobilityBss4);
  m_txPhyBss4->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  m_txPhyBss4->CreateWifiSpectrumPhyInterface (nullptr);
  m_txPhyBss4->SetChannel (channel);
  m_txPhyBss4->SetErrorRateModel (error);
  m_txPhyBss4->SetChannelNumber (CHANNEL_NUMBER_BSS4);
  m_txPhyBss4->SetFrequency (FREQUENCY_BSS4);
  m_txPhyBss4->SetChannelWidth (CHANNEL_WIDTH_BSS4);
  m_txPhyBss4->SetSecondaryChannelOffset (SECONDARY_BSS4);
  m_txPhyBss4->SetTxPowerStart(0);
  m_txPhyBss4->SetTxPowerEnd(0);
  m_txPhyBss4->Initialize ();

  m_rxPhyBss1->TraceConnect ("PhyRxBegin", "BSS1", MakeCallback (&TestChannelBonding::RxCallback, this));
  m_rxPhyBss2->TraceConnect ("PhyRxBegin", "BSS2", MakeCallback (&TestChannelBonding::RxCallback, this));
  m_rxPhyBss3->TraceConnect ("PhyRxBegin", "BSS3", MakeCallback (&TestChannelBonding::RxCallback, this));
  m_rxPhyBss4->TraceConnect ("PhyRxBegin", "BSS4", MakeCallback (&TestChannelBonding::RxCallback, this));
  m_rxPhyBss1->GetState()->TraceConnect ("RxOk", "BSS1", MakeCallback (&TestChannelBonding::RxOkCallback, this));
  m_rxPhyBss2->GetState()->TraceConnect ("RxOk", "BSS2", MakeCallback (&TestChannelBonding::RxOkCallback, this));
  m_rxPhyBss3->GetState()->TraceConnect ("RxOk", "BSS3", MakeCallback (&TestChannelBonding::RxOkCallback, this));
  m_rxPhyBss4->GetState()->TraceConnect ("RxOk", "BSS4", MakeCallback (&TestChannelBonding::RxOkCallback, this));
  m_rxPhyBss1->GetState()->TraceConnect ("RxError", "BSS1", MakeCallback (&TestChannelBonding::RxErrorCallback, this));
  m_rxPhyBss2->GetState()->TraceConnect ("RxError", "BSS2", MakeCallback (&TestChannelBonding::RxErrorCallback, this));
  m_rxPhyBss3->GetState()->TraceConnect ("RxError", "BSS3", MakeCallback (&TestChannelBonding::RxErrorCallback, this));
  m_rxPhyBss4->GetState()->TraceConnect ("RxError", "BSS4", MakeCallback (&TestChannelBonding::RxErrorCallback, this));
}

void
TestChannelBonding::DoRun (void)
{
  RngSeedManager::SetSeed (1);
  RngSeedManager::SetRun (1);
  int64_t streamNumber = 0;
  m_rxPhyBss1->AssignStreams (streamNumber);
  m_rxPhyBss2->AssignStreams (streamNumber);
  m_rxPhyBss3->AssignStreams (streamNumber);
  m_rxPhyBss4->AssignStreams (streamNumber);
  m_txPhyBss1->AssignStreams (streamNumber);
  m_txPhyBss2->AssignStreams (streamNumber);
  m_txPhyBss3->AssignStreams (streamNumber);
  m_txPhyBss4->AssignStreams (streamNumber);

  //CASE 1: each BSS send a packet on its channel to verify the received power per band for each receiver
  //and whether the packet is successfully received or not.*/

  //CASE 1A: BSS 1
  Simulator::Schedule (Seconds (0.9), &TestChannelBonding::Reset, this);
  Simulator::Schedule (Seconds (1.0), &TestChannelBonding::SendPacket, this, 1);
  Simulator::Schedule (Seconds (1.5), &TestChannelBonding::VerifyResultsForBss, this, true, false, 1); // successfull reception for BSS 1
  Simulator::Schedule (Seconds (1.5), &TestChannelBonding::VerifyResultsForBss, this, true, false, 3); // successfull reception for BSS 3
  //TODO: fix behavior for BSS 2 and BSS 4

  //CASE 1B: BSS 2
  Simulator::Schedule (Seconds (1.9), &TestChannelBonding::Reset, this);
  Simulator::Schedule (Seconds (2.0), &TestChannelBonding::SendPacket, this, 2);
  Simulator::Schedule (Seconds (2.5), &TestChannelBonding::VerifyResultsForBss, this, true, false, 2); // successfull reception for BSS 2
  Simulator::Schedule (Seconds (2.5), &TestChannelBonding::VerifyResultsForBss, this, true, false, 4); // successfull reception for BSS 4
  //TODO: fix behavior for BSS 1 and BSS 3

  //CASE 1C: BSS 3
  Simulator::Schedule (Seconds (2.9), &TestChannelBonding::Reset, this);
  Simulator::Schedule (Seconds (3.0), &TestChannelBonding::SendPacket, this, 3);
  Simulator::Schedule (Seconds (3.5), &TestChannelBonding::VerifyResultsForBss, this, true, false, 3); // successfull reception for BSS 3
  Simulator::Schedule (Seconds (3.5), &TestChannelBonding::VerifyResultsForBss, this, true, false, 4); // successfull reception for BSS 4
  Simulator::Schedule (Seconds (3.5), &TestChannelBonding::VerifyResultsForBss, this, false, false, 1); // no reception for BSS 1 since channel width is not supported
  Simulator::Schedule (Seconds (3.5), &TestChannelBonding::VerifyResultsForBss, this, false, false, 2); // no reception for BSS 2 since channel width is not supported
  Simulator::Schedule (Seconds (3.9), &TestChannelBonding::Reset, this);

  //CASE 1D: BSS 4
  Simulator::Schedule (Seconds (4.0), &TestChannelBonding::SendPacket, this, 4);
  Simulator::Schedule (Seconds (4.5), &TestChannelBonding::VerifyResultsForBss, this, true, false, 3); // successfull reception for BSS 3
  Simulator::Schedule (Seconds (4.5), &TestChannelBonding::VerifyResultsForBss, this, true, false, 4); // successfull reception for BSS 4
  Simulator::Schedule (Seconds (4.5), &TestChannelBonding::VerifyResultsForBss, this, false, false, 1); // no reception for BSS 1 since channel width is not supported
  Simulator::Schedule (Seconds (4.5), &TestChannelBonding::VerifyResultsForBss, this, false, false, 2); // no reception for BSS 2 since channel width is not supported
  
  //CASE 2: verify reception on channel 36 (BSS 1) when channel 40 is used (BSS 2) at the same time
  Simulator::Schedule (Seconds (4.9), &TestChannelBonding::Reset, this);
  Simulator::Schedule (Seconds (5.0), &TestChannelBonding::SendPacket, this, 1);
  Simulator::Schedule (Seconds (5.0), &TestChannelBonding::SendPacket, this, 2);
  Simulator::Schedule (Seconds (5.0), &TestChannelBonding::SetExpectedSnrForBss, this, 25.0, 1); // BSS 1 expects SNR around 25 dB
  Simulator::Schedule (Seconds (5.0), &TestChannelBonding::SetExpectedSnrForBss, this, 25.0, 1); // BSS 2 expects SNR around 24 dB
  Simulator::Schedule (Seconds (5.5), &TestChannelBonding::VerifyResultsForBss, this, true, false, 1); // successfull reception for BSS 1
  Simulator::Schedule (Seconds (5.5), &TestChannelBonding::VerifyResultsForBss, this, true, false, 2); // successfull reception for BSS 2

  //CASE 3: verify reception on channel 38 (BSS 3) when channel 36 is used (BSS 1) at the same time
  Simulator::Schedule (Seconds (5.9), &TestChannelBonding::Reset, this);
  Simulator::Schedule (Seconds (6.0), &TestChannelBonding::SendPacket, this, 3);
  Simulator::Schedule (Seconds (6.0), &TestChannelBonding::SendPacket, this, 1);
  Simulator::Schedule (Seconds (6.0), &TestChannelBonding::SetExpectedSnrForBss, this, 3.0, 1); // BSS 1 expects SNR around 3 dB
  Simulator::Schedule (Seconds (6.5), &TestChannelBonding::VerifyResultsForBss, this, false, true, 1); // PHY header passed but payload failed for BSS 1
  Simulator::Schedule (Seconds (6.5), &TestChannelBonding::VerifyResultsForBss, this, false, false, 3); // PHY header failed for BSS 3
  Simulator::Schedule (Seconds (6.5), &TestChannelBonding::VerifyResultsForBss, this, false, false, 4); // PHY header failed for BSS 4

  //CASE 4: verify reception on channel 38 (BSS 3) when channel 40 is used (BSS 2) at the same time
  Simulator::Schedule (Seconds (6.9), &TestChannelBonding::Reset, this);
  Simulator::Schedule (Seconds (7.0), &TestChannelBonding::SendPacket, this, 3);
  Simulator::Schedule (Seconds (7.0), &TestChannelBonding::SendPacket, this, 2);
  Simulator::Schedule (Seconds (7.0), &TestChannelBonding::SetExpectedSnrForBss, this, 3.0, 2); // BSS 2 expects SNR around 3 dB
  Simulator::Schedule (Seconds (7.5), &TestChannelBonding::VerifyResultsForBss, this, false, true, 2); // PHY header passed but payload failed for BSS 2
  Simulator::Schedule (Seconds (7.5), &TestChannelBonding::VerifyResultsForBss, this, false, false, 3); // PHY header failed for BSS 3

  //CASE 5: verify reception on channel 38 (BSS 4) when channel 36 is used (BSS 1) at the same time
  Simulator::Schedule (Seconds (7.9), &TestChannelBonding::Reset, this);
  Simulator::Schedule (Seconds (8.0), &TestChannelBonding::SendPacket, this, 4);
  Simulator::Schedule (Seconds (8.0), &TestChannelBonding::SendPacket, this, 1);
  Simulator::Schedule (Seconds (8.0), &TestChannelBonding::SetExpectedSnrForBss, this, 3.0, 1); // BSS 1 expects SNR around 3 dB
  Simulator::Schedule (Seconds (8.5), &TestChannelBonding::VerifyResultsForBss, this, false, true, 1); // PHY header passed but payload failed for BSS 1
  Simulator::Schedule (Seconds (8.5), &TestChannelBonding::VerifyResultsForBss, this, false, false, 4); // PHY header failed for BSS 4

  //CASE 6: verify reception on channel 38 (BSS 4) when channel 40 is used (BSS 2) at the same time
  Simulator::Schedule (Seconds (8.9), &TestChannelBonding::Reset, this);
  Simulator::Schedule (Seconds (9.0), &TestChannelBonding::SendPacket, this, 4);
  Simulator::Schedule (Seconds (9.0), &TestChannelBonding::SendPacket, this, 2);
  Simulator::Schedule (Seconds (9.0), &TestChannelBonding::SetExpectedSnrForBss, this, 3.0, 2); // BSS 2 expects SNR around 3 dB
  Simulator::Schedule (Seconds (9.5), &TestChannelBonding::VerifyResultsForBss, this, false, true, 2); // PHY header passed but payload failed for BSS 2
  Simulator::Schedule (Seconds (9.5), &TestChannelBonding::VerifyResultsForBss, this, false, false, 3); // PHY header failed for BSS 3
  Simulator::Schedule (Seconds (9.5), &TestChannelBonding::VerifyResultsForBss, this, false, false, 4); // PHY header failed for BSS 4

  //CASE 7: verify reception on channel 38 (BSS 3) when channels 36 (BSS 1) and 40 (BSS 2) are used at the same time
  Simulator::Schedule (Seconds (9.9), &TestChannelBonding::Reset, this);
  Simulator::Schedule (Seconds (10.0), &TestChannelBonding::SendPacket, this, 3);
  Simulator::Schedule (Seconds (10.0), &TestChannelBonding::SendPacket, this, 1);
  Simulator::Schedule (Seconds (10.0), &TestChannelBonding::SendPacket, this, 2);
  Simulator::Schedule (Seconds (10.0), &TestChannelBonding::SetExpectedSnrForBss, this, 3.0, 1); // BSS 1 expects SNR around 3 dB
  Simulator::Schedule (Seconds (10.0), &TestChannelBonding::SetExpectedSnrForBss, this, 3.0, 2); // BSS 2 expects SNR around 3 dB
  Simulator::Schedule (Seconds (10.5), &TestChannelBonding::VerifyResultsForBss, this, false, true, 1); // PHY header passed but payload failed for BSS 1
  Simulator::Schedule (Seconds (10.5), &TestChannelBonding::VerifyResultsForBss, this, false, true, 2); // PHY header passed but payload failed for BSS 2
  Simulator::Schedule (Seconds (10.5), &TestChannelBonding::VerifyResultsForBss, this, false, false, 3); // PHY header failed for BSS 3
  Simulator::Schedule (Seconds (10.5), &TestChannelBonding::VerifyResultsForBss, this, false, false, 4); // PHY header failed for BSS 4

  //CASE 8: verify reception on channel 38 (BSS 4) when channels 36 (BSS 1) and 40 (BSS 2) are used at the same time
  Simulator::Schedule (Seconds (10.9), &TestChannelBonding::Reset, this);
  Simulator::Schedule (Seconds (11.0), &TestChannelBonding::SendPacket, this, 4);
  Simulator::Schedule (Seconds (11.0), &TestChannelBonding::SendPacket, this, 1);
  Simulator::Schedule (Seconds (11.0), &TestChannelBonding::SendPacket, this, 2);
  Simulator::Schedule (Seconds (11.0), &TestChannelBonding::SetExpectedSnrForBss, this, 3.0, 1); // BSS 1 expects SNR around 3 dB
  Simulator::Schedule (Seconds (11.0), &TestChannelBonding::SetExpectedSnrForBss, this, 3.0, 2); // BSS 2 expects SNR around 3 dB
  Simulator::Schedule (Seconds (11.5), &TestChannelBonding::VerifyResultsForBss, this, false, true, 1); // PHY header passed but payload failed for BSS 1
  Simulator::Schedule (Seconds (11.5), &TestChannelBonding::VerifyResultsForBss, this, false, true, 2); // PHY header passed but payload failed for BSS 2
  Simulator::Schedule (Seconds (11.5), &TestChannelBonding::VerifyResultsForBss, this, false, false, 3); // PHY header failed for BSS 3
  Simulator::Schedule (Seconds (11.5), &TestChannelBonding::VerifyResultsForBss, this, false, false, 4); // PHY header failed for BSS 4

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
  AddTestCase (new TestChannelBonding, TestCase::QUICK);
}

static WifiChannelBondingTestSuite wifiChannelBondingTestSuite; ///< the test suite

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
static const uint32_t FREQUENCY_BSS1 = 5180; // MHz
static const uint32_t FREQUENCY_BSS2 = 5200; // MHz
static const uint32_t FREQUENCY_BSS3 = 5190; // MHz
static const uint16_t CHANNEL_WIDTH_BSS1 = 20; // MHz
static const uint16_t CHANNEL_WIDTH_BSS2 = 20; // MHz
static const uint16_t CHANNEL_WIDTH_BSS3 = 40; // MHz

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief channel bonding
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
  Ptr<SpectrumWifiPhy> m_txPhyBss1; ///< TX Phy BSS #1
  Ptr<SpectrumWifiPhy> m_txPhyBss2; ///< TX Phy BSS #2
  Ptr<SpectrumWifiPhy> m_txPhyBss3; ///< TX Phy BSS #3

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

private:
  virtual void DoRun (void);
};

TestChannelBonding::TestChannelBonding ()
  : TestCase ("Channel bonding test")
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
          double expectedRxPowerMin = 10 /* TX power */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 1 RX PHY is too low");
        }
      else if (size == 1032) //TX is in BSS 2
        {
          double expectedRxPowerMax = 10 /* TX power */ - 20 /* rejection */ - 50 /* loss */;
          NS_TEST_EXPECT_MSG_LT (WToDbm (it->second), expectedRxPowerMax, "Received power for BSS 2 RX PHY is too high");
        }
      else if (size == 2130) //TX is in BSS 3
        {
          double expectedRxPowerMin = 10 /* TX power */ - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
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
          double expectedRxPowerMax = 10 /* TX power */ - 20 /* rejection */ - 50 /* loss */;
          NS_TEST_EXPECT_MSG_LT (WToDbm (it->second), expectedRxPowerMax, "Received power for BSS 2 RX PHY is too high");
        }
      else if (size == 1032) //TX is in BSS 2
        {
          double expectedRxPowerMin = 10 /* TX power */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 1 RX PHY is too low");
        }
      else if (size == 2130) //TX is in BSS 3
        {
          double expectedRxPowerMin = 10 /* TX power */ - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 1 RX PHY is too low");
        }
    }
  else if (context == "BSS3") //RX is in BSS 3
    {
      //auto band = std::make_pair (FREQUENCY_BSS1, 20); //to be fixed
      auto band = std::make_pair (FREQUENCY_BSS3, 20);
      auto it = rxPowersW.find (band);
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 3 received packet with size " << size << " and power in primary 20 MHz band: " << WToDbm (it->second));
      if (size == 1031) //TX is in BSS 1
        {
          double expectedRxPowerMin = 10 /* TX power */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power in primary channel for BSS 3 RX PHY is too low");
        }
      else if (size == 1032) //TX is in BSS 2
        {
          double expectedRxPowerMax = 10 /* TX power */ - 20 /* rejection */ - 50 /* loss */;
          NS_TEST_EXPECT_MSG_LT (WToDbm (it->second), expectedRxPowerMax, "Received power for BSS 3 RX PHY is too high");
        }
      else if (size == 2130) //TX is in BSS 3
        {
          double expectedRxPowerMin = 10 /* TX power */ - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 1 RX PHY is too low");
        }

      //band = std::make_pair (FREQUENCY_BSS2, 20); //to be fixed
      band = std::make_pair (FREQUENCY_BSS3 + 20, 20);
      it = rxPowersW.find (band);
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 3 received packet with size " << size << " and power in secondary 20 MHz band: " << WToDbm (it->second));
      if (size == 1031) //TX is in BSS 1
        {
          double expectedRxPowerMax = 10 /* TX power */ - 20 /* rejection */ - 50 /* loss */;
          NS_TEST_EXPECT_MSG_LT (WToDbm (it->second), expectedRxPowerMax, "Received power for BSS 3 RX PHY is too high");
        }
      else if (size == 1032) //TX is in BSS 2
        {
          double expectedRxPowerMin = 10 /* TX power */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power in primary channel for BSS 3 RX PHY is too low");
        }
      else if (size == 2130) //TX is in BSS 3
        {
          double expectedRxPowerMin = 10 /* TX power */ - 3 /* half band */ - 50 /* loss */ - 1 /* precision */;
          NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 1 RX PHY is too low");
        }
  
      band = std::make_pair (FREQUENCY_BSS3, 40);
      it = rxPowersW.find (band);
      NS_ASSERT (it != rxPowersW.end ());
      NS_LOG_INFO ("BSS 3 received packet with size " << size << " and power in 40 MHz band: " << WToDbm (it->second));
      double expectedRxPowerMin = 10 /* TX power */ - 50 /* loss */ - 1 /* precision */;
      NS_TEST_EXPECT_MSG_GT (WToDbm (it->second), expectedRxPowerMin, "Received power for BSS 3 RX PHY is too low");
    }
}

void
TestChannelBonding::RxOkCallback (std::string context, Ptr<const Packet> p, double snr, WifiMode mode, WifiPreamble preamble)
{
  NS_LOG_INFO ("RxOkCallback: BSS=" << context << " SNR=" << RatioToDb (snr));
}

void
TestChannelBonding::RxErrorCallback (std::string context, Ptr<const Packet> p, double snr)
{
  NS_LOG_INFO ("RxErrorCallback: BSS=" << context << " SNR=" << RatioToDb (snr));
}

TestChannelBonding::~TestChannelBonding ()
{
  m_rxPhyBss1 = 0;
  m_rxPhyBss2 = 0;
  m_rxPhyBss3 = 0;
  m_txPhyBss1 = 0;
  m_txPhyBss2 = 0;
  m_txPhyBss3 = 0;
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
  m_rxPhyBss1->SetTxPowerStart(10);
  m_rxPhyBss1->SetTxPowerEnd(10);
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
  m_txPhyBss1->SetTxPowerStart(10);
  m_txPhyBss1->SetTxPowerEnd(10);
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
  m_rxPhyBss2->SetTxPowerStart(10);
  m_rxPhyBss2->SetTxPowerEnd(10);
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
  m_txPhyBss2->SetTxPowerStart(10);
  m_txPhyBss2->SetTxPowerEnd(10);
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
  m_rxPhyBss3->SetTxPowerStart(10);
  m_rxPhyBss3->SetTxPowerEnd(10);
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
  m_txPhyBss3->SetTxPowerStart(10);
  m_txPhyBss3->SetTxPowerEnd(10);
  m_txPhyBss3->Initialize ();

  m_rxPhyBss1->TraceConnect ("PhyRxBegin", "BSS1", MakeCallback (&TestChannelBonding::RxCallback, this));
  m_rxPhyBss2->TraceConnect ("PhyRxBegin", "BSS2", MakeCallback (&TestChannelBonding::RxCallback, this));
  m_rxPhyBss3->TraceConnect ("PhyRxBegin", "BSS3", MakeCallback (&TestChannelBonding::RxCallback, this));
  m_rxPhyBss1->GetState()->TraceConnect ("RxOk", "BSS1", MakeCallback (&TestChannelBonding::RxOkCallback, this));
  m_rxPhyBss2->GetState()->TraceConnect ("RxOk", "BSS2", MakeCallback (&TestChannelBonding::RxOkCallback, this));
  m_rxPhyBss3->GetState()->TraceConnect ("RxOk", "BSS3", MakeCallback (&TestChannelBonding::RxOkCallback, this));
  m_rxPhyBss1->GetState()->TraceConnect ("RxError", "BSS1", MakeCallback (&TestChannelBonding::RxErrorCallback, this));
  m_rxPhyBss2->GetState()->TraceConnect ("RxError", "BSS2", MakeCallback (&TestChannelBonding::RxErrorCallback, this));
  m_rxPhyBss3->GetState()->TraceConnect ("RxError", "BSS3", MakeCallback (&TestChannelBonding::RxErrorCallback, this));
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
  m_txPhyBss1->AssignStreams (streamNumber);
  m_txPhyBss2->AssignStreams (streamNumber);
  m_txPhyBss3->AssignStreams (streamNumber);

  //CASE 1: each BSS send a packet on its channel to verify the received power per band for each receiver
  Simulator::Schedule (Seconds (1.0), &TestChannelBonding::SendPacket, this, 1);
  Simulator::Schedule (Seconds (2.0), &TestChannelBonding::SendPacket, this, 2);
  Simulator::Schedule (Seconds (3.0), &TestChannelBonding::SendPacket, this, 3);

  //CASE 2: verify reception on channel 36 (BSS 1) when channel 40 is used (BSS 2) at the same time
  Simulator::Schedule (Seconds (4.0), &TestChannelBonding::SendPacket, this, 1);
  Simulator::Schedule (Seconds (4.0), &TestChannelBonding::SendPacket, this, 2);
  //TODO: verify SNR is at least 20 dB (rejection)

  //CASE 3: verify reception on channel 38 (BSS 3) when channel 36 is used (BSS 1) at the same time
  Simulator::Schedule (Seconds (5.0), &TestChannelBonding::SendPacket, this, 3);
  Simulator::Schedule (Seconds (5.0), &TestChannelBonding::SendPacket, this, 1);
  //TODO: verify SNR is around 3dB for RX PHY 1 => PHY header passed but payload failed
  //TODO: verify SNR is around -3dB for RX PHY 3 => PHY header reception failed

  //CASE 4: verify reception on channel 38 (BSS 3) when channel 40 is used (BSS 2) at the same time
  Simulator::Schedule (Seconds (6.0), &TestChannelBonding::SendPacket, this, 3);
  Simulator::Schedule (Seconds (6.0), &TestChannelBonding::SendPacket, this, 2);
  //TODO: verify SNR is around 3dB for RX PHY 2 => PHY header passed but payload failed
  //TODO: verify SNR is around 20 dB for preamble/header and 0dB for payload for RX PHY 3 => PHY header passed but payload failed

  //CASE 5: verify reception on channel 38 (BSS 3) when channels 36 (BSS 1) and 40 (BSS 2) are used at the same time
  Simulator::Schedule (Seconds (7.0), &TestChannelBonding::SendPacket, this, 3);
  Simulator::Schedule (Seconds (7.0), &TestChannelBonding::SendPacket, this, 1);
  Simulator::Schedule (Seconds (7.0), &TestChannelBonding::SendPacket, this, 2);
  //TODO: verify SNR is around 3dB (2-3) for RX PHY 1 => PHY header passed but payload failed
  //TODO: verify SNR is around 3dB (2-3) for RX PHY 2 => PHY header passed but payload failed
  //TODO: verify SNR is around -3 dB (-3 - -4) for preamble/header => PHY header failed

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

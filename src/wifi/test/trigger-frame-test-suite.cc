/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018
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
 * Author: Stefano Avallone <stavallo@unina.it>
 */

#include "ns3/test.h"
#include "ns3/ctrl-headers.h"
#include "ns3/packet.h"

using namespace ns3;

/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * Test Trigger frame serialization and deserialization.
 *
 * One trigger frame (of any of the supported types) with two User Info
 * fields is built. The first User Info field is addressed to a specific STA
 * while the second one includes a Random Access RU for unassociated STAs.
 */
class TriggerFrameSerialization : public TestCase
{
public:
  TriggerFrameSerialization (TriggerFrameType triggerType);
  virtual ~TriggerFrameSerialization ();
private:
  virtual void DoRun (void);
  TriggerFrameType m_triggerType;
};

TriggerFrameSerialization::TriggerFrameSerialization (TriggerFrameType triggerType)
  : TestCase ("Check the correct serialization and deserialization of Trigger frames"),
    m_triggerType (triggerType)
{
}

TriggerFrameSerialization::~TriggerFrameSerialization ()
{
}

void
TriggerFrameSerialization::DoRun (void)
{
  uint16_t ulLength = 2100;
  bool moreTF = true;
  bool csRequired = true;
  uint8_t ulBandwidth = 160;
  int8_t apTxPower = 40;
  uint16_t ulSpatialReuse = 60500;

  CtrlTriggerHeader trigger;

  trigger.SetType (m_triggerType);
  trigger.SetUlLength (ulLength);
  trigger.SetMoreTF (moreTF);
  trigger.SetCsRequired (csRequired);
  trigger.SetUlBandwidth (ulBandwidth);
  trigger.SetApTxPower (apTxPower);
  trigger.SetUlSpatialReuse (ulSpatialReuse);

  uint16_t aid1 = 1555;
  bool primary80MHz1 = true;
  HeRu::RuType ru1 = HeRu::RU_2x996_TONE;
  std::size_t index1 = 1;
  bool ldpc1 = true;
  uint8_t mcs1 = 5;
  bool ulDcm1 = false;
  uint8_t startSs = 2;
  uint8_t nSs = 2;
  int8_t ulTargetRssi = -40;
  uint8_t spacingFactor1 = 2;
  uint8_t tidLimit1 = 4;
  AcIndex prefAc1 = AC_BK;
  uint8_t tidInfo1 = 3;
  uint16_t startingSeq1 = 4095;      // in the range 0..2^12-1

  // Add a User Info field
  CtrlTriggerUserInfoField& ui1 = trigger.AddUserInfoField ();
  ui1.SetAid12 (aid1);
  ui1.SetRuAllocation ({primary80MHz1, ru1, index1});
  ui1.SetUlFecCodingType (ldpc1);
  ui1.SetUlMcs (mcs1);
  ui1.SetUlDcm (ulDcm1);
  ui1.SetSsAllocation (startSs, nSs);
  ui1.SetUlTargetRssi (ulTargetRssi);
  if (m_triggerType == BASIC_TRIGGER)
    {
      ui1.SetBasicTriggerDepUserInfo (spacingFactor1, tidLimit1, prefAc1);
    }
  else if (m_triggerType == MU_BAR_TRIGGER)
    {
      CtrlBAckRequestHeader bar;
      bar.SetType (COMPRESSED_BLOCK_ACK);
      bar.SetTidInfo (tidInfo1);
      bar.SetStartingSequence (startingSeq1);
      ui1.SetMuBarTriggerDepUserInfo (bar);
    }

  bool primary80MHz2 = false;
  HeRu::RuType ru2 = HeRu::RU_26_TONE;
  std::size_t index2 = 3;
  bool ldpc2 = false;
  uint8_t mcs2 = 11;
  bool ulDcm2 = true;
  uint8_t nRaRu = 2;
  bool noMoreRaRu = true;
  uint8_t spacingFactor2 = 1;
  uint8_t tidLimit2 = 6;
  AcIndex prefAc2 = AC_VI;
  uint8_t tidInfo2 = 6;
  uint16_t startingSeq2 = 2195;      // in the range 0..2^12-1

  // Add another User Info field
  CtrlTriggerUserInfoField& ui2 = trigger.AddUserInfoField ();
  ui2.SetAid12 (2045);
  ui2.SetRuAllocation ({primary80MHz2, ru2, index2});
  ui2.SetUlFecCodingType (ldpc2);
  ui2.SetUlMcs (mcs2);
  ui2.SetUlDcm (ulDcm2);
  ui2.SetRaRuInformation (nRaRu, noMoreRaRu);
  ui2.SetUlTargetRssiMaxTxPower ();
  if (m_triggerType == BASIC_TRIGGER)
    {
      ui2.SetBasicTriggerDepUserInfo (spacingFactor2, tidLimit2, prefAc2);
    }
  else if (m_triggerType == MU_BAR_TRIGGER)
    {
      CtrlBAckRequestHeader bar;
      bar.SetType (COMPRESSED_BLOCK_ACK);
      bar.SetTidInfo (tidInfo2);
      bar.SetStartingSequence (startingSeq2);
      ui2.SetMuBarTriggerDepUserInfo (bar);
    }

  // Check that fields have been set correctly
  NS_TEST_EXPECT_MSG_EQ ((m_triggerType == BASIC_TRIGGER && trigger.IsBasic ())
                         || (m_triggerType == MU_BAR_TRIGGER && trigger.IsMuBar ())
                         || (m_triggerType == MU_RTS_TRIGGER && trigger.IsMuRts ())
                         || (m_triggerType == BSRP_TRIGGER && trigger.IsBsrp ())
                         || (m_triggerType == BQRP_TRIGGER && trigger.IsBqrp ()),
                         true, "Different trigger frame type found");
  NS_TEST_EXPECT_MSG_EQ (trigger.GetUlLength (), ulLength,
                         "Different UL Length found");
  NS_TEST_EXPECT_MSG_EQ (trigger.GetMoreTF (), moreTF,
                         "Different MoreTF found");
  NS_TEST_EXPECT_MSG_EQ (trigger.GetCsRequired (), csRequired,
                         "Different CS Required found");
  NS_TEST_EXPECT_MSG_EQ (trigger.GetUlBandwidth (), ulBandwidth,
                         "Different UL Bandwidth found");
  NS_TEST_EXPECT_MSG_EQ (trigger.GetApTxPower (), apTxPower,
                         "Different AP TX Power found");
  NS_TEST_EXPECT_MSG_EQ (trigger.GetUlSpatialReuse (), ulSpatialReuse,
                         "Different UL Spatial Reuse found");

  CtrlTriggerHeader::Iterator it = trigger.begin ();

  NS_TEST_EXPECT_MSG_EQ (it->GetAid12 (), aid1,
                         "Different AID found");
  NS_TEST_EXPECT_MSG_EQ (it->GetRuAllocation ().primary80MHz, primary80MHz1,
                         "Different RU allocation band found");
  NS_TEST_EXPECT_MSG_EQ (it->GetRuAllocation ().ruType, ru1,
                         "Different RU allocation type found");
  NS_TEST_EXPECT_MSG_EQ (it->GetRuAllocation ().index, index1,
                         "Different RU allocation index found");
  NS_TEST_EXPECT_MSG_EQ (it->GetUlFecCodingType (), ldpc1,
                         "Different UL FEC Coding type found");
  NS_TEST_EXPECT_MSG_EQ (it->GetUlMcs (), mcs1,
                         "Different UL MCS found");
  NS_TEST_EXPECT_MSG_EQ (it->GetUlDcm (), ulDcm1,
                         "Different UL DCM found");
  NS_TEST_EXPECT_MSG_EQ (it->GetStartingSs (), startSs,
                         "Different Starting SS found");
  NS_TEST_EXPECT_MSG_EQ (it->GetNss (), nSs,
                         "Different Number of SS found");
  NS_TEST_EXPECT_MSG_EQ (it->GetUlTargetRssi (), ulTargetRssi,
                         "Different UL Target RSSI found");
  if (m_triggerType == BASIC_TRIGGER)
    {
      NS_TEST_EXPECT_MSG_EQ (it->GetMpduMuSpacingFactor (), spacingFactor1,
                             "Different MPDU MU Spacing Factor found");
      NS_TEST_EXPECT_MSG_EQ (it->GetTidAggregationLimit (), tidLimit1,
                             "Different TID Aggregation Limit found");
      NS_TEST_EXPECT_MSG_EQ (it->GetPreferredAc (), prefAc1,
                             "Different Preferred AC found");
    }
  else if (m_triggerType == MU_BAR_TRIGGER)
    {
      NS_TEST_EXPECT_MSG_EQ (it->GetMuBarTriggerDepUserInfo ().IsCompressed (), true,
                             "The BAR Control subfield does not indicate a Compressed BAR");
      NS_TEST_EXPECT_MSG_EQ (it->GetMuBarTriggerDepUserInfo ().GetTidInfo (), tidInfo1,
                             "Different TID Info found");
      NS_TEST_EXPECT_MSG_EQ (it->GetMuBarTriggerDepUserInfo ().GetStartingSequence (), startingSeq1,
                             "Different Block Ack Starting Sequence");
    }

  it++;

  NS_TEST_EXPECT_MSG_EQ (it->HasRaRuForUnassociatedSta (), true,
                         "User Info field allocating RA-RU for unassociated STA not found");
  NS_TEST_EXPECT_MSG_EQ (it->GetRuAllocation ().primary80MHz, primary80MHz2,
                         "Different RU allocation band found");
  NS_TEST_EXPECT_MSG_EQ (it->GetRuAllocation ().ruType, ru2,
                         "Different RU allocation type found");
  NS_TEST_EXPECT_MSG_EQ (it->GetRuAllocation ().index, index2,
                         "Different RU allocation index found");
  NS_TEST_EXPECT_MSG_EQ (it->GetUlFecCodingType (), ldpc2,
                         "Different UL FEC Coding type found");
  NS_TEST_EXPECT_MSG_EQ (it->GetUlMcs (), mcs2,
                         "Different UL MCS found");
  NS_TEST_EXPECT_MSG_EQ (it->GetUlDcm (), ulDcm2,
                         "Different UL DCM found");
  NS_TEST_EXPECT_MSG_EQ (it->GetNRaRus (), nRaRu,
                         "Different Number of RA-RU found");
  NS_TEST_EXPECT_MSG_EQ (it->GetNoMoreRaRu (), noMoreRaRu,
                         "Different No more RA-RU found");
  NS_TEST_EXPECT_MSG_EQ (it->IsUlTargetRssiMaxTxPower (), true,
                         "Different UL Target RSSI found");
  if (m_triggerType == BASIC_TRIGGER)
    {
      NS_TEST_EXPECT_MSG_EQ (it->GetMpduMuSpacingFactor (), spacingFactor2,
                             "Different MPDU MU Spacing Factor found");
      NS_TEST_EXPECT_MSG_EQ (it->GetTidAggregationLimit (), tidLimit2,
                             "Different TID Aggregation Limit found");
      NS_TEST_EXPECT_MSG_EQ (it->GetPreferredAc (), prefAc2,
                             "Different Preferred AC found");
    }
  else if (m_triggerType == MU_BAR_TRIGGER)
    {
      NS_TEST_EXPECT_MSG_EQ (it->GetMuBarTriggerDepUserInfo ().IsCompressed (), true,
                             "The BAR Control subfield does not indicate a Compressed BAR");
      NS_TEST_EXPECT_MSG_EQ (it->GetMuBarTriggerDepUserInfo ().GetTidInfo (), tidInfo2,
                             "Different TID Info found");
      NS_TEST_EXPECT_MSG_EQ (it->GetMuBarTriggerDepUserInfo ().GetStartingSequence (), startingSeq2,
                             "Different Block Ack Starting Sequence");
    }

  // Serialize the header
  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader (trigger);

  // Deserialize the header
  CtrlTriggerHeader triggerCopy;
  packet->RemoveHeader (triggerCopy);

  // Check that the header has been correctly deserialized
  NS_TEST_EXPECT_MSG_EQ ((m_triggerType == BASIC_TRIGGER && triggerCopy.IsBasic ())
                         || (m_triggerType == MU_BAR_TRIGGER && triggerCopy.IsMuBar ())
                         || (m_triggerType == MU_RTS_TRIGGER && trigger.IsMuRts ())
                         || (m_triggerType == BSRP_TRIGGER && trigger.IsBsrp ())
                         || (m_triggerType == BQRP_TRIGGER && trigger.IsBqrp ()),
                         true, "Different trigger frame type found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (triggerCopy.GetUlLength (), ulLength,
                         "Different UL Length found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (triggerCopy.GetMoreTF (), moreTF,
                         "Different MoreTF found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (triggerCopy.GetCsRequired (), csRequired,
                         "Different CS Required found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (triggerCopy.GetUlBandwidth (), ulBandwidth,
                         "Different UL Bandwidth found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (triggerCopy.GetApTxPower (), apTxPower,
                         "Different AP TX Power found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (triggerCopy.GetUlSpatialReuse (), ulSpatialReuse,
                         "Different UL Spatial Reuse found in deserialized header");

  CtrlTriggerHeader::ConstIterator itCopy = triggerCopy.FindUserInfoWithRaRuAssociated ();

  NS_TEST_EXPECT_MSG_EQ ((itCopy == triggerCopy.end ()), true,
                         "User Info field allocating RA-RU for associated STA found in deserialized header");

  itCopy = triggerCopy.FindUserInfoWithAid (aid1);

  NS_TEST_EXPECT_MSG_EQ ((itCopy != triggerCopy.end ()), true,
                         "No User Info field allocating RU for the given STA found in deserialized header");

  NS_TEST_EXPECT_MSG_EQ (itCopy->GetAid12 (), aid1,
                         "Different AID found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (itCopy->GetRuAllocation ().primary80MHz, primary80MHz1,
                         "Different RU allocation band found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (itCopy->GetRuAllocation ().ruType, ru1,
                         "Different RU allocation type found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (itCopy->GetRuAllocation ().index, index1,
                         "Different RU allocation index found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (itCopy->GetUlFecCodingType (), ldpc1,
                         "Different UL FEC Coding type found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (itCopy->GetUlMcs (), mcs1,
                         "Different UL MCS found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (itCopy->GetUlDcm (), ulDcm1,
                         "Different UL DCM found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (itCopy->GetStartingSs (), startSs,
                         "Different Starting SS found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (itCopy->GetNss (), nSs,
                         "Different Number of SS found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (itCopy->GetUlTargetRssi (), ulTargetRssi,
                         "Different UL Target RSSI found in deserialized header");
  if (m_triggerType == BASIC_TRIGGER)
    {
      NS_TEST_EXPECT_MSG_EQ (itCopy->GetMpduMuSpacingFactor (), spacingFactor1,
                             "Different MPDU MU Spacing Factor found in deserialized header");
      NS_TEST_EXPECT_MSG_EQ (itCopy->GetTidAggregationLimit (), tidLimit1,
                             "Different TID Aggregation Limit found in deserialized header");
      NS_TEST_EXPECT_MSG_EQ (itCopy->GetPreferredAc (), prefAc1,
                             "Different Preferred AC found in deserialized header");
    }
  else if (m_triggerType == MU_BAR_TRIGGER)
    {
      NS_TEST_EXPECT_MSG_EQ (itCopy->GetMuBarTriggerDepUserInfo ().IsCompressed (), true,
                             "The BAR Control subfield does not indicate a Compressed BAR");
      NS_TEST_EXPECT_MSG_EQ (itCopy->GetMuBarTriggerDepUserInfo ().GetTidInfo (), tidInfo1,
                             "Different TID Info found");
      NS_TEST_EXPECT_MSG_EQ (itCopy->GetMuBarTriggerDepUserInfo ().GetStartingSequence (), startingSeq1,
                             "Different Block Ack Starting Sequence");
    }

  itCopy = triggerCopy.FindUserInfoWithRaRuUnassociated ();

  NS_TEST_EXPECT_MSG_EQ ((itCopy != triggerCopy.end ()), true,
                         "User Info field allocating RA-RU for unassociated STA not found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (itCopy->HasRaRuForUnassociatedSta (), true,
                         "User Info field allocating RA-RU for unassociated STA not found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (itCopy->GetRuAllocation ().primary80MHz, primary80MHz2,
                         "Different RU allocation band found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (itCopy->GetRuAllocation ().ruType, ru2,
                         "Different RU allocation type found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (itCopy->GetRuAllocation ().index, index2,
                         "Different RU allocation index found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (itCopy->GetUlFecCodingType (), ldpc2,
                         "Different UL FEC Coding type found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (itCopy->GetUlMcs (), mcs2,
                         "Different UL MCS found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (itCopy->GetUlDcm (), ulDcm2,
                         "Different UL DCM found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (itCopy->GetNRaRus (), nRaRu,
                         "Different Number of RA-RU found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (itCopy->GetNoMoreRaRu (), noMoreRaRu,
                         "Different No more RA-RU found in deserialized header");
  NS_TEST_EXPECT_MSG_EQ (itCopy->IsUlTargetRssiMaxTxPower (), true,
                         "Different UL Target RSSI found in deserialized header");
  if (m_triggerType == BASIC_TRIGGER)
    {
      NS_TEST_EXPECT_MSG_EQ (itCopy->GetMpduMuSpacingFactor (), spacingFactor2,
                             "Different MPDU MU Spacing Factor found in deserialized header");
      NS_TEST_EXPECT_MSG_EQ (itCopy->GetTidAggregationLimit (), tidLimit2,
                             "Different TID Aggregation Limit found in deserialized header");
      NS_TEST_EXPECT_MSG_EQ (itCopy->GetPreferredAc (), prefAc2,
                             "Different Preferred AC found in deserialized header");
    }
  else if (m_triggerType == MU_BAR_TRIGGER)
    {
      NS_TEST_EXPECT_MSG_EQ (itCopy->GetMuBarTriggerDepUserInfo ().IsCompressed (), true,
                             "The BAR Control subfield does not indicate a Compressed BAR");
      NS_TEST_EXPECT_MSG_EQ (itCopy->GetMuBarTriggerDepUserInfo ().GetTidInfo (), tidInfo2,
                             "Different TID Info found");
      NS_TEST_EXPECT_MSG_EQ (itCopy->GetMuBarTriggerDepUserInfo ().GetStartingSequence (), startingSeq2,
                             "Different Block Ack Starting Sequence");
    }

  // Test range based for loop
  for (auto& ui : triggerCopy)
    {
      NS_TEST_EXPECT_MSG_EQ ((ui.GetAid12 () == aid1 || ui.HasRaRuForUnassociatedSta ()), true,
                             "Unexpected User Info field found in deserialized header");
    }
}


/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * Test overlap of allocated RUs
 */
class RuAllocationOverlap : public TestCase
{
public:
  RuAllocationOverlap ();
  virtual ~RuAllocationOverlap ();
private:
  virtual void DoRun (void);
};

RuAllocationOverlap::RuAllocationOverlap ()
  : TestCase ("Check that overlapping RUs are detected correctly")
{
}

RuAllocationOverlap::~RuAllocationOverlap ()
{
}

void
RuAllocationOverlap::DoRun (void)
{
  /*
   * Check that all the RUs of a given type that are available for a given
   * bandwidth (20, 40 or 80) do not overlap
   */
  for (auto& ru : HeRu::m_heRuSubcarrierGroups)
    {
      CtrlTriggerHeader trigger;
      trigger.SetType (BASIC_TRIGGER);
      trigger.SetUlBandwidth (ru.first.first);

      for (std::size_t index = 1; index <= ru.second.size (); index++)
        {
          CtrlTriggerUserInfoField& ui = trigger.AddUserInfoField ();
          ui.SetRuAllocation ({true, ru.first.second, index});
        }

      NS_TEST_EXPECT_MSG_EQ (trigger.IsValid (), true, "Found overlapping RUs");

      // Adding the first RU of the same type (again) makes the frame invalid
      CtrlTriggerUserInfoField& ui = trigger.AddUserInfoField ();
      ui.SetRuAllocation ({true, ru.first.second, 1});

      NS_TEST_EXPECT_MSG_EQ (trigger.IsValid (), false, "RUs not detected to be overlapping");
    }

  /*
   * When the channel bandwidth is 160MHz, primary 80MHz channel and secondary
   * 80MHz channel are orthogonal.
   */
  for (auto& ru : HeRu::m_heRuSubcarrierGroups)
    {
      if (ru.first.first != 80)
        {
          continue;
        }

      CtrlTriggerHeader trigger;
      trigger.SetType (BASIC_TRIGGER);
      trigger.SetUlBandwidth (160);

      for (std::size_t index = 1; index <= ru.second.size (); index++)
        {
          CtrlTriggerUserInfoField& ui = trigger.AddUserInfoField ();
          ui.SetRuAllocation ({true, ru.first.second, index});   // primary 80MHz channel
        }

      for (std::size_t index = 1; index <= ru.second.size (); index++)
        {
          CtrlTriggerUserInfoField& ui = trigger.AddUserInfoField ();
          ui.SetRuAllocation ({false, ru.first.second, index});   // secondary 80MHz channel
        }

      NS_TEST_EXPECT_MSG_EQ (trigger.IsValid (), true, "Found overlapping RUs");
    }

  /*
   * The 2x996-tone RU overlaps with any other RU
   */
  for (auto& ru : HeRu::m_heRuSubcarrierGroups)
    {
      if (ru.first.first != 80)
        {
          continue;
        }

      CtrlTriggerHeader trigger;
      trigger.SetType (BASIC_TRIGGER);
      trigger.SetUlBandwidth (160);

      CtrlTriggerUserInfoField& ui1 = trigger.AddUserInfoField ();
      ui1.SetRuAllocation ({true, ru.first.second, 1});

      CtrlTriggerUserInfoField& ui2 = trigger.AddUserInfoField ();
      ui2.SetRuAllocation ({false, HeRu::RU_2x996_TONE, 1});

      NS_TEST_EXPECT_MSG_EQ (trigger.IsValid (), false, "2x996-tone RU not found to overlap");
    }

  /*
   * In an 80MHz channel, the following RUs do not overlap:
   * RU_484_TONE (1), RU_26_TONE (19), RU_242_TONE (3), RU_106_TONE (7),
   * RU_26_TONE (33), RU_52_TONE (15,16)
   */
  {
    CtrlTriggerHeader trigger;
      trigger.SetType (BASIC_TRIGGER);
      trigger.SetUlBandwidth (80);

      CtrlTriggerUserInfoField& ui1 = trigger.AddUserInfoField ();
      ui1.SetRuAllocation ({true, HeRu::RU_484_TONE, 1});

      CtrlTriggerUserInfoField& ui2 = trigger.AddUserInfoField ();
      ui2.SetRuAllocation ({true, HeRu::RU_26_TONE, 19});

      CtrlTriggerUserInfoField& ui3 = trigger.AddUserInfoField ();
      ui3.SetRuAllocation ({true, HeRu::RU_242_TONE, 3});

      CtrlTriggerUserInfoField& ui4 = trigger.AddUserInfoField ();
      ui4.SetRuAllocation ({true, HeRu::RU_106_TONE, 7});

      CtrlTriggerUserInfoField& ui7 = trigger.AddUserInfoField ();
      ui7.SetRuAllocation ({true, HeRu::RU_26_TONE, 33});

      CtrlTriggerUserInfoField& ui5 = trigger.AddUserInfoField ();
      ui5.SetRuAllocation ({true, HeRu::RU_52_TONE, 15});

      CtrlTriggerUserInfoField& ui6 = trigger.AddUserInfoField ();
      ui6.SetRuAllocation ({true, HeRu::RU_52_TONE, 16});

      NS_TEST_EXPECT_MSG_EQ (trigger.IsValid (), true, "Found overlapping RUs");
  }
}


/**
 * \ingroup wifi-test
 * \ingroup tests
 *
 * \brief Trigger Frame Test Suite
 */
class TriggerFrameTestSuite : public TestSuite
{
public:
  TriggerFrameTestSuite ();
};

TriggerFrameTestSuite::TriggerFrameTestSuite ()
  : TestSuite ("wifi-trigger-frame", UNIT)
{
  AddTestCase (new TriggerFrameSerialization (BASIC_TRIGGER), TestCase::QUICK);
  AddTestCase (new TriggerFrameSerialization (MU_BAR_TRIGGER), TestCase::QUICK);
  AddTestCase (new TriggerFrameSerialization (MU_RTS_TRIGGER), TestCase::QUICK);
  AddTestCase (new TriggerFrameSerialization (BSRP_TRIGGER), TestCase::QUICK);
  AddTestCase (new TriggerFrameSerialization (BQRP_TRIGGER), TestCase::QUICK);
  AddTestCase (new RuAllocationOverlap, TestCase::QUICK);
}

static TriggerFrameTestSuite g_triggerFrameTestSuite; ///< the test suite

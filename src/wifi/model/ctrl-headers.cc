/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 MIRKO BANCHI
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
 * Author: Mirko Banchi <mk.banchi@gmail.com>
 */

#include "ctrl-headers.h"
#include <algorithm>

namespace ns3 {


/***********************************
 *       Block ack request
 ***********************************/

NS_OBJECT_ENSURE_REGISTERED (CtrlBAckRequestHeader);

CtrlBAckRequestHeader::CtrlBAckRequestHeader ()
  : m_barAckPolicy (false),
    m_baType (BASIC_BLOCK_ACK)
{
}

CtrlBAckRequestHeader::~CtrlBAckRequestHeader ()
{
}

TypeId
CtrlBAckRequestHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CtrlBAckRequestHeader")
    .SetParent<Header> ()
    .SetGroupName ("Wifi")
    .AddConstructor<CtrlBAckRequestHeader> ()
  ;
  return tid;
}

TypeId
CtrlBAckRequestHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
CtrlBAckRequestHeader::Print (std::ostream &os) const
{
  os << "TID_INFO=" << m_tidInfo << ", StartingSeq=" << std::hex << m_startingSeq << std::dec;
}

uint32_t
CtrlBAckRequestHeader::GetSerializedSize () const
{
  uint32_t size = 0;
  size += 2; //Bar control
  switch (m_baType)
    {
      case BASIC_BLOCK_ACK:
      case COMPRESSED_BLOCK_ACK:
      case EXTENDED_COMPRESSED_BLOCK_ACK:
        size += 2;
        break;
      case MULTI_TID_BLOCK_ACK:
        size += (2 + 2) * (m_tidInfo + 1);
        break;
      default:
        NS_FATAL_ERROR ("Invalid BA type");
        break;
    }
  return size;
}

void
CtrlBAckRequestHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteHtolsbU16 (GetBarControl ());
  switch (m_baType)
    {
      case BASIC_BLOCK_ACK:
      case COMPRESSED_BLOCK_ACK:
      case EXTENDED_COMPRESSED_BLOCK_ACK:
        i.WriteHtolsbU16 (GetStartingSequenceControl ());
        break;
      case MULTI_TID_BLOCK_ACK:
        NS_FATAL_ERROR ("Multi-tid block ack is not supported.");
        break;
      default:
        NS_FATAL_ERROR ("Invalid BA type");
        break;
    }
}

uint32_t
CtrlBAckRequestHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  SetBarControl (i.ReadLsbtohU16 ());
  switch (m_baType)
    {
      case BASIC_BLOCK_ACK:
      case COMPRESSED_BLOCK_ACK:
      case EXTENDED_COMPRESSED_BLOCK_ACK:
        SetStartingSequenceControl (i.ReadLsbtohU16 ());
        break;
      case MULTI_TID_BLOCK_ACK:
        NS_FATAL_ERROR ("Multi-tid block ack is not supported.");
        break;
      default:
        NS_FATAL_ERROR ("Invalid BA type");
        break;
    }
  return i.GetDistanceFrom (start);
}

uint16_t
CtrlBAckRequestHeader::GetBarControl (void) const
{
  uint16_t res = 0;
  switch (m_baType)
    {
      case BASIC_BLOCK_ACK:
        break;
      case COMPRESSED_BLOCK_ACK:
        res |= (0x02 << 1);
        break;
      case EXTENDED_COMPRESSED_BLOCK_ACK:
        res |= (0x01 << 1);
        break;
      case MULTI_TID_BLOCK_ACK:
        res |= (0x03 << 1);
        break;
      default:
        NS_FATAL_ERROR ("Invalid BA type");
        break;
    }
  res |= (m_tidInfo << 12) & (0xf << 12);
  return res;
}

void
CtrlBAckRequestHeader::SetBarControl (uint16_t bar)
{
  m_barAckPolicy = ((bar & 0x01) == 1) ? true : false;
  if (((bar >> 1) & 0x0f) == 0x03)
    {
      m_baType = MULTI_TID_BLOCK_ACK;
    }
  else if (((bar >> 1) & 0x0f) == 0x01)
    {
      m_baType = EXTENDED_COMPRESSED_BLOCK_ACK;
    }
  else if (((bar >> 1) & 0x0f) == 0x02)
    {
      m_baType = COMPRESSED_BLOCK_ACK;
    }
  else
    {
      m_baType = BASIC_BLOCK_ACK;
    }
  m_tidInfo = (bar >> 12) & 0x0f;
}

uint16_t
CtrlBAckRequestHeader::GetStartingSequenceControl (void) const
{
  return (m_startingSeq << 4) & 0xfff0;
}

void
CtrlBAckRequestHeader::SetStartingSequenceControl (uint16_t seqControl)
{
  m_startingSeq = (seqControl >> 4) & 0x0fff;
}

void
CtrlBAckRequestHeader::SetHtImmediateAck (bool immediateAck)
{
  m_barAckPolicy = immediateAck;
}

void
CtrlBAckRequestHeader::SetType (BlockAckType type)
{
  m_baType = type;
}

BlockAckType
CtrlBAckRequestHeader::GetType (void) const
{
  return m_baType;
}

void
CtrlBAckRequestHeader::SetTidInfo (uint8_t tid)
{
  m_tidInfo = static_cast<uint16_t> (tid);
}

void
CtrlBAckRequestHeader::SetStartingSequence (uint16_t seq)
{
  m_startingSeq = seq;
}

bool
CtrlBAckRequestHeader::MustSendHtImmediateAck (void) const
{
  return m_barAckPolicy;
}

uint8_t
CtrlBAckRequestHeader::GetTidInfo (void) const
{
  uint8_t tid = static_cast<uint8_t> (m_tidInfo);
  return tid;
}

uint16_t
CtrlBAckRequestHeader::GetStartingSequence (void) const
{
  return m_startingSeq;
}

bool
CtrlBAckRequestHeader::IsBasic (void) const
{
  return (m_baType == BASIC_BLOCK_ACK) ? true : false;
}

bool
CtrlBAckRequestHeader::IsCompressed (void) const
{
  return (m_baType == COMPRESSED_BLOCK_ACK) ? true : false;
}

bool
CtrlBAckRequestHeader::IsExtendedCompressed (void) const
{
  return (m_baType == EXTENDED_COMPRESSED_BLOCK_ACK) ? true : false;
}

bool
CtrlBAckRequestHeader::IsMultiTid (void) const
{
  return (m_baType == MULTI_TID_BLOCK_ACK) ? true : false;
}


/***********************************
 *       Block ack response
 ***********************************/

NS_OBJECT_ENSURE_REGISTERED (CtrlBAckResponseHeader);

CtrlBAckResponseHeader::CtrlBAckResponseHeader ()
  : m_baAckPolicy (false),
    m_baType (BASIC_BLOCK_ACK)
{
  memset (&bitmap, 0, sizeof (bitmap));
}

CtrlBAckResponseHeader::~CtrlBAckResponseHeader ()
{
}

TypeId
CtrlBAckResponseHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CtrlBAckResponseHeader")
    .SetParent<Header> ()
    .SetGroupName ("Wifi")
    .AddConstructor<CtrlBAckResponseHeader> ()
  ;
  return tid;
}

TypeId
CtrlBAckResponseHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
CtrlBAckResponseHeader::Print (std::ostream &os) const
{
  os << "TID_INFO=" << m_tidInfo << ", StartingSeq=" << std::hex << m_startingSeq << std::dec;
}

uint32_t
CtrlBAckResponseHeader::GetSerializedSize (void) const
{
  uint32_t size = 0;
  size += 2; //Bar control
  switch (m_baType)
    {
      case BASIC_BLOCK_ACK:
        size += (2 + 128);
        break;
      case COMPRESSED_BLOCK_ACK:
        size += (2 + 8);
        break;
      case EXTENDED_COMPRESSED_BLOCK_ACK:
        size += (2 + 32);
        break;
      case MULTI_TID_BLOCK_ACK:
        size += (2 + 2 + 8) * (m_tidInfo + 1); //Multi-tid block ack
        break;
      default:
        NS_FATAL_ERROR ("Invalid BA type");
        break;
    }
  return size;
}

void
CtrlBAckResponseHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteHtolsbU16 (GetBaControl ());
  switch (m_baType)
    {
      case BASIC_BLOCK_ACK:
      case COMPRESSED_BLOCK_ACK:
      case EXTENDED_COMPRESSED_BLOCK_ACK:
        i.WriteHtolsbU16 (GetStartingSequenceControl ());
        i = SerializeBitmap (i);
        break;
      case MULTI_TID_BLOCK_ACK:
        NS_FATAL_ERROR ("Multi-tid block ack is not supported.");
        break;
      default:
        NS_FATAL_ERROR ("Invalid BA type");
        break;
    }
}

uint32_t
CtrlBAckResponseHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  SetBaControl (i.ReadLsbtohU16 ());
  switch (m_baType)
    {
      case BASIC_BLOCK_ACK:
      case COMPRESSED_BLOCK_ACK:
      case EXTENDED_COMPRESSED_BLOCK_ACK:
        SetStartingSequenceControl (i.ReadLsbtohU16 ());
        i = DeserializeBitmap (i);
        break;
      case MULTI_TID_BLOCK_ACK:
        NS_FATAL_ERROR ("Multi-tid block ack is not supported.");
        break;
      default:
        NS_FATAL_ERROR ("Invalid BA type");
        break;
    }
  return i.GetDistanceFrom (start);
}

void
CtrlBAckResponseHeader::SetHtImmediateAck (bool immediateAck)
{
  m_baAckPolicy = immediateAck;
}

void
CtrlBAckResponseHeader::SetType (BlockAckType type)
{
  m_baType = type;
}

BlockAckType
CtrlBAckResponseHeader::GetType (void) const
{
  return m_baType;
}

void
CtrlBAckResponseHeader::SetTidInfo (uint8_t tid)
{
  m_tidInfo = static_cast<uint16_t> (tid);
}

void
CtrlBAckResponseHeader::SetStartingSequence (uint16_t seq)
{
  m_startingSeq = seq;
}

bool
CtrlBAckResponseHeader::MustSendHtImmediateAck (void) const
{
  return (m_baAckPolicy) ? true : false;
}

uint8_t
CtrlBAckResponseHeader::GetTidInfo (void) const
{
  uint8_t tid = static_cast<uint8_t> (m_tidInfo);
  return tid;
}

uint16_t
CtrlBAckResponseHeader::GetStartingSequence (void) const
{
  return m_startingSeq;
}

bool
CtrlBAckResponseHeader::IsBasic (void) const
{
  return (m_baType == BASIC_BLOCK_ACK) ? true : false;
}

bool
CtrlBAckResponseHeader::IsCompressed (void) const
{
  return (m_baType == COMPRESSED_BLOCK_ACK) ? true : false;
}

bool
CtrlBAckResponseHeader::IsExtendedCompressed (void) const
{
  return (m_baType == EXTENDED_COMPRESSED_BLOCK_ACK) ? true : false;
}

bool
CtrlBAckResponseHeader::IsMultiTid (void) const
{
  return (m_baType == MULTI_TID_BLOCK_ACK) ? true : false;
}

uint16_t
CtrlBAckResponseHeader::GetBaControl (void) const
{
  uint16_t res = 0;
  if (m_baAckPolicy)
    {
      res |= 0x1;
    }
  switch (m_baType)
    {
      case BASIC_BLOCK_ACK:
        break;
      case COMPRESSED_BLOCK_ACK:
        res |= (0x02 << 1);
        break;
      case EXTENDED_COMPRESSED_BLOCK_ACK:
        res |= (0x01 << 1);
        break;
      case MULTI_TID_BLOCK_ACK:
        res |= (0x03 << 1);
        break;
      default:
        NS_FATAL_ERROR ("Invalid BA type");
        break;
    }
  res |= (m_tidInfo << 12) & (0xf << 12);
  return res;
}

void
CtrlBAckResponseHeader::SetBaControl (uint16_t ba)
{
  m_baAckPolicy = ((ba & 0x01) == 1) ? true : false;
  if (((ba >> 1) & 0x0f) == 0x03)
    {
      m_baType = MULTI_TID_BLOCK_ACK;
    }
  else if (((ba >> 1) & 0x0f) == 0x01)
    {
      m_baType = EXTENDED_COMPRESSED_BLOCK_ACK;
    }
  else if (((ba >> 1) & 0x0f) == 0x02)
    {
      m_baType = COMPRESSED_BLOCK_ACK;
    }
  else
    {
      m_baType = BASIC_BLOCK_ACK;
    }
  m_tidInfo = (ba >> 12) & 0x0f;
}

uint16_t
CtrlBAckResponseHeader::GetStartingSequenceControl (void) const
{
  return (m_startingSeq << 4) & 0xfff0;
}

void
CtrlBAckResponseHeader::SetStartingSequenceControl (uint16_t seqControl)
{
  m_startingSeq = (seqControl >> 4) & 0x0fff;
}

Buffer::Iterator
CtrlBAckResponseHeader::SerializeBitmap (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  switch (m_baType)
    {
      case BASIC_BLOCK_ACK:
          for (uint8_t j = 0; j < 64; j++)
            {
              i.WriteHtolsbU16 (bitmap.m_bitmap[j]);
            }
          break;
      case COMPRESSED_BLOCK_ACK:
          i.WriteHtolsbU64 (bitmap.m_compressedBitmap);
          break;
      case EXTENDED_COMPRESSED_BLOCK_ACK:
          i.WriteHtolsbU64 (bitmap.m_extendedCompressedBitmap[0]);
          i.WriteHtolsbU64 (bitmap.m_extendedCompressedBitmap[1]);
          i.WriteHtolsbU64 (bitmap.m_extendedCompressedBitmap[2]);
          i.WriteHtolsbU64 (bitmap.m_extendedCompressedBitmap[3]);
          break;
      case MULTI_TID_BLOCK_ACK:
        NS_FATAL_ERROR ("Multi-tid block ack is not supported.");
        break;
      default:
        NS_FATAL_ERROR ("Invalid BA type");
        break;
    }
  return i;
}

Buffer::Iterator
CtrlBAckResponseHeader::DeserializeBitmap (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  switch (m_baType)
    {
      case BASIC_BLOCK_ACK:
          for (uint8_t j = 0; j < 64; j++)
            {
              bitmap.m_bitmap[j] = i.ReadLsbtohU16 ();
            }
          break;
      case COMPRESSED_BLOCK_ACK:
          bitmap.m_compressedBitmap = i.ReadLsbtohU64 ();
          break;
      case EXTENDED_COMPRESSED_BLOCK_ACK:
          bitmap.m_extendedCompressedBitmap[0] = i.ReadLsbtohU64 ();
          bitmap.m_extendedCompressedBitmap[1] = i.ReadLsbtohU64 ();
          bitmap.m_extendedCompressedBitmap[2] = i.ReadLsbtohU64 ();
          bitmap.m_extendedCompressedBitmap[3] = i.ReadLsbtohU64 ();
          break;
      case MULTI_TID_BLOCK_ACK:
        NS_FATAL_ERROR ("Multi-tid block ack is not supported.");
        break;
      default:
        NS_FATAL_ERROR ("Invalid BA type");
        break;
    }
  return i;
}

void
CtrlBAckResponseHeader::SetReceivedPacket (uint16_t seq)
{
  if (!IsInBitmap (seq))
    {
      return;
    }
  switch (m_baType)
    {
      case BASIC_BLOCK_ACK:
        {
          /* To set correctly basic block ack bitmap we need fragment number too.
             So if it's not specified, we consider packet not fragmented. */
          bitmap.m_bitmap[IndexInBitmap (seq)] |= 0x0001;
          break;
        }
      case COMPRESSED_BLOCK_ACK:
        {
          bitmap.m_compressedBitmap |= (uint64_t (0x0000000000000001) << IndexInBitmap (seq));
          break;
        }
      case EXTENDED_COMPRESSED_BLOCK_ACK:
        {
          uint16_t index = IndexInBitmap (seq);
          bitmap.m_extendedCompressedBitmap[index/64] |= (uint64_t (0x0000000000000001) << index);
          break;
        }
      case MULTI_TID_BLOCK_ACK:
        {
          NS_FATAL_ERROR ("Multi-tid block ack is not supported.");
          break;
        }
      default:
        {
          NS_FATAL_ERROR ("Invalid BA type");
          break;
        }
    }
}

void
CtrlBAckResponseHeader::SetReceivedFragment (uint16_t seq, uint8_t frag)
{
  NS_ASSERT (frag < 16);
  if (!IsInBitmap (seq))
    {
      return;
    }
  switch (m_baType)
    {
      case BASIC_BLOCK_ACK:
        bitmap.m_bitmap[IndexInBitmap (seq)] |= (0x0001 << frag);
        break;
      case COMPRESSED_BLOCK_ACK:
      case EXTENDED_COMPRESSED_BLOCK_ACK:
        /* We can ignore this...compressed block ack doesn't support
           acknowledgement of single fragments */
        break;
      case MULTI_TID_BLOCK_ACK:
        NS_FATAL_ERROR ("Multi-tid block ack is not supported.");
        break;
      default:
        NS_FATAL_ERROR ("Invalid BA type");
        break;
    }
}

bool
CtrlBAckResponseHeader::IsPacketReceived (uint16_t seq) const
{
  if (!IsInBitmap (seq))
    {
      return false;
    }
  switch (m_baType)
    {
      case BASIC_BLOCK_ACK:
        {
          /*It's impossible to say if an entire packet was correctly received. */
          return false;
        }
      case COMPRESSED_BLOCK_ACK:
        {
          /* Although this could make no sense, if packet with sequence number
             equal to <i>seq</i> was correctly received, also all of its fragments
             were correctly received. */
          uint64_t mask = uint64_t (0x0000000000000001);
          return (((bitmap.m_compressedBitmap >> IndexInBitmap (seq)) & mask) == 1) ? true : false;
        }
      case EXTENDED_COMPRESSED_BLOCK_ACK:
        {
          uint64_t mask = uint64_t (0x0000000000000001);
          uint16_t index = IndexInBitmap (seq);
          return (((bitmap.m_extendedCompressedBitmap[index/64] >> index) & mask) == 1) ? true : false;
        }
      case MULTI_TID_BLOCK_ACK:
        {
          NS_FATAL_ERROR ("Multi-tid block ack is not supported.");
          break;
        }
      default:
        {
          NS_FATAL_ERROR ("Invalid BA type");
          break;
        }
    }
  return false;
}

bool
CtrlBAckResponseHeader::IsFragmentReceived (uint16_t seq, uint8_t frag) const
{
  NS_ASSERT (frag < 16);
  if (!IsInBitmap (seq))
    {
      return false;
    }
  switch (m_baType)
    {
      case BASIC_BLOCK_ACK:
        {
          return ((bitmap.m_bitmap[IndexInBitmap (seq)] & (0x0001 << frag)) != 0x0000) ? true : false;
        }
      case COMPRESSED_BLOCK_ACK:
        {
          /* Although this could make no sense, if packet with sequence number
             equal to <i>seq</i> was correctly received, also all of its fragments
             were correctly received. */
          uint64_t mask = uint64_t (0x0000000000000001);
          return (((bitmap.m_compressedBitmap >> IndexInBitmap (seq)) & mask) == 1) ? true : false;
        }
      case EXTENDED_COMPRESSED_BLOCK_ACK:
        {
          uint64_t mask = uint64_t (0x0000000000000001);
          uint16_t index = IndexInBitmap (seq);
          return (((bitmap.m_extendedCompressedBitmap[index/64] >> index) & mask) == 1) ? true : false;
        }
      case MULTI_TID_BLOCK_ACK:
        {
          NS_FATAL_ERROR ("Multi-tid block ack is not supported.");
          break;
        }
      default:
        {
          NS_FATAL_ERROR ("Invalid BA type");
          break;
        }
    }
  return false;
}

uint16_t
CtrlBAckResponseHeader::IndexInBitmap (uint16_t seq) const
{
  uint16_t index;
  if (seq >= m_startingSeq)
    {
      index = seq - m_startingSeq;
    }
  else
    {
      index = 4096 - m_startingSeq + seq;
    }
  if (m_baType == EXTENDED_COMPRESSED_BLOCK_ACK)
    {
      NS_ASSERT (index <= 255);
    }
  else
    {
      NS_ASSERT (index <= 63);
    }
  return index;
}

bool
CtrlBAckResponseHeader::IsInBitmap (uint16_t seq) const
{
  if (m_baType == EXTENDED_COMPRESSED_BLOCK_ACK)
    {
      return (seq - m_startingSeq + 4096) % 4096 < 256;
    }
  else
    {
      return (seq - m_startingSeq + 4096) % 4096 < 64;
    }
}

const uint16_t*
CtrlBAckResponseHeader::GetBitmap (void) const
{
  return bitmap.m_bitmap;
}

uint64_t
CtrlBAckResponseHeader::GetCompressedBitmap (void) const
{
  return bitmap.m_compressedBitmap;
}

const uint64_t*
CtrlBAckResponseHeader::GetExtendedCompressedBitmap (void) const
{
  return bitmap.m_extendedCompressedBitmap;
}

void
CtrlBAckResponseHeader::ResetBitmap (void)
{
  memset (&bitmap, 0, sizeof (bitmap));
}


/***********************************
 * Trigger frame - User Info field
 ***********************************/

CtrlTriggerUserInfoField::CtrlTriggerUserInfoField (uint8_t triggerType)
  : m_aid12 (0),
    m_ruAllocation (0),
    m_ulFecCodingType (false),
    m_ulMcs (0),
    m_ulDcm (false),
    m_ulTargetRssi (0),
    m_triggerType (triggerType),
    m_basicTriggerDependentUserInfo (0)
{
  memset (&m_bits26To31, 0, sizeof (m_bits26To31));
}

CtrlTriggerUserInfoField::~CtrlTriggerUserInfoField ()
{
}

void
CtrlTriggerUserInfoField::Print (std::ostream &os) const
{
  os << ", USER_INFO AID=" << m_aid12 << ", RU_Allocation=" << m_ruAllocation << ", MCS=" << +m_ulMcs;
}

uint32_t
CtrlTriggerUserInfoField::GetSerializedSize (void) const
{
  uint32_t size = 0;
  size += 5;   // User Info (excluding Trigger Dependent User Info)

  switch (m_triggerType)
  {
  case BASIC_TRIGGER:
  case BFRP_TRIGGER:
    size += 1;
    break;
  case MU_BAR_TRIGGER:
    size += m_muBarTriggerDependentUserInfo.GetSerializedSize (); // BAR Control and BAR Information
    break;
  // The Trigger Dependent User Info subfield is not present in the other variants
  }

  return size;
}

Buffer::Iterator
CtrlTriggerUserInfoField::Serialize (Buffer::Iterator start) const
{
  NS_ABORT_MSG_IF (m_triggerType == BFRP_TRIGGER, "BFRP Trigger frame is not supported");
  NS_ABORT_MSG_IF (m_triggerType == GCR_MU_BAR_TRIGGER, "GCR-MU-BAR Trigger frame is not supported");
  NS_ABORT_MSG_IF (m_triggerType == NFRP_TRIGGER, "NFRP Trigger frame is not supported");

  Buffer::Iterator i = start;

  uint32_t userInfo = 0;   // User Info except the MSB
  userInfo |= (m_aid12 & 0x0fff);
  userInfo |= (m_ruAllocation << 12);
  userInfo |= (m_ulFecCodingType ? 1 << 20 : 0);
  userInfo |= (m_ulMcs & 0x0f) << 21;
  userInfo |= (m_ulDcm ? 1 << 25 : 0);

  if (m_aid12 != 0 && m_aid12 != 2045)
    {
      userInfo |= (m_bits26To31.ssAllocation.startingSs & 0x07) << 26;
      userInfo |= (m_bits26To31.ssAllocation.nSs & 0x07) << 29;
    }
  else
    {
      userInfo |= (m_bits26To31.raRuInformation.nRaRu & 0x1f) << 26;
      userInfo |= (m_bits26To31.raRuInformation.noMoreRaRu ? 1 << 31 : 0);
    }

  i.WriteHtolsbU32 (userInfo);
  // Here we need to write 8 bits covering the UL Target RSSI (7 bits) and the
  // Reserved bit. Given how m_ulTargetRssi is set, writing m_ulTargetRssi
  // leads to setting the Reserved bit to zero
  i.WriteU8 (m_ulTargetRssi);

  if (m_triggerType == BASIC_TRIGGER)
    {
      i.WriteU8 (m_basicTriggerDependentUserInfo);
    }
  else if (m_triggerType == MU_BAR_TRIGGER)
    {
      m_muBarTriggerDependentUserInfo.Serialize (i);
      i.Next (m_muBarTriggerDependentUserInfo.GetSerializedSize ());
    }

  return i;
}

Buffer::Iterator
CtrlTriggerUserInfoField::Deserialize (Buffer::Iterator start)
{
  NS_ABORT_MSG_IF (m_triggerType == BFRP_TRIGGER, "BFRP Trigger frame is not supported");
  NS_ABORT_MSG_IF (m_triggerType == GCR_MU_BAR_TRIGGER, "GCR-MU-BAR Trigger frame is not supported");
  NS_ABORT_MSG_IF (m_triggerType == NFRP_TRIGGER, "NFRP Trigger frame is not supported");

  Buffer::Iterator i = start;

  uint32_t userInfo = i.ReadLsbtohU32 ();

  m_aid12 = userInfo & 0x0fff;
  NS_ABORT_MSG_IF (m_aid12 == 4095, "Cannot deserialize a Padding field");
  m_ruAllocation = (userInfo >> 12) & 0xff;
  m_ulFecCodingType = (userInfo >> 20) & 0x01;
  m_ulMcs = (userInfo >> 21) & 0x0f;
  m_ulDcm = (userInfo >> 25) & 0x01;

  if (m_aid12 != 0 && m_aid12 != 2045)
    {
      m_bits26To31.ssAllocation.startingSs = (userInfo >> 26) & 0x07;
      m_bits26To31.ssAllocation.nSs = (userInfo >> 29) & 0x07;
    }
  else
    {
      m_bits26To31.raRuInformation.nRaRu = (userInfo >> 26) & 0x1f;
      m_bits26To31.raRuInformation.noMoreRaRu = (userInfo >> 31) & 0x01;
    }

  m_ulTargetRssi = i.ReadU8 () & 0x7f;  // B39 is reserved

  if (m_triggerType == BASIC_TRIGGER)
    {
      m_basicTriggerDependentUserInfo= i.ReadU8 ();
    }
  else if (m_triggerType == MU_BAR_TRIGGER)
    {
      uint32_t len = m_muBarTriggerDependentUserInfo.Deserialize (i);
      i.Next (len);
    }

  return i;
}

void
CtrlTriggerUserInfoField::SetAid12 (uint16_t aid)
{
  m_aid12 = aid & 0x0fff;
}

uint16_t
CtrlTriggerUserInfoField::GetAid12 (void) const
{
  return m_aid12;
}

bool
CtrlTriggerUserInfoField::HasRaRuForAssociatedSta (void) const
{
  return (m_aid12 == 0);
}

bool
CtrlTriggerUserInfoField::HasRaRuForUnassociatedSta (void) const
{
  return (m_aid12 == 2045);
}

void
CtrlTriggerUserInfoField::SetRuAllocation (HeRu::RuSpec ru)
{
  NS_ABORT_MSG_IF (ru.index == 0, "Valid indices start at 1");

  switch (ru.ruType)
    {
    case HeRu::RU_26_TONE:
      m_ruAllocation = ru.index - 1;
      break;
    case HeRu::RU_52_TONE:
      m_ruAllocation = ru.index + 36;
      break;
    case HeRu::RU_106_TONE:
      m_ruAllocation = ru.index + 52;
      break;
    case HeRu::RU_242_TONE:
      m_ruAllocation = ru.index + 60;
      break;
    case HeRu::RU_484_TONE:
      m_ruAllocation = ru.index + 64;
      break;
    case HeRu::RU_996_TONE:
      m_ruAllocation = 67;
      break;
    case HeRu::RU_2x996_TONE:
      m_ruAllocation = 68;
      break;
    default:
      NS_FATAL_ERROR ("RU type unknown.");
      break;
    }

  NS_ABORT_MSG_IF (m_ruAllocation > 68, "Reserved value.");

  m_ruAllocation += (ru.primary80MHz ? 0 : 0x80);
}

HeRu::RuSpec
CtrlTriggerUserInfoField::GetRuAllocation (void) const
{
  HeRu::RuSpec ru;

  ru.primary80MHz = ((m_ruAllocation & 0x80) == 0);

  uint8_t val = m_ruAllocation & 0x7f;

  if (val < 37)
    {
      ru.ruType = HeRu::RU_26_TONE;
      ru.index = val + 1;
    }
  else if (val < 53)
    {
      ru.ruType = HeRu::RU_52_TONE;
      ru.index = val - 36;
    }
  else if (val < 61)
    {
      ru.ruType = HeRu::RU_106_TONE;
      ru.index = val - 52;
    }
  else if (val < 65)
    {
      ru.ruType = HeRu::RU_242_TONE;
      ru.index = val - 60;
    }
  else if (val < 67)
    {
      ru.ruType = HeRu::RU_484_TONE;
      ru.index = val - 64;
    }
  else if (val == 67)
    {
      ru.ruType = HeRu::RU_996_TONE;
      ru.index = 1;
    }
  else if (val == 68)
    {
      ru.ruType = HeRu::RU_2x996_TONE;
      ru.index = 1;
    }
  else
    {
      NS_FATAL_ERROR ("Reserved value.");
    }

  return ru;
}

void
CtrlTriggerUserInfoField::SetUlFecCodingType (bool ldpc)
{
  m_ulFecCodingType = ldpc;
}

bool
CtrlTriggerUserInfoField::GetUlFecCodingType (void) const
{
  return m_ulFecCodingType;
}

void
CtrlTriggerUserInfoField::SetUlMcs (uint8_t mcs)
{
  NS_ABORT_MSG_IF (mcs > 11, "Invalid MCS index");
  m_ulMcs = mcs;
}

uint8_t
CtrlTriggerUserInfoField::GetUlMcs (void) const
{
  return m_ulMcs;
}

void
CtrlTriggerUserInfoField::SetUlDcm (bool dcm)
{
  m_ulDcm = dcm;
}

bool
CtrlTriggerUserInfoField::GetUlDcm (void) const
{
  return m_ulDcm;
}

void
CtrlTriggerUserInfoField::SetSsAllocation (uint8_t startingSs, uint8_t nSs)
{
  NS_ABORT_MSG_IF (m_aid12 == 0 || m_aid12 == 2045, "SS Allocation subfield not present");
  NS_ABORT_MSG_IF (!startingSs || startingSs > 8, "Starting SS must be from 1 to 8");
  NS_ABORT_MSG_IF (!nSs || nSs > 8, "Number of SS must be from 1 to 8");

  m_bits26To31.ssAllocation.startingSs = startingSs - 1;
  m_bits26To31.ssAllocation.nSs = nSs - 1;
}

uint8_t
CtrlTriggerUserInfoField::GetStartingSs (void) const
{
  if (m_aid12 == 0 || m_aid12 == 2045)
    {
      return 1;
    }
  return m_bits26To31.ssAllocation.startingSs + 1;
}

uint8_t
CtrlTriggerUserInfoField::GetNss (void) const
{
  if (m_aid12 == 0 || m_aid12 == 2045)
    {
      return 1;
    }
  return m_bits26To31.ssAllocation.nSs + 1;
}

void
CtrlTriggerUserInfoField::SetRaRuInformation (uint8_t nRaRu, bool noMoreRaRu)
{
  NS_ABORT_MSG_IF (m_aid12 != 0 && m_aid12 != 2045, "RA-RU Information subfield not present");
  NS_ABORT_MSG_IF (!nRaRu || nRaRu > 32, "Number of contiguous RA-RUs must be from 1 to 32");

  m_bits26To31.raRuInformation.nRaRu = nRaRu - 1;
  m_bits26To31.raRuInformation.noMoreRaRu = noMoreRaRu;
}

uint8_t
CtrlTriggerUserInfoField::GetNRaRus (void) const
{
  NS_ABORT_MSG_IF (m_aid12 != 0 && m_aid12 != 2045, "RA-RU Information subfield not present");

  return m_bits26To31.raRuInformation.nRaRu + 1;
}

bool
CtrlTriggerUserInfoField::GetNoMoreRaRu (void) const
{
  NS_ABORT_MSG_IF (m_aid12 != 0 && m_aid12 != 2045, "RA-RU Information subfield not present");

  return m_bits26To31.raRuInformation.noMoreRaRu;
}

void
CtrlTriggerUserInfoField::SetUlTargetRssiMaxTxPower (void)
{
  m_ulTargetRssi = 127;  // see Table 9-25i of 802.11ax amendment D3.0
}

void
CtrlTriggerUserInfoField::SetUlTargetRssi (int8_t dBm)
{
  NS_ABORT_MSG_IF (dBm < -110 || dBm > -20, "Invalid values for signal power");

  m_ulTargetRssi = static_cast<uint8_t> (110 + dBm);
}

bool
CtrlTriggerUserInfoField::IsUlTargetRssiMaxTxPower (void) const
{
  return (m_ulTargetRssi == 127);
}

int8_t
CtrlTriggerUserInfoField::GetUlTargetRssi (void) const
{
  NS_ABORT_MSG_IF (m_ulTargetRssi == 127, "STA must use its max TX power");

  return static_cast<int8_t> (m_ulTargetRssi) - 110;
}

void
CtrlTriggerUserInfoField::SetBasicTriggerDepUserInfo (uint8_t spacingFactor, uint8_t tidLimit, AcIndex prefAc)
{
  NS_ABORT_MSG_IF (m_triggerType != BASIC_TRIGGER, "Not a Basic Trigger frame");

  m_basicTriggerDependentUserInfo = (spacingFactor & 0x03)
                                     | (tidLimit & 0x07) << 2
                                     // B5 is reserved
                                     | (prefAc & 0x03) << 6;
}

uint8_t
CtrlTriggerUserInfoField::GetMpduMuSpacingFactor (void) const
{
  NS_ABORT_MSG_IF (m_triggerType != BASIC_TRIGGER, "Not a Basic Trigger frame");

  return m_basicTriggerDependentUserInfo & 0x03;
}

uint8_t
CtrlTriggerUserInfoField::GetTidAggregationLimit (void) const
{
  NS_ABORT_MSG_IF (m_triggerType != BASIC_TRIGGER, "Not a Basic Trigger frame");

  return (m_basicTriggerDependentUserInfo & 0x1c) >> 2;
}

AcIndex
CtrlTriggerUserInfoField::GetPreferredAc (void) const
{
  NS_ABORT_MSG_IF (m_triggerType != BASIC_TRIGGER, "Not a Basic Trigger frame");

  return AcIndex ((m_basicTriggerDependentUserInfo & 0xc0) >> 6);
}

void
CtrlTriggerUserInfoField::SetMuBarTriggerDepUserInfo (const CtrlBAckRequestHeader& bar)
{
  NS_ABORT_MSG_IF (m_triggerType != MU_BAR_TRIGGER, "Not a MU-BAR Trigger frame");
  NS_ABORT_MSG_IF (bar.GetType () != COMPRESSED_BLOCK_ACK && bar.GetType () != MULTI_TID_BLOCK_ACK,
                   "BAR Control indicates it is neither the Compressed nor the Multi-TID variant");
  m_muBarTriggerDependentUserInfo = bar;
}

const CtrlBAckRequestHeader&
CtrlTriggerUserInfoField::GetMuBarTriggerDepUserInfo (void) const
{
  NS_ABORT_MSG_IF (m_triggerType != MU_BAR_TRIGGER, "Not a MU-BAR Trigger frame");

  return m_muBarTriggerDependentUserInfo;
}

/***********************************
 *       Trigger frame
 ***********************************/

NS_OBJECT_ENSURE_REGISTERED (CtrlTriggerHeader);

CtrlTriggerHeader::CtrlTriggerHeader ()
  : m_triggerType (0),
    m_ulLength (0),
    m_moreTF (false),
    m_csRequired (false),
    m_ulBandwidth (0),
    m_apTxPower (0),
    m_ulSpatialReuse (0)
{
}

CtrlTriggerHeader::~CtrlTriggerHeader ()
{
}

TypeId
CtrlTriggerHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CtrlTriggerHeader")
    .SetParent<Header> ()
    .SetGroupName ("Wifi")
    .AddConstructor<CtrlTriggerHeader> ()
  ;
  return tid;
}

TypeId
CtrlTriggerHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
CtrlTriggerHeader::Print (std::ostream &os) const
{
  os << "TriggerType=" << m_triggerType << ", Bandwidth=" << GetUlBandwidth ();

  for (auto& ui : m_userInfoFields)
    {
      ui.Print (os);
    }
}

uint32_t
CtrlTriggerHeader::GetSerializedSize (void) const
{
  uint32_t size = 0;
  size += 8;  // Common Info (excluding Trigger Dependent Common Info)

  // Add the size of the Trigger Dependent Common Info subfield
  if (m_triggerType == GCR_MU_BAR_TRIGGER)
    {
      size += 4;
    }

  for (auto& ui : m_userInfoFields)
    {
      size += ui.GetSerializedSize ();
    }

  size += 2;  // Padding field

  return size;
}

void
CtrlTriggerHeader::Serialize (Buffer::Iterator start) const
{
  NS_ABORT_MSG_IF (m_triggerType == BFRP_TRIGGER, "BFRP Trigger frame is not supported");
  NS_ABORT_MSG_IF (m_triggerType == GCR_MU_BAR_TRIGGER, "GCR-MU-BAR Trigger frame is not supported");
  NS_ABORT_MSG_IF (m_triggerType == NFRP_TRIGGER, "NFRP Trigger frame is not supported");

  Buffer::Iterator i = start;

  uint64_t commonInfo = 0;
  commonInfo |= (m_triggerType & 0x0f);
  commonInfo |= (m_ulLength & 0x0fff) << 4;
  commonInfo |= (m_moreTF ? 1 << 16 : 0);
  commonInfo |= (m_csRequired ? 1 << 17 : 0);
  commonInfo |= (m_ulBandwidth & 0x03) << 18;
  commonInfo |= static_cast<uint64_t> (m_apTxPower & 0x3f) << 28;
  commonInfo |= static_cast<uint64_t> (m_ulSpatialReuse) << 37;

  i.WriteHtolsbU64 (commonInfo);

  for (auto& ui : m_userInfoFields)
    {
      i = ui.Serialize (i);
    }

  i.WriteHtolsbU16 (0xffff);   // Padding field, used as delimiter
}

uint32_t
CtrlTriggerHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  uint64_t commonInfo = i.ReadLsbtohU64 ();

  m_triggerType = (commonInfo & 0x0f);
  m_ulLength = (commonInfo >> 4) & 0x0fff;
  m_moreTF = (commonInfo >> 16) & 0x01;
  m_csRequired = (commonInfo >> 17) & 0x01;
  m_ulBandwidth = (commonInfo >> 18) & 0x03;
  m_apTxPower = (commonInfo >> 28) & 0x3f;
  m_ulSpatialReuse = (commonInfo >> 37) & 0xffff;

  NS_ABORT_MSG_IF (m_triggerType == BFRP_TRIGGER, "BFRP Trigger frame is not supported");
  NS_ABORT_MSG_IF (m_triggerType == GCR_MU_BAR_TRIGGER, "GCR-MU-BAR Trigger frame is not supported");
  NS_ABORT_MSG_IF (m_triggerType == NFRP_TRIGGER, "NFRP Trigger frame is not supported");

  bool isPadding = false;

  // We always add a Padding field (of two octets of all 1s) as delimiter
  while (!isPadding)
    {
      // read the first 2 bytes to check if we encountered the Padding field
      if (i.ReadU16 () == 0xffff)
        {
          isPadding = true;
        }
      else
        {
          // go back 2 bytes to deserialize the User Info field from the beginning
          i.Prev (2);
          CtrlTriggerUserInfoField& ui = AddUserInfoField ();
          i = ui.Deserialize (i);
        }
    }

  return i.GetDistanceFrom (start);
}

void
CtrlTriggerHeader::SetType (TriggerFrameType type)
{
  m_triggerType = type;
}

bool
CtrlTriggerHeader::IsBasic (void) const
{
  return (m_triggerType == BASIC_TRIGGER);
}

bool
CtrlTriggerHeader::IsBfrp (void) const
{
  return (m_triggerType == BFRP_TRIGGER);
}

bool
CtrlTriggerHeader::IsMuBar (void) const
{
  return (m_triggerType == MU_BAR_TRIGGER);
}

bool
CtrlTriggerHeader::IsMuRts (void) const
{
  return (m_triggerType == MU_RTS_TRIGGER);
}

bool
CtrlTriggerHeader::IsBsrp (void) const
{
  return (m_triggerType == BSRP_TRIGGER);
}

bool
CtrlTriggerHeader::IsGcrMuBar (void) const
{
  return (m_triggerType == GCR_MU_BAR_TRIGGER);
}

bool
CtrlTriggerHeader::IsBqrp (void) const
{
  return (m_triggerType == BQRP_TRIGGER);
}

bool
CtrlTriggerHeader::IsNfrp (void) const
{
  return (m_triggerType == NFRP_TRIGGER);
}

void
CtrlTriggerHeader::SetUlLength (uint16_t len)
{
  m_ulLength = (len & 0x0fff);
}

uint16_t
CtrlTriggerHeader::GetUlLength (void) const
{
  return m_ulLength;
}

void
CtrlTriggerHeader::SetMoreTF (bool more)
{
  m_moreTF = more;
}

bool
CtrlTriggerHeader::GetMoreTF (void) const
{
  return m_moreTF;
}

void
CtrlTriggerHeader::SetCsRequired (bool cs)
{
  m_csRequired = cs;
}

bool
CtrlTriggerHeader::GetCsRequired (void) const
{
  return m_csRequired;
}

void
CtrlTriggerHeader::SetUlBandwidth (uint8_t bw)
{
  switch (bw)
    {
    case 20:
      m_ulBandwidth = 0;
      break;
    case 40:
      m_ulBandwidth = 1;
      break;
    case 80:
      m_ulBandwidth = 2;
      break;
    case 160:
      m_ulBandwidth = 3;
      break;
    default:
      NS_FATAL_ERROR ("Bandwidth value not allowed.");
      break;
    }
}

uint8_t
CtrlTriggerHeader::GetUlBandwidth (void) const
{
  return (1 << m_ulBandwidth) * 20;
}

void
CtrlTriggerHeader::SetApTxPower (int8_t power)
{
  // see Table 9-25f "AP Tx Power subfield encoding" of 802.11ax amendment D3.0
  NS_ABORT_MSG_IF (power < -20 || power > 40, "Out of range power values");

  m_apTxPower = static_cast<uint8_t> (power + 20);
}

int8_t
CtrlTriggerHeader::GetApTxPower (void) const
{
  // see Table 9-25f "AP Tx Power subfield encoding" of 802.11ax amendment D3.0
  return static_cast<int8_t> (m_apTxPower) - 20;
}

void
CtrlTriggerHeader::SetUlSpatialReuse (uint16_t sr)
{
  m_ulSpatialReuse = sr;
}

uint16_t
CtrlTriggerHeader::GetUlSpatialReuse (void) const
{
  return m_ulSpatialReuse;
}

CtrlTriggerHeader
CtrlTriggerHeader::GetCommonInfoField (void) const
{
  // make a copy of this Trigger Frame and remove the User Info fields from the copy
  CtrlTriggerHeader trigger (*this);
  trigger.m_userInfoFields.clear ();
  return trigger;
}

CtrlTriggerUserInfoField&
CtrlTriggerHeader::AddUserInfoField (void)
{
  m_userInfoFields.emplace_back (m_triggerType);
  return m_userInfoFields.back ();
}

CtrlTriggerHeader::ConstIterator
CtrlTriggerHeader::begin (void) const
{
  return m_userInfoFields.begin ();
}

CtrlTriggerHeader::ConstIterator
CtrlTriggerHeader::end (void) const
{
  return m_userInfoFields.end ();
}

CtrlTriggerHeader::Iterator
CtrlTriggerHeader::begin (void)
{
  return m_userInfoFields.begin ();
}

CtrlTriggerHeader::Iterator
CtrlTriggerHeader::end (void)
{
  return m_userInfoFields.end ();
}

std::size_t
CtrlTriggerHeader::GetNUserInfoFields (void) const
{
  return m_userInfoFields.size ();
}

CtrlTriggerHeader::ConstIterator
CtrlTriggerHeader::FindUserInfoWithAid (ConstIterator start, uint16_t aid12) const
{
  // the lambda function returns true if a User Info field has the AID12 subfield
  // equal to the given aid12 value
  return std::find_if (start, end (), [aid12] (const CtrlTriggerUserInfoField& ui) -> bool
                                      { return (ui.GetAid12 () == aid12); });
}

CtrlTriggerHeader::ConstIterator
CtrlTriggerHeader::FindUserInfoWithAid (uint16_t aid12) const
{
  return FindUserInfoWithAid (m_userInfoFields.begin (), aid12);
}

CtrlTriggerHeader::ConstIterator
CtrlTriggerHeader::FindUserInfoWithRaRuAssociated (ConstIterator start) const
{
  return FindUserInfoWithAid (start, 0);
}

CtrlTriggerHeader::ConstIterator
CtrlTriggerHeader::FindUserInfoWithRaRuAssociated (void) const
{
  return FindUserInfoWithAid (0);
}

CtrlTriggerHeader::ConstIterator
CtrlTriggerHeader::FindUserInfoWithRaRuUnassociated (ConstIterator start) const
{
  return FindUserInfoWithAid (start, 2045);
}

CtrlTriggerHeader::ConstIterator
CtrlTriggerHeader::FindUserInfoWithRaRuUnassociated (void) const
{
  return FindUserInfoWithAid (2045);
}

bool
CtrlTriggerHeader::IsValid (void) const
{
  // check that allocated RUs do not overlap
  // TODO This is not a problem in case of UL MU-MIMO
  std::vector<HeRu::RuSpec> prevRus;

  for (auto& ui : m_userInfoFields)
    {
      if (HeRu::DoesOverlap (GetUlBandwidth (), ui.GetRuAllocation (), prevRus))
        {
          return false;
        }
      prevRus.push_back (ui.GetRuAllocation ());
    }
  return true;
}

}  //namespace ns3

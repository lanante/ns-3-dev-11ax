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

#ifndef CTRL_HEADERS_H
#define CTRL_HEADERS_H

#include <list>
#include "ns3/header.h"
#include "block-ack-type.h"
#include "trigger-frame-type.h"
#include "he-ru.h"
#include "qos-utils.h"

namespace ns3 {

/**
 * \ingroup wifi
 * \brief Headers for Block ack request.
 *
 *  802.11n standard includes three types of block ack:
 *    - Basic block ack (unique type in 802.11e)
 *    - Compressed block ack
 *    - Multi-TID block ack
 *  For now only basic block ack and compressed block ack
 *  are supported.
 *  Basic block ack is also default variant.
 */
class CtrlBAckRequestHeader : public Header
{
public:
  CtrlBAckRequestHeader ();
  ~CtrlBAckRequestHeader ();
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  void Print (std::ostream &os) const;
  uint32_t GetSerializedSize (void) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);

  /**
   * Enable or disable HT immediate ACK.
   *
   * \param immediateAck enable or disable HT immediate ACK
   */
  void SetHtImmediateAck (bool immediateAck);
  /**
   * Set the block ACK type.
   *
   * \param type
   */
  void SetType (BlockAckType type);
  /**
   * Set Traffic ID (TID).
   *
   * \param tid
   */
  void SetTidInfo (uint8_t tid);
  /**
   * Set the starting sequence number from the given
   * raw sequence control field.
   *
   * \param seq the raw sequence control
   */
  void SetStartingSequence (uint16_t seq);

  /**
   * Check if the current ACK policy is immediate.
   *
   * \return true if the current ACK policy is immediate,
   *         false otherwise
   */
  bool MustSendHtImmediateAck (void) const;
  /**
   * Return the block ACK type ID.
   *
   * \return type
   */
  BlockAckType GetType (void) const;
  /**
   * Return the Traffic ID (TID).
   *
   * \return TID
   */
  uint8_t GetTidInfo (void) const;
  /**
   * Return the starting sequence number.
   *
   * \return the starting sequence number
   */
  uint16_t GetStartingSequence (void) const;
  /**
   * Check if the current ACK policy is basic
   * (i.e. not multiple TID and not compressed ACK).
   *
   * \return true if the current ACK policy is basic,
   *         false otherwise
   */
  bool IsBasic (void) const;
  /**
   * Check if the current ACK policy is compressed ACK
   * and not multiple TID.
   *
   * \return true if the current ACK policy is compressed ACK,
   *         false otherwise
   */
  bool IsCompressed (void) const;
  /**
   * Check if the current BA policy is extended compressed block ACK.
   *
   * \return true if the current BA policy is extended compressed block ACK,
   *         false otherwise
   */
  bool IsExtendedCompressed (void) const;
  /**
   * Check if the current ACK policy has multiple TID.
   *
   * \return true if the current ACK policy has multiple TID,
   *         false otherwise
   */
  bool IsMultiTid (void) const;

  /**
   * Return the starting sequence control.
   *
   * \return the starting sequence control
   */
  uint16_t GetStartingSequenceControl (void) const;


private:
  /**
   * Set the starting sequence control with the given
   * sequence control value
   *
   * \param seqControl
   */
  void SetStartingSequenceControl (uint16_t seqControl);
  /**
   * Return the Block ACK control.
   *
   * \return the Block ACK control
   */
  uint16_t GetBarControl (void) const;
  /**
   * Set the Block ACK control.
   *
   * \param bar
   */
  void SetBarControl (uint16_t bar);

  /**
   * The lsb bit of the BAR control field is used only for the
   * HT (High Throughput) delayed block ack configuration.
   * For now only non HT immediate block ack is implemented so this field
   * is here only for a future implementation of HT delayed variant.
   */
  bool m_barAckPolicy;    ///< bar ack policy
  BlockAckType m_baType;  ///< BA type
  uint16_t m_tidInfo;     ///< TID info
  uint16_t m_startingSeq; ///< starting seq
};


/**
 * \ingroup wifi
 * \brief Headers for Block ack response.
 *
 *  802.11n standard includes three types of block ack:
 *    - Basic block ack (unique type in 802.11e)
 *    - Compressed block ack
 *    - Multi-TID block ack
 *  For now only basic block ack and compressed block ack
 *  are supported.
 *  Basic block ack is also default variant.
 */
class CtrlBAckResponseHeader : public Header
{
public:
  CtrlBAckResponseHeader ();
  ~CtrlBAckResponseHeader ();
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  void Print (std::ostream &os) const;
  uint32_t GetSerializedSize (void) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);

  /**
   * Enable or disable HT immediate ACK.
   *
   * \param immediateAck enable or disable HT immediate ACK
   */
  void SetHtImmediateAck (bool immediateAck);
  /**
   * Set the block ACK type.
   *
   * \param type
   */
  void SetType (BlockAckType type);
  /**
   * Set Traffic ID (TID).
   *
   * \param tid
   */
  void SetTidInfo (uint8_t tid);
  /**
   * Set the starting sequence number from the given
   * raw sequence control field.
   *
   * \param seq the raw sequence control
   */
  void SetStartingSequence (uint16_t seq);

  /**
   * Check if the current ACK policy is immediate.
   *
   * \return true if the current ACK policy is immediate,
   *         false otherwise
   */
  bool MustSendHtImmediateAck (void) const;
  /**
   * Return the block ACK type ID.
   *
   * \return type
   */
  BlockAckType GetType (void) const;
  /**
   * Return the Traffic ID (TID).
   *
   * \return TID
   */
  uint8_t GetTidInfo (void) const;
  /**
   * Return the starting sequence number.
   *
   * \return the starting sequence number
   */
  uint16_t GetStartingSequence (void) const;
  /**
   * Check if the current BA policy is basic block ACK.
   *
   * \return true if the current BA policy is basic block ACK,
   *         false otherwise
   */
  bool IsBasic (void) const;
  /**
   * Check if the current BA policy is compressed block ACK.
   *
   * \return true if the current BA policy is compressed block ACK,
   *         false otherwise
   */
  bool IsCompressed (void) const;
  /**
   * Check if the current BA policy is extended compressed block ACK.
   *
   * \return true if the current BA policy is extended compressed block ACK,
   *         false otherwise
   */
  bool IsExtendedCompressed (void) const;
  /**
   * Check if the current BA policy is multi-TID block ACK.
   *
   * \return true if the current BA policy is multi-TID block ACK,
   *         false otherwise
   */
  bool IsMultiTid (void) const;

  /**
   * Set the bitmap that the packet with the given sequence
   * number was received.
   *
   * \param seq
   */
  void SetReceivedPacket (uint16_t seq);
  /**
   * Set the bitmap that the packet with the given sequence
   * number and fragment number was received.
   *
   * \param seq
   * \param frag
   */
  void SetReceivedFragment (uint16_t seq, uint8_t frag);
  /**
   * Check if the packet with the given sequence number
   * was ACKed in this Block ACK response.
   *
   * \param seq
   * \return true if the packet with the given sequence number
   *         was ACKed in this Block ACK response, false otherwise
   */
  bool IsPacketReceived (uint16_t seq) const;
  /**
   * Check if the packet with the given sequence number
   * and fragment number was ACKed in this Block ACK response.
   *
   * \param seq
   * \param frag
   * \return true if the packet with the given sequence number
   *         and sequence number was ACKed in this Block ACK response,
   *         false otherwise
   */
  bool IsFragmentReceived (uint16_t seq, uint8_t frag) const;

  /**
   * Return the starting sequence control.
   *
   * \return the starting sequence control
   */
  uint16_t GetStartingSequenceControl (void) const;
  /**
   * Set the starting sequence control with the given
   * sequence control value
   *
   * \param seqControl
   */
  void SetStartingSequenceControl (uint16_t seqControl);
  /**
   * Return the bitmap from the block ACK response header.
   *
   * \return the bitmap from the block ACK response header
   */

  const uint16_t* GetBitmap (void) const;
  /**
   * Return the compressed bitmap from the block ACK response header.
   *
   * \return the compressed bitmap from the block ACK response header
   */
  uint64_t GetCompressedBitmap (void) const;
  /**
   * Return the extended compressed bitmap from the block ACK response header.
   *
   * \return the extended compressed bitmap from the block ACK response header
   */
  const uint64_t* GetExtendedCompressedBitmap (void) const;

  /**
   * Reset the bitmap to 0.
   */
  void ResetBitmap (void);


private:
  /**
   * Return the block ACK control.
   *
   * \return the block ACK control
   */
  uint16_t GetBaControl (void) const;
  /**
   * Set the block ACK control.
   *
   * \param bar
   */
  void SetBaControl (uint16_t bar);

  /**
   * Serialize bitmap to the given buffer.
   *
   * \param start
   * \return Buffer::Iterator to the next available buffer
   */
  Buffer::Iterator SerializeBitmap (Buffer::Iterator start) const;
  /**
   * Deserialize bitmap from the given buffer.
   *
   * \param start
   * \return Buffer::Iterator to the next available buffer
   */
  Buffer::Iterator DeserializeBitmap (Buffer::Iterator start);

  /**
   * This function is used to correctly index in both bitmap
   * and compressed bitmap, one bit or one block of 16 bits respectively.
   *
   * for more details see 7.2.1.8 in IEEE 802.11n/D4.00
   *
   * \param seq the sequence number
   *
   * \return If we are using basic block ack, return value represents index of
   * block of 16 bits for packet having sequence number equals to <i>seq</i>.
   * If we are using compressed block ack, return value represents bit
   * to set to 1 in the compressed bitmap to indicate that packet having
   * sequence number equals to <i>seq</i> was correctly received.
   */
  uint16_t IndexInBitmap (uint16_t seq) const;

  /**
   * Checks if sequence number <i>seq</i> can be acknowledged in the bitmap.
   *
   * \param seq the sequence number
   *
   * \return
   */
  bool IsInBitmap (uint16_t seq) const;

  /**
   * The lsb bit of the BA control field is used only for the
   * HT (High Throughput) delayed block ack configuration.
   * For now only non HT immediate block ack is implemented so this field
   * is here only for a future implementation of HT delayed variant.
   */
  bool m_baAckPolicy;     ///< BA ack policy
  BlockAckType m_baType;  ///< BA type
  uint16_t m_tidInfo;     ///< TID info
  uint16_t m_startingSeq; ///< starting seq

  union
  {
    uint16_t m_bitmap[64]; ///< the block ack bitmap
    uint64_t m_compressedBitmap; ///< the compressed block ack bitmap
    uint64_t m_extendedCompressedBitmap[4]; ///< the extended compressed block ack bitmap
  } bitmap; ///< bitmap union type
};


/**
 * \ingroup wifi
 * \brief User Info field of Trigger frames.
 *
 * Trigger frames, introduced by 802.11ax amendment (see Section 9.3.1.23 of D3.0),
 * include one or more User Info fields, each of which carries information about the
 * HE TB PPDU that the addressed station sends in response to the Trigger frame.
 */
class CtrlTriggerUserInfoField
{
public:
  /**
   * Constructor
   *
   * \param triggerType the Trigger frame type
   */
  CtrlTriggerUserInfoField (uint8_t triggerType);
  /**
   * Destructor
   */
  ~CtrlTriggerUserInfoField ();
  /**
   * Print the content of this User Info field
   *
   * \param os output stream
   */
  void Print (std::ostream &os) const;
  /**
   * Get the expected size of this User Info field
   *
   * \return the expected size of this User Info field.
   */
  uint32_t GetSerializedSize (void) const;
  /**
   * Serialize the User Info field to the given buffer.
   *
   * \param start
   * \return Buffer::Iterator to the next available buffer
   */
  Buffer::Iterator Serialize (Buffer::Iterator start) const;
  /**
   * Deserialize the User Info field from the given buffer.
   *
   * \param start
   * \return Buffer::Iterator to the next available buffer
   */
  Buffer::Iterator Deserialize (Buffer::Iterator start);
  /**
   * Set the AID12 subfield, which carries the 12 LSBs of the AID of the
   * station for which this User Info field is intended. The whole AID can
   * be passed, since the passed value is properly masked.
   *
   * \param aid the value for the AID12 subfield
   */
  void SetAid12 (uint16_t aid);
  /**
   * Get the value of the AID12 subfield.
   *
   * \return the AID12 subfield
   */
  uint16_t GetAid12 (void) const;
  /**
   * Check if this User Info field allocates a Random Access RU for stations
   * associated with the AP that transmitted the Trigger frame.
   *
   * \return true if a Random Access RU for associated stations is allocated
   */
  bool HasRaRuForAssociatedSta (void) const;
  /**
   * Check if this User Info field allocates a Random Access RU for stations
   * not associated with the AP that transmitted the Trigger frame.
   *
   * \return true if a Random Access RU for unassociated stations is allocated
   */
  bool HasRaRuForUnassociatedSta (void) const;
  /**
   * Set the RU Allocation subfield according to the specified RU.
   *
   * \param ru the RU this User Info field is allocating
   */
  void SetRuAllocation (HeRu::RuSpec ru);
  /**
   * Get the RU specified by the RU Allocation subfield.
   *
   * \return the RU this User Info field is allocating
   */
  HeRu::RuSpec GetRuAllocation (void) const;
  /**
   * Set the UL FEC Coding Type subfield, which indicates whether BCC or LDPC is used
   *
   * \param ldpc whether to use LDPC or not
   */
  void SetUlFecCodingType (bool ldpc);
  /**
   * Get the UL FEC Coding Type subfield, which indicates whether BCC or LDPC is used
   *
   * \return true if LDPC is used
   */
  bool GetUlFecCodingType (void) const;
  /**
   * Set the UL MCS subfield, which indicates the MCS of the solicited HE TB PPDU
   *
   * \param mcs the MCS index (a value between 0 and 11)
   */
  void SetUlMcs (uint8_t mcs);
  /**
   * Get the UL MCS subfield, which indicates the MCS of the solicited HE TB PPDU
   *
   * \return the MCS index (a value between 0 and 11)
   */
  uint8_t GetUlMcs (void) const;
  /**
   * Set the UL DCM subfield, which indicates whether or not DCM is used
   *
   * \param dcm whether to use DCM or not
   */
  void SetUlDcm (bool dcm);
  /**
   * Get the UL DCM subfield, which indicates whether or not DCM is used
   *
   * \return true if DCM is used
   */
  bool GetUlDcm (void) const;
  /**
   * Set the SS Allocation subfield, which is present when the AID12 subfield
   * is neither 0 nor 2045. This method must be called after setting the AID12
   * subfield to a value other than 0 and 2045.
   *
   * \param startingSs the starting spatial stream (a value from 1 to 8)
   * \param nSs the number of spatial streams (a value from 1 to 8)
   */
  void SetSsAllocation (uint8_t startingSs, uint8_t nSs);
  /**
   * Get the starting spatial stream.
   *
   * \return the starting spatial stream (a value between 1 and 8)
   */
  uint8_t GetStartingSs (void) const;
  /**
   * Get the number of spatial streams.
   *
   * \return the number of spatial streams (a value between 1 and 8)
   */
  uint8_t GetNss (void) const;
  /**
   * Set the RA-RU Information subfield, which is present when the AID12 subfield
   * is 0 or 2045. This method must be called after setting the AID12 subfield to
   * 0 or 2045.
   *
   * \param nRaRu the number (from 1 to 32) of contiguous RUs allocated for Random Access.
   * \param noMoreRaRu whether RA-RUs are not allocated in subsequent Trigger frames
   */
  void SetRaRuInformation (uint8_t nRaRu, bool noMoreRaRu);
  /**
   * Get the number of contiguous RUs for Random Access. This method can only be
   * called if the AID12 subfield has been set to 0 or 2045
   *
   * \return the number of contiguous RA-RUs (a value between 1 and 32)
   */
  uint8_t GetNRaRus (void) const;
  /**
   * Return true if no more RA-RUs are allocated in subsequent Trigger frames
   * that are sent before the end of the current TXOP. This method can only be
   * called if the AID12 subfield has been set to 0 or 2045
   *
   * \return true if no more RA-RUs are allocated in subsequent Trigger frames
   */
  bool GetNoMoreRaRu (void) const;
  /**
   * Set the UL Target RSSI subfield to indicate to the station to transmit an
   * HE TB PPDU response at its maximum transmit power for the assigned MCS
   */
  void SetUlTargetRssiMaxTxPower (void);
  /**
   * Set the UL Target RSSI subfield to indicate the expected receive signal
   * power in dBm
   *
   * \param dBm the expected receive signal power (a value between -110 and -20)
   */
  void SetUlTargetRssi (int8_t dBm);
  /**
   * Return true if the UL Target RSSI subfield indicates to the station to transmit
   * an HE TB PPDU response at its maximum transmit power for the assigned MCS
   *
   * \return true if the UL Target RSSI subfield indicates to the station to transmit
   * an HE TB PPDU response at its maximum transmit power for the assigned MCS
   */
  bool IsUlTargetRssiMaxTxPower (void) const;
  /**
   * Get the expected receive signal power for the solicited HE TB PPDU. This
   * method can only be called if IsUlTargetRssiMaxTxPower returns false.
   *
   * \return the expected receive signal power in dBm
   */
  int8_t GetUlTargetRssi (void) const;
  /**
   * Set the Trigger Dependent User Info subfield for Basic Trigger frames.
   *
   * \param spacingFactor the MPDU MU spacing factor
   * \param tidLimit the value for the TID Aggregation Limit subfield
   * \param prefAc the lowest AC recommended for aggregation of MPDUs
   */
  void SetBasicTriggerDepUserInfo (uint8_t spacingFactor, uint8_t tidLimit, AcIndex prefAc);
  /**
   * Get the MPDU MU spacing factor. This method can only be called if this
   * User Info field is included in a Basic Trigger frame.
   *
   * \return the MPDU MU spacing factor
   */
  uint8_t GetMpduMuSpacingFactor (void) const;
  /**
   * Get the TID Aggregation Limit. This method can only be called if this
   * User Info field is included in a Basic Trigger frame.
   *
   * \return the TID Aggregation Limit
   */
  uint8_t GetTidAggregationLimit (void) const;
  /**
   * Get the Preferred AC subfield. This method can only be called if this
   * User Info field is included in a Basic Trigger frame.
   *
   * \return the Preferred AC subfield
   */
  AcIndex GetPreferredAc (void) const;
  /**
   * Set the Trigger Dependent User Info subfield for the MU-BAR variant of
   * Trigger frames, which includes a BAR Control subfield and a BAR Information
   * subfield. The BAR Control subfield must indicate either a Compressed
   * BlockAckReq variant or a Multi-TID BlockAckReq variant.
   *
   * \param bar the BlockAckRequest header object including the BAR Control
   *            subfield and the BAR Information subfield
   */
  void SetMuBarTriggerDepUserInfo (const CtrlBAckRequestHeader& bar);
  /**
   * Get the Trigger Dependent User Info subfield for the MU-BAR variant of
   * Trigger frames, which includes a BAR Control subfield and a BAR Information
   * subfield. The BAR Control subfield must indicate either a Compressed
   * BlockAckReq variant or a Multi-TID BlockAckReq variant.
   *
   * \return the BlockAckRequest header object including the BAR Control
   *         subfield and the BAR Information subfield
   */
   const CtrlBAckRequestHeader& GetMuBarTriggerDepUserInfo (void) const;

private:
  uint16_t m_aid12;   //!< Association ID of the addressed station
  uint8_t m_ruAllocation;   //!< RU Allocation
  bool m_ulFecCodingType;   //!< UL FEC Coding Type
  uint8_t m_ulMcs;   //!< MCS to be used by the addressed station
  bool m_ulDcm;   //!< whether or not to use Dual Carrier Modulation
  union
  {
    struct
    {
      uint8_t startingSs;   //!< Starting spatial stream
      uint8_t nSs;   //!< Number of spatial streams
    } ssAllocation;   //!< Used when AID12 is neither 0 nor 2045
    struct
    {
      uint8_t nRaRu;   //!< Number of Random Access RUs
      bool noMoreRaRu;   //!< No more RA-RU in subsequent Trigger frames
    } raRuInformation;   //!< Used when AID12 is 0 or 2045
  } m_bits26To31;   //!< Fields occupying bits 26-31 in the User Info field
  uint8_t m_ulTargetRssi;   //!< Expected receive signal power
  uint8_t m_triggerType; //!< Trigger frame type
  /// Trigger Dependent User Info subfield
  uint8_t m_basicTriggerDependentUserInfo;   //!< Basic Trigger variant
  CtrlBAckRequestHeader m_muBarTriggerDependentUserInfo;   //!< MU-BAR variant
};


/**
 * \ingroup wifi
 * \brief Headers for Trigger frames.
 *
 * 802.11ax amendment defines eight types of Trigger frames (see Section 9.3.1.23 of D3.0):
 *   - Basic
 *   - Beamforming Report Poll (BFRP)
 *   - Multi-User Block Ack Request (MU-BAR)
 *   - Multi-User Request To Send (MU-RTS)
 *   - Buffer Status Report Poll (BSRP)
 *   - Groupcast with Retries (GCR) MU-BAR
 *   - Bandwidth Query Report Poll (BQRP)
 *   - NDP Feedback Report Poll (NFRP)
 * TODO For now only the Basic, MU-BAR, MU-RTS, BSRP and BQRP variants are supported.
 * Basic Trigger is also the default variant.
 *
 * The Padding field is optional, given that other techniques (post-EOF A-MPDU
 * padding, aggregating other MPDUs in the A-MPDU) are available to satisfy the
 * minimum time requirement. Currently, however, a Padding field of the minimum
 * length (2 octets) is appended to every Trigger frame that is transmitted. In
 * such a way, deserialization stops when a User Info field with the AID12 subfield
 * set to 4095 (which indicates the start of a Padding field) is encountered.
 */
class CtrlTriggerHeader : public Header
{
public:
  CtrlTriggerHeader ();
  ~CtrlTriggerHeader ();
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;
  void Print (std::ostream &os) const;
  uint32_t GetSerializedSize (void) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);

  /**
   * Set the Trigger frame type.
   *
   * \param type the Trigger frame type
   */
  void SetType (TriggerFrameType type);
  /**
   * Check if this is a Basic Trigger frame.
   *
   * \return true if this is a Basic Trigger frame,
   *         false otherwise
   */
  bool IsBasic (void) const;
  /**
   * Check if this is a Beamforming Report Poll Trigger frame.
   *
   * \return true if this is a Beamforming Report Poll Trigger frame,
   *         false otherwise
   */
  bool IsBfrp (void) const;
  /**
   * Check if this is a MU-BAR Trigger frame.
   *
   * \return true if this is a MU-BAR Trigger frame,
   *         false otherwise
   */
  bool IsMuBar (void) const;
  /**
   * Check if this is a MU-RTS Trigger frame.
   *
   * \return true if this is a MU-RTS Trigger frame,
   *         false otherwise
   */
  bool IsMuRts (void) const;
  /**
   * Check if this is a Buffer Status Report Poll Trigger frame.
   *
   * \return true if this is a Buffer Status Report Poll Trigger frame,
   *         false otherwise
   */
  bool IsBsrp (void) const;
  /**
   * Check if this is a Groupcast with Retries (GCR) MU-BAR Trigger frame.
   *
   * \return true if this is a Groupcast with Retries (GCR) MU-BAR Trigger frame,
   *         false otherwise
   */
  bool IsGcrMuBar (void) const;
  /**
   * Check if this is a Bandwidth Query Report Poll Trigger frame.
   *
   * \return true if this is a Bandwidth Query Report Poll Trigger frame,
   *         false otherwise
   */
  bool IsBqrp (void) const;
  /**
   * Check if this is a NDP Feedback Report Poll Trigger frame.
   *
   * \return true if this is a NDP Feedback Report Poll Trigger frame,
   *         false otherwise
   */
  bool IsNfrp (void) const;
  /**
   * Set the UL Length subfield of the Common Info field.
   *
   * \param len the value for the UL Length subfield
   */
  void SetUlLength (uint16_t len);
  /**
   * Get the UL Length subfield of the Common Info field.
   *
   * \return the UL Length subfield
   */
  uint16_t GetUlLength (void) const;
  /**
   * Set the More TF subfield of the Common Info field.
   *
   * \param more the value for the More TF subfield
   */
  void SetMoreTF (bool more);
  /**
   * Get the More TF subfield of the Common Info field.
   *
   * \return the More TF subfield
   */
  bool GetMoreTF (void) const;
  /**
   * Set the CS Required subfield of the Common Info field.
   *
   * \param cs the value for the CS Required subfield
   */
  void SetCsRequired (bool cs);
  /**
   * Get the CS Required subfield of the Common Info field.
   *
   * \return the CS Required subfield
   */
  bool GetCsRequired (void) const;
  /**
   * Set the bandwidth of the solicited HE TB PPDU.
   *
   * \param bw bandwidth (allowed values: 20, 40, 80, 160)
   */
  void SetUlBandwidth (uint8_t bw);
  /**
   * Get the bandwidth of the solicited HE TB PPDU.
   *
   * \return the bandwidth (20, 40, 80 or 160)
   */
  uint8_t GetUlBandwidth (void) const;
  /**
   * Set the AP TX Power subfield of the Common Info field.
   *
   * \param power the value (from -20 to 40) for the AP TX Power (dBm)
   */
  void SetApTxPower (int8_t power);
  /**
   * Get the power value (dBm) indicated by the AP TX Power subfield of the
   * Common Info field.
   *
   * \return the AP TX Power (dBm)
   */
  int8_t GetApTxPower (void) const;
  /**
   * Set the UL Spatial Reuse subfield of the Common Info field.
   *
   * \param sr the value for the UL Spatial Reuse subfield
   */
  void SetUlSpatialReuse (uint16_t sr);
  /**
   * Get the UL Spatial Reuse subfield of the Common Info field.
   *
   * \return the UL Spatial Reuse subfield
   */
  uint16_t GetUlSpatialReuse (void) const;
  /**
   * Get a copy of the Common Info field of this Trigger frame.
   *
   * \return a Trigger frame including a copy of the Common Info field of this frame.
   */
  CtrlTriggerHeader GetCommonInfoField (void) const;

  /**
   * Append a new User Info field to this Trigger frame and return
   * a non-const reference to it. Make sure to call this method after
   * setting the type of the Trigger frame.
   *
   * \return a non-const reference to the newly added User Info field
   */
  CtrlTriggerUserInfoField& AddUserInfoField (void);

  /// User Info fields list const iterator
  typedef std::list<CtrlTriggerUserInfoField>::const_iterator ConstIterator;

  /// User Info fields list iterator
  typedef std::list<CtrlTriggerUserInfoField>::iterator Iterator;

  /**
   * \brief Get a const iterator pointing to the first User Info field in the list.
   *
   * \return a const iterator pointing to the first User Info field in the list
   */
  ConstIterator begin (void) const;
  /**
   * \brief Get a const iterator indicating past-the-last User Info field in the list.
   *
   * \return a const iterator indicating past-the-last User Info field in the list
   */
  ConstIterator end (void) const;
  /**
   * \brief Get an iterator pointing to the first User Info field in the list.
   *
   * \return an iterator pointing to the first User Info field in the list
   */
  Iterator begin (void);
  /**
   * \brief Get an iterator indicating past-the-last User Info field in the list.
   *
   * \return an iterator indicating past-the-last User Info field in the list
   */
  Iterator end (void);
  /**
   * \brief Get the number of User Info fields in this Trigger Frame.
   *
   * \return the number of User Info fields in this Trigger Frame
   */
  std::size_t GetNUserInfoFields (void) const;
  /**
   * Get a const iterator pointing to the first User Info field found (starting from
   * the one pointed to by the given iterator) whose AID12 subfield is set to
   * the given value.
   *
   * \param start a const iterator pointing to the User Info field to start the search from
   * \param aid12 the value of the AID12 subfield to match
   * \return a const iterator pointing to the User Info field matching the specified
   * criterion, if any, or an iterator indicating past-the-last User Info field.
   */
  ConstIterator FindUserInfoWithAid (ConstIterator start, uint16_t aid12) const;
  /**
   * Get a const iterator pointing to the first User Info field found whose AID12
   * subfield is set to the given value.
   *
   * \param aid12 the value of the AID12 subfield to match
   * \return a const iterator pointing to the User Info field matching the specified
   * criterion, if any, or an iterator indicating past-the-last User Info field.
   */
  ConstIterator FindUserInfoWithAid (uint16_t aid12) const;
  /**
   * Get a const iterator pointing to the first User Info field found (starting from
   * the one pointed to by the given iterator) which allocates a Random Access
   * RU for associated stations.
   *
   * \param start a const iterator pointing to the User Info field to start the search from
   * \return a const iterator pointing to the User Info field matching the specified
   * criterion, if any, or an iterator indicating past-the-last User Info field.
   */
  ConstIterator FindUserInfoWithRaRuAssociated (ConstIterator start) const;
  /**
   * Get a const iterator pointing to the first User Info field found which allocates
   * a Random Access RU for associated stations.
   *
   * \return a const iterator pointing to the User Info field matching the specified
   * criterion, if any, or an iterator indicating past-the-last User Info field.
   */
  ConstIterator FindUserInfoWithRaRuAssociated (void) const;
  /**
   * Get a const iterator pointing to the first User Info field found (starting from
   * the one pointed to by the given iterator) which allocates a Random Access
   * RU for unassociated stations.
   *
   * \param start a const iterator pointing to the User Info field to start the search from
   * \return a const iterator pointing to the User Info field matching the specified
   * criterion, if any, or an iterator indicating past-the-last User Info field.
   */
  ConstIterator FindUserInfoWithRaRuUnassociated (ConstIterator start) const;
  /**
   * Get a const iterator pointing to the first User Info field found which allocates
   * a Random Access RU for unassociated stations.
   *
   * \return a const iterator pointing to the User Info field matching the specified
   * criterion, if any, or an iterator indicating past-the-last User Info field.
   */
  ConstIterator FindUserInfoWithRaRuUnassociated (void) const;
  /**
   * Check the validity of this Trigger frame.
   * TODO Implement the checks listed in Section 27.5.3.2.3 of 802.11ax amendment
   * D3.0 (Allowed settings of the Trigger frame fields and TRS Control subfield).
   *
   * This function shall be invoked before transmitting and upon receiving
   * a Trigger frame.
   *
   * \return true if the Trigger frame is valid, false otherwise.
   */
  bool IsValid (void) const;

private:
  /**
   * Common Info field
   */
  uint8_t m_triggerType;   //!< Trigger type
  uint16_t m_ulLength;    //!< Value for the L-SIG Length field
  bool m_moreTF;   //!< True if a subsequent Trigger frame follows
  bool m_csRequired;   //!< Carrier Sense required
  uint8_t m_ulBandwidth;   //!< UL BW subfield
  uint8_t m_apTxPower;   //!< Tx Power used by AP to transmit the Trigger Frame
  uint16_t m_ulSpatialReuse;   //!< Value for the Spatial Reuse field in HE-SIG-A
  // TODO Add CP and LTF Type, Number of LTFs, HE-SIG-A reserved
  /**
   * List of User Info fields
   */
  std::list<CtrlTriggerUserInfoField> m_userInfoFields;   //!< list of User Info fields
};


} //namespace ns3

#endif /* CTRL_HEADERS_H */

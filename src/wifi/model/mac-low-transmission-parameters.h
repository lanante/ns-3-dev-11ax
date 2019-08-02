/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005, 2006 INRIA
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
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Mirko Banchi <mk.banchi@gmail.com>
 */

#ifndef MAC_LOW_TRANSMISSION_PARAMETERS_H
#define MAC_LOW_TRANSMISSION_PARAMETERS_H

#include "ns3/uinteger.h"
#include "ns3/mac48-address.h"
#include "block-ack-type.h"
#include "mu-tx-ack-types.h"
#include <map>
#include <list>

namespace ns3 {

/**
 * \brief control how a packet is transmitted.
 * \ingroup wifi
 *
 * The ns3::MacLow::StartTransmission method expects
 * an instance of this class to describe how the packet
 * should be transmitted.
 *
 * For SU PPDUs, call:
 * - EnableAck () if the frame is followed by a Normal Ack;
 *
 * - EnableBlockAck (BlockAckType) if the frame is followed by a Block Ack
 * of the given type;
 *
 * - EnableBlockAckRequest (BlockAckReqType, BlockAckType) if the sender
 * will subsequently transmit a Block Ack Request of the given type, followed
 * by a Block Ack of the given type.
 *
 * For DL MU PPDUs using a sequence of Block Ack Requests/Block Ack responses, call:
 * - EnableAck (address) if the station with the given address will reply with a
 * Normal Ack a SIFS after the transmission of the DL MU PPDU;
 *
 * - EnableBlockAck (address) if the station with the given address will reply with a
 * Block Ack of the given type a SIFS after the transmission of the DL MU PPDU;
 *
 * -EnableBlockAckRequest (address, BlockAckReqType, BlockAckType) if the sender
 * will transmit a Block Ack Request of the given type to the station with the
 * given address, which will reply with a Block Ack of the given type.
 *
 * For DL MU PPDUs using acknowledgment via a separate MU-BAR Trigger Frame, call:
 * - EnableBlockAckRequest (address, BlockAckReqType, BlockAckType) to include a
 * User Info subfield addressed to the station with the given address in the
 * following MU-BAR Trigger Frame.
 *
 * For a MU-BAR Trigger Frame transmitted after a DL MU PPDU, call:
 * - EnableBlockAck (address, BlockAckType) for each of the stations addressed
 * by the MU-BAR Trigger Frame.
 *
 * For a DL MU PPDU with aggregated MU-BAR Trigger Frames, call:
 * - EnableBlockAck (address, BlockAckType) for each of the stations addressed
 * by the DL MU PPDU.
 */
class MacLowTransmissionParameters
{
public:
  MacLowTransmissionParameters ();

  /**
   * Wait ACKTimeout for an ACK. If we get an ACK
   * on time, call MacLowTransmissionListener::GotAck.
   * Call MacLowTransmissionListener::MissedAck otherwise.
   */
  void EnableAck (void);
  /**
   * Wait the timeout corresponding to the given Block Ack Response type.
   *
   * \param type the Block Ack Response type
   */
  void EnableBlockAck (BlockAckType type);
  /**
   * Schedule the transmission of a Block Ack Request of the given type.
   *
   * \param type the Block Ack Request type
   */
  void EnableBlockAckRequest (BlockAckType type);
  /**
   * Send a RTS, and wait CTSTimeout for a CTS. If we get a
   * CTS on time, call MacLowTransmissionListener::GotCts
   * and send data. Otherwise, call MacLowTransmissionListener::MissedCts
   * and do not send data.
   */
  void EnableRts (void);
  /**
   * \param size size of next data to send after current packet is
   *        sent.
   *
   * Add the transmission duration of the next data to the
   * durationId of the outgoing packet and call
   * MacLowTransmissionListener::StartNextFragment at the end of
   * the current transmission + SIFS.
   */
  void EnableNextData (uint32_t size);
  /**
   * For a multi-user transmission, record that the given station is expected to
   * transmit a Normal Ack.
   *
   * \param address the MAC address of the station from which a response is expected
   */
  void EnableAck (Mac48Address address);
  /**
   * For a multi-user transmission, record that the given station is expected to
   * transmit a Block Ack Response of the given type.
   *
   * \param address the MAC address of the station from which a response is expected
   * \param type the Block Ack Response type
   */
  void EnableBlockAck (Mac48Address address, BlockAckType type);
  /**
   * For a multi-user transmission, schedule the transmission of a Block Ack Request
   * of the given type to the given station.
   *
   * \param address the MAC address of the station to which a BAR has to be sent
   * \param type the Block Ack Request type
   */
  void EnableBlockAckRequest (Mac48Address address, BlockAckType type);
  /**
   * Do not wait for Ack after data transmission. Typically
   * used for Broadcast and multicast frames.
   */
  void DisableAck (void);
  /**
   * Do not send Block Ack Request after data transmission
   */
  void DisableBlockAckRequest (void);
  /**
   * Do not send rts and wait for cts before sending data.
   */
  void DisableRts (void);
  /**
   * Do not attempt to send data burst after current transmission
   */
  void DisableNextData (void);
  /**
   * For a multi-user transmission, record that the given station is not expected to
   * transmit a response.
   *
   * \param address the MAC address of the station from which a response is not expected
   */
  void DisableAck (Mac48Address address);
  /**
   * For a multi-user transmission, do not schedule the transmission of a Block Ack Request
   * to the given station.
   *
   * \param address the MAC address of the station to which a BAR has not to be sent
   */
  void DisableBlockAckRequest (Mac48Address address);
  /**
   * \returns true if normal ACK protocol should be used, false
   *          otherwise.
   *
   * \sa EnableAck
   */
  bool MustWaitNormalAck (void) const;
  /**
   * \returns true if block ack mechanism is used, false otherwise.
   *
   * \sa EnableBlockAck
   */
  bool MustWaitBlockAck (void) const;
  /**
   * \returns the selected block ack variant.
   *
   * Only call this method if the block ack mechanism is used.
   */
  BlockAckType GetBlockAckType (void) const;
  /**
   * \returns true if a block ack request must be sent, false otherwise.
   *
   * Return true if a block ack request must be sent, false otherwise.
   */
  bool MustSendBlockAckRequest (void) const;
  /**
   * \returns the selected block ack request variant.
   *
   * Only call this method if a block ack request must be sent.
   */
  BlockAckType GetBlockAckRequestType (void) const;
  /**
   * \returns true if RTS should be sent and CTS waited for before
   *          sending data, false otherwise.
   */
  bool MustSendRts (void) const;
  /**
   * \returns true if EnableNextData was called, false otherwise.
   */
  bool HasNextPacket (void) const;
  /**
   * \returns the size specified by EnableNextData.
   */
  uint32_t GetNextPacketSize (void) const;
  /**
   * \param type the type of the ack sequence for the DL MU transmission.
   *
   * Record that this is a DL MU transmission and set the ack sequence type
   * to the given type.
   */
  void SetDlMuAckSequenceType (DlMuAckSequenceType type);
  /**
   * \param type the type of the ack sequence for the UL MU transmission.
   *
   * Record that this is an UL MU transmission and set the ack sequence type
   * to the given type.
   */
  void SetUlMuAckSequenceType (UlMuAckSequenceType type);
  /**
   * \returns true if the current transmission is followed by a DL MU ack sequence,
   *          false otherwise.
   */
  bool HasDlMuAckSequence (void) const;
  /**
   * \returns true if the current transmission is followed by an UL MU ack sequence,
   *          false otherwise.
   */
  bool HasUlMuAckSequence (void) const;
  /**
   * \return the type of the ack sequence for the DL MU transmission.
   */
  DlMuAckSequenceType GetDlMuAckSequenceType (void) const;
  /**
   * \return the type of the ack sequence for the UL MU transmission.
   */
  UlMuAckSequenceType GetUlMuAckSequenceType (void) const;
  /**
   * \return a list of stations from which a Normal Ack response is expected.
   *
   * This method is intended for DL MU or UL MU transmissions only.
   */
  std::list<Mac48Address> GetStationsReplyingWithNormalAck (void) const;
  /**
   * \return a list of stations from which a Block Ack response is expected.
   *
   * This method is intended for DL MU or UL MU transmissions only.
   */
  std::list<Mac48Address> GetStationsReplyingWithBlockAck (void) const;
  /**
   * \return a list of stations to which a Block Ack Request is going to be sent.
   *
   * This method is intended for DL MU or UL MU transmissions only.
   */
  std::list<Mac48Address> GetStationsSendBlockAckRequestTo (void) const;
  /**
   * For a multi-user transmission, return the type of Block Ack response
   * expected from the given station.
   *
   * \param address the MAC address of the given station
   * \returns the selected Block Ack response variant.
   */
  BlockAckType GetBlockAckType (Mac48Address address) const;
  /**
   * For a multi-user transmission, return the type of Block Ack Request
   * to send to the given station.
   *
   * \param address the MAC address of the given station
   * \returns the selected Block Ack Request variant.
   */
  BlockAckType GetBlockAckRequestType (Mac48Address address) const;

private:
  friend std::ostream &operator << (std::ostream &os, const MacLowTransmissionParameters &params);
  /// Struct storing the type of Ack to wait for
  struct WaitAckType
  {
    enum {NONE, NORMAL, BLOCK_ACK} m_type;
    BlockAckType m_baType;
  };
  /// Struct storing the type of BAR to send
  struct SendBarType
  {
    enum {NONE, BLOCK_ACK_REQ} m_type;
    BlockAckType m_barType;
  };

  uint32_t m_nextSize;                              //!< the next size
  WaitAckType m_waitAck;                            //!< type of Ack to wait for
  SendBarType m_sendBar;                            //!< type of BAR to send
  bool m_sendRts;                                   //!< whether to send an RTS or not
  /// Params for MU PPDUs
  std::map<Mac48Address, WaitAckType> m_muWaitAck;  //!< wait block ack from multiple stations
  std::map<Mac48Address, SendBarType> m_muSendBar;  //|< send bar to multiple stations
  DlMuAckSequenceType m_dlMuAckType;                //!< Ack sequence type for DL MU
  UlMuAckSequenceType m_ulMuAckType;                //!< Ack sequence type for UL MU
};

/**
 * Serialize MacLowTransmissionParameters to ostream in a human-readable form.
 *
 * \param os std::ostream
 * \param params MacLowTransmissionParameters
 * \return std::ostream
 */
std::ostream &operator << (std::ostream &os, const MacLowTransmissionParameters &params);

} //namespace ns3

#endif /* MAC_LOW_TRANSMISSION_PARAMETERS_H */

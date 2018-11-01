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
 *
 * Author: Tom Henderson <tomhend@u.washington.edu>
 */

#ifndef VOICE_APPLICATION_H
#define VOICE_APPLICATION_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/sequence-number.h"
#include "ns3/basic-data-calculators.h"

namespace ns3 {

class Address;
class Socket;

/**
 * \ingroup applications
 * \defgroup voice VoiceApplication
 *
 */
class VoiceApplication : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  VoiceApplication ();

  virtual ~VoiceApplication ();

  /**
   * \brief Get the socket this application is attached to.
   * \return pointer to associated socket
   */
  Ptr<Socket> GetSocket (void) const;

  // Counters
  /**
   * Get the number of packets sent, including those whose
   * transmission was suppressed for some reason.  This value is typically
   * a quantity equal to or nearly equal to the duration of time the 
   * application was running, divided by the transmission interval.
   *
   * \return number of packets sent
   */
  uint64_t GetNumSent (void) const;
  /**
   * Get the number of packets successfully received.  Success is defined
   * as a segment within the receive window whose end-to-end latency does 
   * not exceed the latency threshold.  Note, the receive window in this
   * version of the application does not allow for out-of-order segments
   * (segments whose sequence number is less than the highest seen so
   * far are discarded and considered to be lost).
   *
   * \return number of packets received
   */
  uint64_t GetNumReceived (void) const;
  /**
   * Get the number of packets lost, including those that didn't arrive
   * but for which the loss can be implicitly assumed due to a sequence
   * number gap.
   *
   * \return number of packets lost
   */
  uint64_t GetNumLost (void) const;
  /**
   * Get the number of packets that would have been successfully received but
   * were delayed beyond the latency threshold.  
   *
   * \return number of packets delayed
   */
  uint64_t GetNumDelayed (void) const;
  /**
   * Get the mean latency (in seconds) of all received packet samples (including
   * those above the latency threshold or otherwise not counted as received
   * due to being out of order).
   *
   * \return mean latency (seconds)
   */
  double GetLatencyMean (void) const;
  /**
   * Get the stddev latency (in seconds) of all received packet samples 
   * (including those above the latency threshold or otherwise not counted 
   * as received due to being out of order).
   *
   * \return stddev latency (seconds)
   */
  double GetLatencyStddev (void) const;

protected:
  virtual void DoDispose (void);
private:
  // inherited from Application base class.
  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleRead (Ptr<Socket> socket);

  Ptr<Socket>     m_socket;       //!< Associated socket
  Address         m_peer;         //!< Peer address
  Address         m_local;         //!< Local address
  bool            m_connected;    //!< True if connected
  bool            m_isRunning;    //!< True if application has been started
  bool            m_sendEnabled;  //!< True if allowed to send
  uint32_t        m_spaceAvailable; //!< Buffer space available
  SequenceNumber32 m_sendSequence;
  SequenceNumber32 m_recvSequence;//!< highest sequence number received
  Time            m_interval;
  Time            m_threshold;
  EventId         m_nextSendEvent; 
  uint32_t        m_packetSize;     //!< Size of data to send each time
  TypeId          m_tid;          //!< The type of protocol to use.
  uint8_t         m_tos;         //!< Type of service
  
  uint64_t        m_sent;      //!< number of packets sent
  uint64_t        m_lost;      //!< number of packets lost
  uint64_t        m_delayed;   //!< number of packets delayed
  uint64_t        m_received;   //!< number of packets received
  MinMaxAvgTotalCalculator<double> m_latency; //!< latency calculator

  /// Traced Callback: sent packets
  TracedCallback<Ptr<const Packet> > m_txTrace;
  /// Traced Callback: received packets
  TracedCallback<Ptr<const Packet> > m_rxTrace;

private:
  // unimplemented
  VoiceApplication (VoiceApplication const &a); // Copy constructor disabled
  VoiceApplication &operator = (VoiceApplication const &a); // operator disabled
  
  /**
   * \brief Connection Succeeded (called by Socket through a callback)
   * \param socket the connected socket
   */
  void ConnectionSucceeded (Ptr<Socket> socket);
  /**
   * \brief Connection Failed (called by Socket through a callback)
   * \param socket the connected socket
   */
  void ConnectionFailed (Ptr<Socket> socket);
  /**
   * \brief Close Succeeded (called by Socket through a callback)
   * \param socket the closed socket
   */
  void CloseSucceeded (Ptr<Socket> socket);
  /**
   * \brief Close Failed (called by Socket through a callback)
   * \param socket the closed socket
   */
  void CloseFailed (Ptr<Socket> socket);
  /**
   *
   * The lower layer may become blocked and transmit buffer space may 
   * fill up.  This callback exists to provide backpressure on the application.
   * If there is insufficient space available, the packet will not be sent.
   * The sequence number will be incremented, which will result in a sequence
   * gap at the receiver (i.e. there is effectively a drop in the application).
   *
   * \brief Callback to register for notifications of available buffer space
   * \param socket the socket in use
   * \param spaceAvailable the amount of space available
   */
  void TransmitBufferSpaceCallback (Ptr<Socket> socket, uint32_t spaceAvailable);
  /**
   * \brief Send a packet if allowed to
   *
   * This method will send a packet if there is a connected socket and
   * there is space available in the underlying transmit buffer.
   */
  void Send (void);
  /**
   *
   */
  void SetSendEnabled (bool sendEnabled);
  /**
   *
   */
  bool GetSendEnabled (void) const;
};

} // namespace ns3

#endif /* VOICE_APPLICATION_H */

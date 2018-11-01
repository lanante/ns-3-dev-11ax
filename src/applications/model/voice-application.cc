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

#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/packet-socket-factory.h"
#include "voice-application.h"
#include "ns3/seq-ts-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("VoiceApplication");

NS_OBJECT_ENSURE_REGISTERED (VoiceApplication);

// static const uint8_t TOS_EF = 184; // IP TOS value for expedited forwarding
// in latest code, TOS_EF maps to TID 5 (video); we need TOS_CS6 to map to
static const uint8_t TOS_CS6 = 192; // IP TOS value for CS6

TypeId
VoiceApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::VoiceApplication")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<VoiceApplication> ()
    .AddAttribute ("PacketSize", "The number of payload bytes per packet.  Includes any bytes above UDP header or above link layer header if packet socket used.  Minimum of 12 bytes.",
                   UintegerValue (32),
                   MakeUintegerAccessor (&VoiceApplication::m_packetSize),
                   MakeUintegerChecker<uint32_t> (12))
    .AddAttribute ("Interval", "Time interval between packets",
                   TimeValue (MilliSeconds (20)),
                   MakeTimeAccessor (&VoiceApplication::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("SendEnabled", "True if application will send packets",
                   BooleanValue (true),
                   MakeBooleanAccessor (&VoiceApplication::SetSendEnabled,
                                        &VoiceApplication::GetSendEnabled),
                   MakeBooleanChecker ())
    .AddAttribute ("Local", "The local address to bind to",
                   AddressValue (),
                   MakeAddressAccessor (&VoiceApplication::m_local),
                   MakeAddressChecker ())
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&VoiceApplication::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("Protocol", "The type of protocol to use.",
                   TypeIdValue (PacketSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&VoiceApplication::m_tid),
                   MakeTypeIdChecker ())
    .AddAttribute ("LatencyThreshold", "Accept packet if latency not greater than threshold",
                   TimeValue (MilliSeconds (50)),
                   MakeTimeAccessor (&VoiceApplication::m_threshold),
                   MakeTimeChecker ())
    .AddAttribute ("TypeOfService", "IP Type of Service value to set for socket",
                   UintegerValue (TOS_CS6),
                   MakeUintegerAccessor (&VoiceApplication::m_tos),
                   MakeUintegerChecker<uint8_t> ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&VoiceApplication::m_txTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("Rx", "A new packet is received",
                     MakeTraceSourceAccessor (&VoiceApplication::m_rxTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}

VoiceApplication::VoiceApplication ()
  : m_socket (0),
    m_connected (false),
    m_isRunning (false),
    m_spaceAvailable (65535), // initialize to something to permit sending
    m_sendSequence (0),
    m_recvSequence (0),
    m_nextSendEvent (EventId ()),
    m_sent (0),
    m_lost (0),
    m_delayed (0),
    m_received (0)
{
  NS_LOG_FUNCTION (this);
}

VoiceApplication::~VoiceApplication ()
{
  NS_LOG_FUNCTION (this);
  if (m_nextSendEvent.IsRunning ())
    {
      m_nextSendEvent.Cancel ();
    }
}

Ptr<Socket>
VoiceApplication::GetSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

void
VoiceApplication::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  if (m_nextSendEvent.IsRunning ())
    {
      m_nextSendEvent.Cancel ();
    }
  m_socket = 0;
  // chain up
  Application::DoDispose ();
}

// Application Methods
void
VoiceApplication::StartApplication (void) // Called at time specified by Start
{
  NS_LOG_FUNCTION (GetNode ()->GetId () << this);
  NS_LOG_DEBUG ("VoiceApplication start time " << Simulator::Now ().As (Time::S));
  m_isRunning = true;
  m_sendSequence = 0;
  m_sendSequence--;  // initialize to one fewer than 0
  m_recvSequence = 0;
  m_recvSequence--;  // initialize to one fewer than 0
  if (m_socket)
    {
      NS_LOG_FUNCTION ("Socket exists; ignoring request");
    }
  else
    {
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
    }
  m_socket->SetConnectCallback (
    MakeCallback (&VoiceApplication::ConnectionSucceeded, this),
    MakeCallback (&VoiceApplication::ConnectionFailed, this));
  m_socket->SetSendCallback (
    MakeCallback (&VoiceApplication::TransmitBufferSpaceCallback, this));
  m_socket->SetCloseCallbacks (
    MakeCallback (&VoiceApplication::CloseSucceeded, this),
    MakeCallback (&VoiceApplication::CloseFailed, this));
  m_socket->SetRecvCallback (MakeCallback (&VoiceApplication::HandleRead, this));
  // Open the connection
  if (m_tid != PacketSocketFactory::GetTypeId ())
    {
      // User must provide a valid Local attribute to bind to incoming port
      // and possibly also incoming IP address
      m_socket->Bind (m_local);
    }
  else
    {
      m_socket->Bind (); // it suffices to Bind() for packet sockets
    }
  m_socket->Connect (m_peer);
  if (m_tid != PacketSocketFactory::GetTypeId () && m_tos)
    {
      // For IP-based sockets, set the TOS
      m_socket->SetIpTos (m_tos);
    }
  if (m_sendEnabled)
    {
      // Schedule first send event after the first interval expires
      m_nextSendEvent = Simulator::Schedule (m_interval, &VoiceApplication::Send, this);
    }
}

void
VoiceApplication::Send (void)
{
  NS_LOG_FUNCTION (this);
  m_sendSequence++;  // We consume a sequence number even if we suppress the send
  m_sent++;
  NS_LOG_DEBUG ("Send time " << Simulator::Now ().As (Time::S));
  if (m_socket == 0)
    {
      NS_LOG_DEBUG ("No socket available; rescheduling the send event");
      m_nextSendEvent = Simulator::Schedule (m_interval, &VoiceApplication::Send, this);
      return;
    }
  if (m_connected == 0)
    {
      NS_LOG_DEBUG ("Socket not connected; rescheduling the send event");
      m_nextSendEvent = Simulator::Schedule (m_interval, &VoiceApplication::Send, this);
      return;
    }
  if (m_spaceAvailable  < m_packetSize)
    {
      NS_LOG_DEBUG ("Insufficent buffer space; rescheduling the send event");
      m_nextSendEvent = Simulator::Schedule (m_interval, &VoiceApplication::Send, this);
      return;
    }
  SeqTsHeader seqTs;
  int32_t paddingSize = m_packetSize - seqTs.GetSerializedSize ();
  NS_ASSERT (paddingSize >= 0);
  Ptr<Packet> packet = Create<Packet> (static_cast<uint32_t> (paddingSize));
  seqTs.SetSeq (m_sendSequence.GetValue ());
  packet->AddHeader (seqTs);

  int returnCode = m_socket->Send (packet);
  if (returnCode > 0)
    {
      NS_LOG_DEBUG ("Socket::Send  " << returnCode);
      m_txTrace (packet);
    }
  else
    {
      NS_LOG_ERROR ("Socket::Send failed with code " << returnCode);
    }
  m_nextSendEvent = Simulator::Schedule (m_interval, &VoiceApplication::Send, this);
}

void
VoiceApplication::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->GetSize () > 0)
        {
          NS_LOG_DEBUG ("Receive time " << Simulator::Now ().As (Time::S));
          m_rxTrace (packet);
          SeqTsHeader seqTs;
          packet->RemoveHeader (seqTs);
          SequenceNumber32 currentSequenceNumber (seqTs.GetSeq ());
          Time sendTime = seqTs.GetTs ();
          double latencySample = Simulator::Now ().GetSeconds () - sendTime.GetSeconds ();
          NS_LOG_DEBUG ("Received packet; size: " << packet->GetSize ()
                        << " seq: " << currentSequenceNumber.GetValue ()
                        << " node: " << GetNode ()->GetId ()
                        << " time: " << Simulator::Now ().As (Time::MS)
                        << " timestamp: " << sendTime.As (Time::MS)
                        << " latency: " << latencySample);
          m_latency.Update (latencySample);
          if (currentSequenceNumber <= m_recvSequence)
            {
              NS_LOG_DEBUG ("Ignore; old segment");
            }
          if (currentSequenceNumber == (m_recvSequence + 1))
            {
              NS_LOG_DEBUG ("In sequence");
              if (Simulator::Now () - sendTime > m_threshold)
                {
                  NS_LOG_DEBUG ("Latency threshold exceeded");
                  m_delayed++;
                }
              else
                {
                  m_received++;
                }
              m_recvSequence = currentSequenceNumber;
            }
          else
            {
              NS_LOG_DEBUG ("Gap in sequence space detected from " << m_recvSequence.GetValue () << " to " << currentSequenceNumber.GetValue ());
              SequenceNumber32 numLost (currentSequenceNumber);
              numLost = numLost - (m_recvSequence + 1);
              m_lost += numLost.GetValue ();
              NS_LOG_DEBUG ("Adding " << numLost.GetValue () << " to loss count");
              if (Simulator::Now () - sendTime > m_threshold)
                {
                  NS_LOG_DEBUG ("Latency threshold exceeded");
                  m_delayed++;
                }
              else
                {
                  m_received++;
                }
              m_recvSequence = currentSequenceNumber;
            }
        }
    }

}

void
VoiceApplication::StopApplication (void) // Called at time specified by Stop
{
  NS_LOG_FUNCTION (GetNode ()->GetId () << this);
  NS_LOG_DEBUG ("VoiceApplication stop time " << Simulator::Now ().As (Time::S));

  m_isRunning = false;
  if (m_nextSendEvent.IsRunning ())
    {
      m_nextSendEvent.Cancel ();
    }
  if (m_socket != 0)
    {
      m_socket->Close ();
      m_connected = false;
    }
  else
    {
      NS_LOG_WARN ("VoiceApplication found null socket to close in StopApplication");
    }
}

void
VoiceApplication::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_DEBUG ("VoiceApplication Connection succeeded");
  m_connected = true;
}

void
VoiceApplication::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_DEBUG ("VoiceApplication Connection failed");
  m_socket->Close ();
}

void
VoiceApplication::CloseSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_DEBUG ("VoiceApplication Close succeeded");
  m_socket = 0;
}

void
VoiceApplication::CloseFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_DEBUG ("VoiceApplication Close failed");
  m_socket = 0;
}

void
VoiceApplication::TransmitBufferSpaceCallback (Ptr<Socket> socket, uint32_t spaceAvailable)
{
  NS_LOG_FUNCTION (this << socket << spaceAvailable);
  m_spaceAvailable = spaceAvailable;
  NS_LOG_DEBUG ("Node " << GetNode ()->GetId () << " space available " << spaceAvailable << " bytes ");
}

void
VoiceApplication::SetSendEnabled (bool sendEnabled)
{
  NS_LOG_FUNCTION (this << sendEnabled);
  if (m_isRunning == true && m_sendEnabled == true && sendEnabled == false)
    {
      if (m_nextSendEvent.IsRunning ())
        {
          NS_LOG_DEBUG ("Cancelling pending send event");
          m_nextSendEvent.Cancel ();
        }
    }
  else if (m_isRunning == true && m_sendEnabled == false && sendEnabled == true)
    {
      NS_LOG_DEBUG ("Scheduling new pending send event");
      m_nextSendEvent = Simulator::Schedule (m_interval, &VoiceApplication::Send, this);
    }
  m_sendEnabled = sendEnabled;
}

bool
VoiceApplication::GetSendEnabled (void) const
{
  NS_LOG_FUNCTION (this);
  return m_sendEnabled;
}

uint64_t
VoiceApplication::GetNumSent (void) const
{
  NS_LOG_FUNCTION (this);
  return m_sent;
}

uint64_t
VoiceApplication::GetNumReceived (void) const
{
  NS_LOG_FUNCTION (this);
  return m_received;
}

uint64_t
VoiceApplication::GetNumLost (void) const
{
  NS_LOG_FUNCTION (this);
  return m_lost;
}

uint64_t
VoiceApplication::GetNumDelayed (void) const
{
  NS_LOG_FUNCTION (this);
  return m_delayed;
}

double
VoiceApplication::GetLatencyMean (void) const
{
  NS_LOG_FUNCTION (this);
  return m_latency.getMean ();
}

double
VoiceApplication::GetLatencyStddev (void) const
{
  NS_LOG_FUNCTION (this);
  return m_latency.getStddev ();
}

} // Namespace ns3

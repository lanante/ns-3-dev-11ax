/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
 * Copyright (c) 2009 MIRKO BANCHI
 * Copyright (c) 2013 University of Surrey
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
 *          Konstantinos Katsaros <dinos.katsaros@gmail.com>
 */

#include "ns3/double.h"
#include "wifi-rx-tag.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (WifiRxTag);

TypeId
WifiRxTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WifiRxTag")
    .SetParent<Tag> ()
    .SetGroupName ("Wifi")
    .AddConstructor<WifiRxTag> ()
  ;
  return tid;
}

TypeId
WifiRxTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

WifiRxTag::WifiRxTag ()
  : m_snr (0),
  m_rxPower (0)
{
}

uint32_t
WifiRxTag::GetSerializedSize (void) const
{
  return 2 * sizeof (double);
}

void
WifiRxTag::Serialize (TagBuffer i) const
{
  i.WriteDouble (m_snr);
  i.WriteDouble (m_rxPower);
}

void
WifiRxTag::Deserialize (TagBuffer i)
{
  m_snr = i.ReadDouble ();
  m_rxPower = i.ReadDouble ();
}

void
WifiRxTag::Print (std::ostream &os) const
{
  os << "Snr=" << m_snr << ", RxPower=" << m_rxPower;
}

void
WifiRxTag::SetSnr (double snr)
{
  NS_ABORT_MSG_IF (snr < 0, "Snr (linear ratio) cannot be negative");
  m_snr = snr;
}

double
WifiRxTag::GetSnr (void) const
{
  return m_snr;
}

void
WifiRxTag::SetRxPower (double rxPower)
{
  NS_ABORT_MSG_IF (rxPower < 0, "Receive power (watts) cannot be negative");
  m_rxPower = rxPower;
}

double
WifiRxTag::GetRxPower (void) const
{
  return m_rxPower;
}

}

/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
 * Copyright (c) 2009 MIRKO BANCHI
 * Copyright (c) 2013 University of Surrey
 * Copyright (c) 2018 University of Washington
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

#ifndef WIFI_RX_TAG_H
#define WIFI_RX_TAG_H

#include "ns3/tag.h"

namespace ns3 {

class Tag;

/**
 * \brief Tag for cross-layer information about received packet
 *        
 * \ingroup wifi
 *
 * This Tag can be used for carrying cross-layer information as a received
 * packet moves up the stack.  It should be stripped before the packet exits
 * the device.
 */
class WifiRxTag : public Tag
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;

  /**
   * Create a WifiRxTag; snr and received power are initialized to zero
   */
  WifiRxTag ();

  uint32_t GetSerializedSize (void) const;
  void Serialize (TagBuffer i) const;
  void Deserialize (TagBuffer i);
  void Print (std::ostream &os) const;

  /**
   * Set the SNR to the given value.
   *
   * \param snr the value of the snr (linear ratio) to set
   */
  void SetSnr (double snr);
  /**
   * Return the SNR value.
   *
   * \return the SNR value (linear ratio)
   */
  double GetSnr (void) const;
  /**
   * Set the packet's receive signal power (in watts) to the given value.
   *
   * \param rxPower value (in watts) of the power to set
   */
  void SetRxPower (double rxPower);
  /**
   * Return the receive signal power (watts) value.
   *
   * \return the receive power (watts) value
   */
  double GetRxPower (void) const;

private:
  double m_snr;  //!< SNR value
  double m_rxPower;  //!< Received signal power value
};

}

#endif /* WIFI_RX_TAG_H */

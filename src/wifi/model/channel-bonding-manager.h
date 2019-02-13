/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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

#ifndef CHANNEL_BONDING_MANAGER_H
#define CHANNEL_BONDING_MANAGER_H

#include "ns3/object.h"

namespace ns3 {

class WifiPhy;

/**
 * \brief Channel Bonding Manager interface
 * \ingroup wifi
 *
 * This object provides the interface for all channel bonding manager implementations
 * and is designed to be subclassed.
 *
 */
class ChannelBondingManager : public Object
{
public:
  static TypeId GetTypeId (void);

  /**
   * Sets the WifiPhy this manager is associated with.
   *
   * \param phy the WWifiPhy this manager is associated with
   */
  void SetPhy (const Ptr<WifiPhy> phy);

  /**
   * Returns the selected channel width (in MHz).
   *
   * \return the selected channel width in MHz
   */
  virtual uint16_t GetUsableChannelWidth (void) = 0;


protected:
  virtual void DoDispose (void);

  Ptr<WifiPhy> m_phy; //!< Pointer to the WifiPhy
};

} //namespace ns3

#endif /* CHANNEL_BONDING_MANAGER_H */

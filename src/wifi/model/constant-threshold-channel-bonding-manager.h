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

#ifndef CONSTANT_THRESHOLD_CHANNEL_BONDING_MANAGER_H
#define CONSTANT_THRESHOLD_CHANNEL_BONDING_MANAGER_H

#include "channel-bonding-manager.h"

namespace ns3 {

/**
 * \brief Constant Threshold Channel Bonding Manager
 * \ingroup wifi
 *
 * This object provides an implementation for dynamically selecting the channel width.
 *
 */
class ConstantThresholdChannelBondingManager : public ChannelBondingManager
{
public:
  ConstantThresholdChannelBondingManager ();

  static TypeId GetTypeId (void);

  /**
   * Returns the selected channel width (in MHz).
   *
   * \return the selected channel width in MHz
   */
  uint16_t GetUsableChannelWidth (void);
};

} //namespace ns3

#endif /* CONSTANT_THRESHOLD_CHANNEL_BONDING_MANAGER_H */

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

#include "ns3/log.h"
#include "constant-threshold-channel-bonding-manager.h"
#include "wifi-phy.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ConstantThresholdChannelBondingManager");
NS_OBJECT_ENSURE_REGISTERED (ConstantThresholdChannelBondingManager);

TypeId
ConstantThresholdChannelBondingManager::GetTypeId (void)
{
  static ns3::TypeId tid = ns3::TypeId ("ns3::ConstantThresholdChannelBondingManager")
    .SetParent<ChannelBondingManager> ()
    .SetGroupName ("Wifi")
  ;
  return tid;
}

uint16_t
ConstantThresholdChannelBondingManager::GetUsableChannelWidth (void)
{
  NS_LOG_FUNCTION (this);
  if (m_phy->GetChannelWidth () == 40)
    {
      if (m_phy->GetDelaySinceIdle (true) >= m_phy->GetPifs ())
        {
          return 40;
        }
      else
        {
          return 20;
        }
    }
  //TODO: handle 80 and 160 MHZ
  return m_phy->GetChannelWidth ();
}

} //namespace ns3

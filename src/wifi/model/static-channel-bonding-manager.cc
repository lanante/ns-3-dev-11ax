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
#include "static-channel-bonding-manager.h"
#include "wifi-phy.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("StaticChannelBondingManager");
NS_OBJECT_ENSURE_REGISTERED (StaticChannelBondingManager);

StaticChannelBondingManager::StaticChannelBondingManager ()
  : ChannelBondingManager ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
StaticChannelBondingManager::GetTypeId (void)
{
  static ns3::TypeId tid = ns3::TypeId ("ns3::StaticChannelBondingManager")
    .SetParent<ChannelBondingManager> ()
    .SetGroupName ("Wifi")
    .AddConstructor<StaticChannelBondingManager> ()
  ;
  return tid;
}

uint16_t
StaticChannelBondingManager::GetUsableChannelWidth (WifiMode mode)
{
  if ((m_phy->GetChannelWidth () < 40) || (m_phy->GetDelaySinceChannelIsIdle (m_phy->GetChannelWidth ()) >= m_phy->GetPifs ()))
    {
      return m_phy->GetChannelWidth ();
    }
  else
    {
      return 0;
    }
}

} //namespace ns3

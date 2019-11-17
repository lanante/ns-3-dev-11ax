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
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "constant-threshold-channel-bonding-manager.h"
#include "wifi-phy.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ConstantThresholdChannelBondingManager");
NS_OBJECT_ENSURE_REGISTERED (ConstantThresholdChannelBondingManager);

ConstantThresholdChannelBondingManager::ConstantThresholdChannelBondingManager ()
  : ChannelBondingManager ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
ConstantThresholdChannelBondingManager::GetTypeId (void)
{
  static ns3::TypeId tid = ns3::TypeId ("ns3::ConstantThresholdChannelBondingManager")
    .SetParent<ChannelBondingManager> ()
    .SetGroupName ("Wifi")
    .AddConstructor<ConstantThresholdChannelBondingManager> ()
    .AddAttribute ("CcaEdThresholdSecondary",
                   "The energy of a non Wi-Fi received signal should be higher than "
                   "this threshold (dbm) to allow the PHY layer to declare CCA BUSY state. "
                   "This check is performed on the secondary channel(s) only.",
                   DoubleValue (-72.0),
                   MakeDoubleAccessor (&ConstantThresholdChannelBondingManager::SetCcaEdThresholdSecondary),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

void
ConstantThresholdChannelBondingManager::SetCcaEdThresholdSecondary (double threshold)
{
  NS_LOG_FUNCTION (this << threshold);
  m_ccaEdThresholdSecondaryDbm = threshold;
  if (m_phy)
    {
      m_phy->AddCcaEdThresholdSecondary (threshold);
    }
}

void
ConstantThresholdChannelBondingManager::SetPhy (const Ptr<WifiPhy> phy)
{
  phy->AddCcaEdThresholdSecondary (m_ccaEdThresholdSecondaryDbm);
  ChannelBondingManager::SetPhy (phy);
}

double
ConstantThresholdChannelBondingManager::GetCcaEdThresholdSecondaryForMode (WifiMode mode)
{
  NS_LOG_FUNCTION (this << mode );
  return 0;
}
uint16_t
ConstantThresholdChannelBondingManager::GetUsableChannelWidth (WifiMode mode)
{
  if (m_phy->GetChannelWidth () < 40)
    {
//std::cout<<Simulator::Now ().GetMicroSeconds ()<<" Returning usable channel(=20): "<<m_phy->GetChannelWidth ()<<std::endl;
      return m_phy->GetChannelWidth ();
    }
  uint16_t usableChannelWidth = 20;
  for (uint16_t width = m_phy->GetChannelWidth (); width > 20; )
    {
      if (m_phy->GetDelaySinceChannelIsIdle (width, m_ccaEdThresholdSecondaryDbm) >= m_phy->GetPifs ())
        {
          usableChannelWidth = width;
          break;
        }
      width /= 2;
    }
//std::cout<<Simulator::Now ().GetMicroSeconds ()<<" Returning usable channel(>20): "<<m_phy->GetChannelWidth ()<<std::endl;
  return usableChannelWidth;
}

} //namespace ns3

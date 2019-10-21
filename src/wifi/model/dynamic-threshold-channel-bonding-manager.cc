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
#include "ns3/double.h"
#include "dynamic-threshold-channel-bonding-manager.h"
#include "wifi-phy.h"

namespace ns3 {

ATTRIBUTE_CHECKER_IMPLEMENT_WITH_NAME (CcaThresholdPerMcs, "std::map<uint8_t, double>");
ATTRIBUTE_VALUE_IMPLEMENT_WITH_NAME (CcaThresholdPerMcsMap, CcaThresholdPerMcs);

NS_LOG_COMPONENT_DEFINE ("DynamicThresholdChannelBondingManager");
NS_OBJECT_ENSURE_REGISTERED (DynamicThresholdChannelBondingManager);

DynamicThresholdChannelBondingManager::DynamicThresholdChannelBondingManager ()
  : ChannelBondingManager ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
DynamicThresholdChannelBondingManager::GetTypeId (void)
{
  static ns3::TypeId tid = ns3::TypeId ("ns3::DynamicThresholdChannelBondingManager")
    .SetParent<ChannelBondingManager> ()
    .SetGroupName ("Wifi")
    .AddConstructor<DynamicThresholdChannelBondingManager> ()
    .AddAttribute ("CcaEdThresholdSecondaryMap",
                   "The energy of a non Wi-Fi received signal should be higher than "
                   "this threshold (dbm) to allow the PHY layer to declare CCA BUSY state. "
                   "This check is performed on the secondary channel(s) only.",
                   CcaThresholdPerMcsValue ({ {0, -72.0}, {1, -72.0}, {2, -72.0}, {3, -72.0},
                                              {4, -72.0}, {5, -72.0}, {6, -72.0}, {7, -72.0},
                                              {8, -72.0}, {9, -72.0}, {10, -72.0}, {11, -72.0} }),
                   MakeCcaThresholdPerMcsAccessor (&DynamicThresholdChannelBondingManager::m_ccaEdThresholdsSecondaryDbm),
                   MakeCcaThresholdPerMcsChecker ())
  ;
  return tid;
}

void
DynamicThresholdChannelBondingManager::SetCcaEdThresholdSecondaryForMcs (uint8_t mcs, double threshold)
{
  NS_LOG_FUNCTION (this << +mcs << threshold);
  m_ccaEdThresholdsSecondaryDbm.insert ({mcs, threshold});
  if (m_phy)
    {
      m_phy->AddCcaEdThresholdSecondary (threshold);
    }
}

void
DynamicThresholdChannelBondingManager::SetPhy (const Ptr<WifiPhy> phy)
{
  for (auto const& ccaThreshold : m_ccaEdThresholdsSecondaryDbm)
    {
      phy->AddCcaEdThresholdSecondary (ccaThreshold.second);
    }
  ChannelBondingManager::SetPhy (phy);
}

uint16_t
DynamicThresholdChannelBondingManager::GetUsableChannelWidth (uint8_t mcs)
{
  //TODO
  NS_ASSERT (false);
  return m_phy->GetChannelWidth ();
  /*if (m_phy->GetChannelWidth () < 40)
    {
      return m_phy->GetChannelWidth ();
    }
  uint16_t usableChannelWidth = 20;
  for (uint16_t width = m_phy->GetChannelWidth (); width > 20; )
    {
      if (m_phy->GetDelaySinceChannelIsIdle (width) >= m_phy->GetPifs ())
        {
          usableChannelWidth = width;
          break;
        }
      width /= 2;
    }
  return usableChannelWidth;*/
}

std::ostream& operator<< (std::ostream& os, CcaThresholdPerMcsMap ccaThresholdPerMcs)
{
  return os;
}

std::istream &operator>> (std::istream &is, CcaThresholdPerMcsMap &ccaThresholdPerMcs)
{
  return is;
}

} //namespace ns3

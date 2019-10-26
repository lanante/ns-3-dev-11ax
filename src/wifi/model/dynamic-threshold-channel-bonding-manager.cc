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
#include "dynamic-threshold-channel-bonding-manager.h"
#include "wifi-phy.h"
#include "wifi-utils.h"

namespace ns3 {

ATTRIBUTE_CHECKER_IMPLEMENT_WITH_NAME (CcaThresholdPerWifiMode, "std::map<WifiMode, double>");
ATTRIBUTE_VALUE_IMPLEMENT_WITH_NAME (CcaThresholdPerWifiModeMap, CcaThresholdPerWifiMode);

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
                   CcaThresholdPerWifiModeValue (),
                   MakeCcaThresholdPerWifiModeAccessor (&DynamicThresholdChannelBondingManager::m_ccaEdThresholdsSecondaryDbm),
                   MakeCcaThresholdPerWifiModeChecker ())
  ;
  return tid;
}

void
DynamicThresholdChannelBondingManager::SetCcaEdThresholdSecondaryForMode (WifiMode mode, double threshold)
{
  NS_LOG_FUNCTION (this << mode << threshold);
  auto it = m_ccaEdThresholdsSecondaryDbm.find (mode);
  if (it == m_ccaEdThresholdsSecondaryDbm.end ())
    {
      m_ccaEdThresholdsSecondaryDbm.insert ({mode, threshold});
    }
  else
    {
      if (m_phy)
        {
          m_phy->RemoveCcaEdThresholdSecondary (it->second);
        }
      it->second = threshold;
    }
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
DynamicThresholdChannelBondingManager::GetUsableChannelWidth (WifiMode mode)
{
  if (m_phy->GetChannelWidth () < 40)
    {
      return m_phy->GetChannelWidth ();
    }
  double threshold;
  auto it = m_ccaEdThresholdsSecondaryDbm.find (mode);
  if (it != m_ccaEdThresholdsSecondaryDbm.end ())
    {
      threshold = it->second;
    }
  else
    {
      threshold = WToDbm (m_phy->GetDefaultCcaEdThresholdSecondary ());
    }
  uint16_t usableChannelWidth = 20;
  for (uint16_t width = m_phy->GetChannelWidth (); width > 20; )
    {
      if (m_phy->GetDelaySinceChannelIsIdle (width, threshold) >= m_phy->GetPifs ())
        {
          usableChannelWidth = width;
          break;
        }
      width /= 2;
    }
  return usableChannelWidth;
}

std::ostream& operator<< (std::ostream& os, CcaThresholdPerWifiModeMap ccaThresholdPerWifiMode)
{
  //TODO
  return os;
}

std::istream &operator>> (std::istream &is, CcaThresholdPerWifiModeMap &ccaThresholdPerWifiMode)
{
  //TODO
  return is;
}

} //namespace ns3

/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Sébastien Deronne <sebastien.deronne@gmail.com>
 */

#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/config.h"
#include "obss-pd-algorithm.h"
#include "wifi-net-device.h"
#include "regular-wifi-mac.h"
#include "wifi-phy.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ObssPdAlgorithm");
NS_OBJECT_ENSURE_REGISTERED (ObssPdAlgorithm);

TypeId
ObssPdAlgorithm::GetTypeId (void)
{
  static ns3::TypeId tid = ns3::TypeId ("ns3::ObssPdAlgorithm")
    .SetParent<Object> ()
    .SetGroupName ("Wifi")
    .AddAttribute ("ObssPdLevelMin",
                   "Minimum value (dBm) of OBSS_PD level",
                   DoubleValue (-82.0),
                   MakeDoubleAccessor (&ObssPdAlgorithm::m_obssPdLevelMin),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ObssPdLevelMax",
                   "Maximum value (dBm) of OBSS_PD level",
                   DoubleValue (-62.0),
                   MakeDoubleAccessor (&ObssPdAlgorithm::m_obssPdLevelMax),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

void
ObssPdAlgorithm::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_device = 0;
}

void
ObssPdAlgorithm::SetWifiNetDevice (const Ptr<WifiNetDevice> device)
{
  NS_LOG_FUNCTION (this << device);
  m_device = device;
}

Ptr<WifiNetDevice>
ObssPdAlgorithm::GetWifiNetDevice (void) const
{
  return m_device;
}

void
ObssPdAlgorithm::SetupCallbacks ()
{
  uint32_t nodeid = GetWifiNetDevice ()->GetNode ()->GetId ();

  std::ostringstream oss;
  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/*/";
  std::string devicepath = oss.str ();

  // PhyEndOfHePreamble - used to test that the PHY EndOfHePreamble event has fired
  Config::ConnectWithoutContext (devicepath + "$ns3::WifiNetDevice/Phy/EndOfHePreamble", MakeCallback (&ObssPdAlgorithm::ReceiveHeSigA, this));
}

bool
ObssPdAlgorithm::IsObssPdLevelAllowed (double level)
{
  return ((level >= m_obssPdLevelMin) && (level <= m_obssPdLevelMax));
}

} //namespace ns3

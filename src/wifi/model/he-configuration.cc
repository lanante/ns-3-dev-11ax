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
 */

#include "he-configuration.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("HeConfiguration");
NS_OBJECT_ENSURE_REGISTERED (HeConfiguration);

HeConfiguration::HeConfiguration ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
HeConfiguration::GetTypeId (void)
{
  static ns3::TypeId tid = ns3::TypeId ("ns3::HeConfiguration")
    .SetParent<Object> ()
    .SetGroupName ("Wifi")
    .AddConstructor<HeConfiguration> ()
    .AddAttribute ("BssColor", "BSS color",
                   UintegerValue (0),
                   MakeUintegerAccessor (&HeConfiguration::m_bssColor),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("ObssPdThreshold",
                   "Energy threshold (dBm) of received signal below which "
                   "the PHY layer can avoid declaring CCA BUSY for inter-BSS frames.",
                   DoubleValue (-99.0),
                   MakeDoubleAccessor (&HeConfiguration::m_obssPdThreshold),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ObssPdThresholdMin",
                   "Minimum value (dBm) of OBSS_PD threshold",
                   DoubleValue (-82.0),
                   MakeDoubleAccessor (&HeConfiguration::m_obssPdThresholdMin),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ObssPdThresholdMax",
                   "Maximum value (dBm) of OBSS_PD threshold",
                   DoubleValue (-62.0),
                   MakeDoubleAccessor (&HeConfiguration::m_obssPdThresholdMax),
                   MakeDoubleChecker<double> ())
    ;
    return tid;
}

} //namespace ns3

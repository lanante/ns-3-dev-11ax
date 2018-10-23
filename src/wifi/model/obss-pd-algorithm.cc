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

#include "obss-pd-algorithm.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ObssPdAlgorithm");
NS_OBJECT_ENSURE_REGISTERED (ObssPdAlgorithm);

ObssPdAlgorithm::ObssPdAlgorithm ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
ObssPdAlgorithm::GetTypeId (void)
{
  static ns3::TypeId tid = ns3::TypeId ("ns3::ObssPdAlgorithm")
    .SetParent<Object> ()
    .SetGroupName ("Wifi")
    .AddConstructor<ObssPdAlgorithm> ()
    ;
    return tid;
}

void
ObssPdAlgorithm::ReceiveHeSigA (HeSigAParameters params)
{
  NS_LOG_FUNCTION (this);

  // TODO get phy and mac
  // Ptr<WifiPhy> phy = DynamicCast<WifiPhy> (device->GetPhy());
  // Ptr<RegularWifiMac> mac = device->GetMac ()->GetObject<RegularWifiMac> ();

  NS_LOG_DEBUG ("Evaluating OBSS PD algorithm.  RSSI[W]=" << params.rssiW << " bssColor=" << ((uint32_t) params.bssColor));
}

} //namespace ns3

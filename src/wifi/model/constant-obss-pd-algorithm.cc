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
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "constant-obss-pd-algorithm.h"
#include "sta-wifi-mac.h"
#include "wifi-utils.h"
#include "wifi-phy.h"
#include "wifi-net-device.h"
#include "he-configuration.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ConstantObssPdAlgorithm");
NS_OBJECT_ENSURE_REGISTERED (ConstantObssPdAlgorithm);

ConstantObssPdAlgorithm::ConstantObssPdAlgorithm ()
  : ObssPdAlgorithm ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
ConstantObssPdAlgorithm::GetTypeId (void)
{
  static ns3::TypeId tid = ns3::TypeId ("ns3::ConstantObssPdAlgorithm")
    .SetParent<ObssPdAlgorithm> ()
    .SetGroupName ("Wifi")
    .AddConstructor<ConstantObssPdAlgorithm> ()
    .AddAttribute ("ObssPdLevel",
                   "The constant OBSS PD level.",
                   DoubleValue (-82.0),
                   MakeDoubleAccessor (&ConstantObssPdAlgorithm::SetObssPdLevel),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

void
ConstantObssPdAlgorithm::SetObssPdLevel (double level)
{
  //NS_ABORT_MSG_IF (!IsObssPdLevelAllowed (level), "Configured OBSS PD level " << level <<" is not in the allowed range");
  m_obssPdLevel = level;
}

void
ConstantObssPdAlgorithm::ReceiveHeSigA (HePreambleParameters params)
{
  NS_LOG_FUNCTION (this);

  Ptr<StaWifiMac> mac = GetWifiNetDevice ()->GetMac ()->GetObject<StaWifiMac>();
  if (!mac || !mac->IsAssociated ())
    {
      NS_LOG_DEBUG ("This is not an associated STA: skip OBSS_PD SR");
      return;
    }

  NS_LOG_DEBUG ("RSSI(dBm)=" << WToDbm (params.rssiW) << ", BSS color=" << +params.bssColor);

  Ptr<HeConfiguration> heConfiguration = GetWifiNetDevice ()->GetHeConfiguration ();
  NS_ASSERT (heConfiguration);
  UintegerValue bssColorAttribute;
  heConfiguration->GetAttribute ("BssColor", bssColorAttribute);
  uint8_t bssColor = bssColorAttribute.Get ();

  if (bssColor == 0)
    {
      NS_LOG_DEBUG ("BSS color is 0: OBSS_PD SR is not allowed!");
    }
  //TODO: SRP_AND_NON-SRG_OBSS-PD_PROHIBITED=1 => OBSS_PD SR is not allowed

  bool isObss = (bssColor != params.bssColor);
  if (isObss && (WToDbm (params.rssiW) < m_obssPdLevel))
    {
      Ptr<WifiPhy> phy = GetWifiNetDevice ()->GetPhy();
      NS_LOG_DEBUG ("Frame is OBSS and RSSI is below OBSS-PD level: reset PHY to IDLE");
      phy->ResetCca ();
    }
}

void
ConstantObssPdAlgorithm::ReceiveBeacon (HeBeaconReceptionParameters params)
{
  NS_LOG_FUNCTION (this);
}

} //namespace ns3

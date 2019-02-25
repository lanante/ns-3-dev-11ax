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
#include "beacon-rssi-obss-pd-algorithm.h"
#include "sta-wifi-mac.h"
#include "wifi-phy.h"
#include "wifi-net-device.h"
#include "he-configuration.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BeaconRssiObssPdAlgorithm");
NS_OBJECT_ENSURE_REGISTERED (BeaconRssiObssPdAlgorithm);

BeaconRssiObssPdAlgorithm::BeaconRssiObssPdAlgorithm ()
  : ObssPdAlgorithm (),
    m_beaconCount (0),
    m_rssiAve (0),
    m_txPower (0)
{
  NS_LOG_FUNCTION (this);
  memset (m_rssiArray, 0, sizeof (m_rssiArray));
}

TypeId
BeaconRssiObssPdAlgorithm::GetTypeId (void)
{
  static ns3::TypeId tid = ns3::TypeId ("ns3::BeaconRssiObssPdAlgorithm")
    .SetParent<ObssPdAlgorithm> ()
    .SetGroupName ("Wifi")
    .AddConstructor<BeaconRssiObssPdAlgorithm> ()
  ;
  return tid;
}

void
BeaconRssiObssPdAlgorithm::SetupCallbacks ()
{
  uint32_t nodeid = GetWifiNetDevice ()->GetNode ()->GetId ();
        
  std::ostringstream oss;
  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/*/";
  std::string devicepath = oss.str ();
        
  Config::ConnectWithoutContext (devicepath + "$ns3::WifiNetDevice/Mac/$ns3::StaWifiMac/BeaconReception", MakeCallback (&BeaconRssiObssPdAlgorithm::ReceiveBeacon, this));
}

void
BeaconRssiObssPdAlgorithm::ReceiveHeSig (HePreambleParameters params)
{
  NS_LOG_FUNCTION (this);
}

void
BeaconRssiObssPdAlgorithm::ReceiveBeacon (HeBeaconReceptionParameters params)
{
  NS_LOG_FUNCTION (this);
  Ptr<HeConfiguration> heConfiguration = GetWifiNetDevice ()->GetHeConfiguration ();
  NS_ASSERT (heConfiguration);
  UintegerValue myBssColor;
  heConfiguration->GetAttribute ("BssColor", myBssColor);
  if (params.bssColor == myBssColor.Get ())
    {
      double aveRxPower = 0;
      if (m_beaconCount < 10)
        {
          m_rssiArray[m_beaconCount] = WToDbm (params.rssiW);
          for (int i = 0; i < m_beaconCount + 1; i++)
            {
              aveRxPower = aveRxPower + m_rssiArray[i];
            }
          aveRxPower = aveRxPower/(m_beaconCount + 1);
        }
      else
        {
          for (int i = 0; i < 9; i++)
            {
              m_rssiArray[i] = m_rssiArray[i+1];
              aveRxPower = aveRxPower + m_rssiArray[i];
            }
          m_rssiArray[9] = WToDbm (params.rssiW);
          aveRxPower = aveRxPower + m_rssiArray[9];
          aveRxPower = aveRxPower/10;
        }
      if (m_beaconCount == 0)
        {
          m_txPower = GetWifiNetDevice ()->GetPhy()->GetTxPowerEnd ();
        }
      m_beaconCount++;
      if (Abs (m_rssiAve - (aveRxPower - 5)) > 2)
        {
          m_rssiAve = aveRxPower-5;
          SetObssPdLevel (m_rssiAve);
          UpdateObssPdLevel ();
        }
    }
}

void
BeaconRssiObssPdAlgorithm::UpdateObssPdLevel (void)
{
  NS_LOG_FUNCTION (this);
  double tmpMax = std::max (GetObssPdLevelMin (), std::min(GetObssPdLevelMax (), GetObssPdLevelMin () + GetTxPowerRefSiso () - m_txPower));
  if (GetObssPdLevel () > tmpMax)
    {
      NS_LOG_DEBUG ("Updating ObssPdLevel value to " << tmpMax);
      SetObssPdLevel (tmpMax);
    }
}

} //namespace ns3

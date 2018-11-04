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

ObssPdAlgorithm::ObssPdAlgorithm () :
  m_obssPdLevel (-82.0),
  m_txPwr (10.0)
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
    .AddAttribute ("ObssPdLevel",
                   "The OBSS PD level.",
                   DoubleValue (-82.0),
                   MakeDoubleAccessor (&ObssPdAlgorithm::GetObssPdLevel),
                   MakeDoubleChecker<double> ())
    .AddTraceSource ("ObssPdLevel",
                     "The OBSS PD level.",
                     MakeTraceSourceAccessor (&ObssPdAlgorithm::m_obssPdLevel),
                     "ns3::TracedValueCallback::Double")
    .AddAttribute ("TxPwr",
                   "The TxPwr.",
                   DoubleValue (10.0),
                   MakeDoubleAccessor (&ObssPdAlgorithm::GetTxPwr),
                   MakeDoubleChecker<double> ())
    .AddTraceSource ("TxPwr",
                     "The TxPwr.",
                     MakeTraceSourceAccessor (&ObssPdAlgorithm::m_txPwr),
                     "ns3::TracedValueCallback::Double")
    ;
    return tid;
}

void
ObssPdAlgorithm::SetObssPdLevel (double level)
{
  NS_LOG_FUNCTION (this << level);
  m_obssPdLevel = level;
}

double
ObssPdAlgorithm::GetObssPdLevel (void) const
{
  return m_obssPdLevel;
}

void
ObssPdAlgorithm::SetTxPwr (double txPwr)
{
  NS_LOG_FUNCTION (this << txPwr);
  m_txPwr = txPwr;
}

double
ObssPdAlgorithm::GetTxPwr (void) const
{
  return m_txPwr;
}

Ptr<WifiNetDevice>
ObssPdAlgorithm::GetWifiNetDevice (void) const
{
  Ptr<WifiNetDevice> wifiNetDevice = this->GetObject<WifiNetDevice> ();
  NS_ASSERT (wifiNetDevice);
  return wifiNetDevice;
}

Ptr<WifiPhy>
ObssPdAlgorithm::GetWifiPhy (void) const
{
  Ptr<WifiNetDevice> wifiNetDevice = GetWifiNetDevice ();
  Ptr<WifiPhy> wifiPhy = DynamicCast<WifiPhy> (wifiNetDevice->GetPhy());
  NS_ASSERT (wifiPhy);
  return wifiPhy;
}

Ptr<RegularWifiMac>
ObssPdAlgorithm::GetRegularWifiMac (void) const
{
  Ptr<WifiNetDevice> wifiNetDevice = GetWifiNetDevice ();
  Ptr<RegularWifiMac> regularWifiMac = wifiNetDevice->GetMac ()->GetObject<RegularWifiMac> ();
  NS_ASSERT (regularWifiMac);
  return regularWifiMac;
}

Ptr<HeConfiguration>
ObssPdAlgorithm::GetHeConfiguration (void) const
{

  Ptr<RegularWifiMac> regularWifiMac = GetRegularWifiMac ();
  Ptr<HeConfiguration> heConfiguration = regularWifiMac->GetHeConfiguration ();
  // assume that there must be an HeConfiguration object for this device
  NS_ASSERT (heConfiguration);
  return heConfiguration;
}

void
ObssPdAlgorithm::ReceiveHeSigA (HeSigAParameters params)
{
  NS_LOG_FUNCTION (this);

  DoReceiveHeSigA (params);
}

void
ObssPdAlgorithm::DoReceiveHeSigA (HeSigAParameters params)
{
  NS_LOG_FUNCTION (this);

  // TODO get phy and mac
  // Ptr<WifiPhy> phy = DynamicCast<WifiPhy> (device->GetPhy());
  // Ptr<RegularWifiMac> mac = device->GetMac ()->GetObject<RegularWifiMac> ();

  NS_LOG_DEBUG ("Evaluating OBSS PD algorithm.  RSSI[W]=" << params.rssiW << " bssColor=" << ((uint32_t) params.bssColor));

  // perform PHY reset if receive OBSS frame below constant threshold

  // 1. RSSI<OBSS PD level
  // 2. Own BSS color!=Signal BSS color
  // 3. SRP_AND_NON-SRG _OBSS-PD_PROHIBITED ==0
}

void
ObssPdAlgorithm::DoReceiveBeacon ()
{
  NS_LOG_FUNCTION (this);
}

} //namespace ns3

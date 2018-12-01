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

#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/config.h"
#include "constant-obss-pd-algorithm.h"
#include "wifi-phy.h"
#include "regular-wifi-mac.h"
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
                   "The OBSS PD level.",
                   DoubleValue (-82.0),
                   MakeDoubleAccessor (&ConstantObssPdAlgorithm::SetObssPdLevel),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxPower",
                   "The TX power.",
                   DoubleValue (10.0),
                   MakeDoubleAccessor (&ConstantObssPdAlgorithm::SetTxPower),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

void
ConstantObssPdAlgorithm::SetObssPdLevel (double level)
{
  NS_LOG_FUNCTION (this << level);
  m_obssPdLevel = level;
}

double
ConstantObssPdAlgorithm::GetObssPdLevel (void) const
{
  return m_obssPdLevel;
}

void
ConstantObssPdAlgorithm::SetTxPower (double txPower)
{
  NS_LOG_FUNCTION (this << txPower);
  m_txPower = txPower;
}

double
ConstantObssPdAlgorithm::GetTxPower (void) const
{
  return m_txPower;
}

void
ConstantObssPdAlgorithm::SetupCallbacks ()
{
  Ptr<WifiNetDevice> wifiNetDevice = GetWifiNetDevice ();
  Ptr<Node> node = wifiNetDevice->GetNode ();
  uint32_t nodeid = node->GetId ();

  std::ostringstream oss;
  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/*/";
  std::string devicepath = oss.str ();

  // PhyEndOfHePreamble - used to test that the PHY EndOfHePreamble event has fired
  Config::ConnectWithoutContext (devicepath + "$ns3::WifiNetDevice/Phy/EndOfHePreamble", MakeCallback (&ConstantObssPdAlgorithm::ReceiveHeSigA, this));

  // @TODO set up callback for StaWifiMac BeaconReception
  // This might go here, or it might be delegrated to a derived class.  Not all OBSS PD algorithms require
  // beacon reception information?
}

Ptr<WifiNetDevice>
ConstantObssPdAlgorithm::GetWifiNetDevice (void) const
{
  Ptr<WifiNetDevice> wifiNetDevice = this->GetObject<WifiNetDevice> ();
  NS_ASSERT (wifiNetDevice);
  return wifiNetDevice;
}

Ptr<WifiPhy>
ConstantObssPdAlgorithm::GetPhy (void) const
{
  Ptr<WifiNetDevice> wifiNetDevice = GetWifiNetDevice ();
  Ptr<WifiPhy> wifiPhy = DynamicCast<WifiPhy> (wifiNetDevice->GetPhy ());
  NS_ASSERT (wifiPhy);
  return wifiPhy;
}

Ptr<HeConfiguration>
ConstantObssPdAlgorithm::GetHeConfiguration (void) const
{
  Ptr<HeConfiguration> heConfiguration = GetWifiNetDevice ()->GetHeConfiguration ();
  // assume that there must be an HeConfiguration object for this device
  NS_ASSERT (heConfiguration);
  return heConfiguration;
}

void
ConstantObssPdAlgorithm::ReceiveHeSigA (HeSigAParameters params)
{
  NS_LOG_FUNCTION (this);

  // TODO get phy and mac
  // Ptr<WifiPhy> phy = DynamicCast<WifiPhy> (device->GetPhy());
  // Ptr<RegularWifiMac> mac = device->GetMac ()->GetObject<RegularWifiMac> ();

  NS_LOG_DEBUG ("Evaluating OBSS PD algorithm.  RSSI[W]=" << params.rssiW << " bssColor=" << ((uint32_t) params.bssColor));

  // perform PHY reset if receive OBSS frame below constant threshold

  // @TODO:
  // OBSS PD level needs to be "set" prior to evaluation here
  // Own BSS color needs to be obtained from device.  If STA, it may be 0 until association with
  // an 11ax AP is completed.
  // the bits in (3) below are defined in wifi-phy.h but not yet being sent.

  // 1. RSSI<OBSS PD level
  // 2. Own BSS color!=Signal BSS color
  // 3. SRP_AND_NON-SRG _OBSS-PD_PROHIBITED ==0
}

void
ConstantObssPdAlgorithm::ReceiveBeacon (HeBeaconReceptionParameters params)
{
  NS_LOG_FUNCTION (this);
}

} //namespace ns3

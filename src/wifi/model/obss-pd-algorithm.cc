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
    .AddAttribute ("ObssPdLevel",
                   "The current OBSS PD level.",
                   DoubleValue (-82.0),
                   MakeDoubleAccessor (&ObssPdAlgorithm::SetObssPdLevel),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ObssPdLevelMin",
                   "Minimum value (dBm) of OBSS PD level.",
                   DoubleValue (-82.0),
                   MakeDoubleAccessor (&ObssPdAlgorithm::SetObssPdLevelMin),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ObssPdLevelMax",
                   "Maximum value (dBm) of OBSS PD level.",
                   DoubleValue (-62.0),
                   MakeDoubleAccessor (&ObssPdAlgorithm::SetObssPdLevelMax),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxPowerRef",
                   "The SISO reference TX power level (dBm).",
                   DoubleValue (21),
                   MakeDoubleAccessor (&ObssPdAlgorithm::SetTxPowerRef),
                   MakeDoubleChecker<double> ())
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
ObssPdAlgorithm::SetObssPdLevelMin (double level)
{
  NS_LOG_FUNCTION (this << level);
  m_obssPdLevelMin = level;
}

double
ObssPdAlgorithm::GetObssPdLevelMin (void) const
{
  return m_obssPdLevelMin;
}

void
ObssPdAlgorithm::SetObssPdLevelMax (double level)
{
  NS_LOG_FUNCTION (this << level);
  m_obssPdLevelMax = level;
}

double
ObssPdAlgorithm::GetObssPdLevelMax (void) const
{
  return m_obssPdLevelMax;
}

void
ObssPdAlgorithm::SetTxPowerRef (double power)
{
  NS_LOG_FUNCTION (this << power);
  m_txPowerRef = power;
}

double
ObssPdAlgorithm::GetTxPowerRef (void) const
{
  return m_txPowerRef;
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

void
ObssPdAlgorithm::ResetPhy()
{
  double txPowerMax = 0;
  bool powerRestricted = false;
  Ptr<WifiPhy> phy = GetWifiNetDevice ()->GetPhy();
  if ((m_obssPdLevel > m_obssPdLevelMin) || (m_obssPdLevel >= m_obssPdLevelMax))
    {
      txPowerMax = m_txPowerRef - (m_obssPdLevel - m_obssPdLevelMin);
      powerRestricted = true;
    }
  phy->ResetCca (powerRestricted, txPowerMax);
}

} //namespace ns3

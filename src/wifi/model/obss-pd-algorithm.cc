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
#include "obss-pd-algorithm.h"
#include "wifi-net-device.h"
#include "sta-wifi-mac.h"
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
                   MakeDoubleAccessor (&ObssPdAlgorithm::SetObssPdLevel,
                                       &ObssPdAlgorithm::GetObssPdLevel),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ObssPdLevelMin",
                   "Minimum value (dBm) of OBSS PD level.",
                   DoubleValue (-82.0),
                   MakeDoubleAccessor (&ObssPdAlgorithm::SetObssPdLevelMin,
                                       &ObssPdAlgorithm::GetObssPdLevelMin),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ObssPdLevelMax",
                   "Maximum value (dBm) of OBSS PD level.",
                   DoubleValue (-62.0),
                   MakeDoubleAccessor (&ObssPdAlgorithm::SetObssPdLevelMax,
                                       &ObssPdAlgorithm::GetObssPdLevelMax),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxPowerRefSiso",
                   "The SISO reference TX power level (dBm).",
                   DoubleValue (21),
                   MakeDoubleAccessor (&ObssPdAlgorithm::m_txPowerRefSiso),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxPowerRefMimo",
                   "The MIMO reference TX power level (dBm).",
                   DoubleValue (25),
                   MakeDoubleAccessor (&ObssPdAlgorithm::m_txPowerRefMimo),
                   MakeDoubleChecker<double> ())
    .AddTraceSource ("Reset", "Trace CCA Reset event",
                     MakeTraceSourceAccessor (&ObssPdAlgorithm::m_resetEvent),
                     "ns3::ObssPdAlgorithm::ResetTracedCallback")
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

double
ObssPdAlgorithm::GetTxPowerRefSiso (void) const
{
  return m_txPowerRefSiso;
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
ObssPdAlgorithm::ResetPhy (HePreambleParameters params)
{
  double txPowerMaxSiso = 0;
  double txPowerMaxMimo = 0;
  bool powerRestricted = false;
  // Fetch my BSS color
  Ptr<HeConfiguration> heConfiguration = GetWifiNetDevice ()->GetHeConfiguration ();
  NS_ASSERT (heConfiguration);
  UintegerValue bssColorAttribute;
  heConfiguration->GetAttribute ("BssColor", bssColorAttribute);
  uint8_t bssColor = bssColorAttribute.Get ();
  NS_LOG_DEBUG ("My BSS color " << (uint16_t) bssColor << " received frame " << (uint16_t) params.bssColor);

  Ptr<WifiPhy> phy = GetWifiNetDevice ()->GetPhy();
  if ((m_obssPdLevel > m_obssPdLevelMin) || (m_obssPdLevel <= m_obssPdLevelMax))
    {
      txPowerMaxSiso = m_txPowerRefSiso - (m_obssPdLevel - m_obssPdLevelMin);
      txPowerMaxMimo = m_txPowerRefMimo - (m_obssPdLevel - m_obssPdLevelMin);
      powerRestricted = true;
    }
  m_resetEvent (bssColor, WToDbm (params.rssiW), powerRestricted, txPowerMaxSiso, txPowerMaxMimo);
  phy->ResetCca (powerRestricted, txPowerMaxSiso, txPowerMaxMimo);
}

} //namespace ns3

/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 * 
 */

#include <ns3/log.h>
#include <ns3/double.h>
#include <ns3/mobility-model.h>

#include <cmath>
#include <algorithm>

#include "ieee-80211ax-indoor-propagation-loss-model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Ieee80211axIndoorPropagationLossModel");

NS_OBJECT_ENSURE_REGISTERED (Ieee80211axIndoorPropagationLossModel);

Ieee80211axIndoorPropagationLossModel::Ieee80211axIndoorPropagationLossModel ()
{
  m_shadowing.SetGetSigmaCallback (MakeCallback (&Ieee80211axIndoorPropagationLossModel::GetSigma, this));
}

TypeId
Ieee80211axIndoorPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Ieee80211axIndoorPropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .AddConstructor <Ieee80211axIndoorPropagationLossModel> ()
    .SetGroupName ("LaaWifiCoexistence")
    .AddAttribute ("Frequency",
                   "The frequency [Hz] at which the model is evaluated.",
                   DoubleValue (5e9),
                   MakeDoubleAccessor (&Ieee80211axIndoorPropagationLossModel::m_frequency),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Sigma",
                   "Standard deviation of the shadowing loss in dB",
                   DoubleValue (5),
                   MakeDoubleAccessor (&Ieee80211axIndoorPropagationLossModel::m_sigma),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DistanceBreakpoint",
                   "Distance breakpoint (m), to parameterize the model.",
                   DoubleValue (10),
                   MakeDoubleAccessor (&Ieee80211axIndoorPropagationLossModel::m_distance_breakpoint),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Walls",
                   "Number of walls intersected, to parameterize the model.",
                   DoubleValue (0),
                   MakeDoubleAccessor (&Ieee80211axIndoorPropagationLossModel::m_walls),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("WallsFactor",
                   "Loss per wall intersected (dB), to parameterize the model.",
                   DoubleValue (0),
                   MakeDoubleAccessor (&Ieee80211axIndoorPropagationLossModel::m_wall_factor),
                   MakeDoubleChecker<double> ());
  return tid;
}

double
Ieee80211axIndoorPropagationLossModel::GetPathLossDb (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  NS_LOG_FUNCTION (this << a << b);
  double distance3D = a->GetDistanceFrom (b);
  double d = std::max (distance3D, 1.0);
  double lossDb = 40.05
    + 20*std::log10 (m_frequency/(2.4*1e9))
    + 20*std::log10(std::min(d, m_distance_breakpoint))
    + ((d>m_distance_breakpoint) ? 35*std::log10(d/m_distance_breakpoint) : 0)
    + m_walls*m_wall_factor;
  NS_LOG_LOGIC ("d = " << d << " lossDb = " << lossDb);
  return lossDb;
}

double 
Ieee80211axIndoorPropagationLossModel::DoCalcRxPower (double txPowerDbm,
						Ptr<MobilityModel> a,
						Ptr<MobilityModel> b) const
{
  if (a->GetDistanceFrom (b) == 0.0)
    {
      NS_LOG_LOGIC ("Co-located NetDevices. No path loss nor shadowing loss to consider");
      return txPowerDbm;
    }
  return (txPowerDbm - GetPathLossDb (a, b) - m_shadowing.GetLossDb (a, b));
}

double
Ieee80211axIndoorPropagationLossModel::GetSigma (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  return m_sigma;
}


int64_t
Ieee80211axIndoorPropagationLossModel::DoAssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  return m_shadowing.AssignStreams (stream);
}

} // namespace ns3

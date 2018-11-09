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

#include "constant-obss-pd-algorithm.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/wifi-net-device.h"
#include "ns3/he-configuration.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ConstantObssPdAlgorithm");
NS_OBJECT_ENSURE_REGISTERED (ConstantObssPdAlgorithm);

ConstantObssPdAlgorithm::ConstantObssPdAlgorithm ()
  : ObssPdAlgorithm (),
    m_constantObssPdLevel (-82.0)
{
  NS_LOG_FUNCTION (this);
}

TypeId
ConstantObssPdAlgorithm::GetTypeId (void)
{
  static ns3::TypeId tid = ns3::TypeId ("ns3::ConstantObssPdAlgorithm")
    .SetParent<Object> ()
    .SetGroupName ("Wifi")
    .AddConstructor<ConstantObssPdAlgorithm> ()
    .AddAttribute ("ConstantObssPdLevel",
                   "The OBSS PD level.",
                   DoubleValue (-82.0),
                   MakeDoubleAccessor (&ConstantObssPdAlgorithm::GetConstantObssPdLevel),
                   MakeDoubleChecker<double> ())
    .AddTraceSource ("ConstantObssPdLevel",
                     "The OBSS PD level.",
                     MakeTraceSourceAccessor (&ConstantObssPdAlgorithm::m_constantObssPdLevel),
                     "ns3::TracedValueCallback::Double");
  return tid;
}

void
ConstantObssPdAlgorithm::SetConstantObssPdLevel (double level)
{
  NS_LOG_FUNCTION (this << level);

  m_constantObssPdLevel = level;
}

double
ConstantObssPdAlgorithm::GetConstantObssPdLevel (void) const
{
  return m_constantObssPdLevel;
}

void
ConstantObssPdAlgorithm::ReceiveHeSigA (HeSigAParameters params)
{
  NS_LOG_FUNCTION (this);

  // for now, delegate to base class
  ObssPdAlgorithm::ReceiveHeSigA (params);
}

} //namespace ns3

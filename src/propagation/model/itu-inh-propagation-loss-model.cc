/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2016 University of Washington
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
 * Adapted from UmI to InH model by Tom Henderson <tomhend@ee.washington.edu>
 */

#include <ns3/log.h>
#include <ns3/double.h>
#include <ns3/mobility-model.h>

#include <cmath>
#include <algorithm>

#include "itu-inh-propagation-loss-model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ItuInhPropagationLossModel");

NS_OBJECT_ENSURE_REGISTERED (ItuInhPropagationLossModel);

ItuInhPropagationLossModel::ItuInhPropagationLossModel ()
{
  m_shadowing.SetGetSigmaCallback (MakeCallback (&ItuInhPropagationLossModel::GetSigma, this));
  m_losNlosClassifier.SetGetLosProbabilityCallback (MakeCallback (&ItuInhPropagationLossModel::GetLosProbability, this));
}

TypeId
ItuInhPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ItuInhPropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .AddConstructor <ItuInhPropagationLossModel> ()
    .SetGroupName ("Propagation")
    .AddAttribute ("Frequency",
                   "The frequency [Hz] at which the model is evaluated.",
                   DoubleValue (5e9),
                   MakeDoubleAccessor (&ItuInhPropagationLossModel::m_frequency),
                   MakeDoubleChecker<double> ());
  return tid;
}

double
ItuInhPropagationLossModel::GetLosPathLossDb (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  NS_LOG_FUNCTION (this << a << b);

  // constant parameters from the model specification in 3GPP TR 36.814
  double d = a->GetDistanceFrom (b); // 3D distance
  double lossDb = 0.0;
  
  // the model is unspecified for d <= 3, in that case use the value for d = 3 
  d = std::max (d, 3.0);

  // the model is unspecified for d > 100, in that case use the value for d = 100 
  if (d > 100)
    {
      NS_LOG_WARN ("distance d " << d << " greater than range of model");
      d = std::min (d, 100.0);
    }

  lossDb =  16.9 * std::log10 (d) + 32.8 + 20*std::log10 (m_frequency/1e9);
  
  NS_LOG_LOGIC (" path loss dB = " << lossDb << " for distance " << d);
  return lossDb;

}

double
ItuInhPropagationLossModel::GetNlosPathLossDb (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  NS_LOG_FUNCTION (this << a << b);
  double d = a->GetDistanceFrom (b); // 3D distance

  // the model is unspecified for d <= 10, in that case use the value for d = 10 
  d = std::max (d, 10.0);

  // the model is unspecified for d > 150, in that case use the value for d = 150 
  if (d > 150)
    {
      NS_LOG_WARN ("distance d " << d << " greater than range of model");
      d = std::min (d, 150.0);
    }

  double lossDb =  43.3*std::log10 (d) + 11.5 + 20*std::log10 (m_frequency/1e9); 
  NS_LOG_LOGIC (" path loss dB = " << lossDb << " for distance " << d);
  return lossDb;
}


double 
ItuInhPropagationLossModel::DoCalcRxPower (double txPowerDbm,
						Ptr<MobilityModel> a,
						Ptr<MobilityModel> b) const
{
  NS_LOG_FUNCTION (this << txPowerDbm);
  double pathLossDb = 0.0;
  if (m_losNlosClassifier.IsLos (a, b))
    {
      pathLossDb = GetLosPathLossDb (a, b);
    }
  else
    {
      pathLossDb = GetNlosPathLossDb (a, b);
    }
   double shadowingLossDb = m_shadowing.GetLossDb (a, b);
   NS_LOG_LOGIC (" path loss = " << pathLossDb << " dB, shadowing = " << shadowingLossDb << " dB");
  return txPowerDbm - pathLossDb - shadowingLossDb;
}

double
ItuInhPropagationLossModel::GetSigma (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  NS_LOG_FUNCTION (this << a << b);
  if (m_losNlosClassifier.IsLos (a, b))
    {
      NS_LOG_LOGIC ("Returning 3 (LOS)");
      return 3;
    }
  else
    {
      NS_LOG_LOGIC ("Returning 4 (NLOS)");
      return 4;
    }      
}

double
ItuInhPropagationLossModel::GetLosProbability (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  NS_LOG_FUNCTION (this << a << b);
  double d = a->GetDistanceFrom (b); // 3D distance
  double p = 1; // d <= 18
  if (d > 18 && d < 37)
    {
      p = std::exp (-1.0 * (d - 18)/27);
    }
  else if (d >= 37)
    {
      p = 0.5;
    }
  NS_LOG_LOGIC ("Returning LosProbability " << p << " for distance " << d);
  return p;
}

int64_t
ItuInhPropagationLossModel::DoAssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  int64_t currentStream = stream;
  currentStream += m_shadowing.AssignStreams (currentStream);
  currentStream += m_losNlosClassifier.AssignStreams (currentStream);
  NS_LOG_LOGIC ("Returning " << (currentStream - stream) << " streams");
  return (currentStream - stream);
}


} // namespace ns3

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

#include "itu-umi-propagation-loss-model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ItuUmiPropagationLossModel");

NS_OBJECT_ENSURE_REGISTERED (ItuUmiPropagationLossModel);

ItuUmiPropagationLossModel::ItuUmiPropagationLossModel ()
{
  m_shadowing.SetGetSigmaCallback (MakeCallback (&ItuUmiPropagationLossModel::GetSigma, this));
  m_losNlosClassifier.SetGetLosProbabilityCallback (MakeCallback (&ItuUmiPropagationLossModel::GetLosProbability, this));
}

TypeId
ItuUmiPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ItuUmiPropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .AddConstructor <ItuUmiPropagationLossModel> ()
    .SetGroupName ("LaaWifiCoexistence")
    .AddAttribute ("Frequency",
                   "The frequency [Hz] at which the model is evaluated.",
                   DoubleValue (5e9),
                   MakeDoubleAccessor (&ItuUmiPropagationLossModel::m_frequency),
                   MakeDoubleChecker<double> ());
  return tid;
}

double
ItuUmiPropagationLossModel::GetLosPathLossDb (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  NS_LOG_FUNCTION (this << a << b);

  // constant parameters from the model specification in 3GPP TR 36.814
  const double c = 3.0e8; // c [m/s] 
  const double hBS = 10.0; // h_{BS} [m]
  const double hUT = 1.5; // h_{UT} [m]
  const double hPrimeBS = hBS - 1.0; // h'_{BS} [m]
  const double hPrimeUT = hUT - 1.0; // h'_{UT} [m]
  const double dPrimeBP = 4.0 * hBS * hUT * m_frequency / c;

  double d = a->GetDistanceFrom (b); // 3D distance
  
  // the model is unspecified for d < 10, in that case use the value for d = 10
  d = std::max (d, 10.0);

  double lossDb = 0.0;
  if (d < dPrimeBP)
    {
      lossDb =  22*std::log10 (d) + 28 + 20*std::log10 (m_frequency/1e9);
    }
  else
    {
      lossDb = 40*std::log10 (d) + 7.8 - 18*std::log (hPrimeBS) 
        - 18*std::log(hPrimeUT) + 2*std::log(m_frequency/1e9);
    }
  
  NS_LOG_LOGIC (" path loss dB = " << lossDb);
  return lossDb;

}

double
ItuUmiPropagationLossModel::GetNlosPathLossDb (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  NS_LOG_FUNCTION (this << a << b);
  double d = a->GetDistanceFrom (b); // 3D distance
  double lossDb =  36.7*std::log10 (d) + 22.7 + 26*std::log10 (m_frequency/1e9); 

  return lossDb;
}


double 
ItuUmiPropagationLossModel::DoCalcRxPower (double txPowerDbm,
						Ptr<MobilityModel> a,
						Ptr<MobilityModel> b) const
{
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
ItuUmiPropagationLossModel::GetSigma (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  if (m_losNlosClassifier.IsLos (a, b))
    {
      return 3;
    }
  else
    {
      return 4;
    }      
}

double
ItuUmiPropagationLossModel::GetLosProbability (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  double d = a->GetDistanceFrom (b); // 3D distance
  double p = std::min (18.0/d, 1.0) * (1.0 - std::exp (-d/36)) + std::exp (-d/36);
  return p;
}

int64_t
ItuUmiPropagationLossModel::DoAssignStreams (int64_t stream)
{
  int64_t currentStream = stream;
  currentStream += m_shadowing.AssignStreams (currentStream);
  currentStream += m_losNlosClassifier.AssignStreams (currentStream);
  return (currentStream - stream);
}


} // namespace ns3

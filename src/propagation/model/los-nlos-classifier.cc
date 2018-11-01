/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011, 2015 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * part of the codes comes from the shadowing implementation in
 * BuildingsPropagationLossModel by Marco Miozzo and Nicola Baldo 
 */


#include "los-nlos-classifier.h"

#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LosNlosClassifier");

  
LosNlosClassifier::LosNlosClassifier ()
{
  NS_LOG_FUNCTION (this);
  m_uniformRandomVariable = CreateObject<UniformRandomVariable> ();
}

void 
LosNlosClassifier::SetGetLosProbabilityCallback (LosNlosClassifierGetLosProbabilityCallback c)
{
  NS_LOG_FUNCTION (this);
  m_getLosProbabilityCallback = c;
}

bool
LosNlosClassifier::IsLos (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  NS_LOG_FUNCTION (this << a << b);

  // eventually swap arguments for simmetry
  if (a > b)
    {
      return IsLos(b, a);
    }

  std::map<Ptr<MobilityModel>,  std::map<Ptr<MobilityModel>, double> >::iterator 
    ait = m_isLosMap.find (a);
  
  if (ait != m_isLosMap.end ())
    {
      std::map<Ptr<MobilityModel>, double>::iterator 
	bit = ait->second.find (b);
      if (bit != ait->second.end ())
        {
          return (bit->second);
        }
      else
        {
          double p = m_getLosProbabilityCallback (a, b);
          double x = m_uniformRandomVariable->GetValue (0.0, 1.0);
          bool isLos = (x <= p);
          // side effect: will create new entry          
          ait->second[b] = isLos;          
          return (isLos);
        }
    }
  else
    {
      double p = m_getLosProbabilityCallback (a, b);
      double x = m_uniformRandomVariable->GetValue (0.0, 1.0);
      bool isLos = (x <= p);
      // side effect: will create new entries in both maps
      m_isLosMap[a][b] = isLos;  
      return (isLos);       
    }
}

int64_t
LosNlosClassifier::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uniformRandomVariable->SetStream (stream);
  return 1;
}


} // namespace ns3

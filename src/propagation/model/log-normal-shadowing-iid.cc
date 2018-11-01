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
 * part of the codes comes from a previous shadowing implementation in
 * BuildingsPropagationLossModel by Marco Miozzo and Nicola Baldo 
 */


#include "log-normal-shadowing-iid.h"

#include <ns3/log.h>

#include <functional>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LogNormalShadowingIid");

  
LogNormalShadowingIid::LogNormalShadowingIid ()
{
  NS_LOG_FUNCTION (this);
  m_normalRandomVariable = CreateObject<NormalRandomVariable> ();
}

void 
LogNormalShadowingIid::SetGetSigmaCallback (LogNormalShadowingIidGetSigmaCallback c)
{
  NS_LOG_FUNCTION (this);
  m_getSigmaCallback = c;
}

double
LogNormalShadowingIid::GetLossDb (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  NS_LOG_FUNCTION (this << a << b);

  // eventually swap arguments for simmetry
  if (a > b)
    {
      return GetLossDb (b, a);
    }

  std::map<Ptr<MobilityModel>,  std::map<Ptr<MobilityModel>, double> >::iterator 
    ait = m_shadowingLossMap.find (a);
  
  if (ait != m_shadowingLossMap.end ())
    {
      std::map<Ptr<MobilityModel>, double>::iterator 
	bit = ait->second.find (b);
      if (bit != ait->second.end ())
        {
          return (bit->second);
        }
      else
        {
          double sigma = m_getSigmaCallback (a, b);
          // side effect: will create new entry          
          // sigma is standard deviation, not variance
          double shadowingValue = m_normalRandomVariable->GetValue (0.0, (sigma*sigma));
          ait->second[b] = shadowingValue;          
          return (shadowingValue);
        }
    }
  else
    {
      double sigma = m_getSigmaCallback (a, b);
      // side effect: will create new entries in both maps
      // sigma is standard deviation, not variance
      double shadowingValue = m_normalRandomVariable->GetValue (0.0, (sigma*sigma));
      m_shadowingLossMap[a][b] = shadowingValue;  
      return (shadowingValue);       
    }
}

int64_t
LogNormalShadowingIid::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_normalRandomVariable->SetStream (stream);
  return 1;
}


} // namespace ns3

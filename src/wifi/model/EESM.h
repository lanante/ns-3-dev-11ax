/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 CTTC
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
 * Author: Rohan Patidar <rpatidar@uw.edu>
 */

#ifndef EESM_H
#define EESM_H


#include <ns3/wifi-mode.h>
#include <ns3/object.h>
#include <ns3/wifi-phy-standard.h>
#include <string>
#include <map>
#include <vector>
#include <ns3/spectrum-value.h>
namespace ns3 {


class EESM : public Object
{
public:
  EESM ();
  ~EESM ();
  
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

 // double GetEffectiveSNR(double MI, std::string modulationtype, int constellationsize) const;
 double GetEffectivePower (Ptr<const SpectrumValue> input,uint32_t channelWidth,uint32_t GuardBW,uint8_t McsValue,std::string fastfading,WifiPhyStandard Standard);
 void LoadBetaEesm(uint32_t channelWidth, std::map<uint8_t,double> &m_mcsBeta,std::string fastfading);
     
};

} // namespace ns3
#endif /* EESM_H */

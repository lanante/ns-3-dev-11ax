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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 */

#ifndef TABLE_SPECTRUM_PROPAGATION_LOSS_H
#define TABLE_SPECTRUM_PROPAGATION_LOSS_H


#include <ns3/spectrum-propagation-loss-model.h>
#include <ns3/wifi-mode.h>
#include <ns3/object.h>
#include <string>
namespace ns3 {

//class MobilityModel;


/**
 * \ingroup spectrum
 * \brief Table spectrum propagation loss model
 *
 * The propagation loss is calculated according to a simplified version of Table'
 * formula in which antenna gains are unitary:
 *
 * \f$ L = \frac{4 \pi * d * f}{C^2}\f$
 *
 * where C = 3e8 m/s is the light speed in the vacuum. The intended
 * use is to calculate Prx = Ptx * G
 */
class TableSpectrumPropagationLossModel : public Object
{
public:
  TableSpectrumPropagationLossModel ();
  ~TableSpectrumPropagationLossModel ();
  
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  Ptr<SpectrumValue> CalRxSpectralDensity (Ptr<const SpectrumValue> input, WifiModulationClass ModulationClass, std::string channeltype, double BandBW,uint16_t CenterFrequency, uint8_t ChannelWidth, uint32_t GuardBW) const;
  Ptr<SpectrumValue> GetChannelResponse (WifiModulationClass ModulationClass,uint16_t CenterFrequency, uint8_t ChannelWidth, double txPowerW, uint32_t GuardBW,std::string channeltype,uint32_t nGuardBands) const;
};

} // namespace ns3

#endif /* TABLE_SPECTRUM_PROPAGATION_LOSS_MODEL_H */

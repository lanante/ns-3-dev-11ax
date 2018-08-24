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

#ifndef IEEE_80211AX_INDOOR_PROPAGATION_LOSS_MODEL_H
#define IEEE_80211AX_INDOOR_PROPAGATION_LOSS_MODEL_H

#include <ns3/propagation-loss-model.h>
#include <ns3/log-normal-shadowing-iid.h>

namespace ns3 {

/**
 * This class implements the indoor propagation model described in
 * IEEE 802.11-14/0980r9 "TGax Simulation Scenarios"
 * 
 */
class Ieee80211axIndoorPropagationLossModel : public PropagationLossModel
{

public:

  /** 
   * constructor
   */
  Ieee80211axIndoorPropagationLossModel ();

  // inherited from Object
  static TypeId GetTypeId (void);

  /** 
   * 
   * 
   * \param a the first mobility model
   * \param b the second mobility model
   * 
   * \return the loss in dB for the propagation between
   * the two given mobility models
   */
  double GetPathLossDb (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;


  /** 
   * 
   * 
   * \param a 
   * \param b 
   * 
   * \return the standard deviation of the shadowing loss in dB to be
   * considered for the link between a and b 
   */
  double GetSigma (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

private:

  // inherited from PropagationLossModel
  virtual double DoCalcRxPower (double txPowerDbm,
                                Ptr<MobilityModel> a,
                                Ptr<MobilityModel> b) const;

  virtual int64_t DoAssignStreams (int64_t stream);
  
  /**
   *  frequency in Hz
   * 
   */
  double m_frequency;

  /**
   * shadowing loss implementation
   * 
   */
  LogNormalShadowingIid m_shadowing;

  /**
   * standard deviation of the shadowing loss in dB
   * 
   */
  double m_sigma;

};

} // namespace ns3


#endif // IEEE_80211AX_INDOOR_PROPAGATION_LOSS_MODEL_H

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

#ifndef ITU_UMI_PROPAGATION_LOSS_MODEL_H
#define ITU_UMI_PROPAGATION_LOSS_MODEL_H

#include <ns3/propagation-loss-model.h>
#include <ns3/log-normal-shadowing-iid.h>
#include <ns3/los-nlos-classifier.h>

namespace ns3 {

/**
 * This class implements the ITU UMi 
 * propagation model described in 3GPP TR 36.814
 */
class ItuUmiPropagationLossModel : public PropagationLossModel
{

public:

  /** 
   * constructor
   */
  ItuUmiPropagationLossModel ();

  // inherited from Object
  static TypeId GetTypeId (void);

  /** 
   * 
   * 
   * \param a the first mobility model
   * \param b the second mobility model
   * 
   * \return the LOS path loss in dB between
   * the two given mobility models
   */
  double GetLosPathLossDb (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;


  /** 
   * 
   * 
   * \param a the first mobility model
   * \param b the second mobility model
   * 
   * \return the NLOS path loss in dB between
   * the two given mobility models
   */
  double GetNlosPathLossDb (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

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

  /** 
   * 
   * 
   * \param a 
   * \param b 
   * 
   * \return the LOS probability for the link between a and b 
   */
  double GetLosProbability (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

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
   * LOS/NLOS classification implementation
   * 
   */
  LosNlosClassifier m_losNlosClassifier;
};

} // namespace ns3


#endif // ITU_UMI_PROPAGATION_LOSS_MODEL_H

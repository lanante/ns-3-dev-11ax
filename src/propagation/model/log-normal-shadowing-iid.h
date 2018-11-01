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


#ifndef LOG_NORMAL_SHADOWING_H
#define LOG_NORMAL_SHADOWING_H

#include <ns3/mobility-model.h>
#include <ns3/random-variable-stream.h>

#include <map>

namespace ns3 {

/**
 * Callback used to return the value of the standard deviation in dB
 * of the shadowing loss between two nodes identified by the 
 * 
 */
typedef Callback< double, Ptr<MobilityModel>, Ptr<MobilityModel> > LogNormalShadowingIidGetSigmaCallback;

/**
 * This class is used as a building block to implement
 * PropagationLossModels that include a log-normal shadowing loss
 * component which is randomly assigned for every pair of nodes at the
 * first evaluation, and then stays constant throughout the simulation.
 * 
 */
class LogNormalShadowingIid 
{
public:

  /**
   * Constructor
   */
  LogNormalShadowingIid ();

  /** 
   * Set the GetSigma callback to be used by this
   * LogNormalShadowingIid instance
   * 
   * \param c the GetSigma callback
   */
  void SetGetSigmaCallback (LogNormalShadowingIidGetSigmaCallback c);

  /** 
   * Provide the shadowing loss bewteen two points A and B,
   * getting a new random value if it's the first time that A and B
   * are evaluated, or retrieving the previously calculated value if A
   * and B (or their simmetric B and A) have been evaluated before.
   * 
   * \param a mobility model A
   * \param b mobility model B
   * 
   * \return the shadowing loss in dB for the link between A and B
   */
  double GetLossDb (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

  /** 
   * set the stream numbers of the random variables used internally
   * by this class to the integers starting with the offset 'stream'.  
   * 
   * \param stream the offset 
   * 
   * \return the number of stream indices assigned by this model 
   */
  int64_t AssignStreams (int64_t stream);

private:

  /**
   * The GetSigma callback used by this
   * LogNormalShadowingIid instance
   * 
   */
  LogNormalShadowingIidGetSigmaCallback m_getSigmaCallback;

  /**
   * Data structure storing previously calculated loss values. It is
   * mutable so that GetLossDb () can be a const method, which makes
   * it easy for users of this class to implement
   * PropagationLossModel::CalcRxPower which is also a const method. 
   * 
   */
  mutable std::map<Ptr<MobilityModel>,  std::map<Ptr<MobilityModel>, double> > m_shadowingLossMap;

  /**
   * normal random variable used to generate loss values
   * 
   */
  Ptr<NormalRandomVariable> m_normalRandomVariable;
};


}

#endif // LOG_NORMAL_SHADOWING_H

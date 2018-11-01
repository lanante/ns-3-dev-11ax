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


#ifndef LOS_NLOS_CLASSIFIER_H
#define LOS_NLOS_CLASSIFIER_H

#include <ns3/mobility-model.h>
#include <ns3/random-variable-stream.h>

#include <map>

namespace ns3 {

/**
 * Callback used to return the probability that the link between the two mobility model instances is LOS
 * 
 */
typedef Callback< double, Ptr<MobilityModel>, Ptr<MobilityModel> > LosNlosClassifierGetLosProbabilityCallback;

/**
 * This class is used as a building block to implement
 * PropagationLossModels that randomly classify links between pairs of
 * nodes as either Line Of Sight (LOS) or Non Line Of Sight (NLOS) upon
 * the first evaluation, and then keep this classification constant throughout the simulation.
 * 
 */
class LosNlosClassifier 
{
public:

  /**
   * Constructor
   */
  LosNlosClassifier ();

  /** 
   * Set the GetLosProbability callback to be used by this
   * LosNlosClassifier instance
   * 
   * \param c the GetLosProbability callback
   */
  void SetGetLosProbabilityCallback (LosNlosClassifierGetLosProbabilityCallback c);

  /** 
   * Provide the LOS/NLOS classification bewteen two points A and B,
   * getting a new random value if it's the first time that A and B
   * are evaluated, or retrieving the previously determined value if A
   * and B (or their simmetric B and A) have been evaluated before.
   * 
   * \param a mobility model A
   * \param b mobility model B
   * 
   * \return true if the link between A and B is LOS, false if it is NLOS
   */
  bool IsLos (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

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
   * LosNlosClassifier instance
   * 
   */
  LosNlosClassifierGetLosProbabilityCallback m_getLosProbabilityCallback;

  /**
   * Data structure storing previously calculated isLos attributes. It is
   * mutable so that GetLossDb () can be a const method, which makes
   * it easy for users of this class to implement
   * PropagationLossModel::CalcRxPower which is also a const method. 
   * 
   */
  mutable std::map<Ptr<MobilityModel>,  std::map<Ptr<MobilityModel>, double> > m_isLosMap;

  /**
   * normal random variable used to generate loss values
   * 
   */
  Ptr<UniformRandomVariable> m_uniformRandomVariable;
};


}

#endif // LOS_NLOS_CLASSIFIER_H

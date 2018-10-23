/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 University of Washington
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
 */

#ifndef OBSS_PD_ALGORITHM_H
#define OBSS_PD_ALGORITHM_H

#include "ns3/object.h"
#include "ns3/wifi-phy.h"
#include "ns3/regular-wifi-mac.h"

namespace ns3 {

/**
 * \brief OBSS PD algorithm
 * \ingroup wifi
 *
 * This object executes an algorithm for OBSS PD to evalute if a receiving 
 * signal should be accepted or rejeted.
 *
 */
class ObssPdAlgorithm : public Object
{
public:
  ObssPdAlgorithm ();
  static TypeId GetTypeId (void);

  /**
   * \param params the HE SIG A parameters
   *
   * Evaluate the algorithm.
   */
  void ReceiveHeSigA (HeSigAParameters params);
private:
};

} //namespace ns3

#endif /* OBSS_PD_ALGORITHM_H */

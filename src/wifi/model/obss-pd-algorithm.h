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

namespace ns3 {

struct HeSigAParameters;
struct HeBeaconReceptionParameters;

/**
 * \brief OBSS PD algorithm interface
 * \ingroup wifi-he
 *
 * This object provides the interface for all OBSS_PD adjustment algorithms
 * and is designed to be subclassed.
 *
 */
class ObssPdAlgorithm : public Object
{
public:
  static TypeId GetTypeId (void);

  /**
   * \param params the HE SIG A parameters
   *
   * Evaluate the receipt of HE SIG-A.
   */
  virtual void ReceiveHeSigA (HeSigAParameters params) = 0;

  /**
   * \param params the HE Beacon parameters
   *
   * Evaluate the receipt of a beacon.
   */
  virtual void ReceiveBeacon (HeBeaconReceptionParameters params) = 0;
};

} //namespace ns3

#endif /* OBSS_PD_ALGORITHM_H */

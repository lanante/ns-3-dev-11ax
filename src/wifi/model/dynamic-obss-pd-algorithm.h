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
 * Author: Leonardo Lanante Jr <leonardolanante@gmail.com>
 */

#ifndef DYNAMIC_OBSS_PD_ALGORITHM_H
#define DYNAMIC_OBSS_PD_ALGORITHM_H

#include "obss-pd-algorithm.h"

namespace ns3 {

/**
 * \brief Dynamic OBSS PD algorithm
 * \ingroup wifi
 *
 * TBD
 */
class DynamicObssPdAlgorithm : public ObssPdAlgorithm
{
public:
  DynamicObssPdAlgorithm ();

  static TypeId GetTypeId (void);

  /**
   * Connect the WifiNetDevice and setup eventual callbacks.
   *
   * \param device the WifiNetDevice
   */
  void ConnectWifiNetDevice (const Ptr<WifiNetDevice> device);

  /**
   * \param params the HE SIG parameters
   *
   * Evaluate the receipt of HE SIG.
   */
  void ReceiveHeSig (HePreambleParameters params);
  /**
   * \param params the HE Beacon parameters
   *
   * Evaluate the receipt of a beacon.
   */
  void ReceiveBeacon (HeBeaconReceptionParameters params);
};

} //namespace ns3

#endif /* DYNAMIC_OBSS_PD_ALGORITHM_H */

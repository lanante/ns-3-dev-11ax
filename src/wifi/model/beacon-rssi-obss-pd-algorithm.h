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
 * Author: Sébastien Deronne <sebastien.deronne@gmail.com>
 */

#ifndef BEACON_RSSI_OBSS_PD_ALGORITHM_H
#define BEACON_RSSI_OBSS_PD_ALGORITHM_H

#include "obss-pd-algorithm.h"

namespace ns3 {

/**
 * \brief Beacon RSSI OBSS PD algorithm
 * \ingroup wifi
 *
 * This object executes an algorithm for OBSS PD to evalute if a receiving
 * signal should be accepted or rejected.
 *
 */
class BeaconRssiObssPdAlgorithm : public ObssPdAlgorithm
{
public:
  BeaconRssiObssPdAlgorithm ();

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
   * Evaluate the algorithm.
   */
  void ReceiveHeSig (HePreambleParameters params);
  /**
   * \param params the HE Beacon parameters
   *
   * Evaluate the receipt of a beacon.
   */
  void ReceiveBeacon (HeBeaconReceptionParameters params);


private:
  /**
   * Utility function to update ObssPdLevel
   */
  void UpdateObssPdLevel (void);

  uint16_t m_beaconCount;        ///< # of beacon for AVE
  double m_rssiArray [10];       ///< array for beacon RSSI
  double m_rssiAve;              ///< beacon RSSI Average
  double m_txPower;              ///< TX Power
};

} //namespace ns3

#endif /* BEACON_RSSI_OBSS_PD_ALGORITHM_H */

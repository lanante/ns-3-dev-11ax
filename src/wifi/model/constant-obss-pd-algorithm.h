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

#ifndef CONSTANT_OBSS_PD_ALGORITHM_H
#define CONSTANT_OBSS_PD_ALGORITHM_H

#include "obss-pd-algorithm.h"

namespace ns3 {

class HeConfiguration;
class WifiNetDevice;
class WifiMac;
class WifiPhy;

/**
 * \brief Constant OBSS PD algorithm
 * \ingroup wifi
 *
 * This object executes an algorithm for OBSS PD to evalute if a receiving
 * signal should be accepted or rejected.
 *
 */
class ConstantObssPdAlgorithm : public ObssPdAlgorithm
{
public:
  ConstantObssPdAlgorithm ();
  static TypeId GetTypeId (void);

  /**
    * Sets the OBSS PD level.
    *
    * \param level the OBSS PD level in dBm
    */
  void SetObssPdLevel (double level);
  /**
   * Return the OBSS PD level (dBm).
   *
   * \return the OBSS PD level in dBm
   */
  double GetObssPdLevel (void) const;
  /**
    * Sets the TX power (in dBm).
    *
    * \param txPower the TX power in dBm
    */
  void SetTxPower (double txPower);
  /**
   * Return TX power (in dBm).
   *
   * \return the TX power in dBm
   */
  double GetTxPower (void) const;

  /**
   * \param params the HE SIG A parameters
   *
   * Evaluate the algorithm.
   */
  void ReceiveHeSigA (HeSigAParameters params);
  /**
   * \param params the HE Beacon parameters
   *
   * Evaluate the receipt of a beacon.
   */
  void ReceiveBeacon (HeBeaconReceptionParameters params);


private:
  /**
   * Setup callbacks.
   *
   */
  void SetupCallbacks ();

  /**
   * Return the WifiNetDevce aggregated to this object.
   *
   * \return the Ptr<WifiNetDevice> aggregated to this object
   */
  Ptr<WifiNetDevice> GetWifiNetDevice (void) const;

  /**
   * Return the WifiPhy associated with this object.
   *
   * \return the Ptr<WifiPhy> associated with this object
   */
  Ptr<WifiPhy> GetPhy (void) const;

  /**
   * Return the WifiMac associated with this object.
   *
   * \return the Ptr<WifiMac> associated with this object
   */
  Ptr<WifiMac> GetMac (void) const;

  /**
   * Return the HeConfiguration object associated with this object.
   *
   * \return the Ptr<RegularWifiMac> assocaited with this object
   */
  Ptr<HeConfiguration> GetHeConfiguration (void) const;

  double m_obssPdLevel; ///< OBSS PD level
  double m_txPower;     ///< TX power
};

} //namespace ns3

#endif /* CONSTANT_OBSS_PD_ALGORITHM_H */

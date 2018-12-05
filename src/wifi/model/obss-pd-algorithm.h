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

#ifndef OBSS_PD_ALGORITHM_H
#define OBSS_PD_ALGORITHM_H

#include "ns3/object.h"

namespace ns3 {

struct HePreambleParameters;
struct HeBeaconReceptionParameters;

class WifiNetDevice;

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
   * Sets the WifiNetDevice this PHY is associated with.
   *
   * \param device the WifiNetDevice this PHY is associated with
   */
  void SetWifiNetDevice (const Ptr<WifiNetDevice> device);
  /**
   * Returns the WifiNetDevice this PHY is associated with.
   *
   * \return the WifiNetDevice this PHY is associated with
   */
  Ptr<WifiNetDevice> GetWifiNetDevice (void) const;
  /**
   * Returns whether the selected OBSS PD level is in the allowed range.
   *
   * \param level the level that we want to verify
   * \return whether the selected OBSS PD level is in the allowed range
   */
  bool IsObssPdLevelAllowed (double level);

  /**
   * \param params the HE SIG A parameters
   *
   * Evaluate the receipt of HE SIG-A.
   */
  virtual void ReceiveHeSigA (HePreambleParameters params) = 0;

  /**
   * \param params the HE Beacon parameters
   *
   * Evaluate the receipt of a beacon.
   */
  virtual void ReceiveBeacon (HeBeaconReceptionParameters params) = 0;


protected:
  virtual void DoDispose (void);

  /**
   * Setup callbacks.
   *
   */
  virtual void SetupCallbacks ();


private:
  Ptr<WifiNetDevice> m_device; //!< Pointer to the WifiNetDevice

  double m_obssPdLevelMin; ///< Minimum OBSS PD level
  double m_obssPdLevelMax; ///< Maximum OBSS PD level
};

} //namespace ns3

#endif /* OBSS_PD_ALGORITHM_H */

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
   * Setup callbacks.
   */
  virtual void SetupCallbacks () = 0;

  /**
   * Sets the OBSS PD level (in dBm).
   *
   * \param level the OBSS PD level in dBm
   */
  void SetObssPdLevel (double level);
  /**
   * Returns the OBSS PD level (in dBm).
   *
   * \return the OBSS PD level in dBm
   */
  double GetObssPdLevel (void) const;
  /**
   * Sets the minimum OBSS PD level (in dBm).
   *
   * \param level the minimum OBSS PD level in dBm
   */
  void SetObssPdLevelMin (double level);
  /**
   * Returns the minimum OBSS PD level (in dBm).
   *
   * \return the minimum OBSS PD level in dBm
   */
  double GetObssPdLevelMin (void) const;
  /**
   * Sets the maximum OBSS PD level (in dBm).
   *
   * \param level the maximum OBSS PD level in dBm
   */
  void SetObssPdLevelMax (double level);
  /**
   * Returns the maximum OBSS PD level (in dBm).
   *
   * \return the maximum OBSS PD level in dBm
   */
  double GetObssPdLevelMax (void) const;
  /**
   * Sets the SISO reference TX power level (in dBm).
   *
   * \param power the SISO reference TX power level in dBm
   */
  void SetTxPowerRef (double power);
  /**
   * Returns the SISO reference TX power level (in dBm).
   *
   * \return the SISO reference TX power level in dBm
   */
  double GetTxPowerRef (void) const;

  /**
   * Reset PHY to IDLE.
   */
  void ResetPhy ();

  /**
   * \param params the HE SIG parameters
   *
   * Evaluate the receipt of HE SIG.
   */
  virtual void ReceiveHeSig (HePreambleParameters params) = 0;

  /**
   * \param params the HE Beacon parameters
   *
   * Evaluate the receipt of a beacon.
   */
  virtual void ReceiveBeacon (HeBeaconReceptionParameters params) = 0;


protected:
  virtual void DoDispose (void);


private:
  Ptr<WifiNetDevice> m_device; //!< Pointer to the WifiNetDevice

  double m_obssPdLevelMin; ///< Minimum OBSS PD level
  double m_obssPdLevelMax; ///< Maximum OBSS PD level
  double m_obssPdLevel;    ///< Current OBSS PD level
  double m_txPowerRef;     ///< SISO reference TX power level
};

} //namespace ns3

#endif /* OBSS_PD_ALGORITHM_H */

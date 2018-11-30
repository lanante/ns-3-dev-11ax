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
#include "ns3/traced-value.h"

namespace ns3 {

struct HeSigAParameters;
struct HeBeaconReceptionParameters;

class HeConfiguration;
class WifiNetDevice;
class WifiMac;
class WifiPhy;

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
  ObssPdAlgorithm ();
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
    * Sets the TxPwr.
    *
    * \param txPwr the TxPwr in dBm
    */
  void SetTxPwr (double txPwr);
  /**
   * Return the TxPwr (dBm).
   *
   * \return the TxPwr in dBm
   */
  double GetTxPwr (void) const;

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

  /**
   * \param params the HE SIG A parameters
   *
   * Evaluate the receipt of HE SIG-A.
   */
  void ReceiveHeSigA (HeSigAParameters params);

  /**
   * \param params the HE Beacon parameters
   *
   * Evaluate the receipt of a beacon.
   */
  void ReceiveBeacon (HeBeaconReceptionParameters params);


private:
  TracedValue<double> m_obssPdLevel;
  TracedValue<double> m_txPwr;

  /**
   * \param params the HE SIG A parameters
   *
   * Evaluate the algorithm.
   */
  virtual void DoReceiveHeSigA (HeSigAParameters params);

  /**
   *
   * Receive beacon.
   */
  virtual void DoReceiveBeacon (HeBeaconReceptionParameters params);
};

} //namespace ns3

#endif /* OBSS_PD_ALGORITHM_H */

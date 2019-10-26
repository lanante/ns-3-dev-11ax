/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 University of Washington
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

#ifndef DYNAMIC_THRESHOLD_CHANNEL_BONDING_MANAGER_H
#define DYNAMIC_THRESHOLD_CHANNEL_BONDING_MANAGER_H

#include "ns3/attribute-helper.h"
#include "channel-bonding-manager.h"

namespace ns3 {

typedef std::map<WifiMode,double> CcaThresholdPerWifiModeMap;

ATTRIBUTE_VALUE_DEFINE_WITH_NAME (CcaThresholdPerWifiModeMap, CcaThresholdPerWifiMode);
ATTRIBUTE_ACCESSOR_DEFINE (CcaThresholdPerWifiMode);
ATTRIBUTE_CHECKER_DEFINE (CcaThresholdPerWifiMode);

/**
 * \brief Dynamic Threshold Channel Bonding Manager
 * \ingroup wifi
 *
 * This object provides an implementation for dynamically selecting the channel width.
 *
 */
class DynamicThresholdChannelBondingManager : public ChannelBondingManager
{
public:
  DynamicThresholdChannelBondingManager ();

  static TypeId GetTypeId (void);

  /**
   * Sets the WifiPhy this manager is associated with.
   *
   * \param phy the WifiPhy this manager is associated with
   */
  void SetPhy (const Ptr<WifiPhy> phy) override;

  /**
   * Sets the CCA threshold (dBm) for the secondary channels for a given WifiMode.
   * The energy of a received signal should be higher than this threshold to allow
   * the PHY layer to declare CCA BUSY state. When checking the secondary state prior
   * to a transmission, it will consider the threshold that is configured for the
   * WifiMode that is going to be used for the upcoming transmission.
   *
   * \param threshold the CCA threshold in dBm for the secondary channels
   */
  void SetCcaEdThresholdSecondaryForMode (WifiMode mode, double threshold);

  /**
   * Returns the selected channel width (in MHz).
   *
   * \param mode the WifiMode that will be used for the transmission
   *
   * \return the selected channel width in MHz
   */
  uint16_t GetUsableChannelWidth (WifiMode mode) override;


private:
  CcaThresholdPerWifiModeMap m_ccaEdThresholdsSecondaryDbm; //!< Clear channel assessment (CCA) thresholds for secondary channel(s) in dBm, per WifiMode
};

/**
 * \param os  output stream
 * \param ccaThresholdPerWifiMode  CCA threshold per WifiMode map to stringify
 * \return output stream
 */
std::ostream& operator<< (std::ostream& os, CcaThresholdPerWifiModeMap ccaThresholdPerWifiMode);

/**
* \param is input stream.
* \param ccaThresholdPerWifiMode CCA threshold per WifiMode map to set
* \return input stream.
*/
std::istream &operator>> (std::istream &is, CcaThresholdPerWifiModeMap &ccaThresholdPerWifiMode);


} //namespace ns3

#endif /* DYNAMIC_THRESHOLD_CHANNEL_BONDING_MANAGER_H */

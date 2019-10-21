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

typedef std::map<uint8_t,double> CcaThresholdPerMcsMap;

ATTRIBUTE_VALUE_DEFINE_WITH_NAME (CcaThresholdPerMcsMap, CcaThresholdPerMcs);
ATTRIBUTE_ACCESSOR_DEFINE (CcaThresholdPerMcs);
ATTRIBUTE_CHECKER_DEFINE (CcaThresholdPerMcs);

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
   * Sets the CCA threshold (dBm) for the secondary channels. The energy of a received signal
   * should be higher than this threshold to allow the PHY layer to declare CCA BUSY state.
   *
   * \param threshold the CCA threshold in dBm for the secondary channels
   */
  void SetCcaEdThresholdSecondaryForMcs (uint8_t mcs, double threshold);

  /**
   * Returns the selected channel width (in MHz).
   *
   * \param mcs the MCS that will be used for the transmission
   *
   * \return the selected channel width in MHz
   */
  uint16_t GetUsableChannelWidth (uint8_t mcs) override;


private:
  CcaThresholdPerMcsMap m_ccaEdThresholdsSecondaryDbm; //!< Clear channel assessment (CCA) thresholds for secondary channel(s) in dBm, per MCS
};

/**
 * \param os  output stream
 * \param ccaThresholdPerMcs  CCA threshold per MCS map to stringify
 * \return output stream
 */
std::ostream& operator<< (std::ostream& os, CcaThresholdPerMcsMap ccaThresholdPerMcs);

/**
* \param is input stream.
* \param ccaThresholdPerMcs CCA threshold per MCS map to set
* \return input stream.
*/
std::istream &operator>> (std::istream &is, CcaThresholdPerMcsMap &ccaThresholdPerMcs);


} //namespace ns3

#endif /* DYNAMIC_THRESHOLD_CHANNEL_BONDING_MANAGER_H */

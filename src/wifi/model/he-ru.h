/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018
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
 * Author: Stefano Avallone <stavallo@unina.it>
 */

#ifndef HE_RU_H
#define HE_RU_H

#include <map>
#include <vector>

namespace ns3 {


/**
 * This class stores the subcarrier ranges of all the available HE RUs.
 */
class HeRu
{
public:
  /**
   * The different HE Resource Unit (RU) types.
   */
  enum RuType
  {
    RU_26_TONE = 0,
    RU_52_TONE,
    RU_106_TONE,
    RU_242_TONE,
    RU_484_TONE,
    RU_996_TONE,
    RU_2x996_TONE
  };

  /// (lowest index, highest index) pairs defining subcarrier ranges
  typedef std::vector<std::pair<int16_t, int16_t>> Indices;

  /**
   * RU Specification. Stores the information carried by the RU Allocation
   * subfield of the User Info field of Trigger frames. Note that primary80MHz
   * must be false if ruType is RU_2x996_TONE.
   */
  typedef struct
  {
    bool primary80MHz;   //!< true if the RU is allocated in the primary 80MHz channel
    RuType ruType;   //!< RU type
    std::size_t index;   //!< index (starting at 1)
  } RuSpec;


  /**
   * Get the number of distinct RUs of the given type (number of tones)
   * available in a HE PPDU of the given bandwidth.
   *
   * \param bw the bandwidth (MHz) of the HE PPDU (20, 40, 80, 160)
   * \param ruType the RU type (number of tones)
   * \return the number of distinct RUs available
   */
  static std::size_t GetNRus (uint8_t bw, RuType ruType);

  /**
   * Get the subcarrier range of the RU having the given index among all the
   * RUs of the given type (number of tones) available in a HE PPDU of the
   * given bandwidth. A subcarrier range is defined as one or more pairs
   * indicating the lowest frequency index and the highest frequency index.
   * Note that subcarrier ranges are only available for channel widths of 20,
   * 40 or 80 MHz.
   *
   * \param bw the bandwidth (MHz) of the HE PPDU (20, 40, 80)
   * \param ruType the RU type (number of tones)
   * \param index the index (starting at 1) of the RU
   * \return the subcarrier range of the specified RU
   */
  static Indices GetSubcarrierRange (uint8_t bw, RuType ruType, std::size_t index);

  /**
   * Check whether the given RU overlaps with the given set of RUs.
   *
   * \param bw the bandwidth (MHz) of the HE PPDU (20, 40, 80, 160)
   * \param ru the given RU allocation
   * \param v the given set of RUs
   * \return true if the given RU overlaps with the given set of RUs.
   */
  static bool DoesOverlap (uint8_t bw, RuSpec ru, const std::vector<RuSpec> &v);

  /// (bandwidth, number of tones) pair
  typedef std::pair<uint8_t, RuType> BwTonesPair;

  /// map (bandwidth, number of tones) pairs to the range of subcarrier indices
  typedef std::map<BwTonesPair, std::vector<Indices>> SubcarrierRanges;

  //!< Subcarrier ranges for all RUs
  static const SubcarrierRanges  m_heRuSubcarrierRanges;
};

/**
 * \brief Stream insertion operator.
 *
 * \param os the stream
 * \param ruType the RU type
 * \returns a reference to the stream
 */
std::ostream& operator<< (std::ostream& os, const HeRu::RuType &ruType);

/**
 * \brief Stream insertion operator.
 *
 * \param os the stream
 * \param ru the RU
 * \returns a reference to the stream
 */
std::ostream& operator<< (std::ostream& os, const HeRu::RuSpec &ru);

} //namespace ns3

#endif /* HE_RU_H */

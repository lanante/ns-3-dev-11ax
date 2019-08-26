/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 Universita' degli Studi di Napoli Federico II
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

#ifndef MU_TX_ACK_TYPES_H
#define MU_TX_ACK_TYPES_H

namespace ns3 {

/**
 * \ingroup wifi
  * Enumeration of the possible types of transmission involving OFDMA
  */
enum OfdmaTxFormat
{
  NON_OFDMA = 0,
  DL_OFDMA,
  UL_OFDMA
};

/**
 * \ingroup wifi
  * Enumeration of the possible acknowledgment sequences for HE MU PPDUs
  */
enum DlMuAckSequenceType
{
  DL_NONE = 0,                 //!< No acknowledgment
  DL_SU_FORMAT,                //!< Acknowledgment in SU format
  DL_MU_BAR,                   //!< MU-BAR Trigger Frame sent as SU PPDU
  DL_AGGREGATE_TF,             //!< MU-BAR Trigger Frames aggregated to A-MPDUs
  DL_COUNT                     //!< Only used to get the number of ack sequence types
};

/**
 * \ingroup wifi
  * Enumeration of the possible acknowledgment sequences for HE TB PPDUs
  */
enum UlMuAckSequenceType
{
  UL_NONE = 0,                 //!< No acknowledgment
  UL_INDIVIDUAL_BLOCK_ACKS,    //!< Block Acks sent in an MU DL PPDU
  UL_MULTI_STA_BLOCK_ACK,      //!< Single Multi-STA Block Ack
  UL_COUNT                     //!< Only used to get the number of ack sequence types
};

} //namespace ns3

#endif /* MU_TX_ACK_TYPES_H */

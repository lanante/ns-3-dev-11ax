/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Stefano Avallone <stavallo@gmail.com>
 */

#ifndef TRIGGER_FRAME_TYPE_H
#define TRIGGER_FRAME_TYPE_H

namespace ns3 {

/**
 * \ingroup wifi
 * The different Trigger frame types.
 */
enum TriggerFrameType
{
  BASIC_TRIGGER = 0,       // Basic
  BFRP_TRIGGER = 1,        // Beamforming Report Poll
  MU_BAR_TRIGGER = 2,      // Multi-User Block Ack Request
  MU_RTS_TRIGGER = 3,      // Multi-User Request To Send
  BSRP_TRIGGER = 4,        // Buffer Status Report Poll
  GCR_MU_BAR_TRIGGER = 5,  // Groupcast with Retries MU-BAR
  BQRP_TRIGGER = 6,        // Bandwidth Query Report Poll
  NFRP_TRIGGER = 7         // NDP Feedback Report Poll
};

} //namespace ns3

#endif /* TRIGGER_FRAME_TYPE_H */

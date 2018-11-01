/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2015 University of Washington
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
 * Created on: Aug 11, 2015
 * Authors: Biljana Bojovic <bbojovic@cttc.es> and Tom Henderson <tomh@tomh.org>
 */

#include <ns3/object.h>
#include <ns3/nstime.h>
#include <ns3/callback.h>

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

#include "ns3/channel-access-manager.h"
#include "ns3/wifi-mac.h"
#include "ns3/mac-low.h"
#include "ns3/lte-enb-phy.h"
#include "ns3/lte-enb-mac.h"
#include "ns3/lte-channel-access-manager.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ff-mac-common.h"


#ifndef LBTACCESSMANAGER_H_
#define LBTACCESSMANAGER_H_

namespace ns3 {

class SpectrumWifiPhy;
class WifiMac;
class LbtPhyListener;

class LbtAccessManager : public LteChannelAccessManager
{
public:
  enum LbtState
  {
    IDLE = 0,
    BUSY,
    WAIT_FOR_DEFER,
    WAIT_FOR_BACKOFF,
    TXOP_GRANTED
  };

  enum CWUpdateRule_t
  {
    ALL_NACKS,
    ANY_NACK,
    NACKS_10_PERCENT,
    NACKS_80_PERCENT,
  };

  static TypeId GetTypeId (void);

  LbtAccessManager ();
  virtual ~LbtAccessManager ();
 /**
  * Assign a fixed random variable stream number to the random variables
  * used by this model.  Return the number of streams (possibly zero) that
  * have been assigned.
  *
  * \param stream first stream index to use
  * \return the number of stream indices assigned by this model
  */
  int64_t AssignStreams (int64_t stream);
  void SetupPhyListener (Ptr<SpectrumWifiPhy> phy);
  void SetWifiPhy (Ptr<SpectrumWifiPhy> phy);
  void SetLteEnbMac (Ptr<LteEnbMac> lteEnbMac);
  void SetLteEnbPhy (Ptr<LteEnbPhy> lteEnbPhy);
  void NotifyRxStartNow (Time duration);
  void NotifyRxEndOkNow ();
  void NotifyRxEndErrorNow ();
  void NotifyTxStartNow (Time duration);
  void NotifyMaybeCcaBusyStartNow (Time duration);
  void NotifySwitchingStartNow (Time duration);
  void NotifySleepNow ();
  void NotifyOffNow ();
  void NotifyOnNow ();
  void NotifyWakeupNow ();
  void NotifyNavStartNow (Time duration);
  void NotifyNavResetNow (Time duration);
  void NotifyAckTimeoutStartNow (Time duration);
  void NotifyAckTimeoutResetNow ();
  void NotifyCtsTimeoutStartNow (Time duration);
  void NotifyCtsTimeoutResetNow ();
  void UpdateState ();

  void ResetCw (void);
  void UpdateCw (void);
  LbtState GetLbtState () const;
  uint32_t GetCurrentBackoffCount (void) const;

private:
  virtual void DoRequestAccess ();
  void RequestAccessAfterDefer ();
  void RequestAccessAfterBackoff ();
  void TransitionToBusy (Time duration);
  void TransitionFromBusy ();
  uint32_t GetBackoffSlots ();
  void UpdateFailedCw ();
  void UpdateCwBasedOnHarq (std::vector<DlInfoListElement_s> harqFeedback);
  void SetGrant();

  void SetLbtState (LbtState state);
  Ptr<LteEnbMac> m_lteEnbMac;
  Ptr<LteEnbPhy> m_lteEnbPhy;
  Ptr<SpectrumWifiPhy> m_wifiPhy;
  LbtPhyListener* m_lbtPhyListener;
  CWUpdateRule_t m_cwUpdateRule;
  LbtState m_state;
  Time m_slotTime;
  Time m_deferTime;
  uint32_t m_cwMin;
  uint32_t m_cwMax;
  TracedValue<uint32_t> m_cw;
  TracedValue<uint32_t> m_currentBackoffSlots;  // last value drawn for backoff
  uint32_t m_backoffCount;  // counter for current backoff remaining
  bool m_grantRequested;
  Ptr<UniformRandomVariable> m_rng;
  Time m_txop;
  bool m_reservationSignal;
  EventId m_waitForDeferEventId;
  EventId m_waitForBackoffEventId;
  EventId m_waitForBusyEventId;
  Time m_lastCWUpdateTime;
  Time m_backoffStartTime;
  Time m_lastBusyTime;
  Time m_harqFeedbackDelay;  // delay between subframe being transmitted and harq feedback being received for it
  Time m_harqFeedbackExpirationTime;
  std::vector<Time> m_txopHistory;
  Time m_lastConsideredTxopStartTime;
};

} // namespace ns3

#endif /* LBTACCESSMANAGER_H_ */
